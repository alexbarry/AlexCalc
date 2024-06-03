#!/usr/bin/env python3
from __future__ import print_function

import subprocess
import collections

CalcOutput = collections.namedtuple('CalcOutput', field_names=['stdout', 'stderr'])


class CalcTester(object):

	INIT_CMDS_DEFAULT = [
		':print_eof off',
	]

	def __init__(self, calc_bin_file, init_cmds=None, debug=False):
		self.calc_bin_file = calc_bin_file

		if init_cmds is None:
			self.init_cmds = self.INIT_CMDS_DEFAULT
		else:
			self.init_cmds = init_cmds

		self.debug = debug

		for init_cmd in self.init_cmds:
			output = self.run_cmd(init_cmd)
			if output.stdout != '' or output.stderr != '':
				raise Exception('expected no output from init cmd %r, recvd %r' % (init_cmd, output))

	def run_cmd(self, cmd):
		if self.debug:
			print('Debug: running cmd %r' % cmd)

		p = subprocess.Popen(self.calc_bin_file, stdin =subprocess.PIPE,
		                                         stdout=subprocess.PIPE,
		                                         stderr=subprocess.PIPE)
		cmd_bytes = bytes(cmd + '\n', encoding='utf-8')
		output, output_err   = p.communicate(input=cmd_bytes, timeout=1)
		output_err = output_err.decode('utf-8')
		output = output.decode( 'utf-8' )

		if len(output) > 0:
			if output[-1] != '\n':
				print('WARNING: expected stdout to end with newline after calc cmd %r' % cmd)
			else:
				output = output[:-1]

		return CalcOutput(stdout=output, stderr=output_err)

