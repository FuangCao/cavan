#!/usr/bin/python

import sys, os, re, time
from getopt import getopt
from xml.dom.minidom import parse, Document

from cavan_file import file_read_line, file_read_lines, \
		file_write_line, file_write_lines, file_append_line, file_append_lines

from git_manager import CavanGitManager
from cavan_xml import getFirstElement, getFirstElementData

class SvnInfoParser:
	def loadXml(self, pathname):
		dom = parse(pathname)
		if not dom:
			return False
		self.mRootElement = getFirstElement(dom.documentElement, "entry")
		if self.mRootElement == None:
			return False
		return True

	def getPath(self):
		return self.mRootElement.getAttribute("path")

	def getRevesion(self):
		revision = self.mRootElement.getAttribute("revision")
		if not revision:
			return 0
		return int(revision)

	def getKind(self):
		return self.mRootElement.getAttribute("kind")

	def getUrl(self):
		return getFirstElementData(self.mRootElement, "url")

	def getRepository(self):
		return getFirstElement(self.mRootElement, "repository")

	def getRoot(self):
		tag = self.getRepository()
		if tag == None:
			return None
		return getFirstElementData(tag, "root")

	def getUuid(self):
		tag = self.getRepository()
		if tag == None:
			return None
		return getFirstElementData(tag, "uuid")

	def getWcInfo(self):
		return getFirstElement(self.mRootElement, "wc-info")

	def getWcRootAbsPath(self):
		tag = self.getWcInfo()
		if tag == None:
			return None
		return getFirstElementData(tag, "wcroot-abspath")

	def getSchedule(self):
		tag = self.getWcInfo()
		if tag == None:
			return None
		return getFirstElementData(tag, "schedule")

	def getDepth(self):
		tag = self.getWcInfo()
		if tag == None:
			return None
		return getFirstElementData(tag, "depth")

	def getCommit(self):
		return getFirstElement(self.mRootElement, "commit")

	def getCommitRevision(self):
		tag = self.getCommit()
		if tag == None:
			return None
		return tag.getAttribute("revision")

	def getAuthor(self):
		tag = self.getCommit()
		if tag == None:
			return None
		return getFirstElementData(tag, "author")

	def getDate(self):
		tag = self.getCommit();
		if tag == None:
			return None
		return getFirstElementData(tag, "date")

class SvnLogParser:
	def loadXml(self, pathname):
		dom = parse(pathname)
		if dom == None:
			return False

		self.mRootElement = dom.documentElement
		return True

	def getLogEntrys(self):
		return self.mRootElement.getElementsByTagName("logentry")

class SvnLogEntry:
	def __init__(self, element):
		self.mRootElement = element

	def getRevesion(self):
		return self.mRootElement.getAttribute("revision")

	def getAuthor(self):
		return getFirstElementData(self.mRootElement, "author")

	def getDate(self):
		return getFirstElementData(self.mRootElement, "date")

	def getMessage(self):
		return getFirstElementData(self.mRootElement, "msg")

