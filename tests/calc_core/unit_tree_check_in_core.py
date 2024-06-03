#!/usr/bin/env python3
from __future__ import print_function

import sys

import json
import collections

import core_test_common

TestArgs = collections.namedtuple('TestArgs', field_names = ['calc'])

UNITS_JSON_FILEPATH = 'tests/calc_core/units.json'

def unit_name_json_to_calc_unit(unit_name_json):
    output = ''
    first = True
    for unit_part_json in unit_name_json:
        if not first:
            output += ' '
        first = False
        output  += unit_part_json["name"]
        unit_pow = unit_part_json["pow"]
        if unit_pow != 1:
            output += '^%d' % unit_pow
        
    return output

def check_unit_group(group, args):
    for child_group in group["child_groups"]:
        check_unit_group(child_group, args)

    for unit in group["child_units"]:
        check_unit(unit, args)

def check_unit(unit, args):
    for unit_name_json in unit["unit_names"]:
        unit = unit_name_json_to_calc_unit(unit_name_json)
        #print("Checking unit_name %r" % unit)
        calc = args.calc
        output = calc.run_cmd('1 %s' % unit)

        if any(map(lambda x: 'not defined' in x, [output.stdout, output.stderr])):
            print(output)


if __name__ == '__main__':

    with open(UNITS_JSON_FILEPATH) as units_json_file:
        units = json.load(units_json_file)

    calc_bin_file = sys.argv[1]
    calc = core_test_common.CalcTester(calc_bin_file)

    args = TestArgs(calc=calc)
    check_unit_group(units, args)
