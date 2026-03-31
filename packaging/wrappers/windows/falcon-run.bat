@echo off
IF "%NATS_URL%"=="" SET NATS_URL=nats://host.docker.internal:4222

docker run -it --rm ^
    -v "%cd%:/workspace" ^
    -w /workspace ^
    -e NATS_URL="%NATS_URL%" ^
    falcon-cli:latest falcon-run %*
