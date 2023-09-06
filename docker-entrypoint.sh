#!/bin/env bash
set -e

isCommand() {
  if [ "$1" = "sh" ]; then
    return 1
  fi

  if [ "$1" = "start" ] || [ "$1" = "devices" ]; then
    return 0
  fi

  return 1
}

# check if the first argument passed in looks like a flag or a command
if [ "${1#-}" != "$1" ] || isCommand "$1"; then
  set -- alis "$@"
fi

# if no arguments are provided, set default to 'start'
if [ "$#" -eq 0 ]; then
  set -- alis start

  if [ -n "${DEVICE}" ]; then
    set -- "$@" "--device" "${DEVICE}"
  fi

  if [ -n "${API_URL}" ]; then
      set -- "$@" "--api-url" "${API_URL}"
  fi

  if [ -n "${API_KEY}" ]; then
        set -- "$@" "--api-key" "${API_KEY}"
  fi

  if [ -n "${REC_DIR}" ]; then
    set -- "$@" "--recordings-dir" "${REC_DIR}"
  fi

  if [ -n "${REC_DURATION}" ]; then
    set -- "$@" "--recording-duration" "${REC_DURATION}"
  fi

  if [ -n "${INTERVAL}" ]; then
    set -- "$@" "--scheduler-interval" "${INTERVAL}"
  fi

  if [ -n "${SAMPLE_RATE}" ]; then
    set -- "$@" "--sample_rate" "${SAMPLE_RATE}"
  fi

  if [ -n "${START_FREQ}" ]; then
    set -- "$@" "--freq" "${START_FREQ}"
  fi

  if [ -n "${SONGREC}" ]; then
    set -- "$@" "--songrec" "${SONGREC}"
  fi
fi

exec "$@"
