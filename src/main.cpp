#include <utils.h>
#include <iostream>
#include <argparse/argparse.hpp>
#include <osmosdr/device.h>
#include <msd/channel.hpp>
#include <api/client.h>
#include <workers/recorder.h>
#include <workers/recognizer.h>
#include <workers/scheduler.h>
#include <workers/publisher.h>
#include <spdlog/spdlog.h>
#include <receivers/receiver.h>

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
        const std::string &songrec_path,
        const std::string &recordings_dir,
        int scheduler_interval,
        int duration,
        double start_freq,
        double sample_rate
);

int main(int argc, char *argv[]) {
    auto receiver = new alis::receivers::receiver(
            "0",
            1
    );

    receiver->set_auto_gain(true);
    receiver->start();
    receiver->start_recording("/tmp/alis.wav");

    std::this_thread::sleep_for(std::chrono::seconds(10));

    receiver->stop_recording();

    return 0;
    argparse::ArgumentParser program(PROGRAM_NAME, std::ref(PROGRAM_VERSION));

    argparse::ArgumentParser start_command("start");
    start_command.add_description("start recording");
    start_command.add_argument("-d", "--device")
            .help("device ID(s) (see `devices` command)")
            .required();
    start_command.add_argument("-u", "--api-url")
            .help("API base URL")
            .required();
    start_command.add_argument("-k", "--api-key")
            .help("API key")
            .required();
    start_command.add_argument("-o", "--recordings-dir")
            .help("recordings directory")
            .default_value(DEFAULT_RECORDINGS_DIR);
    start_command.add_argument("-t", "--recording-duration")
            .help("recording duration in seconds")
            .scan<'i', int>()
            .default_value(DEFAULT_DURATION_SECONDS);
    start_command.add_argument("-c", "--scheduler-interval")
            .help("scheduler cycle delay in seconds")
            .scan<'i', int>()
            .default_value(DEFAULT_SCHEDULER_INTERVAL);
    start_command.add_argument("-s", "--sample_rate")
            .help("sample rate")
            .scan<'g', double>()
            .default_value(DEFAULT_SAMPLE_RATE);
    start_command.add_argument("--freq")
            .help("start frequency (not very useful)")
            .scan<'g', double>()
            .default_value(DEFAULT_START_FREQ);
    auto songrec_arg = &start_command.add_argument("--songrec")
            .help("path to SongRec binary");

    auto songrec_path = utils::find_songrec();
    if (!songrec_path.empty()) {
        songrec_arg->default_value(songrec_path);
    } else {
        songrec_arg->required();
    }

    argparse::ArgumentParser devices_command("devices");
    devices_command.add_description("list available devices");

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
        auto recordings_dir = start_command.get<std::string>("--recordings-dir");
        if (!utils::check_dir_or_create(recordings_dir)) {
            spdlog::critical("Failed to create recordings directory: {}", recordings_dir);
            exit(1);
        }
        return start(
                start_command.get<std::string>("--device"),
                start_command.get<std::string>("--api-url"),
                start_command.get<std::string>("--api-key"),
                start_command.get<std::string>("--songrec"),
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
        const std::string &songrec_path,
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

    workers::recognizer recognizer(songrec_path);

    workers::scheduler scheduler(&client, scheduler_interval);

    workers::publisher publisher(&client);

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
