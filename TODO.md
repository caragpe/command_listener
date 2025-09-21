
# 📋 TODO (C++ module)

This TODO outlines a comprehensive refactor and enhancement plan for the C++ command processing module. The goals are to improve maintainability, performance, test coverage, and documentation while ensuring backward compatibility.

## 🚨 P1 – Single Source of Truth (DRY Enforcement)

- [ ] **Create `process_command_impl.hpp`** (internal header — NOT for public consumption)
  - Move **all** string literals:
    - `kPrefix` → rename to `kAckPrefix`
    - `kNackPrefix`
    - `kErrorMsg` → rename to `kNullMsg`
    - `kInvalidCmdMsg` → rename to `kInvalidMsg`
    - `kValidCommands` → rename to `kCmds`
  - Move **all** return-code constants as `constexpr int`:
    ```cpp
    constexpr int kSuccess = 0;
    constexpr int kInvalidBuffer = -1;
    constexpr int kNullCmdOverflow = -2;
    constexpr int kAckOverflow = -3;
    constexpr int kNackWritten = -4;  // was kNackOk
    constexpr int kNackOverflow = -5;
    ```
- [ ] **Delete all duplicate literals & constants** from:
  - `process_command.cpp`
  - Test file (`test_process_command.cpp`)
- [ ] **Include `process_command_impl.hpp`** in both:
  - `process_command.cpp`
  - Test file
- [ ] ⚠️ Add header comment:  
  `// INTERNAL USE ONLY — not part of public API. For impl and testing.`
 
> ✅ Validation: `grep -r "ACK: " src/ test/` should return only hits in `process_command_impl.hpp`

---

## ⚡ P2 – O(1) Command Lookup (Scalability & Safety)

- [ ] Replace `std::any_of` linear search in `is_valid_command` with `constexpr` hash + `switch` (assumes command set is small & stable)
- [ ] Add `static_assert` to ensure table size matches switch cases (if using `switch`):
  ```cpp
  static_assert(std::size(kCmds) == 3, "Update switch cases when adding/removing commands");
  ```
- [ ] Preserve exact public behavior — tests must pass unchanged.

> ✅ Validation: Add `COMMAND_4` → only `process_command_impl.hpp` changes, tests still pass.

---

## 🧹 P3 – Test DRY & Cleanup

- [ ] **Collapse test helpers**:
  - Delete `test_command_with_guards` and `test_command`
  - Keep single:  
    ```cpp
    static void test_command(const char *cmd, size_t bufsize, const std::string &expected,
                             int code, const std::string &name, bool use_guards = true)
    ```
- [ ] **Use named constants from `process_command_impl.hpp`** — eliminate magic numbers and duplicated strings in tests.
- [ ] **Rename for clarity**:
  - `kNackOk` → `kNackWritten` (in `process_command_impl.hpp`)
- [ ] Improve test section names to be specification-like:
  ```cpp
  SECTION("Buffer exactly fits ACK → returns kSuccess and writes full message")
  ```

> ✅ Validation: Run tests — coverage and behavior unchanged.

---

## 🧼 P4 – Naming, Style & Modern C++

- [ ] **Rename `build_and_copy` → `format_to_buffer`** (more descriptive)
- [ ] **Mark internal free functions `noexcept`**:
  ```cpp
  bool format_to_buffer(...) noexcept;
  bool is_valid_command(...) noexcept;
  ```
- [ ] **Use `std::string_view` internally** (C++17):
  - Replace `const char*` parameters and `std::strcmp`
  - Avoids redundant `strlen`, safer, more efficient
- [ ] Add `[[nodiscard]]` to helper functions that return success/fail
 
> ✅ Validation: Zero behavior change — all tests pass.

---

## 📚 P5 – Documentation & Contract Clarity

- [ ] **Add Doxygen to `process_command.h`**:
  ```cpp
  /**
   * @brief Processes a command string and writes human-readable ACK/NACK to buffer.
   * @param command Input command (null or empty allowed)
   * @param buffer Output buffer (must not be null if bufsize > 0)
   * @param bufsize Size of output buffer (including NUL)
   * @return 0 on success, negative codes on error (see process_command_impl.hpp for details)
   * @note Thread-safe: contains no static or global state.
   * @note Output format (e.g., "ACK: ...") is part of stable API — do not change without versioning.
   */
  extern "C" int process_command(const char *command, char *buffer, size_t bufsize);
  ```
- [ ] Add thread-safety note:  
  `// Stateless and noexcept — safe for concurrent use.`
- [ ] Add API contract comment in test file:
  ```cpp
  // NOTE: Exact output strings are part of public API. Tests validate literal matches.
  ```

---

## 🛡️ P6 – Future Proofing & Robustness (Optional but Recommended)

- [ ] **Add fuzz target** (e.g., for libFuzzer):
  ```cpp
  extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
      if (size == 0) return 0;
      std::vector<char> buf(256);
      std::string cmd(reinterpret_cast<const char*>(data), size);
      process_command(cmd.c_str(), buf.data(), buf.size());
      return 0;
  }
  ```
- [ ] **Add CI step** to validate single-source rule:
  ```bash
  # In CI or pre-commit:
  if git diff --name-only HEAD^ | grep -v "process_command_impl.hpp" | grep -q ".*"; then
      echo "ERROR: Modified files outside process_command_impl.hpp — violates single-source rule"
      exit 1
  fi
  ```
- [ ] Consider compile-time command validation (if using `switch`):
  ```cpp
  // Ensure all kCmds are handled in switch
  #define VALIDATE_CMD(cmd) case cmd: return true;
  ```

---

## ✅ Validation Checklist (Run After All Tasks)

- [ ] ✅ Adding `COMMAND_4` requires editing ONLY `process_command_impl.hpp`.
- [ ] ✅ All tests pass (including guard-byte checks).
- [ ] ✅ No `const char*` literals duplicated outside `process_command_impl.hpp`.
- [ ] ✅ Doxygen renders correctly.
- [ ] ✅ Fuzz target runs without crashes (if implemented).
- [ ] ✅ CI script validates single-source rule.


📆 **Last Updated**: 2025-04-05  
