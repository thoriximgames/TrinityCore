# Stage 1: Build on Linux
FROM ubuntu:22.04 as builder

RUN apt-get update && apt-get install -y \
    cmake \
    clang \
    make \
    libssl-dev \
    libasio-dev \
    flatbuffers-compiler \
    libflatbuffers-dev \
    libpq-dev \
    libpqxx-dev \
    libhiredis-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cd src/Shared/Protocol && flatc --cpp --csharp --gen-onefile MMO.fbs

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Stage 2: Runtime Linux Image
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    ca-certificates \
    flatbuffers-compiler \
    libpq5 \
    libpqxx-6.4 \
    libhiredis0.14 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /server
COPY --from=builder /app/build/MMOServer .

EXPOSE 8085
CMD ["./MMOServer"]