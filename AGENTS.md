# Repository Guidelines

## Project Structure & Module Organization
- `src/main.c` holds the sample entry point (AMX load/store demo).
- `src/amx_util.c` / `src/amx_util.h` hold shared operand helpers.
- `CMakeLists.txt` defines the CMake build for the `c_amx` and `c_amx_tests` executables.
- `reference/` stores external sources used for study:
  - `reference/amx` mirrors the AMX reverse‑engineering notes.
  - `reference/llvm-project` is a local LLVM monorepo checkout for backend work.
- `cmake-build-debug/` is a local build output directory (regenerate as needed).

## Build, Test, and Development Commands
- Configure and build this repo:
  ```bash
  cmake -S . -B cmake-build-debug -G Ninja
  cmake --build cmake-build-debug
  ```
- Run the sample binary:
  ```bash
  ./cmake-build-debug/c_amx
  ```
## Execution Requirement
- After making code changes, always run the relevant build, run tests, and execute the binary to verify behavior, then check the results.
## Proposal Guidance
- When presenting multiple options, state which option is recommended.
- Build LLVM (used for AMX backend experiments) from `reference/llvm-project`:
  ```bash
  cmake -S reference/llvm-project/llvm -B reference/llvm-project/build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="AArch64"
  ninja -C reference/llvm-project/build
  ```

## Coding Style & Naming Conventions
- C code uses C17 (`CMAKE_C_STANDARD 17`).
- Indentation: 4 spaces, K&R brace style (see `main.c`).
- Filenames: lower_snake_case for C sources (e.g., `amx_intrinsics.c`).
- Keep experimental scripts or notes inside `reference/` to avoid mixing with buildable sources.

## Testing Guidelines
- Tests live under `tests/` and build into `c_amx_tests`.
- For LLVM backend validation, use `lit` from the LLVM build:
  ```bash
  ninja -C reference/llvm-project/build check-clang
  ninja -C reference/llvm-project/build check-llvm
  ```
- When adding tests, prefer LLVM-style `clang/test/CodeGen` and `llvm/test/CodeGen/AArch64` under `reference/llvm-project`.

## Commit & Pull Request Guidelines
- No commit history is available in this directory, so no enforced message convention yet.
- If you introduce a convention, document it here and keep commits focused (one change per commit).
- PRs (if used) should describe the learning phase, link to any relevant notes, and include command output for new tests.

## Security & Configuration Tips
- AMX is undocumented and may vary by chip/OS; avoid assuming behavior is stable.
- Use `reference/llvm-project/build/bin/clang` and `llc` to ensure you are testing against your local LLVM build.
