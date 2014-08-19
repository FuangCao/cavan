#!/usr/bin/python

import sys, os, time, errno

from cavan_file import file_read_line, file_read_lines, \
		file_write_line, file_write_lines, file_append_line, file_append_lines

from cavan_command import CavanCommandBase

MAX_FILELIST_SIZE = 200

class CavanGitManager(CavanCommandBase):
	def __init__(self, pathname = ".", verbose = True, name = None, bare = False):
		if not name:
			name = ".git"

		self.mGitRepoName = name;
		self.mBare = bare;
		CavanCommandBase.__init__(self, pathname, verbose)

	def setRootPath(self, pathname, auto_create = False):
		CavanCommandBase.setRootPath(self, pathname, auto_create)

		if self.mBare:
			self.mPathGitRepo = self.mPathRoot
		else:
			self.mPathGitRepo = self.getAbsPath(self.mGitRepoName)
			self.mPathPatch = os.path.join(self.mPathGitRepo, "cavan-patch")
			if auto_create and not os.path.isdir(self.mPathPatch):
				self.mkdirSafe(self.mPathPatch)

		self.mFileExclude = os.path.join(self.mPathGitRepo, "info/exclude")

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
		return self.findRootPathByFilename(self.mGitRepoName)

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

	def gitCheckout(self, commit = None, option = None):
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
		length = len(listFile)
		if length == 0:
			return True

		if length > MAX_FILELIST_SIZE:
			if self.mVerbose:
				self.prBrownInfo("file count = %d" % length, " is too much")
			if not self.gitAddFileList(listFile[MAX_FILELIST_SIZE:]):
				return False
			listFile = listFile[0:MAX_FILELIST_SIZE]

		listFile.insert(0, "add")
		listFile.insert(1, "-f")

		return self.doExecGitCmd(listFile, verbose = False)

	def isInitialized(self):
		if not os.path.isdir(self.mPathGitRepo):
			return False
		return self.doExecGitCmd(["log", "-1"], of = "/dev/null", ef = "/dev/null")

	def doGitReset(self, revision = None):
		command = ["diff"]
		if revision != None:
			command.append(revision)

		lines = self.doPopenGitCmd(command)
		if lines != None and len(lines) > 0:
			tmNow = time.localtime()
			filename = "%04d-%02d%02d-%02d%02d%02d.diff" % (tmNow.tm_year, tmNow.tm_mon, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec)
			if not os.path.exists(self.mPathPatch):
				self.mkdirSafe(self.mPathPatch);
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

			if not file_write_line(self.mFileExclude, "\n".join([self.mGitRepoName, ".svn"])):
				return False

		return True

	def gitAutoCommit(self):
		if not self.doExecGitCmd(["add", "-f", "."]):
			return False

		if self.doExecGitCmd(["commit", "-asm", "Auto commit by Fuang.Cao"]):
			return True

		lines = self.doGitStatus()
		if lines == None:
			return False

		return len(lines) == 0

	def doGitStatus(self):
		return self.doPopenGitCmd(["status", "-s", "-uno"])

	def isChanged(self):
		if not self.doGitStatus():
			return False
		return True

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

	def doBackupBase(self, srcRoot, destRoot, force = True):
		if not os.path.isdir(srcRoot):
			return False

		if not os.path.exists(destRoot):
			self.mkdirSafe(destRoot)
		elif not os.path.isdir(destRoot):
			self.prRedInfo(destRoot, " is not a directory")
			return False

		self.doExecGitCmd(["config", "--unset", "core.worktree"])

		for filename in os.listdir(srcRoot):
			srcPath = os.path.join(srcRoot, filename)
			destPath = os.path.join(destRoot, filename)
			if os.path.islink(srcPath):
				if not os.path.exists(destPath):
					self.prRedInfo(destPath, " is not exists")
					return False
				os.remove(srcPath)
				os.symlink(destPath, srcPath)
			elif os.path.isdir(srcPath):
				if os.path.exists(destPath) or os.path.islink(destPath):
					if not self.removeSafe(destPath):
						self.prRedInfo("remove ", destPath, " failed")
						return False
				if force:
					os.rename(srcPath, destPath)
					os.symlink(destPath, srcPath)
				else:
					os.symlink(srcPath, destPath)
			elif force or not os.path.exists(destPath):
				self.doCopyFile(srcPath, destPath)

		return True

	def doBackup(self, destRoot):
		if not self.doBackupBase(self.mPathGitRepo, destRoot):
			return False

		if not self.doExecute(["git", "config", "--file", os.path.join(destRoot, "config"), "core.bare", "true"], verbose = False):
			self.prRedInfo("set config core.bare to true failed")
			return False

		return True

	def doRecovery(self, srcRoot):
		if not self.doBackupBase(srcRoot, self.mPathGitRepo, False):
			return False

		if not self.doExecute(["git", "config", "--file", os.path.join(self.mPathGitRepo, "config"), "core.bare", "false"], verbose = False):
			self.prRedInfo("set config core.bare to false failed")
			return False

		return True

	def removeSelf(self):
		self.removeSafe(self.mPathRoot)

	def doPush(self, option = None):
		listCommand = ["push", "--all"]
		if option != None:
			listCommand.extend(option)

		return self.doExecGitCmd(listCommand)

	def doGitClean(self, pathname = "."):
		lines = self.doPopenGitCmd(["clean", "-xdf", "-e", ".svn", "-e", self.mGitRepoName, self.getRelPath(pathname)])
		if lines == None:
			return False

		for line in lines:
			if not line.startswith("Removing"):
				continue

			line = line.rstrip()
			if not line.endswith("/"):
				continue

			line = line[9:]

			if pathname != None:
				line = os.path.join(pathname, line)

			if not self.removeSafe(line):
				return False

		return True

	def hasBranch(self, branch):
		lines = self.doPopenGitCmd(["branch"], ef = "/dev/null")
		if lines == None:
			return False

		for line in lines:
			line = line[2:].rstrip()
			if line == branch:
				return True

		if self.mVerbose:
			self.prBrownInfo("don't have branch %s" % branch)

		return False

	def doGitCheckout(self, branch):
		if self.gitCheckout(branch):
			return True

		if not self.hasBranch(branch):
			return True

		if not self.doGitReset():
			return False

		return self.gitCheckout(branch)

	def gitMerge(self, branch):
		return self.doExecGitCmd(["merge", branch], of = "/dev/null")

	def doGitMerge(self, branch, destBranch = None):
		if destBranch != None:
			if not self.hasBranch(destBranch):
				return True
			if not self.doGitCheckout(destBranch):
				return False

		return self.gitMerge(branch)
