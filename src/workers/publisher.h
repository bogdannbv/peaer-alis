//
// Created by bogdan on 8/15/23.
//

#ifndef RECORDER_PUBLISHER_H
#define RECORDER_PUBLISHER_H

#include <api/client.h>
#include "messages.h"

namespace workers {

    class publisher {
    public:
        explicit publisher(
                api::client *client
        );

//        ~publisher();

        void start(messages::recognitions_channel &recognitions);

    private:
        api::client *client;
    };

} // workers

#endif //RECORDER_PUBLISHER_H
