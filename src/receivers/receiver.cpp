#include "receiver.h"
#include "wfmrx.h"
#include <zero_file.h>

#define TARGET_QUAD_RATE 1e6
#define WAV_FILE_GAIN 0.5
#define AUDIO_SAMPLE_RATE 48000

namespace alis::receivers {

    receiver::receiver(const std::string &device_id, unsigned int decimation) :
            running(false),
            recording(false),
            center_freq(88000000), // Digi FM (RO) - closest to FM band center :D
            sample_rate(1.8 * 1e6),
            decimation(decimation) {

        if (device_id.empty()) {
            throw std::runtime_error("No device ID specified");
        }

        top_block = gr::make_top_block("alis");

        // RTL-SDR Source
        src = osmosdr::source::make(get_rtl_device_string(device_id));

        if (decimation >= 2) {
            try {
                input_decim = make_fir_decim_cc(this->decimation);
            } catch (std::range_error &e) {
                spdlog::error("Couldn't creating input decimator {}: {} Using decimation 1.",
                              this->decimation,
                              e.what());
                this->decimation = 1;
            }

            decimation_rate = sample_rate / this->decimation;
        } else {
            decimation_rate = sample_rate;
        }

        auto ddc_decim = std::max(1, (int) (decimation_rate / 1e6));
        auto quad_rate = decimation_rate / ddc_decim;
        downconverter = make_downconverter_cc(ddc_decim, 0.0, decimation_rate);
        rx = make_wfmrx((float) quad_rate, AUDIO_SAMPLE_RATE);
        rx->set_demod(wfmrx::WFMRX_DEMOD_MONO);

        iq_swap = make_iq_swap_cc(false);
        dc_corr = make_dc_corr_cc(decimation_rate, 1.0);

        wav_gain0 = gr::blocks::multiply_const_ff::make(WAV_FILE_GAIN);
        wav_gain1 = gr::blocks::multiply_const_ff::make(WAV_FILE_GAIN);

        auto file_path = zero_file::get_random_tmp_file(10);
        try {
            wav_sink = gr::blocks::wavfile_sink::make(file_path.c_str(), 2,
                                                      (unsigned int) AUDIO_SAMPLE_RATE,
                                                      gr::blocks::FORMAT_WAV,
                                                      gr::blocks::FORMAT_PCM_16
            );
        } catch (std::runtime_error &e) {
            spdlog::error("Can not open {}: {}", file_path, e.what());
        }

        audio_null_sink0 = gr::blocks::null_sink::make(sizeof(float));
        audio_null_sink1 = gr::blocks::null_sink::make(sizeof(float));

        connect();
    }

    receiver::status receiver::connect() {

        gr::basic_block_sptr last_block;

        last_block = src;

        if (decimation >= 2) {
            spdlog::critical("HEREEE");
            top_block->connect(last_block, 0, input_decim, 0);
            last_block = input_decim;
        }

        top_block->connect(last_block, 0, iq_swap, 0);

        top_block->connect(iq_swap, 0, downconverter, 0);

        top_block->connect(downconverter, 0, rx, 0);

        top_block->connect(rx, 0, audio_null_sink0, 0);

        return receiver::STATUS_OK;
    }

    void receiver::start() {
        if (!running) {
            top_block->start();
            running = true;
        }
    }

    void receiver::stop() {
        if (running) {
            top_block->stop();
            top_block->wait();
            running = false;
        }
    }

    receiver::status receiver::set_auto_gain(bool automatic) {
        src->set_gain_mode(automatic);
        return receiver::STATUS_OK;
    }

    receiver::status receiver::start_recording(const std::string filename) {
        if (recording) {
            spdlog::error("Can not start recording (already recording)");

            return STATUS_ERROR;
        }

        if (!running) {
            spdlog::error("Can not start recording (receiver not running)");

            return STATUS_ERROR;
        }

        top_block->lock();

        top_block->connect(rx, 0, wav_gain0, 0);
        top_block->connect(rx, 1, wav_gain1, 0);
        top_block->connect(wav_gain0, 0, wav_sink, 0);
        top_block->connect(wav_gain1, 0, wav_sink, 1);

        top_block->unlock();
        recording = true;

        spdlog::critical(dot_graph(top_block));

        spdlog::info("Recording audio to {}", filename);

        return STATUS_OK;
    }

    receiver::status receiver::stop_recording() {
        if (!recording) {
            spdlog::error("Can not stop audio recorder (not recording)");

            return STATUS_ERROR;
        }

        if (!running) {
            spdlog::error("Can not stop audio recorder (receiver not running)");

            return STATUS_ERROR;
        }

        top_block->lock();
        wav_sink->close();
        top_block->disconnect(rx, 0, wav_gain0, 0);
        top_block->disconnect(rx, 1, wav_gain1, 0);
        top_block->disconnect(wav_gain0, 0, wav_sink, 0);
        top_block->disconnect(wav_gain1, 0, wav_sink, 1);

        // Temporary workaround for https://github.com/gnuradio/gnuradio/issues/5436
        top_block->disconnect(downconverter, 0, rx, 0);
        top_block->connect(downconverter, 0, rx, 0);
        // End temporary workaronud

        top_block->unlock();
        wav_gain0.reset();
        wav_gain1.reset();
        wav_sink.reset();
        recording = false;

        spdlog::info("Audio recorder stopped");

        return STATUS_OK;
    }

    std::string receiver::get_rtl_device_string(const std::string &device_id) {
        return "rtl=" + device_id;
    }

    receiver::status receiver::set_center_freq(double freq_hz) {
        center_freq = freq_hz;

        src->set_center_freq(center_freq);

        return receiver::STATUS_ERROR;
    }

    double receiver::set_sample_rate(double rate) {
        double current_rate;
        bool rate_has_changed;

        current_rate = src->get_sample_rate();

        rate_has_changed = !(rate == current_rate ||
                             std::abs(rate - current_rate) < std::abs(std::min(rate, current_rate))
                                                             * std::numeric_limits<double>::epsilon());

        top_block->lock();

        try {
            sample_rate = src->set_sample_rate(rate);
        }
        catch (std::runtime_error &e) {
            sample_rate = 0;
        }

        if (sample_rate == 0) {
            // This can be the case when no device is attached and gr-osmosdr
            // puts in a null_source with rate 100 ksps or if the rate has not
            // changed
            if (rate_has_changed) {
                std::cerr << std::endl;
                std::cerr << "Failed to set RX input rate to " << rate << std::endl;
                std::cerr << "Your device may not be working properly." << std::endl;
                std::cerr << std::endl;
            }
            sample_rate = rate;
        }
    }

} // receivers