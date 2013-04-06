#!/usr/bin/python

import sys, os

def command_vision(command):
	print command
	return os.system(command) == 0

def popen_to_list(command):
	print command
	fp = os.popen(command)
	if not fp:
		return None

	result = fp.readlines()
	fp.close()

	return result

def popen_tostring(command, sep = ""):
	result = popen_to_list(command)
	if result == None:
		return None
	result = [line.rstrip("\r\n") for line in result]
	return sep.join(result)

def single_arg(argument):
	return "'" + argument.replace("'", "'\\''") + "'"

def single_arg2(argument):
	return "\"" + argument.replace("\\", "\\\\").replace("\"", "\\\"").replace("'", "'\\''") + "\""

if __name__ == "__main__":
	if len(sys.argv) > 1:
		print popen_tostring(sys.argv[1])
