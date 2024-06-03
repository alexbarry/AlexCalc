#!/usr/bin/env python3
import sys
import os
import os.path

CALC_BIN = 'build/cmd_line/out/calc'
CALC_TEST_PY = 'tests/calc_core/calc_core_test.py'

rc = os.system('build/cmd_line/build.sh -- -j4')
if rc != 0:
    sys.exit(rc)

cmd = ' '.join(['python3', CALC_TEST_PY, CALC_BIN])
rc = os.system(cmd)
sys.exit(rc)
