#include "publisher.h"
#include <spdlog/spdlog.h>

namespace workers {

    publisher::publisher(api::client *client) {
        this->client = client;
    }

    void publisher::start(messages::recognitions_channel &recognitions) {
        for (auto recognition: recognitions) {
            // sleep for retries * 5 seconds
            int sleep_seconds = recognition.publish_tries * 5;
            if (sleep_seconds > 0) {
                spdlog::info("Retrying in {} seconds", sleep_seconds);
            }
            std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds));
            try {
                client->create_playback(
                        recognition.recording.station.id,
                        recognition.recording.started_at,
                        recognition.recording.finished_at,
                        recognition.shazam_response
                );

                spdlog::info("Published playback for {} station",
                             recognition.recording.station.name
                );
            } catch (api::exception &e) {
                spdlog::error("Couldn't create playback: {} for {} station",
                              e.what(),
                              recognition.recording.station.name);

                if (recognition.publish_tries >= 8) {
                    spdlog::error("Reached max tries for {} station playback publishing, skipping",
                                  recognition.recording.station.name);
                    continue;
                }
                spdlog::error("Pushing {} station playback back for retry", recognition.recording.station.name);
                recognition.publish_tries++;
                recognitions << recognition;
                continue;
            }
        }
    }

} // workers