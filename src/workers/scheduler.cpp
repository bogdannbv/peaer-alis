#include "scheduler.h"
#include <spdlog/spdlog.h>

namespace workers {
    scheduler::scheduler(
            api::client *client,
            int interval
    ) {
        this->client = client;
        this->interval = interval;
    }

    void scheduler::start(messages::stations_channel &stations_channel) {
        std::vector<api::station> stations;

        while (true) {
            try {
                stations = client->get_stations();
            } catch (api::exception &e) {
                spdlog::error("Couldn't get stations: {}", e.what());
                if (stations.empty()) {
                    spdlog::error("No cached stations, retrying in {} seconds", interval);
                    std::this_thread::sleep_for(std::chrono::seconds(interval));
                    continue;
                }
            }

            for (auto &station: stations) {
                spdlog::info("Pushing station: {}", station.name);
                stations_channel << messages::station{
                        .id = station.id,
                        .name = station.name,
                        .slug = station.slug,
                        .frequency = station.frequency,
                };
            }
            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }
} // workers
