FROM ubuntu:24.04

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
  linux-libc-dev \
  python3-dev \
  python3 \
  python3-venv \
  autoconf \
  autoconf-archive \
  automake \
  libtool \
  ninja-build \
  lld \
  coreutils && \
  rm -rf /var/lib/apt/lists/*

# Install uv - correct installation for Ubuntu
RUN curl -LsSf https://astral.sh/uv/install.sh | sh
ENV PATH="/root/.local/bin:${PATH}"

# Set up your workspace
RUN git clone --branch dev --single-branch https://github.com/falcon-autotuning/falcon-lib.git /src
WORKDIR /src

# Verify uv is available
RUN which uv && uv --version

# runs the make install-deps command that builds all of the dependencies
ENV SUDO=""
RUN make install-deps 

# runs the make install command that builds the current package and installs it to /opt/falcon/
RUN make install

# Set the environment variables exactly as requested
# (Added /opt/falcon/bin to PATH so your wrappers can find the CLI commands)
ENV PATH="/opt/falcon/bin:/opt/falcon/lib:${PATH}"
ENV LD_LIBRARY_PATH="/opt/falcon/lib:${LD_LIBRARY_PATH}"
ENV PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:${PKG_CONFIG_PATH}"

# Setup the entrypoint
COPY packaging/docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["entrypoint.sh"]
