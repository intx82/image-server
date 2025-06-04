# Stage 1: Build
FROM debian:bookworm as builder

# Install build tools and libraries
RUN apt-get update && apt-get install -y \
    meson ninja-build cmake g++ libmagic-mgc libmagic1 \
    libmagic-dev libjpeg-dev libpng-dev libjsoncpp-dev \
    libspdlog-dev libgtest-dev pkg-config pkgconf pkgconf-bin libpkgconf3\
    git curl ca-certificates && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source and subprojects
COPY . .

# Configure and build
RUN meson setup build src --buildtype=release && \
    meson compile -C build && \
    ls -lah /app/build

# Stage 2: Runtime image
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    libmagic1 libmagic-mgc libjpeg62-turbo libpng16-16 libjsoncpp25 libspdlog1.10 && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy static files and built binary
COPY --from=builder /app/src/static ./static
COPY --from=builder /app/src/config.json ./config.json
COPY --from=builder /app/build/image_server .

# Create upload directory
RUN mkdir uploads

EXPOSE 8080

# Run the image server
ENTRYPOINT ["./image_server"]
