FROM debian:bullseye-slim

# hadolint ignore=DL3008
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libssl-dev \
        libkrb5-dev \
        libgssapi-krb5-2 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# RUN apt-get update && \
#     apt-get install -y --no-install-recommends \
#         build-essential \
#         cmake \
#         libssl-dev \
#         heimdal-dev && \
#     apt-get clean && \
#     rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
