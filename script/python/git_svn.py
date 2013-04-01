#!/usr/bin/python

import sys, os
from getopt import getopt
from xml.dom.minidom import parse

from cavan_file import file_read_line, file_read_lines, file_write_text
from cavan_command import command_vision, popen_tostring
from cavan_stdio import pr_red_info, pr_green_info, pr_bold_info

def getFirstElement(parent, name):
	tags = parent.getElementsByTagName(name)
	if not tags or len(tags) < 1:
		return None
	return tags[0]

def getFirstElementData(parent, name):
	tag = getFirstElement(parent, name)
	if tag == None:
		return None
	node = tag.firstChild
	if not node or node.nodeType != node.TEXT_NODE:
		return None
	return node.data

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
		self.mRemoteName = "svn"
		self.mGitSvnPath = ".git_svn"
		self.mFileSvnIgnore = ".svn/.gitignore"
		self.mFileSvnLog = os.path.join(self.mGitSvnPath, "svn_log.xml")
		self.mFileSvnInfo = os.path.join(self.mGitSvnPath, "svn_info.xml")
		self.mFileSvnList = os.path.join(self.mGitSvnPath, "svn_list.txt")
		self.mFileSvnUpdate = os.path.join(self.mGitSvnPath, "svn_update.txt")
		self.mFileGitRevision = os.path.join(self.mGitSvnPath, "git_revision.txt")
		self.mFileGitMessag = os.path.join(self.mGitSvnPath, "git_message.txt")
		self.mFileGitIgnore = os.path.join(self.mGitSvnPath, ".gitignore")

	def genSvnInfoXml(self, url = None):
		if url == None:
			url = self.mUrl
		return command_vision("svn info --xml %s > %s" % (url, self.mFileSvnInfo))

	def genSvnLogXml(self):
		if self.mGitRevision >= self.mSvnRevision:
			return False
		return command_vision("svn log --xml -r %d:%d %s > %s" % (self.mGitRevision + 1, self.mSvnRevision, self.mUrl, self.mFileSvnLog))

	def genGitRepo(self):
		if os.path.isdir(".git"):
			command_vision("rm .git -rf")

		if command_vision("git init") == False:
			return False

		return command_vision("git remote add %s %s" % (self.mRemoteName, self.mUrl))

	def saveGitRevision(self, revision):
		return file_write_text(self.mFileGitRevision, revision)

	def getGitRevision(self):
		if not os.path.exists(".git") or command_vision("git checkout %s" % self.mFileGitRevision) == False:
			return 0

		line = file_read_line(self.mFileGitRevision)
		if not line:
			return 0

		return int(line.strip())

	def saveGitMessage(self, entry):
		content = "%s\n\ncavan-git-svn-id: %s@%s %s" % (entry.getMessage(), self.mUrl, entry.getRevesion(), self.mUuid)
		return file_write_text(self.mFileGitMessag, content.encode("utf-8"))

	def gitAddFiles(self, listfile):
		if not os.path.exists(listfile):
			return True

		if not os.path.getsize(listfile):
			return True

		if command_vision("git add -f $(cat %s)" % listfile) == True:
			return True

		for line in file_read_lines(listfile):
			line = line.strip()
			if command_vision("git add -f '%s'" % line) == True:
				continue
			if command_vision("git add -f \"%s\"" % line.replace("\'", "\\'")) == True:
				continue
			return False

		return True

	def gitCommit(self, entry):
		if self.gitAddFiles(self.mFileSvnList) == False:
			return False

		if self.saveGitRevision(entry.getRevesion()) == False:
			return False

		if self.saveGitMessage(entry) == False:
			return False

		author = entry.getAuthor()
		author = "%s <%s@%s>" % (author, author, self.mUuid)
		return command_vision("git commit --author \"%s\" --date %s -aF %s" % (author, entry.getDate(), self.mFileGitMessag))

	def genSvnList(self):
		listPath = []
		for line in file_read_lines(self.mFileSvnUpdate):
			line = line.strip()
			if not os.path.isdir(line):
				line = os.path.dirname(line)
				if not line:
					line = "."

			found = False
			for path in listPath:
				if line.startswith(path):
					found = True
					break

			if found == False:
				if line == ".":
					listPath = ["."]
					break
				listPath.append(line)

		if os.path.exists(self.mFileSvnList):
			os.remove(self.mFileSvnList)

		for path in listPath:
			if command_vision("svn list -R %s | sed -e '/\/\+\s*$/d' -e 's/^/%s\//g' >> %s" % (path, path.replace("/", "\/"), self.mFileSvnList)) == False:
				return False

		return True

	def svnCheckout(self, entry):
		if os.path.isdir(".svn"):
			if command_vision("svn update --accept tf --force -r %s | grep '^[UCGER]*A[UCGER]*\s\+' | awk '{print $NF}' > %s" % (entry.getRevesion(), self.mFileSvnUpdate)) == False:
				return False
		else:
			if command_vision("svn checkout %s@%s . && echo '.' > %s" % (self.mUrl, entry.getRevesion(), self.mFileSvnUpdate)) == False:
				return True

			if command_vision("echo '*' > %s && git add -f %s" % (self.mFileSvnIgnore, self.mFileSvnIgnore)) == False:
				return False

		if self.genSvnList() == False:
			return False

		return self.gitCommit(entry)

	def cmd_sync(self):
		url = popen_tostring("git config remote.%s.url" % self.mRemoteName)
		if not url:
			return False

		self.mUrl = url.strip()

		if self.genSvnInfoXml() == False:
			return False

		infoParser = SvnInfoParser()
		if infoParser.loadXml(self.mFileSvnInfo) == False:
			return False

		self.mUuid = infoParser.getUuid()
		self.mSvnRevision = infoParser.getRevesion()
		self.mGitRevision = self.getGitRevision()

		if self.mGitRevision >= self.mSvnRevision:
			pr_green_info("Already up-to-date.")
			return True

		if self.mGitRevision > 0 and command_vision("svn switch %s@%d && svn update -r %d" % (self.mUrl, self.mGitRevision, self.mGitRevision)) == False:
				return False

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

	def cmd_init(self, argv):
		length = len(argv)
		if length > 1:
			self.mUrl = argv[1].rstrip("/")
			if length > 2:
				pathname = argv[2]
			else:
				pathname = os.path.basename(self.mUrl)
		else:
			self.mUrl = None
			pathname = None

		if pathname != None:
			if not os.path.exists(pathname):
				os.makedirs(pathname, 0777)
			os.chdir(pathname)

		if not os.path.isdir(self.mGitSvnPath):
			os.mkdir(self.mGitSvnPath, 0777)
		elif os.path.exists(self.mFileGitRevision):
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

		if file_write_text(self.mFileGitIgnore, "*") == False:
		 	return False

		if self.saveGitRevision("0") == False:
			return False

		if command_vision("git add -f %s %s" % (self.mFileGitIgnore, self.mFileGitRevision)) == False:
			return False

		return self.cmd_sync()

	def main(self, argv):
		length = len(argv)
		if length < 2:
			stdio.pr_red_info("Please give a subcmd")
			return False

		subcmd = argv[1]
		if subcmd in ["init", "clone"]:
			return self.cmd_init(argv[1:])
		elif subcmd in ["update", "sync"]:
			return self.cmd_sync()
		else:
			stdio.pr_red_info("unknown subcmd " + subcmd)
			return False
