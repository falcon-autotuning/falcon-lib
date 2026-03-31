FROM ubuntu:22.04

# Install necessary build dependencies
RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  bison \
  flex \
  clang \
  pkg-config \
  git \
  curl \
  zip \
  unzip \
  tar \
  coreutils

# Set up your workspace
RUN git clone --branch dev --single-branch https://github.com/falcon-autotuning/falcon-lib.git /src
WORKDIR /src

# runs the make install command that builds all of the executables
ENV SUDO=""
RUN make install-deps && make install

# Set the environment variables exactly as requested
# (Added /opt/falcon/bin to PATH so your wrappers can find the CLI commands)
ENV PATH="/opt/falcon/bin:/opt/falcon/lib:${PATH}"
ENV LD_LIBRARY_PATH="/opt/falcon/lib:${LD_LIBRARY_PATH}"
ENV PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:${PKG_CONFIG_PATH}"

# Setup the entrypoint
COPY packaging/docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["entrypoint.sh"]
