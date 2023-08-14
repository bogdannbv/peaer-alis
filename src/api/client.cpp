#include "client.h"
#include <iostream>
#include <nlohmann/json.hpp>

namespace api {

    client::client(const std::string &base_url, const std::string &api_key) {
        this->base_url = base_url;
        this->api_key = api_key;
    }

    client::~client() = default;

    std::vector<station> client::get_stations() {
        std::vector<station> stations;
        auto endpoint = get_url("stations");
        cpr::Response response = cpr::Get(
                cpr::Url{endpoint},
                cpr::Header{
                        {"X-API-Key", api_key},
                        {"Accept", "application/json"},
                }
        );

        if (response.error.code != cpr::ErrorCode::OK) {
            throw exception("Request error: " + response.error.message);
        }

        if (response.status_code != cpr::status::HTTP_OK) {
            throw exception("Unable to get stations: " + response.status_line);
        }

        auto json = nlohmann::json::parse(response.text);

        for (auto &[key, st]: json.items()) {
            stations.push_back(station{
                    .id = st["id"],
                    .name = st["name"],
                    .country = st["country"],
                    .frequency = st["frequency"],
            });
        }

        return stations;
    }

    cpr::Response client::create_playback(int station_id, const std::string &shazam_json) {
        auto endpoint = get_url(std::format("stations/{}/playbacks", station_id));
        cpr::Response response = cpr::Get(
                cpr::Url{endpoint},
                cpr::Header{
                        {"X-API-Key",   api_key},
                        {"Content-Type", "application/json"},
                        {"Accept",       "application/json"},
                },
                cpr::Body{shazam_json}
        );

        if (response.error.code != cpr::ErrorCode::OK) {
            throw exception("Request error: " + response.error.message);
        }

        if (response.status_code != cpr::status::HTTP_OK) {
            throw exception("Unable to create playback: " + response.status_line);
        }

        return response;
    }

    std::string client::get_url(const std::string &path) {
        std::string separator;
        if (!base_url.ends_with("/") && !path.starts_with("/")) {
            separator = "/";
        }
        return std::format("{}{}{}", base_url, separator, path);
    }

}
