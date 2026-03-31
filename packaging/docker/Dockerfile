FROM ubuntu:22.04

# Install necessary build dependencies
RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  bison \
  flex \
  pkg-config 

# Set up your workspace
WORKDIR /src
COPY . .

# ---------------------------------------------------------
# Template for your build chain
# Use '&&' to maintain the directory context across commands
# ---------------------------------------------------------
RUN cd /src/dsl && \
  make clean && \
  make build && \
  cd /src/external && \
  make all && \
  cd /src && \
  make install DESTDIR=/opt/falcon

# Set the environment variables exactly as requested
# (Added /opt/falcon/bin to PATH so your wrappers can find the CLI commands)
ENV PATH="/opt/falcon/bin:/opt/falcon/lib:${PATH}"
ENV LD_LIBRARY_PATH="/opt/falcon/lib:${LD_LIBRARY_PATH}"
ENV PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:${PKG_CONFIG_PATH}"

# Setup the entrypoint
COPY packaging/docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["entrypoint.sh"]
