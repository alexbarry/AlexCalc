#!/usr/bin/env bash
set -e
set -u
set -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "${DIR}"

. ./setup_env.sh

mkdir -p out
cd out
emcmake cmake ../
cmake --build . $@

SRC_DIR="../../../src"
rm -f bin/index.html
cp -r "${SRC_DIR}/html"/* ./
mv calc_wasm.js   js/
mv calc_wasm.wasm js/
mkdir -p graphics
cp "${SRC_DIR}/graphics/favicon.png" graphics/favicon.png
