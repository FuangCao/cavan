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

class CavanCommandBase:
	def __init__(self, pathname = "."):
		reload(sys)
		sys.setdefaultencoding("utf-8")
		self.setRootPath(pathname)

	def setRootPath(self, pathname):
		if not os.path.isdir(pathname):
			os.makedirs(pathname)
		self.mPathRoot = os.path.abspath(pathname)

	def getAbsPath(self, pathname):
		return os.path.join(self.mPathRoot, pathname)

	def getRelPath(self, pathname):
		return os.path.relpath(pathname, self.mPathRoot)

	def doExecute(self, command, output = None, verbose = True):
		if verbose:
			print command

		if output != None:
			res = os.system("%s > %s" % (command, single_arg(output)))
		else:
			res = os.system(command)

		if res == 2:
			sys.exit(-1)

		return res == 0

	def doPathExecute(self, command, output = None, pathname = None, verbose = True):
		if verbose:
			print command

		if not pathname:
			pathname = self.mPathRoot

		return self.doExecute("cd %s && { %s; }" % (single_arg(pathname), command), output, False)

	def popenToList(self, command, verbose = True):
		if verbose:
			print command

		fp = os.popen(command)
		if not fp:
			return None

		lines = fp.readlines()

		try:
			fp.close()
		except:
			return None

		return lines

	def doPathPopen(self, command, pathname = None, verbose = True):
		if verbose:
			print command

		if not pathname:
			pathname = self.mPathRoot

		return self.popenToList("cd %s && { %s; }" % (single_arg(pathname), command), False)

if __name__ == "__main__":
	if len(sys.argv) > 1:
		print popen_tostring(sys.argv[1])
