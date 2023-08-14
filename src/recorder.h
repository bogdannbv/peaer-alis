#ifndef RECORDER_RECORDER_H
#define RECORDER_RECORDER_H

namespace recorder {
    typedef struct {
        int sample_rate;
        int channels;
        int frames;
        int buffer_size;
        int device_index;
        int format;
        int period_size;
        int period_count;
    } station;
}

#endif //RECORDER_RECORDER_H
