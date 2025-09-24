# Certified base image (Ubuntu 22.04 LTS)
FROM ubuntu:22.04 AS build

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    cmake \
    git \
    clang-format \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /app

# Copy source
COPY src/ ./src/
COPY python/ ./python/
COPY CMakeLists.txt .
COPY .clang-format .
COPY pytest.ini .
COPY pyproject.toml .
COPY .flake8 .

# Build C++ library
RUN mkdir -p build \
    && cd build \
    && cmake .. \
    && make

RUN cd build \
    && ls -l libprocess_command.so

RUN mkdir -p ./python/build \
    && cp build/libprocess_command.so ./python/build/

FROM build as test

# After installing system Python
RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

# Install Python dependencies
RUN pip3 install --no-cache-dir -r ./python/examples/requirements_test.txt
RUN pip3 install setuptools
RUN pip3 install -e ./python

# Create and switch to non-root user
RUN useradd --create-home --shell /bin/bash appuser
RUN chown -R appuser:appuser /app
USER appuser

# Set default command — you can override at runtime
CMD ["python3", "python/examples/example.py", "COMMAND_1"]