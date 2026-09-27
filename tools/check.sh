#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
compiler="${HOST_CC:-/home/hero/.local/share/swiftly/bin/clang}"
output=.build/tests
mkdir -p "$output"
for test in probe validation auth client_layout; do
    "$compiler" -std=c11 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined \
        -Isrc "src/$test.c" "tests/test-$test.c" -o "$output/test-$test"
    "$output/test-$test"
done
