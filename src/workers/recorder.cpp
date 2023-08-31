#include "recorder.h"
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace workers {

    recorder::recorder(
            const std::string &device_id,
            double start_frequency,
            double sample_rate,
            int duration_seconds,
            const std::string &dir
    ) {
        this->duration_seconds = duration_seconds;
        this->dir = dir;

        rx = new alis::receivers::receiver(device_id);

        rx->set_center_freq(start_frequency);
    }

    void recorder::start(messages::stations_channel &rx_stations, messages::recordings_channel &tx_recordings) {
        rx->start();

        for (auto station: rx_stations) {
            spdlog::info(
                    "Recording station {} ({} MHz) for {} seconds",
                    station.name,
                    station.frequency / 1e6,
                    duration_seconds
            );

            rx->set_center_freq(station.frequency);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            uint64_t started_at = now_timestamp();

            std::string file_path = fmt::format("{}/{}_{}_{}.wav", dir, station.slug, station.id, started_at);

            rx->start_recording(file_path);

            if (!rx->is_recording()) continue;

            std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));

            rx->stop_recording();

            uint64_t finished_at = now_timestamp();

            tx_recordings << messages::recording{
                    .station = station,
                    .started_at = started_at,
                    .finished_at = finished_at,
                    .file_path = file_path,
            };
        }
    }

    uint64_t recorder::now_timestamp() {
        return std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    recorder::~recorder() {
        rx->stop();
        delete rx;
    }

}
