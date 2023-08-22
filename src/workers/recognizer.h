#ifndef ALIS_WORKERS_RECOGNIZER_H
#define ALIS_WORKERS_RECOGNIZER_H

#include "messages.h"

namespace workers {

    class recognizer {
    public:
        explicit recognizer(
                const std::string &songrec_path

        );

        void start(
                messages::recordings_channel &rx_recordings,
                messages::recognitions_channel &tx_recognitions
        );

    private:
        std::string songrec_path;

    private:
        std::string recognize(const std::string &path);
    };

}

#endif //ALIS_WORKERS_RECOGNIZER_H
