#!/usr/bin/python

import sys, os, subprocess

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

class CavanCommandBase:
	def __init__(self, pathname = ".", shell = "sh"):
		reload(sys)
		sys.setdefaultencoding("utf-8")

		self.setRootPath(pathname)
		self.setShellName(shell)

	def setRootPath(self, pathname):
		if not os.path.isdir(pathname):
			os.makedirs(pathname)
		self.mPathRoot = os.path.abspath(pathname)

	def setShellName(self, name):
		self.mShellName = name

	def getAbsPath(self, pathname):
		return os.path.join(self.mPathRoot, pathname)

	def getRelPath(self, pathname):
		return os.path.relpath(pathname, self.mPathRoot)

	def doExecute(self, args, of = None, ef = None, cwd = None, verbose = True):
		if verbose:
			print args

		if not of:
			fpStdout = None
		else:
			fpStdout = open(of, "w")

		if not ef:
			fpStderr = None
		else:
			fpStderr = open(ef, "w")

		if not cwd:
			cwd = self.mPathRoot

		process = subprocess.Popen(args, stdout = fpStdout, stderr = fpStderr, cwd = cwd)
		if not process:
			res = -1
		else:
			res = process.wait()

		if fpStderr != None:
			fpStderr.close()

		if fpStdout != None:
			fpStdout.close()

		return res == 0

	def buildSystemArgs(self, command):
		return [self.mShellName, "-c", "--", command]

	def doSystemExec(self, command, of = None, ef = None, cwd = None, verbose = True):
		if verbose:
			print command

		return self.doExecute(self.buildSystemArgs(command), of, ef, cwd, False)

	def doPopen(self, args, cwd = None, verbose = True):
		if verbose:
			print args

		if not cwd:
			cwd = self.mPathRoot

		process = subprocess.Popen(args, cwd = cwd, stdout = subprocess.PIPE)
		if not process:
			return None

		lines = process.stdout.readlines()
		if process.wait() != 0:
			return None

		return lines

	def doSystemPopen(self, command, cwd = None, verbose = True):
		if verbose:
			print command

		return self.doPopen(self.buildSystemArgs(command), cwd, False)

if __name__ == "__main__":
	if len(sys.argv) > 1:
		print popen_tostring(sys.argv[1])
