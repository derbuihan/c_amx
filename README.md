# AMX Study

Apple Silicon の **AMX (Apple Matrix Extensions)** 命令を題材に、**clang/llvm にバックエンド（主に AArch64 拡張としての命令・intrinsic・コード生成）を追加する流れ**を学習するための個人用リポジトリです。

- 目的: 自己研鑽（LLVM の TableGen / ISel / CodeGen の理解）
- 前提: AMX は Apple 非公開命令であり、公式サポートはありません（upstream 目的ではない）
- 参照: `reference/amx` (corsix/amx), `reference/llvm-project` (LLVM monorepo)

## Repository Layout

```
.
├── CMakeLists.txt
├── README.md
├── main.c
└── reference
    ├── amx          # https://github.com/corsix/amx
    └── llvm-project # https://github.com/llvm/llvm-project
```

## Environment

- macOS on Apple Silicon (M1/M2/M3/M4)
- clang/llvm: `reference/llvm-project` をローカルでビルドして使う
- build system: CMake + Ninja 推奨

## Build LLVM (local)

例（Debug + AArch64 + clang のみ）:

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

以降、`clang` / `llc` は `reference/llvm-project/build/bin/` 配下を使う想定。

## What is AMX (quick notes)

AMX は CPU から命令を発行し、専用実行ユニット側で計算する形式のアクセラレータです。概念的には以下を押さえます（詳細は `reference/amx/*.md` 参照）:

- レジスタファイル: X(8) + Y(8) + Z(64)（合計約 5KB）
- 64-byte ベクトルを基本単位としてロード/ストア/演算
- 行列（outer product）とベクトル（pointwise）両モードを持つ
- 書き込みマスク、shuffle、indexed load 等の「特殊なオペランド形」が多い

## Learning Plan (Implementation Steps)

この README は「どの順序で LLVM に AMX 対応を足していくか」を管理するための手順書です。
実装は **最小から始め、動作するテストを積み上げる**方針にします。

---

### Phase 0: Read & Verify (基礎理解と動作確認)

**Goal:** AMX 命令の仕様と、LLVM AArch64 backend の構造を把握する。

- AMX仕様を読む
    - `reference/amx/README.md`
    - `reference/amx/RegisterFile.md`
    - `reference/amx/Instructions.md`
- LLVM AArch64 backend の入口を読む（眺める）
    - `llvm/lib/Target/AArch64/`
    - `AArch64InstrInfo.td`, `AArch64RegisterInfo.td`, `AArch64ISelDAGToDAG.cpp`
- AMX 参照実装をビルドして挙動を見る（可能なら）
    - `reference/amx/test.c`, `perf.c`

**Done criteria**
- AMX の “X/Y/Z と outer product” が説明できる
- LLVM の TableGen で命令定義→ISel→asm 出力の流れが説明できる

---

### Phase 1: Minimal Intrinsics (`set` / `clr`)

**Goal:** clang builtin → LLVM intrinsic → AArch64 MC/asm 出力 のパイプラインを通す。

実装イメージ（実際のファイルは LLVM 側の構造に合わせて調整）:

1. clang builtins を追加
    - `clang/include/clang/Basic/BuiltinsAArch64.def`
2. LLVM intrinsics を追加
    - `llvm/include/llvm/IR/IntrinsicsAArch64.td`
3. AArch64 命令定義（TableGen）を追加
    - `llvm/lib/Target/AArch64/*.td`
4. CodeGen テスト（clang, llc）を追加
    - `clang/test/CodeGen/...`
    - `llvm/test/CodeGen/AArch64/...`

**Done criteria**
- `__builtin_...amx_set()` が `amx_set`（または期待するエンコード）を出す
- `lit` テストが通る

---

### Phase 2: Load/Store (`ldx/ldy/stx/sty` から)

**Goal:** AMX の「64-byte span」データ移動を LLVM に載せる。  
ここで **メモリオペランド** と **命令の複雑な即値エンコード**に慣れる。

- 対象候補: `ldx`, `ldy`, `stx`, `sty`
- まずは simplest mode のみ（追加モードは後回し）
- 生成アセンブリと、実機での簡単な sanity check を用意

**Done criteria**
- “メモリ→AMX(X/Y)” と “AMX(X/Y)→メモリ” が一往復できる
- `-O0` / `-O2` でもクラッシュせずに実行できる

---

### Phase 3: Extract/Move (`extrx/extry` と `extrh/extrv` の一部)

**Goal:** Z → X/Y の移し替えや、内部データ移動を扱えるようにする。

- `extrx` / `extry` は比較的単純（X↔Y）
- `extrh` / `extrv` は write mask や shift/sat が絡むため段階的に

**Done criteria**
- Z の内容を X/Y 経由でメモリに取り出せる
- 最低限のテスト（固定入力→固定出力）が作れる

---

### Phase 4: Compute Core (FMA outer product / vector)

**Goal:** AMX の本質（outer product）を CodeGen できるようにする。

最初のターゲット例:
- `fma16` または `fma32` の最小モード
- 可能なら “vector mode (63=1)” より “matrix mode (63=0)” を優先（AMXらしさ）

ポイント:
- “命令1発で Z の複数行/列を更新する” セマンティクスをどう表現するか
    - LLVM IR intrinsic に逃がす（まずはこれが現実的）
    - Pattern matching は後回し

**Done criteria**
- 小さな GEMM タイル（例: 16x16 相当の一部）を AMX 命令列で計算できる
- `reference/amx/test.c` 相当の検算ができる

---

### Phase 5: Indexed load / shuffle / writemask (高度オペランド)

**Goal:** AMX の強力だが厄介なオペランド形を段階的にサポートする。

- `genlut`（indexed load 系）
- `matfp` / `matint` / `vecfp` / `vecint` の一部モード
- shuffle S0..S3 の取り扱い
- writemask（7-bit / 9-bit）のモデリング

**Done criteria**
- 1〜2個の “複雑モード” を end-to-end で動かせる
- 仕様を README にメモし、再現性あるテストがある

---

### Phase 6: Optimization Experiments (optional)

**Goal:** “LLVM が AMX を使うべき” と判断できるようにする（学習用）。

- cost model（TTI）で AMX を高性能命令として扱う
- 既存のループベクトライザ/SLP をどこまで誘導できるか観察
- 実運用狙いではなく「どこが難しいか」を理解する

**Done criteria**
- `-O2` で IR 変換や命令選択がどう変わるか説明できる
- “intrinsic 手書き vs 自動生成” の差分を整理できる

---

## Testing / Debug Tips

- clang が出す LLVM IR を見る:
  ```bash
  reference/llvm-project/build/bin/clang -S -emit-llvm main.c -o main.ll
  ```
- llc で asm を見る:
  ```bash
  reference/llvm-project/build/bin/llc -march=aarch64 main.ll -o main.s
  ```
- llc のデバッグ出力:
  ```bash
  reference/llvm-project/build/bin/llc -debug -march=aarch64 main.ll -o /dev/null
  reference/llvm-project/build/bin/llc -debug-only=isel -march=aarch64 main.ll -o /dev/null
  ```
- lit テスト:
  ```bash
  ninja check-clang
  ninja check-llvm
  ```

## Notes / Risks

- AMX は非公開命令のため、OS/チップ世代によって挙動が変わり得ます
- コンテキストスイッチ等で AMX 状態の扱いが絡む可能性があります（`set`/`clr` の意味も含めて注意）
- upstream 目標ではなく、学習に主眼を置きます

## References

- AMX reverse engineering: `reference/amx`
- LLVM monorepo: `reference/llvm-project`
- `reference/amx/References.md` も参照
