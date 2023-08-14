#include <iostream>
#include <receiver.h>
#include <cpr/cpr.h>
#include <argparse/argparse.hpp>
#include <osmosdr/device.h>
#include <msd/channel.hpp>
#include <HTTPRequest.hpp>
#include <api/client.h>
#include <command.h>
#include <workers/recorder.h>
#include <workers/recognizer.h>
#include <workers/scheduler.h>
#include <workers/publisher.h>

const double DEFAULT_SAMPLE_RATE = 1.8 * 1e6;
const double DEFAULT_START_FREQ = 88 * 1e6;

const int DEFAULT_DURATION_SECONDS = 3;
const int DEFAULT_CYCLE_DELAY_SECONDS = 60;

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("peaer-", "0.1.0");

    argparse::ArgumentParser start_command("start");
    start_command.add_description("Start recording");
    start_command.add_argument("-k", "--key")
            .help("API key")
            .required();
    start_command.add_argument("-d", "--device")
            .help("Device ID(s) (see `devices` command)")
            .required();
    start_command.add_argument("-t", "--duration")
            .help("Recording duration in seconds")
            .scan<'i', int>()
            .default_value(DEFAULT_DURATION_SECONDS);
    start_command.add_argument("-c", "--cycle-delay")
            .help("Recording cycle delay in seconds")
            .scan<'i', int>()
            .default_value(DEFAULT_CYCLE_DELAY_SECONDS);
    start_command.add_argument("-r", "--sample_rate")
            .help("Sample rate")
            .scan<'g', double>()
            .default_value(DEFAULT_SAMPLE_RATE);
    start_command.add_argument("-f", "--freq")
            .help("Start frequency (not very useful)")
            .scan<'g', double>()
            .default_value(DEFAULT_START_FREQ);

    argparse::ArgumentParser devices_command("devices");
    devices_command.add_description("List available devices");

    program.add_subparser(start_command);
    program.add_subparser(devices_command);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cout << err.what() << std::endl;

        if (program.is_subcommand_used("start")) {
            std::cout << start_command;
        } else {
            std::cout << program;
        }

        exit(0);
    }

    if (program.is_subcommand_used("devices")) {
        list_rtl_devices();
        exit(0);
    }

    if (program.is_subcommand_used("start")) {
        std::cout << "HERE" << std::endl;
        std::cout << start_command.get<std::string>("-d") << std::endl;
        return start(
                start_command.get<double>("--freq"),
                start_command.get<double>("--sample_rate"),
                start_command.get<std::string>("--device"),
                start_command.get<std::string>("--key")
        );
    }

    std::cout << program;
    exit(0);
}

int start(double start_freq, double sample_rate, const std::string &device_id, const std::string &key) {
    workers::messages::stations_channel stations_tx;
    workers::messages::recordings_channel recordings;
    workers::messages::recognitions_channel recognitions_rx;

    workers::recorder recorder(
            device_id,
            start_freq,
            sample_rate,
            5,
            "/tmp"
    );

    workers::recognizer recognizer(
            "/usr/bin/songrec"
    );

    auto client = api::client("http://localhost:6969/api", key);

    workers::scheduler scheduler(
            &client,
            90
    );

    workers::publisher publisher(
            &client
    );

    std::jthread recorder_thread(&workers::recorder::start, &recorder, std::ref(stations_tx), std::ref(recordings));
    std::jthread recognizer_thread(&workers::recognizer::start, &recognizer, std::ref(recordings), std::ref(recognitions_rx));
    std::jthread scheduler_thread(&workers::scheduler::start, &scheduler, std::ref(stations_tx));
    std::jthread publisher_thread(&workers::publisher::start, &publisher, std::ref(recognitions_rx));

    return 0;
}

void list_rtl_devices() {
    osmosdr::devices_t devs = osmosdr::device::find();
    osmosdr::devices_t rtl_devs;

    for (auto &dev: devs) {
        if (dev.to_string().find("rtl=") == std::string::npos) {
            continue;
        }

        rtl_devs.push_back(dev);
    }

    if (rtl_devs.empty()) {
        std::cout << "No hardware devices found" << std::endl;
    }

    std::cout << "ID\tLABEL" << std::endl;
    for (auto &dev: rtl_devs) {
        std::cout << dev["rtl"] << "\t" << dev["label"] << std::endl;
    }
}
