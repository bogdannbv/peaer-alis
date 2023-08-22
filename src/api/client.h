#ifndef ALIS_API_CLIENT_H
#define ALIS_API_CLIENT_H

#include <cpr/cpr.h>

namespace api {

    typedef struct {
        int id;
        std::string name;
        std::string country;
        double frequency;
    } station;

    class client {
    public:
        explicit client(
                const std::string &base_url,
                const std::string &api_key
        );

        ~client();

        std::vector<station> get_stations();

        cpr::Response create_playback(
                int station_id,
                uint64_t started_at,
                uint64_t finished_at,
                const std::string &shazam_json
        );

    private:
        std::string api_key;
        std::string base_url;

    private:
        std::string get_url(const std::string &path);
    };

    class exception : public std::runtime_error {
    public:
        explicit exception(const std::string &message) : std::runtime_error{message} {}
    };

}

#endif //ALIS_API_CLIENT_H
