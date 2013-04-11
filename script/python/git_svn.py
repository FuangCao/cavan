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
		self.mBranchName = "master"
		self.mPatternSvnUpdate = re.compile('^A[UCGER ]{4}(.*)$')
		self.mPatternGitRevision = re.compile('^\s*cavan-git-svn-id: .*@([0-9]+) [^ ]+$')
		self.mPatternGitLogCommit = re.compile('^commit (.*)$')
		self.mPatternGitLogAuthor = re.compile('^Author: (.*) \<(.*)\>$')
		self.mPatternGitLogDate = re.compile('^Date:   (.*)$')
		self.mPatternGitWhatChanged = re.compile('^:([0-9]{3}[0-7]{3} ){2}(.{7}\.{3} ){2}A\t(.*)$')

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

	def setRemoteUrl(self, url = None):
		if not url:
			lines = self.doPopen(["git", "config", "remote.%s.url" % self.mRemoteName])
			if not lines:
				return False
			url = lines[0].rstrip("\n")
		elif not self.doExecute(["git", "config", "remote.%s.url" % self.mRemoteName, url]):
			return False

		self.mUrl = url

		return True

	def genGitRepo(self):
		if not CavanCommandBase.genGitRepo(self):
			return False

		return self.setRemoteUrl(self.mUrl)

	def getGitRevision(self):
		lines = self.doPopen(["git", "log", "-1"], ef = "/dev/null")
		if not lines:
			return 0

		gitSvnId = lines.pop()
		match = self.mPatternGitRevision.match(gitSvnId)
		if not match:
			return -1

		return int(match.group(1))

	def getGitHeadLog(self, index = 1, commit = None):
		command = ["git", "log", "-%d" % index]
		if commit != None:
			command.append(commit)

		lines = self.doPopen(command, ef = "/dev/null")
		if not lines:
			return None

		dictLog = {}

		try:
			while index > 0:
				match = self.mPatternGitLogCommit.match(lines.pop(0))
				if match != None:
					index = index - 1
			dictLog["commit"] = match.group(1)

			match = self.mPatternGitLogAuthor.match(lines.pop(0))
			if not match:
				return None
			dictLog["author"] = match.group(1)
			dictLog["email"] = match.group(2)

			match = self.mPatternGitLogDate.match(lines.pop(0))
			if not match:
				return None
			dictLog["date"] = match.group(1)

			lines.pop(0)
			logSvnId = lines.pop()

			match = self.mPatternGitRevision.match(logSvnId)
			if not match:
				lines.append(logSvnId)
				gitSvnId = -1
			else:
				lines.pop()
				gitSvnId = match.group(1)
		except:
			return None

		dictLog["message"] = [line[4:] for line in lines]
		dictLog["svn-id"] = gitSvnId

		return dictLog

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

		if self.doExecute(listCommand, of = "/dev/null"):
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

	def doGitReset(self):
		if not self.doExecute(["git", "checkout", self.mBranchName], of = "/dev/null", ef = "/dev/null"):
			return False

		lines = self.doPopen(["git", "diff"])
		if not lines:
			return True

		tmNow = time.localtime()
		filename = "%04d-%02d%02d-%02d%02d%02d.diff" % (tmNow.tm_year, tmNow.tm_mon, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec)
		file_write_lines(os.path.join(self.mPathPatch, filename), lines)

		return self.doExecute(["git", "reset", "--hard"])

	def doSync(self, url = None):
		self.mGitRevision = self.getGitRevision()
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
			self.prGreenInfo("Already up-to-date.")
			return True

		if self.mGitRevision > 0:
			self.doGitReset()
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

	def svnAddFiles(self, listFile):
		listDir = []

		for node in listFile:
			dirname = os.path.dirname(node.rstrip("/"))
			if len(dirname) > 0 and self.listHasPath(listDir, dirname):
				continue

			if not self.doExecute(["svn", "info", os.path.join(self.mUrl, dirname)], ef = "/dev/null", of = "/dev/null"):
				listDir.append(dirname)
			elif not self.doExecute(["svn", "add", node]):
				return False

		if len(listDir) > 0:
			return self.svnAddFiles(listDir)

		return True

	def doDcommit(self, url = None):
		if not self.setRemoteUrl(url):
			return False

		listPendLog = []
		dictLog = self.getGitHeadLog(commit = self.mBranchName)

		while True:
			if not dictLog:
				return False

			if dictLog["svn-id"] >= 0:
				break

			listPendLog.append(dictLog)
			dictLog = self.getGitHeadLog(2, dictLog["commit"])

		if len(listPendLog) == 0:
			return True

		if not self.doExecute(["svn", "update"]):
			return False

		if not self.doExecute(["svn", "revert", "--quiet", "-R", "."]):
			return False

		lastCommit = dictLog["commit"]

		while len(listPendLog) > 0:
			dictLog = listPendLog.pop()
			if not self.doExecute(["git", "checkout", "--quiet", dictLog["commit"]]):
				return False

			lines = self.doPopen(["git", "whatchanged", "-1"])
			if lines == None:
				return False

			listFile = []

			for line in lines:
				match = self.mPatternGitWhatChanged.match(line)
				if not match:
					continue

				listFile.append(match.group(3))

			if len(listFile) > 0 and not self.svnAddFiles(listFile):
				return False

			if not self.doExecute(["svn", "ci", "-m", "".join(dictLog["message"])]):
				return False

		if not self.doExecute(["git", "checkout", "--quiet", self.mBranchName]):
			return False

		return self.doExecute(["git", "reset", lastCommit])

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
				if not self.doDcommit(url):
					return False

			return self.doSync(url)
		else:
			stdio.self.prRedInfo("unknown subcmd " + subcmd)
			return False
