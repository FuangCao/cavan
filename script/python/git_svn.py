#!/usr/bin/python

import sys, os, stdio
from command import command_vision, popen_tostring
from getopt import getopt
from xml.dom.minidom import parse

DIR_GIT_SVN = ".git_svn"
GIT_REMOTE_SVN = "svn"
FILE_CMD_NAME = "git-svn.py"
FILE_SVN_LOG_XML = os.path.join(DIR_GIT_SVN, "svn_log.xml")
FILE_SVN_INFO_XML = os.path.join(DIR_GIT_SVN, "svn_info.xml")
FILE_SVN_LIST = os.path.join(DIR_GIT_SVN, "svn_list.txt")
FILE_LAST_REVISION = os.path.join(DIR_GIT_SVN, "svn_revision.txt")
FILE_GIT_MESSAGE = os.path.join(DIR_GIT_SVN, "git_message.txt")

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
		return self.mRootElement.getAttribute("revision")

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

		entrys.reverse()
		return entrys

class SvnLogEntryParser:
	def setRootElement(self, element):
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
		return command_vision("svn log --xml %s > %s" % (self.mUrl, pathname))

	def genGitRepo(self, info):
		if os.path.isdir(".git"):
			return True
		if command_vision("git init") == False:
			return False
		return command_vision("git remote add %s %s" % (GIT_REMOTE_SVN, info.getUrl()))

	def saveLastRevision(self, revision):
		return command_vision("echo %s > %s && git add -f %s" % (revision, FILE_LAST_REVISION, FILE_LAST_REVISION))

	def gitCommit(self, info, entry):
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
		fp.write("\n\ngit-svn-id: %s@%s %s" % (info.getUrl(), entry.getRevesion(), info.getUuid()))
		fp.close()

		author = entry.getAuthor()
		author = "%s <%s@%s>" % (author, author, info.getUuid())
		return command_vision("git commit --author \"%s\" --date %s -aF %s" % (author, entry.getDate(), FILE_GIT_MESSAGE))

	def svnCheckout(self, info, entry):
		if os.path.isdir(".svn"):
			command = "svn update -r %s" % entry.getRevesion()
		else:
			command = "svn checkout -r %s %s ." % (entry.getRevesion(), info.getUrl())

		if command_vision("%s | grep '^A\s\+' | awk '{print $NF}' > %s" % (command, FILE_SVN_LIST)) == False:
			return False

		if self.saveLastRevision(entry.getRevesion()) == False:
			return False

		return self.gitCommit(info, entry)

	def cmd_init(self):
		if self.genSvnLogXml() == False:
			return False

		logParser = SvnLogParser()
		if logParser.loadXml() == False:
			return False

		if self.genSvnInfoXml() == False:
			return False

		infoParser = SvnInfoParser()
		if infoParser.loadXml() == False:
			return False

		if self.genGitRepo(infoParser) == False:
			return False

		entryParser = SvnLogEntryParser()
		for entry in logParser.getLogEntrys():
			entryParser.setRootElement(entry)
			if self.svnCheckout(infoParser, entryParser) == False:
				return False

		return True

	def cmd_sync(self):
		return True

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

		if not os.path.exists(FILE_CMD_NAME):
			os.symlink(cmdAbsPath, FILE_CMD_NAME)

		if not os.path.exists(DIR_GIT_SVN):
			os.mkdir(DIR_GIT_SVN, 0777)

		print "url = %s, pathname = %s" % (url, pathname)

		self.mUrl = url

		if subcmd in ["init", "clone"]:
			return self.cmd_init()
		elif subcmd in ["update", "sync"]:
			return self.cmd_sync()
		else:
			stdio.pr_red_info("unknown subcmd " + subcmd)
			return False