class GitSvnManager(CavanGitManager):
	def __init__(self, pathname = None, verbose = True):
		CavanGitManager.__init__(self, pathname, verbose)
		self.mRemoteName = "cavan-svn"
		self.mBranchMaster = "master"
		self.mBranchMerge = "cavan-merge"
		self.mPatternSvnUpdate = re.compile('^([AD])[UCGER ]{4}(.*)$')
		self.mPatternGitWhatChanged = re.compile('^:([0-9]{3}[0-7]{3} ){2}([0-9a-f]{7}\.{3} ){2}([AMD])\t(.*)$')

	def setRootPath(self, pathname, auto_create = False):
		CavanGitManager.setRootPath(self, pathname, auto_create)

		self.mPathGitSvn = os.path.join(self.mPathGitRepo, "cavan-svn")
		if auto_create and not os.path.isdir(self.mPathGitSvn):
			self.mkdirSafe(self.mPathGitSvn)

		self.mPathSvnRepo = self.getAbsPath(".svn")
		if auto_create and not os.path.isdir(self.mPathSvnRepo):
			self.mkdirSafe(self.mPathSvnRepo)

		self.mFileSvnLog = os.path.join(self.mPathGitSvn, "svn_log.xml")
		self.mFileSvnInfo = os.path.join(self.mPathGitSvn, "svn_info.xml")

	def genSvnInfoXml(self, url = None):
		if url == None:
			url = self.mUrl

		listCommand = ["svn", "info", "--xml"]

		if url != None:
			listCommand.append(url)

		return self.doExecute(listCommand, of = self.mFileSvnInfo)

	def getSvnInfo(self, url = None):
		if not self.setRemoteUrl(url):
			return False

		if not self.genSvnInfoXml(url):
			return None

		parser = SvnInfoParser()
		if not parser.loadXml(self.mFileSvnInfo):
			return False

		self.mSvnRevision = parser.getRevesion()
		self.mUuid = parser.getUuid()
		self.mSvnUrl = parser.getUrl()
		self.mPatternGitRevision = re.compile('^cavan-git-svn-id: .*@([0-9]+) %s$' % self.mUuid)

		return parser

	def genSvnLogXml(self):
		if self.mGitRevision >= self.mSvnRevision:
			return False
		return self.doExecute(["svn", "log", "--xml", "-r", "%d:%d" % (self.mGitRevision + 1, self.mSvnRevision), self.mUrl], of = self.mFileSvnLog)

	def getSvnLog(self):
		if not self.genSvnLogXml():
			return None

		logParser = SvnLogParser()
		if not logParser.loadXml(self.mFileSvnLog):
			return None

		return logParser

	def gitCheckoutVersion(self, commit = None, option = None):
		listCommand = ["checkout", "--quiet"]
		if commit != None:
			listCommand.append(commit)

		if option != None:
			for item in option:
				listCommand.append(item)

		return self.doExecGitCmd(listCommand, ef = "/dev/null")

	def genGitRepo(self):
		if not CavanGitManager.genGitRepo(self):
			return False

		return self.setRemoteUrl(self.mUrl)

	def getGitHeadSvnRevision(self, commit = None):
		listMessage = self.getGitHeadMessage(commit)
		if not listMessage:
			return -1

		match = self.mPatternGitRevision.match(listMessage.pop())
		if not match:
			return -1

		return int(match.group(1))

	def listHasPath(self, listPath, path):
		for item in listPath:
			if path.startswith(item):
				return True
		return False

	def gitAddFiles(self, listUpdate):
		listDir = []
		listFile = []

		for line in listUpdate:
			if self.listHasPath(listDir, line):
				continue

			absPath = self.getAbsPath(line)
			if os.path.isdir(absPath) and not os.path.islink(absPath):
				if line in [".git", ".svn"]:
					continue

				if self.mVerbose:
					print "[DIR]  Add " + line
				listDir.append(line + "/")
			else:
				if self.mVerbose:
					print "[FILE] Add " + line
				listFile.append(line)

		if not self.gitAddFileList(listFile):
			return False

		for path in listDir:
			lines = self.doPopen(["svn", "list", "-R", path])
			if lines == None:
				return False

			listFile = []
			for line in lines:
				line = line.rstrip("\n")
				if line.endswith("/") or line.startswith(".git/") or line.find("/.git/") >= 0:
					continue

				listFile.append(os.path.join(path, line))

			if not self.gitAddFileList(listFile):
				return False

		return True

	def gitSvnCommit(self, revision, author, date, message):
		author = "%s <%s@%s>" % (author, author, self.mUuid)
		message = "%s\n\ncavan-git-svn-id: %s@%s %s" % (message, self.mUrl, revision, self.mUuid)

		if not self.doGitCommit(message, author, date):
			return False

		self.mGitRevision = int(revision)
		return True

	def svnUpdate(self, revision):
		lines = self.doPopen(["svn", "update", "--accept", "theirs-full", "--force", "-r", revision])
		if lines == None:
			return None

		listUpdate = []

		for line in lines:
			match = self.mPatternSvnUpdate.match(line)
			if not match:
				continue

			if match.group(1) == "D":
				if not os.path.exists(match.group(2)) and not self.removeSafe(match.group(2)):
					return None
			else:
				listUpdate.append(match.group(2))

		return listUpdate

	def svnCheckout(self, entry):
		revision = entry.getRevesion()

		if os.path.isdir(self.mPathSvnRepo):
			listUpdate = self.svnUpdate(revision)
			if listUpdate == None:
				return False
		else:
			url = "%s@%s" % (self.mUrl, revision)
			if not self.doExecute(["svn", "checkout", "--force", "--quiet", url, "."], of = "/dev/null"):
				return False

			listUpdate = self.doPopen(["svn", "list"])
			if not listUpdate:
				return listUpdate != None

			listUpdate = [line.rstrip("/\n") for line in listUpdate]

		if not self.gitAddFiles(listUpdate):
			return False

		if self.gitSvnCommit(revision, entry.getAuthor(), entry.getDate(), entry.getMessage()):
			return True

		lines = self.doGitStatus();
		if not lines:
			return lines != None

		return False

	def buildSvnUrl(self, url, revision):
		return "%s@%s" % (url, revision)

	def doSync(self, url = None, branch = None):
		if not os.path.isdir(self.mPathSvnRepo) and os.path.isdir(self.mPathGitSvn):
			self.removeSafe(self.mPathSvnRepo)
			os.makedirs(os.path.join(self.mPathSvnRepo, "tmp"))
			relPath = os.path.join("..", self.getRelPath(self.mPathGitSvn))
			for filename in os.listdir(self.mPathGitSvn):
				destPath = os.path.join(self.mPathSvnRepo, filename)
				srcPath = os.path.join(relPath, filename)
				os.symlink(srcPath, destPath)

		if not self.getSvnInfo(url):
			return False

		if not branch:
			branch = self.mBranchMaster

		self.mGitRevision = self.getGitHeadSvnRevision(branch)
		if self.mGitRevision < 0 and self.doExecGitCmd(["log", "-1"], of = "/dev/null", ef = "/dev/null"):
			self.prRedInfo("get svn revision failed")
			return False

		if self.mGitRevision >= self.mSvnRevision:
			if self.mVerbose:
				self.prGreenInfo("Already up-to-date.")
			return True

		if self.mGitRevision > 0:
			self.doGitReset(branch)
			self.doExecute(["svn", "unlock", "--force", "."], ef = "/dev/null", of = "/dev/null")

			if self.mUrl != self.mSvnUrl:
				url = self.buildSvnUrl(self.mUrl, self.mGitRevision)
				if not self.doExecute(["svn", "switch", "--force", "--quiet", "--accept", "theirs-full", url], of = "/dev/null"):
					return False
			elif self.svnUpdate("%s" % self.mGitRevision) == None:
				return False
		else:
			self.doExecute(["rm", "-rf", ".svn"])

			minRevision = 0
			maxRevision = self.mSvnRevision

			while maxRevision >= minRevision:
				revision = (minRevision + maxRevision) / 2
				url = self.buildSvnUrl(self.mUrl, revision)
				if self.doExecute(["svn", "info", url], of = "/dev/null", ef = "/dev/null"):
					maxRevision = revision - 1
				else:
					minRevision = revision + 1

			if self.mVerbose:
				self.prGreenInfo("minRevision = %d, maxRevision = %d" % (minRevision, maxRevision))

			self.mGitRevision = maxRevision

		logParser = self.getSvnLog()
		if not logParser:
			return False

		nodes = logParser.getLogEntrys();
		if not nodes:
			if self.mVerbose:
				self.prGreenInfo("Already up-to-date.")
			return True

		for item in logParser.getLogEntrys():
			entry = SvnLogEntry(item)
			if not self.svnCheckout(entry):
				return False

		for filename in os.listdir(self.mPathSvnRepo):
			if filename in ["tmp", ".gitignore"]:
				continue

			srcPath = os.path.join(self.mPathSvnRepo, filename)
			if os.path.islink(srcPath):
				continue

			destPath = os.path.join(self.mPathGitSvn, filename)
			if os.path.exists(destPath):
				self.doExecute(["rm", "-rf", destPath])

			os.rename(srcPath, destPath)
			relPath = os.path.join("..", self.getRelPath(destPath))
			os.symlink(relPath, srcPath)

		return True

	def svnAddNonRecursive(self, pathname):
		return self.doExecute(["svn", "add", "--non-recursive", "--force", "--quiet", pathname], ef = "/dev/null", verbose = False)

	def svnAddFile(self, pathname):
		if not pathname:
			return False

		if self.svnAddNonRecursive(pathname):
			return True

		if not self.svnAddFile(os.path.dirname(pathname)):
			return False

		return self.svnAddNonRecursive(pathname)

	def svnRemoveFile(self, pathname):
		if self.mVerbose:
			self.prStdInfo("Remove ", pathname)

		if not self.doExecute(["svn", "remove", "--quiet", pathname], verbose = False):
			return True

		if not self.removeSafe(pathname):
			return False

		dirname = os.path.dirname(pathname)
		if not dirname:
			return True

		lines = os.listdir(self.getAbsPath(dirname))
		if not lines:
			return self.svnRemoveFile(dirname)

		return True

	def doDcommit(self, url = None):
		if not self.getSvnInfo(url):
			return False

		listPendLog = []
		hashHead = self.getGitHeadHash(self.mBranchMaster)

		while True:
			if not hashHead:
				return False

			mapRevision = self.getGitHeadRevisionMap(hashHead)
			if not mapRevision:
				return False

			if mapRevision[0] >= 0:
				break

			listPendLog.append((hashHead, mapRevision))
			hashHead = self.getGitHeadHash(hashHead, 2)

		if len(listPendLog) == 0:
			return True

		if not self.gitCheckoutVersion(self.mBranchMaster):
			return False

		if not self.gitCheckoutVersion(hashHead, ["-B", self.mBranchMerge]):
			return False

		if not self.doExecute(["svn", "revert", "--quiet", "-R", "."]):
			return False

		while True:
			if not self.doSync(branch = self.mBranchMerge):
				return False

			try:
				tupCommit = listPendLog.pop()
			except:
				break

			commit = tupCommit[0]

			lines = self.doPopenGitCmd(["whatchanged", "-1", "--pretty=tformat:%H", commit])
			if lines == None:
				return False

			for line in lines:
				match = self.mPatternGitWhatChanged.match(line)
				if not match:
					continue

				action = match.group(3)
				pathname = match.group(4)

				if action == 'D':
					if not self.svnRemoveFile(pathname):
						return False
				else:
					if not self.doExecGitCmd(["checkout", "--quiet", commit, pathname], verbose = False):
						return False

					if action == 'A' and not self.svnAddFile(pathname):
						return False

			if not self.doPopen(["svn", "commit", "-m", "".join(tupCommit[1][1])]):
				return False

		return self.doExecGitCmd(["branch", "-M", self.mBranchMerge, self.mBranchMaster])

	def doInitBase(self, url, pathname = None):
		self.mUrl = url

		if not pathname:
			pathname = self.getRootPath()
		self.setRootPath(pathname, True)

		if self.isInitialized():
			self.prRedInfo("Has been initialized")
			return False

		if self.mUrl == None and not self.getSvnInfo():
			return False

		return self.genGitRepo()

	def doInit(self, argv):
		length = len(argv)
		if length > 0:
			url = argv[0].rstrip("/")
		else:
			url = None

		if length > 1:
			pathname = argv[1].rstrip("/")
		else:
			pathname = None

		return self.doInitBase(url, pathname)

	def doClone(self, argv):
		if len(argv) == 1:
			argv.append(os.path.basename(argv[0].rstrip("/")))

		if not self.doInit(argv):
			return False

		return self.doSync()

	def main(self, argv):
		length = len(argv)
		if length < 2:
			stdio.self.prRedInfo("Please give a subcmd")
			return False

		subcmd = argv[1]
		if subcmd in ["init"]:
			return self.doInit(argv[2:])
		elif subcmd in ["clone"]:
			return self.doClone(argv[2:])
		elif subcmd in ["clean", "cleanup"]:
			if length > 2:
				return self.doGitClean(argv[2])
			return self.doGitClean()
		elif subcmd in ["ln", "link", "symlink", "push"]:
			if len(argv) < 3:
				return False
			return self.doSymlinkBare(argv[2])
		elif subcmd in ["update", "sync", "rebase", "dcommit"]:
			if length > 2:
				url = argv[2]
			else:
				url = None

			if subcmd in ["dcommit"]:
				return self.doDcommit(url)

			return self.doSync(url)
		else:
			self.prRedInfo("unknown subcmd " + subcmd)
			return False
