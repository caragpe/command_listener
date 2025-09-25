# Ground-to-Spacecraft Command Simulator

> **Mission Context**: This component simulates the command interface between Nyx’s ground control (Python) and onboard flight software (C++). It demonstrates a **safe, auditable, and testable** pattern for inter-language communication in mission-critical systems.

## 📌 Overview

The **Command Listener** consists of:
- A **C++ shared library** that validates commands against an allow-list and returns structured ACK/NACK responses.
- A **Python wrapper** that loads the library via `ctypes`, decodes error codes, and provides a CLI for manual testing.

Key design principles:
- **Deterministic behavior**: Fixed command set (`PING`, `STATUS`, `THRUST`) avoids runtime ambiguity.
- **Buffer safety**: Explicit buffer size checks prevent overruns; warnings on truncation.
- **Structured error reporting**: Numeric return codes map to human-readable messages via `error_codes.ini`.
- **Reproducible builds**: Dockerized CI ensures identical toolchain across environments.


## 🚀 Quick Start

All development, testing, and execution are encapsulated in a **reproducible Docker image**. This ensures identical behavior across your laptop, CI, and deployment environments.

### 1. Build the Container
```bash
docker build -t command-listener .
```

> 💡 The image compiles the C++ library, installs Python dependencies, and configures the runtime environment.

### 2. Run Integration Tests Inside the Container
```bash
docker run --rm command-listener pytest -m integration -v
```

This executes the **real integration tests** against the compiled C++ shared library—no host setup required.

### 3. Test Manually via CLI
```bash
# Success case
docker run --rm command-listener python -m python.examples.example PING

# Failure case
docker run --rm command-listener python -m python.examples.example INVALID
```

Expected output:
```
✅ ACK: PING
❌ Error -4: NACK written successfully (invalid command)
```

## 🗂️ Project Layout

| Path | Purpose |
|------|--------|
| `src/process_command.cpp` | C++ implementation (validates `PING`, `STATUS`, `THRUST`) |
| `python/examples/example.py` | Python `ctypes` wrapper + CLI |
| `python/examples/error_codes.ini` | Authoritative error message mapping |
| `python/tests/test_integration.py` | End-to-end tests against real C++ lib |
| `Dockerfile` | Reproducible build/test environment (used in CI) |
| `.github/workflows/ci.yaml` | Full CI pipeline: lint → build → test → coverage |

## 🔒 Safety & Reliability Features

- **Buffer bounds checking**: C++ rejects undersized buffers (`bufsize < 1`) and detects truncation.
- **Guard bytes**: C++ unit tests use guard regions to catch buffer overruns.
- **Explicit error codes**:  
  `-4` = invalid command, `-3` = response too large, etc. (see `error_codes.ini`)


## 🧪 Testing Strategy

| Test Type | Location | Purpose |
|----------|--------|--------|
| **C++ Unit** | `src/tests/` | Validate buffer safety, error codes, edge cases |
| **Python Unit** | `python/tests/test_example.py` | Mocked tests for config, encoding, error handling |
| **Integration** | `python/tests/test_integration.py` | Real `ctypes` call to compiled C++ lib |
| **Linting** | `flake8`, `clang-format` | Enforced in CI |

