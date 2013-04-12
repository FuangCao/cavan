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
	def __init__(self, pathname = ".", verbose = True, shell = "sh", stdout = None, stderr = None):
		reload(sys)
		sys.setdefaultencoding("utf-8")

		self.setRootPath(pathname)
		self.setShellName(shell)

		self.setStdoutFp(stdout)
		self.setStderrFp(stderr)
		self.setVerbose(verbose)

	def setVerbose(self, verbose):
		self.mVerbose = verbose

	def setRootPath(self, pathname):
		if not os.path.isdir(pathname):
			os.makedirs(pathname)
		self.mPathRoot = os.path.abspath(pathname)

	def setShellName(self, name):
		self.mShellName = name

	def setStdoutFp(self, stdout = None):
		if not stdout:
			stdout = sys.stdout

		self.mFpStdout = stdout

	def setStdoutFile(self, pathname, mode = "w"):
		return self.setStdoutFp(open(pathname, mode))

	def setStderrFp(self, stderr = None):
		if not stderr:
			stderr = sys.stderr

		self.mFpStderr = stderr

	def setStderrFile(self, pathname, mode = "w"):
		return self.setStderrFp(open(pathname, mode))

	def prStdInfo(self, *messge):
		stdout = self.mFpStdout

		for node in messge:
			stdout.write(node)

		stdout.write("\n")

	def prStdErr(self, *messge):
		stdout = self.mFpStderr

		for node in messge:
			stdout.write(node)

		stdout.write("\n")

	def prColorInfo(self, color, messge, stdout = None):
		if not stdout:
			stdout = self.mFpStdout

		stdout.write("\033[%sm" % color)

		for node in messge:
			stdout.write(node)

		stdout.write("\n\033[0m")

	def prBoldInfo(self, *messge):
		self.prColorInfo("1", messge)

	def prRedInfo(self, *messge):
		self.prColorInfo("31", messge)

	def prGreenInfo(self, *messge):
		self.prColorInfo(32, messge)

	def prBrownInfo(self, *messge):
		self.prColorInfo(33, messge)

	def prBlueInfo(self, *messge):
		self.prColorInfo(34, messge)

	def getAbsPath(self, pathname):
		return os.path.join(self.mPathRoot, pathname)

	def getRelPath(self, pathname):
		return os.path.relpath(pathname, self.mPathRoot)

	def doExecute(self, args, of = None, ef = None, cwd = None, verbose = True):
		if self.mVerbose and verbose:
			print >> self.mFpStdout, args

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
		if self.mVerbose and verbose:
			self.prStdInfo(command)

		return self.doExecute(self.buildSystemArgs(command), of, ef, cwd, False)

	def doPopen(self, args, cwd = None, ef = None, verbose = True):
		if self.mVerbose and verbose:
			print >> self.mFpStdout, args

		if not cwd:
			cwd = self.mPathRoot

		if not ef:
			fpStderr = None
		else:
			fpStderr = open(ef, "w")

		process = subprocess.Popen(args, cwd = cwd, stdout = subprocess.PIPE, stderr = fpStderr)
		if not process:
			return None

		lines = process.stdout.readlines()
		if process.wait() != 0:
			return None

		return lines

	def doSystemPopen(self, command, cwd = None, ef = None, verbose = True):
		if self.mVerbose and verbose:
			self.prStdInfo(command)

		return self.doPopen(self.buildSystemArgs(command), cwd, ef, False)

	def genGitRepo(self, pathname = None, option = None):
		if not pathname:
			pathname = self.mPathRoot
		elif not os.path.isdir(pathname):
			os.makedirs(pathname)

		if self.doExecute(["git", "branch"], of = "/dev/null", ef = "/dev/null", cwd = pathname):
			return True

		listCommand = ["git", "init"]
		if option != None:
			for node in option:
				listCommand.append(node)

		if not self.doExecute(listCommand, cwd = pathname):
			return False

		if not self.doExecute(["git", "config", "user.name", "Fuang.Cao"], cwd = pathname):
			return False

		return self.doExecute(["git", "config", "user.email", "cavan.cfa@gmail.com"], cwd = pathname)

	def gitAutoCommit(self, pathname = None):
		if not self.doExecute(["git", "add", "-f", "."], cwd = pathname):
			return False

		return self.doExecute(["git", "commit", "-asm", "Auto commit by Fuang.Cao"], cwd = pathname)

if __name__ == "__main__":
	if len(sys.argv) > 1:
		print popen_tostring(sys.argv[1])
