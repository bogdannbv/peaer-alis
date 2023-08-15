#ifndef ALIS_WORKERS_MESSAGES_H
#define ALIS_WORKERS_MESSAGES_H

#include <string>
#include <msd/channel.hpp>

namespace workers::messages {

    struct station {
        int id;
        double frequency;
    };

    struct recording {
        messages::station station;
        uint64_t started_at;
        uint64_t finished_at;
        std::string file_path;
    };

    struct recognition {
        messages::recording recording;
        std::string shazam_response;
        double publish_tries = 0;
    };

    typedef msd::channel<station> stations_channel;
    typedef msd::channel<recording> recordings_channel;
    typedef msd::channel<recognition> recognitions_channel;

} // workers

#endif //ALIS_WORKERS_MESSAGES_H
