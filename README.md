# Alis
A hacked-together FM Radio listener and music recognizer

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