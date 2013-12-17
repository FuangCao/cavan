#!/usr/bin/python

import sys, os, errno

from cavan_file import file_read_line, file_read_lines, \
		file_write_line, file_write_lines, file_append_line, file_append_lines

from cavan_command import CavanCommandBase

class CavanGitManager(CavanCommandBase):
	def __init__(self, pathname = ".", verbose = True, name = ".cavan-git", bare = False):
		if not name:
			name = ".git"

		self.mGitRepoName = name;
		self.mBare = bare;
		CavanCommandBase.__init__(self, pathname, verbose)

	def setRootPath(self, pathname):
		CavanCommandBase.setRootPath(self, pathname)

		if self.mBare:
			self.mPathGitRepo = self.getAbsPath(".")
		else:
			self.mPathGitRepo = self.getAbsPath(self.mGitRepoName)
			self.mPathPatch = os.path.join(self.mPathGitRepo, "cavan-patch")
			if not os.path.isdir(self.mPathPatch):
				self.mkdirSafe(self.mPathPatch)

	def addGitCmdHeader(self, args):
		args.insert(0, "git")

		if not self.mBare and self.mGitRepoName != ".git":
			args.insert(1, "--git-dir")
			args.insert(2, self.mPathGitRepo)
			args.insert(3, "--work-tree")
			args.insert(4, self.mPathRoot)

	def doExecGitCmd(self, args, of = None, ef = None, verbose = True):
		self.addGitCmdHeader(args)
		return self.doExecute(args, of, ef, None, verbose)

	def doPopenGitCmd(self, args, ef = None, verbose = True):
		self.addGitCmdHeader(args)
		return self.doPopen(args, None, ef, verbose)

	def findRootPath(self):
		pathname = self.mPathRoot
		while not os.path.isdir(os.path.join(pathname, ".cavan-git")):
			if not pathname or pathname == '/':
				return None

			pathname = os.path.dirname(pathname)

		return pathname

	def setRemoteUrl(self, url = None):
		if not url:
			lines = self.doPopenGitCmd(["config", "remote.%s.url" % self.mRemoteName])
			if not lines:
				return False
			url = lines[0].rstrip("\n")
		elif not self.doExecGitCmd(["config", "remote.%s.url" % self.mRemoteName, url]):
			return False

		self.mUrl = url

		return True

	def gitCheckoutVersion(self, commit = None, option = None):
		listCommand = ["checkout", "--quiet"]
		if commit != None:
			listCommand.append(commit)

		if option != None:
			listCommand.extend(option)

		return self.doExecGitCmd(listCommand, ef = "/dev/null")

	def getGitHeadLog(self, tformat = None, commit = None, count = 1):
		command = ["log", "-%d" % count]
		if tformat != None:
			command.append("--pretty=tformat:%s" % tformat)

		if commit != None:
			command.append(commit)

		lines = self.doPopenGitCmd(command, ef = "/dev/null")
		if not lines:
			return None

		return lines

	def getGitHeadHash(self, commit = None, index = 1):
		lines = self.getGitHeadLog("%H", commit, index)
		if lines == None or len(lines) < index:
			return None

		return lines.pop().rstrip("\n")

	def getGitHeadMessage(self, commit = None):
		lines = self.getGitHeadLog("%B", commit)
		if not lines or len(lines) < 1:
			return None

		lines.pop()

		return lines

	def getGitHeadRevisionMap(self, commit = None):
		listMessage = self.getGitHeadMessage(commit)
		if not listMessage:
			return None

		lineLast = listMessage.pop()
		match = self.mPatternGitRevision.match(lineLast)
		if not match:
			revision = -1
			listMessage.append(lineLast)
		else:
			revision = int(match.group(1))

		return (revision, listMessage)

	def gitAddFileList(self, listFile):
		if len(listFile) == 0:
			return True

		listFile.insert(0, "add")
		listFile.insert(1, "-f")

		return self.doExecGitCmd(listFile, verbose = True)

	def isInitialized(self):
		if not os.path.isdir(self.mPathRoot):
			return False
		return self.doExecGitCmd(["log", "-1"], of = "/dev/null", ef = "/dev/null")

	def doGitReset(self, branch):
		if not self.gitCheckoutVersion(branch):
			return False

		lines = self.doPopenGitCmd(["diff"])
		if not lines:
			return True

		tmNow = time.localtime()
		filename = "%04d-%02d%02d-%02d%02d%02d.diff" % (tmNow.tm_year, tmNow.tm_mon, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec)
		file_write_lines(os.path.join(self.mPathPatch, filename), lines)

		return self.doExecGitCmd(["reset", "--hard"], of = "/dev/null")

	def genGitRepo(self, option = None):
		if not os.path.isdir(self.mPathRoot):
			os.makedirs(self.mPathRoot)
		elif self.isInitialized():
			return True

		self.mPathGitRepo = self.getAbsPath(self.mGitRepoName)

		listCommand = ["init"]
		if not self.mVerbose:
			listCommand.append("--quiet")

		if self.mBare:
			listCommand.append("--shared")
			listCommand.append("--bare")

		if option != None:
			listCommand.extend(option)

		if not self.doExecGitCmd(listCommand):
			return False

		if not self.mBare:
			if not self.doExecGitCmd(["config", "user.name", "Fuang.Cao"]):
				return False

			if not self.doExecGitCmd(["config", "user.email", "cavan.cfa@gmail.com"]):
				return False

			pathIgnore = os.path.join(self.mPathGitRepo, ".gitignore")
			if not file_write_line(pathIgnore, "*"):
				return False

		return True

	def gitAutoCommit(self):
		if not self.doExecGitCmd(["add", "-f", "."]):
			return False

		return self.doExecGitCmd(["commit", "-asm", "Auto commit by Fuang.Cao"])

	def doGitStatus(self):
		return self.doPopenGitCmd(["status", "-s", "-uno"])

	def doGitCommit(self, message, author = None, date = None):
		listCommand = ["commit", "--all", "--message", message]
		if not self.mVerbose:
			listCommand.append("--quiet")

		if date != None:
			listCommand.append("--date")
			listCommand.append(date)

		if author != None:
			listCommand.append("--author")
			listCommand.append(author)

		if not self.doExecGitCmd(listCommand, of = "/dev/null"):
			return False

		if self.mGitRepoName != ".git":
			destPath = self.getAbsPath(".git")
			if not os.path.exists(destPath):
				os.symlink(self.mGitRepoName, destPath)

		return True

	def doBackup(self, destRoot):
		if not os.path.exists(destRoot):
			os.makedirs(destRoot)
		elif not os.path.isdir(destRoot):
			return False

		self.doExecGitCmd(["config", "--unset", "core.worktree"])

		for filename in os.listdir(self.mPathGitRepo):
			srcPath = os.path.join(self.mPathGitRepo, filename)
			destPath = os.path.join(destRoot, filename)
			if os.path.islink(srcPath):
				if not os.path.exists(destPath):
					self.prRedInfo(destPath, " is not exists")
					return False
				os.remove(srcPath)
				os.symlink(destPath, srcPath)
			elif os.path.isdir(srcPath):
				if os.path.exists(destPath):
					if not self.removeSafe(destPath):
						return False
				os.rename(srcPath, destPath)
				os.symlink(destPath, srcPath)
			else:
				self.doCopyFile(srcPath, destPath)

		if not self.doExecute(["git", "config", "--file", os.path.join(destRoot, "config"), "core.bare", "true"], verbose = False):
			self.prRedInfo("set config core.bare to true failed")
			return False

		return True

	def removeSelf(self):
		self.removeSafe(self.mPathRoot)

	def doPush(self, option = None):
		listCommand = ["push", "--all"]
		if option != None:
			listCommand.extend(option)

		return self.doExecGitCmd(listCommand)
