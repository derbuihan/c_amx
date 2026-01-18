# AMX Study

This repository is for studying Apple Silicon AMX (Apple Matrix Extensions) and learning how to add backend support in clang/llvm (primarily as an AArch64 extension: instructions, intrinsics, and code generation).

- Goal: self-study (understanding LLVM TableGen / ISel / CodeGen)
- Assumption: AMX is undocumented by Apple and not intended for upstream support
- References: `reference/amx` (corsix/amx), `reference/llvm-project` (LLVM monorepo)

## Repository Layout

```
.
├── CMakeLists.txt
├── README.md
├── src
│   ├── amx_asm.S
│   ├── amx_asm.h
│   └── main.c
├── tests
│   └── test_ldst.c
└── reference
    ├── amx          # https://github.com/corsix/amx
    └── llvm-project # https://github.com/llvm/llvm-project
```

## Environment

- macOS on Apple Silicon (M1/M2/M3/M4)
- clang/llvm: build locally under `reference/llvm-project`
- build system: CMake + Ninja recommended

## Build LLVM (local)

Example (Debug + AArch64 + clang only):

```bash
cd reference/llvm-project
mkdir -p build && cd build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLLVM_ENABLE_PROJECTS="clang" \
  -DLLVM_TARGETS_TO_BUILD="AArch64" \
  ../llvm
ninja
```

After that, use `clang` / `llc` from `reference/llvm-project/build/bin/`.

## Build and Run (local)

```bash
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug
./cmake-build-debug/c_amx
```

## Run Tests

```bash
./cmake-build-debug/c_amx_tests
```

## What is AMX (quick notes)

AMX issues instructions from the CPU to a dedicated execution unit. Conceptually:

- Register file: X(8) + Y(8) + Z(64) (about 5KB total)
- 64-byte vectors are the basic load/store/compute unit
- Both matrix (outer product) and vector (pointwise) modes
- Many special operand forms: writemasks, shuffle, indexed load, etc.

## Learning Plan (Implementation Steps)

This README tracks the order for adding AMX support in LLVM. The approach is to start minimal and grow tests incrementally.

---

### Phase 0: Read & Verify

**Goal:** Understand AMX and the LLVM AArch64 backend structure.

- Read AMX documentation
    - `reference/amx/README.md`
    - `reference/amx/RegisterFile.md`
    - `reference/amx/Instructions.md`
- Skim LLVM AArch64 backend entry points
    - `llvm/lib/Target/AArch64/`
    - `AArch64InstrInfo.td`, `AArch64RegisterInfo.td`, `AArch64ISelDAGToDAG.cpp`
- Build and run AMX reference implementation if possible
    - `reference/amx/test.c`, `perf.c`

**Done criteria**
- Can explain AMX X/Y/Z registers and outer product
- Can explain the TableGen -> ISel -> asm flow

---

### Phase 1: Minimal Intrinsics (`set` / `clr`)

**Goal:** Connect clang builtin -> LLVM intrinsic -> AArch64 MC/asm output.

Implementation sketch (adjust to LLVM layout as needed):

1. Add clang builtins
    - `clang/include/clang/Basic/BuiltinsAArch64.def`
2. Add LLVM intrinsics
    - `llvm/include/llvm/IR/IntrinsicsAArch64.td`
3. Add AArch64 instruction definitions (TableGen)
    - `llvm/lib/Target/AArch64/*.td`
4. Add CodeGen tests (clang, llc)
    - `clang/test/CodeGen/...`
    - `llvm/test/CodeGen/AArch64/...`

**Done criteria**
- `__builtin_...amx_set()` emits `amx_set` (or expected encoding)
- `lit` tests pass

---

### Phase 2: Load/Store (`ldx/ldy/stx/sty` first)

**Goal:** Add AMX "64-byte span" data movement. This is where complex memory operands and immediate encodings appear.

- Targets: `ldx`, `ldy`, `stx`, `sty`
- Start with the simplest mode only (others later)
- Verify generated assembly and simple runtime sanity checks

**Done criteria**
- Can round-trip memory <-> AMX X/Y
- Runs without crashing at `-O0` and `-O2`

---

### Phase 3: Extract/Move (`extrx/extry` and parts of `extrh/extrv`)

**Goal:** Support Z -> X/Y transfers and internal data moves.

- `extrx` / `extry` are relatively simple (X <-> Y)
- `extrh` / `extrv` include writemask/shift/sat, so phase it in

**Done criteria**
- Can extract Z contents via X/Y to memory
- Minimal tests with fixed inputs/outputs

---

### Phase 4: Compute Core (FMA outer product / vector)

**Goal:** Enable the core AMX compute (outer product).

Initial target examples:
- Minimal mode of `fma16` or `fma32`
- Prefer matrix mode (bit 63 = 0) over vector mode (bit 63 = 1)

Key point:
- One instruction updates multiple rows/cols in Z. Model it via LLVM IR intrinsics first; pattern matching can come later.

**Done criteria**
- Compute a small GEMM tile (e.g., part of 16x16) via AMX instruction sequences
- Can validate against `reference/amx/test.c`

---

### Phase 5: Indexed load / shuffle / writemask (advanced operands)

**Goal:** Add complex operand forms incrementally.

- `genlut` (indexed load)
- Parts of `matfp` / `matint` / `vecfp` / `vecint`
- Shuffle S0..S3 handling
- Writemask (7-bit / 9-bit) modeling

**Done criteria**
- 1-2 complex modes work end-to-end
- Document behavior in README and add reproducible tests

---

### Phase 6: Optimization Experiments (optional)

**Goal:** Explore when LLVM should choose AMX (learning only).

- Cost model (TTI) treats AMX as high-performance instructions
- Observe how loop vectorizer / SLP behaves
- Not for production use, only to understand the challenges

**Done criteria**
- Can explain IR/selection differences at `-O2`
- Summarize manual intrinsic usage vs auto-generation

## Testing / Debug Tips

- View LLVM IR from clang:
  ```bash
  reference/llvm-project/build/bin/clang -S -emit-llvm src/main.c -o main.ll
  ```
- View asm from llc:
  ```bash
  reference/llvm-project/build/bin/llc -march=aarch64 main.ll -o main.s
  ```
- llc debug output:
  ```bash
  reference/llvm-project/build/bin/llc -debug -march=aarch64 main.ll -o /dev/null
  reference/llvm-project/build/bin/llc -debug-only=isel -march=aarch64 main.ll -o /dev/null
  ```
- lit tests:
  ```bash
  ninja check-clang
  ninja check-llvm
  ```

## Notes / Risks

- AMX is undocumented; behavior may vary across OS and chip generations
- Context switching and AMX state handling may matter (including `set`/`clr` semantics)
- Not intended for upstream LLVM

## References

- AMX reverse engineering: `reference/amx`
- LLVM monorepo: `reference/llvm-project`
- See also `reference/amx/References.md`
