#include "recognizer.h"
#include <command.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace workers {

    recognizer::recognizer(const std::string &songrec_path) {
        this->songrec_path = songrec_path;
    }

    void recognizer::start(
            messages::recordings_channel &rx_recordings,
            messages::recognitions_channel &tx_recognitions
    ) {
        for (auto recording: rx_recordings) {
            std::string path = recording.file_path;

            std::string json = recognize(path);

            if (json.empty()) {
                continue;
            }

            nlohmann::json response = nlohmann::json::parse(json);

            if (!response["matches"].empty()) {
                tx_recognitions << messages::recognition{
                        .recording = recording,
                        .shazam_response = response
                };
            } else {
                spdlog::warn("SongRec couldn't recognize: {}", path);
            }

            spdlog::info("Removing: {} ", path);
            std::remove(path.c_str());
        }
    }

    std::string recognizer::recognize(const std::string &path) {
        auto result = raymii::Command::exec(fmt::format("{} {} {}",
                                                        this->songrec_path,
                                                        "audio-file-to-recognized-song",
                                                        path
        ));

        if (result.exitstatus != 0) {
            spdlog::error("SongRec failed with exit code " + std::to_string(result.exitstatus));

            return "";
        }

        if (result.output.empty()) {
            spdlog::warn("SongRec couldn't recognize: {}", path);

            return "";
        }

        return result.output;
    }

}