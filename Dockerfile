FROM archlinux:base-devel

ARG CPR_VERSION=1.10.4
ARG MAKE_JOBS=4

ENV DEVICE=0
ENV REC_DURATION=5
ENV REC_DIR=/tmp/alis
ENV INTERVAL=120
ENV SAMPLE_RATE=1800000
ENV START_FREQ=88000000
ENV SONGREC=/usr/sbin/songrec

COPY .. /alis
COPY docker-entrypoint.sh /usr/bin/docker-entrypoint.sh

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm \
      git \
      gnuradio \
      gnuradio-osmosdr \
      songrec \
      cmake \
      gcc \
      glibc \
      boost \
      boost-libs \
    && curl -fsSL https://github.com/libcpr/cpr/archive/refs/tags/${CPR_VERSION}.tar.gz -o cpr.tar.gz \
    && tar -xvf cpr.tar.gz \
    && cd cpr-${CPR_VERSION} && mkdir build && cd build \
    && cmake .. -DCPR_USE_SYSTEM_CURL=ON \
    && cmake --build . --target install -j${MAKE_JOBS} \
    && rm -r /cpr* \
    && cd /alis/build \
    && cmake .. \
    && cmake --build . --target alis -j${MAKE_JOBS} \
    && mv src/alis /usr/bin/alis \
    && rm -r /alis \
    && pacman -Scc --noconfirm

ENTRYPOINT ["/usr/bin/docker-entrypoint.sh"]
