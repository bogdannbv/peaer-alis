#include "recorder.h"
#include <spdlog/spdlog.h>

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

        rx = new receiver(
                get_rtl_device_string(device_id),
                "",
                1
        );
        rx->set_auto_gain(true);

        rx->set_demod(receiver::RX_DEMOD_WFM_M);

        rx->set_input_rate(sample_rate);
        rx->set_rf_freq(start_frequency);
    }

    void recorder::start(messages::stations_channel &rx_stations, messages::recordings_channel &tx_recordings) {
        rx->start();

        for (auto station: rx_stations) {
            spdlog::info(
                    "Recording station #{} ({} MHz) for {} seconds",
                    station.id,
                    station.frequency / 1e6,
                    duration_seconds
            );

            rx->set_rf_freq(station.frequency);
            std::this_thread::sleep_for(100ms);

            uint64_t started_at = now_timestamp();

            std::string file_path = std::format("{}/rec_{}_{}.wav", dir, station.id, started_at);

            rx->start_audio_recording(file_path);

            if (!rx->is_recording_audio()) continue;

            std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));

            rx->stop_audio_recording();

            uint64_t finished_at = now_timestamp();

            tx_recordings << messages::recording{
                    .station = station,
                    .started_at = started_at,
                    .finished_at = finished_at,
                    .file_path = file_path,
            };
        }
    }

    std::string recorder::get_rtl_device_string(const std::string &device_id) {
        return "rtl=" + device_id;
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
