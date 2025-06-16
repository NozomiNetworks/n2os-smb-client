FROM debian:bullseye-slim

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libssl-dev \
        heimdal-dev

RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*    

WORKDIR /workspace
