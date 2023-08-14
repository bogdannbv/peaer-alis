#ifndef RECORDER_SCHEDULER_H
#define RECORDER_SCHEDULER_H

#include <string>
#include <api/client.h>
#include "messages.h"

namespace workers {

    class scheduler {
    public:
        scheduler(
                api::client *client,
                int interval
        );

        void start(messages::stations_channel &stations_channel);

    private:
        api::client *client;

        int interval;
    };


}

#endif //RECORDER_SCHEDULER_H
