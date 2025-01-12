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

# TODO only copy specific file types
cp -r "${SRC_DIR}/html"/* ./

ts_files="${SRC_DIR}/html/js"/*.ts
echo "Transpiling $ts_files..."
tsc $ts_files \
	--module ES6 \
	--target ES6  \
	--outDir ./js/

mv calc_wasm.js   js/
mv calc_wasm.wasm js/
mkdir -p graphics
cp "${SRC_DIR}/graphics/favicon.png" graphics/favicon.png
cp "${SRC_DIR}/graphics/GetItOnGooglePlay_Badge_Web_color_English.png" graphics/
cp "${SRC_DIR}/../docs/android_privacy_policy.txt" ./
