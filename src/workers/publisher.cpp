#include "publisher.h"
#include <spdlog/spdlog.h>

namespace workers {

    publisher::publisher(api::client *client) {
        this->client = client;
    }

    void publisher::start(messages::recognitions_channel &recognitions) {
        for (auto recognition: recognitions) {
            recognition.publish_tries++;
            try {
                client->create_playback(
                        recognition.recording.station.id,
                        recognition.shazam_response
                );

                spdlog::info("Published playback for station #{}",
                             recognition.recording.station.id
                );
            } catch (api::exception &e) {
                spdlog::error("Couldn't create playback: {}", e.what());

                if (recognition.publish_tries >= 5) {
                    spdlog::error("Reached max publish tries, skipping");
                    continue;
                }
                spdlog::error("Pushing back for retry");
                recognitions << recognition;
                continue;
            }
        }
    }

} // workers