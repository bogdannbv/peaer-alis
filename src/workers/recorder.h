#ifndef ALIS_WORKERS_RECORDER_H
#define ALIS_WORKERS_RECORDER_H

#include "messages.h"
#include <receiver.h>

namespace workers {

    class recorder {
    public:
        recorder(
                const std::string& device_id,
                double start_frequency,
                double sample_rate,
                int duration_seconds,
                const std::string& dir
        );

        ~recorder();

        void start(
                messages::stations_channel &rx_stations,
                messages::recordings_channel &tx_recordings
        );

    private:
        int duration_seconds;
        std::string dir;
        receiver *rx;

    private:
        static std::string get_rtl_device_string(const std::string &device_id);

        static uint64_t now_timestamp();
    };

}

#endif //ALIS_WORKERS_H
