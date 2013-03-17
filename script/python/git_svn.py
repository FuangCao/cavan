#!/usr/bin/python

import sys, os
from getopt import getopt
from xml.dom.minidom import parse

from cavan_file import file_read_line, file_write_text
from cavan_command import command_vision, popen_tostring
from cavan_stdio import pr_red_info, pr_green_info, pr_bold_info

DIR_GIT_SVN = ".git_svn"
GIT_REMOTE_SVN = "svn"
FILE_SVN_LOG_XML = os.path.join(DIR_GIT_SVN, "svn_log.xml")
FILE_SVN_INFO_XML = os.path.join(DIR_GIT_SVN, "svn_info.xml")
FILE_SVN_LIST = os.path.join(DIR_GIT_SVN, "svn_list.txt")
FILE_GIT_REVISION = os.path.join(DIR_GIT_SVN, "git_revision.txt")
FILE_GIT_MESSAGE = os.path.join(DIR_GIT_SVN, "git_message.txt")
FILE_GIT_IGNORE = os.path.join(DIR_GIT_SVN, ".gitignore")
FILE_SVN_IGNORE = os.path.join(".svn", ".gitignore")

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
	def loadXml(self, pathname = FILE_SVN_INFO_XML):
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
	def loadXml(self, pathname = FILE_SVN_LOG_XML):
		dom = parse(pathname)
		if dom == None:
			return False

		self.mRootElement = dom.documentElement
		return True

	def getLogEntrys(self):
		entrys = self.mRootElement.getElementsByTagName("logentry")
		if not entrys:
			return None
		return entrys

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
	def genSvnInfoXml(self, pathname = FILE_SVN_INFO_XML):
		return command_vision("svn info --xml %s > %s" % (self.mUrl, pathname))

	def genSvnLogXml(self, pathname = FILE_SVN_LOG_XML):
		if self.mGitRevision >= self.mSvnRevision:
			return False
		return command_vision("svn log --xml -r %d:%d %s > %s" % (self.mGitRevision + 1, self.mSvnRevision, self.mUrl, pathname))

	def genGitRepo(self):
		if os.path.isdir(".git"):
			return True
		if command_vision("git init") == False:
			return False
		return command_vision("git remote add %s %s" % (GIT_REMOTE_SVN, self.mUrl))

	def saveGitRevision(self, revision):
		return file_write_text(FILE_GIT_REVISION, revision)

	def getGitRevision(self):
		if not os.path.exists(".git") or command_vision("git checkout %s" % FILE_GIT_REVISION) == False:
			return 0

		line = file_read_line(FILE_GIT_REVISION)
		if not line:
			return 0

		return int(line.strip())

	def gitCommit(self, entry):
		size = os.path.getsize(FILE_SVN_LIST)
		if size > 0 and (command_vision("git add -f $(cat %s)" % FILE_SVN_LIST)) == False:
			fp = open(FILE_LIST_DIFF, "r")
			if not fp:
				return False

			lines = fp.readlines()
			fp.close()

			for line in lines:
				if command_vision("git add -f '%s'" % line.strip()) == False:
					return False

		fp = open(FILE_GIT_MESSAGE, "w")
		if not fp:
			return False

		fp.write(entry.getMessage().encode("UTF-8"))
		fp.write("\n\ngit-svn-id: %s@%s %s" % (self.mUrl, entry.getRevesion(), self.mUuid))
		fp.close()

		author = entry.getAuthor()
		author = "%s <%s@%s>" % (author, author, self.mUuid)
		return command_vision("git commit --author \"%s\" --date %s -aF %s" % (author, entry.getDate(), FILE_GIT_MESSAGE))

	def svnCheckout(self, entry):
		revision = entry.getRevesion()
		if os.path.isdir(".svn"):
			command = "svn update --accept tf --force -r %s" % revision
		else:
			command = "svn checkout -r %s %s ." % (revision, self.mUrl)

		if command_vision("%s | grep '^A\s\+' | awk '{print $NF}' > %s" % (command, FILE_SVN_LIST)) == False:
			return False

		if not os.path.exists(FILE_SVN_IGNORE):
			file_write_text(FILE_SVN_IGNORE, "*")
			command_vision("git add -f %s" % FILE_SVN_IGNORE)

		if self.saveGitRevision(revision) == False:
			return False

		return self.gitCommit(entry)

	def cmd_sync(self):
		if self.mGitRevision >= self.mSvnRevision:
			pr_green_info("Nothing to be done")
			return True

		if self.genSvnLogXml() == False:
			return False

		logParser = SvnLogParser()
		if logParser.loadXml() == False:
			return False

		for item in logParser.getLogEntrys():
			entry = SvnLogEntry(item)
			if self.svnCheckout(entry) == False:
				return False

		return True

	def cmd_init(self):
		if os.path.exists(FILE_GIT_REVISION):
			pr_red_info("Has been initialized")
			return False

		if self.genGitRepo() == False:
			return False

		if file_write_text(FILE_GIT_IGNORE, "*") == False:
		 	return False

		if self.saveGitRevision("0") == False:
			return False

		self.mGitRevision = 0

		if command_vision("git add -f %s %s" % (FILE_GIT_IGNORE, FILE_GIT_REVISION)) == False:
			return False

		return self.cmd_sync()

	def main(self, argv):
		length = len(argv)
		if length < 2:
			stdio.pr_red_info("Too a few argument")
			return False

		cmdAbsPath = os.path.abspath(argv[0])
		subcmd = argv[1]

		if length > 2:
			url = argv[2]
			if length > 3:
				pathname = argv[3]
			else:
				pathname = os.path.basename(url.rstrip("/"))
		elif os.path.isdir(".git"):
			url = popen_tostring("git config remote.%s.url" % GIT_REMOTE_SVN)
			if not url:
				return False
			url = url.strip()
			pathname = "."
		else:
			return False

		pathname = os.path.abspath(pathname)
		if not os.path.exists(pathname):
			os.makedirs(pathname, 0777)

		os.chdir(pathname)

		if not os.path.exists(DIR_GIT_SVN):
			os.mkdir(DIR_GIT_SVN, 0777)

		self.mUrl = url.rstrip("/")
	
		if self.genSvnInfoXml() == False:
			return False

		infoParser = SvnInfoParser()
		if infoParser.loadXml() == False:
			return False

		self.mUuid = infoParser.getUuid()
		self.mSvnRevision = infoParser.getRevesion()
		self.mGitRevision = self.getGitRevision()

		if subcmd in ["init", "clone"]:
			return self.cmd_init()
		elif subcmd in ["update", "sync"]:
			return self.cmd_sync()
		else:
			stdio.pr_red_info("unknown subcmd " + subcmd)
			return False
