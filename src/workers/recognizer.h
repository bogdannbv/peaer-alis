#ifndef RECORDER_RECOGNIZER_H
#define RECORDER_RECOGNIZER_H

#include "messages.h"

namespace workers {

    class recognizer {
    public:
        explicit recognizer(
                const std::string &songrec_path

        );
//        ~recognizer();

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

#endif //RECORDER_RECOGNIZER_H
