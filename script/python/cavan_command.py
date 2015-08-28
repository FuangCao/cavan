#!/usr/bin/env python

import sys, os, subprocess, errno

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
	def __init__(self, pathname = None, verbose = True, shell = "sh", stdout = None, stderr = None):
		reload(sys)
		sys.setdefaultencoding("utf-8")

		self.setShellName(shell)

		self.setStdoutFp(stdout)
		self.setStderrFp(stderr)
		self.setVerbose(verbose)

		if not pathname:
			pathname = self.findRootPath()
			if not pathname:
				pathname = "."
		self.setRootPath(pathname)

	def setVerbose(self, verbose):
		self.mVerbose = verbose

	def setRootPath(self, pathname, auto_create = False):
		self.mPathRoot = os.path.abspath(pathname)
		if self.mVerbose:
			self.prStdInfo("set root path to ", self.mPathRoot)

		if auto_create and not os.path.isdir(self.mPathRoot):
			self.mkdirSafe(self.mPathRoot)

	def getRootPath(self):
		return self.mPathRoot

	def findRootPath(self):
		return None

	def findRootPathByFilename(self, filename):
		pathname = os.path.abspath(".")
		while not os.path.exists(os.path.join(pathname, filename)):
			if not pathname or pathname == '/':
				return None

			pathname = os.path.dirname(pathname)

		return pathname

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

		stdout.write("\033[0m\n")

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
		if os.path.isabs(pathname):
			return pathname
		return os.path.join(self.mPathRoot, pathname)

	def getRelRoot(self, pathname):
		if os.path.isabs(pathname):
			pathname = self.getRelPath(pathname)

		bHasSep = True
		relPath = ""

		for c in pathname:
			if c == '/':
				if not bHasSep:
					bHasSep = True
					relPath = relPath + '/'
			else:
				if bHasSep:
					relPath = relPath + '..'
					bHasSep = False

		return relPath.rstrip('/')

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

	def mkdirSafe(self, pathname):
		pathname = self.getAbsPath(pathname)
		if os.path.exists(pathname):
			if os.path.isdir(pathname):
				return True
			else:
				return False

		dirname = os.path.dirname(pathname)
		if len(dirname) > 0 and not self.mkdirSafe(dirname):
			return False

		if self.mVerbose:
			self.prStdInfo("create directory ", pathname)

		try:
			os.mkdir(pathname)
			return True
		except:
			if os.path.isdir(pathname):
				return True

		tmppath = os.path.join(dirname, ".__CAVAN__")
		if not os.path.isdir(tmppath):
			os.mkdir(tmppath)

		self.prBrownInfo("Fixup ", pathname)

		try:
			os.rename(tmppath, pathname)
		except:
			os.rmdir(tmppath)

			if os.path.isdir(pathname):
				return True
			else:
				return False

		return True

	def removeSafe(self, pathname):
		pathname = self.getAbsPath(pathname)
		if os.path.exists(pathname) or os.path.islink(pathname) or self.doExecute(["touch", pathname], ef = "/dev/null") or self.mkdirSafe(pathname):
			return self.doExecute(["rm", "-rf", pathname], verbose = False)

		return False

	def doCopyFile(self, srcPath, destPath):
		fpSrc = open(srcPath, "r")
		if not fpSrc:
			return False
		fpDest = open(destPath, "w")
		if not fpDest:
			fpSrc.close()
			return False

		lines = fpSrc.readlines()
		fpDest.writelines(lines)

		fpDest.close()
		fpSrc.close()

		return True

	def listHasPath(self, listPath, path):
		for item in listPath:
			if path.startswith(item):
				return True
		return False

	def getChoice(self, message = "Please input you choice", defValue = False):
		while True:
			choice = raw_input("%s? [%s] " % (message, defValue and "Y/n" or "y/N")).lower()
			if not choice:
				return defValue
			if choice in ["yes", "y"]:
				return True
			elif choice in ["no", "n"]:
				return False

	def getEnv(self, key, default = None):
		return os.getenv(key, default)

	def doRaise(self, message):
		raise Exception(message)

	def doSymlink(self, target, pathname = None):
		if not pathname:
			pathname = os.path.basename(target)
		elif os.path.isdir(pathname):
			pathname = os.path.join(pathname, os.path.basename(target))

		if os.path.exists(pathname):
			os.unlink(pathname)

		try:
			os.symlink(target, pathname)
		except:
			return False

		return True

	def doFindFile(self, dirname, filename):
		result = []

		for fn in os.listdir(dirname):
			pathname = os.path.join(dirname, fn)
			if os.path.isdir(pathname):
				result.extend(self.doFindFile(pathname, filename))
			elif fn == filename:
				result.append(pathname)

		return result

if __name__ == "__main__":
	if len(sys.argv) > 1:
		print popen_tostring(sys.argv[1])
