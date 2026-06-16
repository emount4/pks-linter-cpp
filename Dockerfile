# syntax=docker/dockerfile:1

FROM ubuntu:24.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY tests ./tests
COPY sample_project ./sample_project
COPY config.example.ini ./config.example.ini

RUN cmake -S . -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
    && cmake --build /build \
    && if [ ! -e /sample_project ]; then ln -s /src/sample_project /sample_project; fi \
    && if [ ! -e /config.example.ini ]; then ln -s /src/config.example.ini /config.example.ini; fi \
    && ctest --test-dir /build --output-on-failure

FROM builder AS test
WORKDIR /build
ENTRYPOINT ["ctest", "--output-on-failure"]

FROM ubuntu:24.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
COPY --from=builder /build/cpp_linter /usr/local/bin/cpp_linter
COPY --from=builder /src/sample_project /opt/cpp_linter/share/sample_project
COPY --from=builder /src/config.example.ini /opt/cpp_linter/share/config.example.ini

ENTRYPOINT ["cpp_linter"]
CMD ["--project", "/opt/cpp_linter/share/sample_project", "--mode", "full"]
