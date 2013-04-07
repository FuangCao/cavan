#!/usr/bin/python

import sys, os, re
from getopt import getopt
from xml.dom.minidom import parse, Document

from cavan_file import file_read_line, file_read_lines, \
		 file_write_line, file_write_lines, file_append_line, file_append_lines

from cavan_command import command_vision, popen_tostring, popen_to_list, \
		 single_arg, single_arg2
from cavan_stdio import pr_red_info, pr_green_info, pr_bold_info
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

class GitSvnManager:
	def __init__(self):
		reload(sys)
		sys.setdefaultencoding("utf-8")

		self.mRemoteName = "svn"
		self.mGitSvnPath = ".git/svn"
		self.mFileSvnIgnore = ".gitignore"
		self.mFileSvnLog = os.path.join(self.mGitSvnPath, "svn_log.xml")
		self.mFileSvnInfo = os.path.join(self.mGitSvnPath, "svn_info.xml")
		self.mFileSvnList = os.path.join(self.mGitSvnPath, "svn_list.txt")
		self.mFileGitList = os.path.join(self.mGitSvnPath, "git_list.txt")
		self.mFileSvnUpdate = os.path.join(self.mGitSvnPath, "svn_update.txt")
		self.mFileGitMessag = os.path.join(self.mGitSvnPath, "git_message.txt")

	def genSvnInfoXml(self, url = None):
		if url == None:
			url = self.mUrl
		return command_vision("svn info --xml %s > %s" % (single_arg(url), self.mFileSvnInfo))

	def genSvnLogXml(self):
		if self.mGitRevision >= self.mSvnRevision:
			return False
		return command_vision("svn log --xml -r %d:%d %s > %s" % (self.mGitRevision + 1, self.mSvnRevision, single_arg(self.mUrl), self.mFileSvnLog))

	def genGitRepo(self):
		if command_vision("git init") == False:
			return False

		if command_vision("git config user.name Fuang.Cao") == False:
			return False

		if command_vision("git config user.email cavan.cfa@gmail.com") == False:
			return False

		command_vision("git remote add %s %s" % (self.mRemoteName, single_arg(self.mUrl)))

		return True

	def setRemoteUrl(self, url):
		return command_vision("git config remote.svn.url %s" % single_arg(url))

	def getGitRevision(self):
		lines = popen_to_list("git log -1 | tail -1 | sed 's/.*@\([0-9]\+\)\s[^\s]\+$/\\1/g'")
		if not lines or len(lines) != 1:
			return 0

		return int(lines[0].strip())

	def saveGitMessage(self, entry):
		content = "%s\n\ncavan-git-svn-id: %s@%s %s" % (entry.getMessage(), self.mUrl, entry.getRevesion(), self.mUuid)
		return file_write_line(self.mFileGitMessag, content)

	def gitAddFile(self, pathname):
		pathname = pathname.rstrip("\f\r\n")
		if command_vision("git add -f '%s'" % pathname.replace("'", "'\\''")):
			return True
		if command_vision("git add -f \"%s\"" % pathname.replace("\"", "\\\"")):
			return True
		return False

	def gitAddFiles(self):
		fpSvnList = open(self.mFileSvnList, "r")
		if not fpSvnList:
			return True

		while True:
			lines = []
			while len(lines) < 200:
				line = fpSvnList.readline()
				if not line:
					break

				if line.find(" ") < 0 and line.find("\t") < 0:
					lines.append(line)
					continue

				if self.gitAddFile(line) == False:
					fpSvnList.close()
					return False

			if not lines:
				break

			fpGitList = open(self.mFileGitList, "w")
			if not fpGitList:
				fpSvnList.close()
				return False
			fpGitList.writelines(lines)
			fpGitList.close()

			if command_vision("git add -f $(cat %s)" % self.mFileGitList):
				continue

			for line in lines:
				if self.gitAddFile(line) == False:
					fpSvnList.close()
					return False

		fpSvnList.close()
		return True

	def gitCommit(self, entry):
		if self.gitAddFiles() == False:
			return False

		if self.saveGitMessage(entry) == False:
			return False

		author = entry.getAuthor()
		author = "%s <%s@%s>" % (author, author, self.mUuid)
		return command_vision("git commit --author \"%s\" --date %s -aF %s" % (author, entry.getDate(), self.mFileGitMessag))

	def listHasPath(self, path, listPath):
		for item in listPath:
			if path.startswith(item):
				return True
		return False

	def genSvnList(self):
		listDir = []
		listFile = []

		for line in file_read_lines(self.mFileSvnUpdate):
			line = line.rstrip("\r\n")
			if self.listHasPath(line, listDir):
				continue

			if os.path.isdir(line):
				print "[DIR]  Add " + line
				listDir.append(line + "/")
			else:
				print "[FILE] Add " + line
				listFile.append(line + "\n")

		fpSvnList = open(self.mFileSvnList, "w")
		if not fpSvnList:
			return False

		if len(listFile) > 0:
			fpSvnList.writelines(listFile)

		for path in listDir:
			listFile = popen_to_list("svn list -R %s | awk '!/\/+$/ {print %s $0}'" % (single_arg(path), single_arg2(path)))
			if listFile == None:
				fpSvnList.close()
				return False

			if len(listFile) > 0:
				fpSvnList.writelines(listFile)

		fpSvnList.close()
		return True

	def svnCheckout(self, entry):
		if os.path.isdir(".svn"):
			if not command_vision("svn update --accept tf --force -r %s | awk '/^[UCGER]*A[UCGER]*/ {print substr($0, 6)}' > %s" % (entry.getRevesion(), self.mFileSvnUpdate)):
				return False
		else:
			if not command_vision("svn checkout %s . > /dev/null" % single_arg(self.mUrl + "@" + entry.getRevesion())):
				return False

			if not file_write_line(self.mFileSvnUpdate, '.'):
				return False

			if not os.path.exists(self.mFileSvnIgnore):
				lines = ["/.gitignore\n", ".svn\n"]
				if not file_write_lines(self.mFileSvnIgnore, lines):
					return False

		if self.genSvnList() == False:
			return False

		return self.gitCommit(entry)

	def isInitialized(self):
		return command_vision("git branch > /dev/null")

	def doSync(self, url = None):
		self.mGitRevision = self.getGitRevision()
		if self.mGitRevision < 0:
			return False

		if not url:
			url = popen_tostring("git config remote.%s.url" % self.mRemoteName)
			if not url:
				return False
		elif not self.setRemoteUrl(url):
			return False

		self.mUrl = url

		if self.genSvnInfoXml() == False:
			return False

		infoParser = SvnInfoParser()
		if infoParser.loadXml(self.mFileSvnInfo) == False:
			return False

		self.mUuid = infoParser.getUuid()
		self.mSvnRevision = infoParser.getRevesion()

		if self.mGitRevision >= self.mSvnRevision:
			pr_green_info("Already up-to-date.")
			return True

		if self.mGitRevision > 0:
			if not command_vision("svn switch --force --accept tf %s > /dev/null" % single_arg("%s@%d" % (self.mUrl, self.mGitRevision))):
				return False
		else:
			revision = 1
			while revision < self.mSvnRevision:
				res = os.system("svn info %s 2>/dev/null" % single_arg("%s@%d" % (self.mUrl, revision)))
				if res == 0:
					break
				if res == 2:
					return False
				revision = revision + 1
			self.mGitRevision = revision - 1

		if self.genSvnLogXml() == False:
			return False

		logParser = SvnLogParser()
		if logParser.loadXml(self.mFileSvnLog) == False:
			return False

		nodes = logParser.getLogEntrys();
		if not nodes:
			pr_green_info("Already up-to-date.")
			return True

		for item in logParser.getLogEntrys():
			entry = SvnLogEntry(item)
			if self.svnCheckout(entry) == False:
				return False

		return True

	def doInitBase(self, url, pathname = None):
		self.mUrl = url
		if pathname != None:
			if not os.path.exists(pathname):
				os.makedirs(pathname, 0777)
			os.chdir(pathname)

		if not os.path.isdir(self.mGitSvnPath):
			os.makedirs(self.mGitSvnPath, 0777)
		elif self.isInitialized():
			pr_red_info("Has been initialized")
			return False

		if self.mUrl == None:
			if self.genSvnInfoXml("") == False:
				return False

			infoParser = SvnInfoParser()
			if infoParser.loadXml(self.mFileSvnInfo) == False:
				return False

			self.mUrl = infoParser.getUrl()

		if self.genGitRepo() == False:
			return False

		return self.doSync()

	def doInit(self, argv):
		length = len(argv)
		if length > 0:
			url = argv[0].rstrip("/")
			if length > 1:
				pathname = argv[1]
			else:
				pathname = os.path.basename(url)
		else:
			url = None
			pathname = None

		return self.doInitBase(url, pathname)

	def main(self, argv):
		length = len(argv)
		if length < 2:
			stdio.pr_red_info("Please give a subcmd")
			return False

		subcmd = argv[1]
		if subcmd in ["init", "clone"]:
			return self.doInit(argv[2:])
		elif subcmd in ["update", "sync"]:
			return self.doSync()
		else:
			stdio.pr_red_info("unknown subcmd " + subcmd)
			return False
