#ifndef ALIS_WORKERS_PUBLISHER_H
#define ALIS_WORKERS_PUBLISHER_H

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

#endif //ALIS_WORKERS_PUBLISHER_H
