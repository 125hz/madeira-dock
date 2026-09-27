#!/usr/bin/env bash
# Cross-compile only. Running Steam on a host requires the owner's authorization.
set -euo pipefail
cd "$(dirname "$0")/.."
repo="$PWD"
madeira="${MADEIRA_ROOT:-$repo/../Madeira}"
toolchain="${LLVM_MINGW_BIN:-$madeira/.xtool/toolchains/llvm-mingw/bin}"
output="${DOCK_OUTPUT:-$repo/.build/windows}"
mkdir -p "$output"
for arch in x86_64 i686; do
    compiler="$toolchain/$arch-w64-mingw32-clang"
    [[ -x "$compiler" ]] || { echo "Missing cross compiler: $compiler" >&2; exit 1; }
    "$compiler" -std=c11 -O2 -Wall -Wextra -Werror -Wno-cast-function-type \
        -static -Wl,--strip-all -o "$output/dockhost-$arch.exe" \
        src/main.c src/probe.c src/session.c src/launch.c src/validation.c src/auth.c src/client_layout.c src/scm.c -ladvapi32
done
echo "Built x86-64 and i386 probes in $output"
# Staging is explicit: experimental probes are not a new default launch route.
if [[ "${1:-}" == --stage ]]; then
    cp "$output/dockhost-x86_64.exe" "$madeira/app/Madeira/arm64ec-windows/dockhost.exe"
    cat LICENSE notices/MinGW-w64-runtime.txt notices/LLVM.txt > "$madeira/app/Madeira/arm64ec-windows/dock-notices.txt"
    echo "Staged stripped x64 Dock executable; source stays in the private repository"
fi
