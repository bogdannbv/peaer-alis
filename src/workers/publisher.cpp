#include "publisher.h"
#include <spdlog/spdlog.h>

namespace workers {

    publisher::publisher(api::client *client) {
        this->client = client;
    }

    void publisher::start(messages::recognitions_channel &recognitions) {
        for (auto recognition: recognitions) {
            // sleep for retries * 5 seconds
            std::this_thread::sleep_for(std::chrono::seconds(recognition.publish_tries * 5));
            try {
                client->create_playback(
                        recognition.recording.station.id,
                        recognition.recording.started_at,
                        recognition.recording.finished_at,
                        recognition.shazam_response
                );

                spdlog::info("Published playback for station #{}",
                             recognition.recording.station.id
                );
            } catch (api::exception &e) {
                spdlog::error("Couldn't create playback: {}", e.what());

                if (recognition.publish_tries >= 8) {
                    spdlog::error("Reached max publish tries, skipping");
                    continue;
                }
                spdlog::error("Pushing back for retry");
                recognition.publish_tries++;
                recognitions << recognition;
                continue;
            }
        }
    }

} // workers