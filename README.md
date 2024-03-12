# Alis
***A lis***-tener and music recognizer for FM radio stations. \
Based on work done by the [Gqrx](https://github.com/gqrx-sdr/gqrx) team and [GNURadio](https://github.com/gnuradio/gnuradio)

# Dependencies
- [SongRec](https://github.com/marin-m/SongRec)
- [GNURadio](https://github.com/gnuradio/gnuradio) with the following components:
  * gnuradio-runtime
  * gnuradio-analog
  * gnuradio-blocks
  * gnuradio-digital
  * gnuradio-filter
  * gnuradio-fft
- [gnuradio-osmosdr](https://gitea.osmocom.org/sdr/gr-osmosdr)

## Basic Usage
```
./alis start \
    --api-url http://localhost:6969/api \
    --api-key "your-api-key" \
    --device 0 \
    --recordings-dir ./recordings \
    --recording-duration 4 \
    --scheduler-interval 120 \
    --songrec /usr/bin/songrec
```

## Docker build
```shell
docker buildx build --pull \
  -t bogdannbv/alis:latest \ 
  -t bogdannbv/alis:0.1.1 \
  .
```
