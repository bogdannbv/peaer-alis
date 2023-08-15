#include <iostream>
#include <cpr/cpr.h>
#include <argparse/argparse.hpp>
#include <osmosdr/device.h>
#include <msd/channel.hpp>
#include <api/client.h>
#include <workers/recorder.h>
#include <workers/recognizer.h>
#include <workers/scheduler.h>
#include <workers/publisher.h>

const std::string PROGRAM_NAME = "alis";
const std::string PROGRAM_VERSION = "0.1.0";

const double DEFAULT_SAMPLE_RATE = 1800000;
const double DEFAULT_START_FREQ = 88000000;

const int DEFAULT_DURATION_SECONDS = 3;
const int DEFAULT_SCHEDULER_INTERVAL = 120;

const std::string DEFAULT_RECORDINGS_DIR = "/tmp/" + PROGRAM_NAME;

int list_rtl_devices();

int start(
        const std::string &device_id,
        const std::string &api_base_url,
        const std::string &api_key,
        const std::string &recordings_dir,
        int scheduler_interval,
        int duration,
        double start_freq,
        double sample_rate
);

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program(PROGRAM_NAME, PROGRAM_VERSION);

    argparse::ArgumentParser start_command("start");
    start_command.add_description("Start recording");
    start_command.add_argument("-d", "--device")
            .help("Device ID(s) (see `devices` command)")
            .required();
    start_command.add_argument("-u", "--api-url")
            .help("API base URL")
            .required();
    start_command.add_argument("-k", "--api-key")
            .help("API key")
            .required();
    start_command.add_argument("-o", "--recordings-dir")
            .help("Recordings directory")
            .default_value(DEFAULT_RECORDINGS_DIR);
    start_command.add_argument("-t", "--recording-duration")
            .help("Recording duration in seconds")
            .scan<'i', int>()
            .default_value(DEFAULT_DURATION_SECONDS);
    start_command.add_argument("-c", "--scheduler-interval")
            .help("Scheduler cycle delay in seconds")
            .scan<'i', int>()
            .default_value(DEFAULT_SCHEDULER_INTERVAL);
    start_command.add_argument("-s", "--sample_rate")
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
        return list_rtl_devices();
    }

    if (program.is_subcommand_used("start")) {
        std::cout << "HERE" << std::endl;
        std::cout << start_command.get<std::string>("-d") << std::endl;
        return start(
                start_command.get<std::string>("--device"),
                start_command.get<std::string>("--api-url"),
                start_command.get<std::string>("--key"),
                start_command.get<std::string>("--recordings-dir"),
                start_command.get<int>("--scheduler-interval"),
                start_command.get<int>("--recording-duration"),
                start_command.get<double>("--freq"),
                start_command.get<double>("--sample_rate")
        );
    }

    std::cout << program;
    return 0;
}

int start(
        const std::string &device_id,
        const std::string &api_base_url,
        const std::string &api_key,
        const std::string &recordings_dir,
        int scheduler_interval,
        int duration,
        double start_freq,
        double sample_rate
) {
    auto client = api::client(api_base_url, api_key);

    workers::messages::stations_channel stations_tx;
    workers::messages::recordings_channel recordings;
    workers::messages::recognitions_channel recognitions_rx;

    workers::recorder recorder(
            device_id,
            start_freq,
            sample_rate,
            duration,
            recordings_dir
    );

    workers::recognizer recognizer(
            "/usr/bin/songrec"
    );

    workers::scheduler scheduler(
            &client,
            scheduler_interval
    );

    workers::publisher publisher(
            &client
    );

    std::jthread recorder_thread(
            &workers::recorder::start,
            &recorder,
            std::ref(stations_tx),
            std::ref(recordings)
    );

    std::jthread recognizer_thread(
            &workers::recognizer::start,
            &recognizer,
            std::ref(recordings),
            std::ref(recognitions_rx)
    );

    std::jthread scheduler_thread(
            &workers::scheduler::start,
            &scheduler,
            std::ref(stations_tx)
    );

    std::jthread publisher_thread(
            &workers::publisher::start,
            &publisher,
            std::ref(recognitions_rx)
    );

    return 0;
}

int list_rtl_devices() {
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

    return 0;
}
