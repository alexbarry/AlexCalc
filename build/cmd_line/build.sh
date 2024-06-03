#!/usr/bin/env bash

set -e
set -u
set -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "${DIR}"

mkdir -p out
cd out
cmake ../ && cmake --build . $@
