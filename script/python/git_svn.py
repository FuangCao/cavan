#!/usr/bin/python

import sys, os, re, time
from getopt import getopt
from xml.dom.minidom import parse, Document

from cavan_file import file_read_line, file_read_lines, \
		file_write_line, file_write_lines, file_append_line, file_append_lines

from cavan_command import CavanCommandBase
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

class GitSvnManager(CavanCommandBase):
	def __init__(self, pathname = "."):
		CavanCommandBase.__init__(self, pathname)
		self.mRemoteName = "cavan-svn"
		self.mBranchMaster = "master"
		self.mBranchMerge = "cavan-merge"
		self.mPatternSvnUpdate = re.compile('^A[UCGER ]{4}(.*)$')
		self.mPatternGitRevision = re.compile('^cavan-git-svn-id: .*@([0-9]+) [^ ]+$')
		self.mPatternGitLogCommit = re.compile('^commit (.*)$')
		self.mPatternGitLogAuthor = re.compile('^Author: (.*) \<(.*)\>$')
		self.mPatternGitLogDate = re.compile('^Date:   (.*)$')
		self.mPatternGitWhatChanged = re.compile('^:([0-9]{3}[0-7]{3} ){2}(.{7}\.{3} ){2}([AMD])\t(.*)$')

	def setRootPath(self, pathname):
		CavanCommandBase.setRootPath(self, pathname)
		self.mFileSvnIgnore = self.getAbsPath(".gitignore")

		self.mPathGitSvn = self.getAbsPath(".git/cavan-svn")
		if not os.path.isdir(self.mPathGitSvn):
			os.makedirs(self.mPathGitSvn)

		self.mPathPatch = self.getAbsPath(".git/cavan-patch")
		if not os.path.isdir(self.mPathPatch):
			os.makedirs(self.mPathPatch)

		self.mFileSvnLog = os.path.join(self.mPathGitSvn, "svn_log.xml")
		self.mFileSvnInfo = os.path.join(self.mPathGitSvn, "svn_info.xml")

	def genSvnInfoXml(self, url = None):
		if url == None:
			url = self.mUrl

		listCommand = ["svn", "info", "--xml"]

		if url != None:
			listCommand.append(url)

		return self.doExecute(listCommand, of = self.mFileSvnInfo)

	def genSvnLogXml(self):
		if self.mGitRevision >= self.mSvnRevision:
			return False
		return self.doExecute(["svn", "log", "--xml", "-r", "%d:%d" % (self.mGitRevision + 1, self.mSvnRevision), self.mUrl], of = self.mFileSvnLog)

	def setRemoteUrl(self, url = None, verbose = False):
		if not url:
			lines = self.doPopen(["git", "config", "remote.%s.url" % self.mRemoteName], verbose)
			if not lines:
				return False
			url = lines[0].rstrip("\n")
		elif not self.doExecute(["git", "config", "remote.%s.url" % self.mRemoteName, url], verbose):
			return False

		self.mUrl = url

		return True

	def gitCheckoutVersion(self, commit = None, option = None):
		listCommand = ["git", "checkout", "--quiet"]
		if commit != None:
			listCommand.append(commit)

		if option != None:
			for item in option:
				listCommand.append(item)

		return self.doExecute(listCommand, ef = "/dev/null", verbose = False)

	def genGitRepo(self):
		if not CavanCommandBase.genGitRepo(self):
			return False

		return self.setRemoteUrl(self.mUrl)

	def getGitHeadLog(self, tformat = None, commit = None, count = 1):
		command = ["git", "log", "-%d" % count]
		if tformat != None:
			command.append("--pretty=tformat:%s" % tformat)

		if commit != None:
			command.append(commit)

		lines = self.doPopen(command, verbose = False)
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

	def getGitHeadSvnRevision(self, commit = None):
		listMessage = self.getGitHeadMessage(commit)
		if not listMessage:
			return -1

		match = self.mPatternGitRevision.match(listMessage.pop())
		if not match:
			return -1

		return int(match.group(1))

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

	def listHasPath(self, listPath, path):
		for item in listPath:
			if path.startswith(item):
				return True
		return False

	def gitAddFileList(self, listFile):
		if len(listFile) == 0:
			return True

		listFile.insert(0, "git")
		listFile.insert(1, "add")
		listFile.insert(2, "-f")

		return self.doExecute(listFile, verbose = False)

	def gitAddFiles(self, listUpdate):
		listDir = []
		listFile = []

		for line in listUpdate:
			if self.listHasPath(listDir, line):
				continue

			if os.path.isdir(self.getAbsPath(line)):
				print "[DIR]  Add " + line
				listDir.append(line + "/")
			else:
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
				if line.endswith("/"):
					continue

				listFile.append(os.path.join(path, line))

			if not self.gitAddFileList(listFile):
				return False

		return True

	def gitSvnCommit(self, revision, author, date, message):
		listCommand = ["git", "commit", "--quiet", "--all", "--date", date]
		listCommand.append("--author")
		listCommand.append("%s <%s@%s>" % (author, author, self.mUuid))
		listCommand.append("--message")
		listCommand.append("%s\n\ncavan-git-svn-id: %s@%s %s" % (message, self.mUrl, revision, self.mUuid))

		if self.doExecute(listCommand, of = "/dev/null", verbose = False):
			self.mGitRevision = int(revision)
			return True

		return False

	def svnCheckout(self, entry):
		revision = entry.getRevesion()

		if os.path.isdir(self.getAbsPath(".svn")):
			lines = self.doPopen(["svn", "update", "--accept", "theirs-full", "--force", "-r", revision])
			if lines == None:
				return False

			listUpdate = []

			for line in lines:
				match = self.mPatternSvnUpdate.match(line)
				if not match:
					continue
				listUpdate.append(match.group(1))
		else:
			url = "%s@%s" % (self.mUrl, revision)
			if not self.doExecute(["svn", "checkout", "--force", "--quiet", url, "."], of = "/dev/null"):
				return False

			if not os.path.exists(self.mFileSvnIgnore):
				lines = ["/.gitignore\n", ".svn\n"]
				if not file_write_lines(self.mFileSvnIgnore, lines):
					return False

			listUpdate = self.doPopen(["svn", "list"])
			if not listUpdate:
				return listUpdate != None

			listUpdate = [line.rstrip("/\n") for line in listUpdate]

		if not self.gitAddFiles(listUpdate):
			return False

		if self.gitSvnCommit(revision, entry.getAuthor(), entry.getDate(), entry.getMessage()):
			return True

		lines = self.doPopen(["git", "status", "-su", "no"])
		if not lines:
			return lines != None

		return False

	def isInitialized(self):
		return self.doExecute(["git", "branch"], of = "/dev/null", ef = "/dev/null")

	def buildSvnUrl(self, url, revision):
		return "%s@%s" % (url, revision)

	def doGitReset(self, branch):
		if not self.gitCheckoutVersion(branch):
			return False

		lines = self.doPopen(["git", "diff"])
		if not lines:
			return True

		tmNow = time.localtime()
		filename = "%04d-%02d%02d-%02d%02d%02d.diff" % (tmNow.tm_year, tmNow.tm_mon, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec)
		file_write_lines(os.path.join(self.mPathPatch, filename), lines)

		return self.doExecute(["git", "reset", "--hard"])

	def doSync(self, url = None, branch = None):
		if not branch:
			branch = self.mBranchMaster

		self.mGitRevision = self.getGitHeadSvnRevision(branch)
		if self.mGitRevision < 0:
			return False

		if not self.setRemoteUrl(url):
			return False

		if self.genSvnInfoXml() == False:
			return False

		infoParser = SvnInfoParser()
		if infoParser.loadXml(self.mFileSvnInfo) == False:
			return False

		self.mUuid = infoParser.getUuid()
		self.mSvnRevision = infoParser.getRevesion()

		if self.mGitRevision >= self.mSvnRevision:
			return True

		if self.mGitRevision > 0:
			self.doGitReset(branch)
			self.doExecute(["svn", "unlock", "--force", "."], ef = "/dev/null", of = "/dev/null")

			url = self.buildSvnUrl(self.mUrl, self.mGitRevision)
			if not self.doExecute(["svn", "switch", "--force", "--quiet", "--accept", "theirs-full", url], of = "/dev/null"):
				return False
		else:
			self.doExecute(["rm", "-rf", ".svn"])

			minRevision = 0
			maxRevision = self.mSvnRevision - 1

			while True:
				revision = (minRevision + maxRevision) / 2
				if revision <= minRevision:
					break

				url = self.buildSvnUrl(self.mUrl, revision)
				if self.doExecute(["svn", "info", url], of = "/dev/null", ef = "/dev/null"):
					maxRevision = revision - 1
				else:
					minRevision = revision

			self.mGitRevision = minRevision

		if self.genSvnLogXml() == False:
			return False

		logParser = SvnLogParser()
		if logParser.loadXml(self.mFileSvnLog) == False:
			return False

		nodes = logParser.getLogEntrys();
		if not nodes:
			self.prGreenInfo("Already up-to-date.")
			return True

		for item in logParser.getLogEntrys():
			entry = SvnLogEntry(item)
			if self.svnCheckout(entry) == False:
				return False

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
		if not self.doExecute(["svn", "remove", "--quiet", pathname], verbose = False):
			return True

		dirname = os.path.dirname(pathname)
		if not dirname:
			return True

		lines = self.doPopen(["svn", "list", dirname], verbose = False)
		if lines == None:
			return False

		if not lines:
			return True

		return self.svnRemoveFile(dirname)

	def doDcommit(self, url = None):
		if not self.setRemoteUrl(url):
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

			lines = self.doPopen(["git", "whatchanged", "-1", "--pretty=tformat:%H", commit], verbose = False)
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
					if not self.doExecute(["git", "checkout", "--quiet", commit, pathname], verbose = False):
						return False

					if action == 'A' and not self.svnAddFile(pathname):
						return False

			if not self.doPopen(["svn", "commit", "-m", "".join(tupCommit[1][1])], verbose = False):
				return False

		return self.doExecute(["git", "branch", "-M", self.mBranchMerge, self.mBranchMaster])

	def doInitBase(self, url, pathname = None):
		self.mUrl = url
		if pathname != None:
			self.setRootPath(pathname)

		if self.isInitialized():
			self.prRedInfo("Has been initialized")
			return False

		if self.mUrl == None:
			if self.genSvnInfoXml() == False:
				return False

			infoParser = SvnInfoParser()
			if infoParser.loadXml(self.mFileSvnInfo) == False:
				return False

			self.mUrl = infoParser.getUrl()

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
		if subcmd in ["clone"]:
			return self.doClone(argv[2:])
		elif subcmd in ["update", "sync", "rebase", "dcommit"]:
			if length > 2:
				url = argv[2]
			else:
				url = None

			if subcmd in ["dcommit"]:
				return self.doDcommit(url)

			return self.doSync(url)
		else:
			stdio.self.prRedInfo("unknown subcmd " + subcmd)
			return False
