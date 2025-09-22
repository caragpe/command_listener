# certified base image (Ubuntu 22.04 LTS)
FROM ubuntu:22.04

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    cmake \
    git \
    clang-format \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /app

COPY include/ ./include/
COPY src/ ./src/
COPY tests/ ./tests/
COPY examples/example.py ./examples/
COPY examples/requirements/requirements_test.txt ./examples/requirements.txt
COPY CMakeLists.txt .
COPY .clang-format .

RUN mkdir build && cd build && cmake .. && make && cmake --build . --target format

RUN pip3 install --no-cache-dir -r examples/requirements.txt

# Run as non-root user
RUN useradd --create-home --shell /bin/bash appuser
RUN chown -R appuser:appuser /app
USER appuser

CMD ["python3", "examples/example.py"]