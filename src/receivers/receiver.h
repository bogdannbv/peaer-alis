#ifndef ALIS_RECEIVER_H
#define ALIS_RECEIVER_H

#include <gnuradio/blocks/file_sink.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/wavfile_sink.h>
#include <gnuradio/blocks/wavfile_source.h>
#include <gnuradio/top_block.h>
#include <osmosdr/source.h>
#include <string>

#include "dsp/correct_iq_cc.h"
#include "dsp/downconverter.h"
#include "dsp/filter/fir_decim.h"
#include "dsp/rx_noise_blanker_cc.h"
#include "dsp/rx_filter.h"
#include "dsp/rx_meter.h"
#include "dsp/rx_agc_xx.h"
#include "dsp/rx_demod_fm.h"
#include "dsp/rx_demod_am.h"
#include "dsp/rx_fft.h"
#include "dsp/sniffer_f.h"
#include "dsp/resampler_xx.h"
#include "receivers/receiver_base.h"

#include "zero_file.h"

namespace alis::receivers {

    class receiver {
    public:
        enum status {
            STATUS_OK = 0,
            STATUS_ERROR = 1,
        };

        enum filter_shape {
            FILTER_SHAPE_SOFT = 0,   /*!< Soft: Transition band is TBD of width. */
            FILTER_SHAPE_NORMAL = 1, /*!< Normal: Transition band is TBD of width. */
            FILTER_SHAPE_SHARP = 2   /*!< Sharp: Transition band is TBD of width. */
        };

        explicit receiver(const std::string &device_id = "", unsigned int decimation = 1);

//        ~receiver();

        void start();

        void stop();

        double set_sample_rate(double rate);

        status set_center_freq(double freq_hz);

        status set_auto_gain(bool automatic);

//        status set_demod(rx_demod demod, bool force = false);

        status set_af_gain(float gain_db);

        status start_recording(const std::string filename);

        bool is_recording(void) const { return recording; }

        status stop_recording();

    private:
        bool running;
        bool recording;
        double center_freq;
        double sample_rate;
        unsigned int decimation;
        double decimation_rate;

        // GNURadio Blocks
        gr::top_block_sptr top_block;

        osmosdr::source::sptr src;
        fir_decim_cc_sptr input_decim;
        receiver_base_cf_sptr rx;

        dc_corr_cc_sptr dc_corr;
        iq_swap_cc_sptr iq_swap;

        rx_fft_c_sptr iq_fft;
        rx_fft_f_sptr audio_fft;

        downconverter_cc_sptr downconverter;

        gr::blocks::multiply_const_ff::sptr wav_gain0; /*!< WAV file gain block. */
        gr::blocks::multiply_const_ff::sptr wav_gain1; /*!< WAV file gain block. */

        gr::blocks::wavfile_sink::sptr      wav_sink;
        gr::blocks::null_sink::sptr         audio_null_sink0;
        gr::blocks::null_sink::sptr         audio_null_sink1;

        sniffer_f_sptr    sniffer;
        resampler_ff_sptr sniffer_rr;

    private:
        static std::string get_rtl_device_string(const std::string &device_id);

        status connect();
    };

} // receivers

#endif //ALIS_RECEIVER_H
