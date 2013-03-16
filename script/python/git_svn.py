#!/usr/bin/python

import sys, os, stdio
from command import command_vision
from getopt import getopt
from xml.dom.minidom import parse

FILE_SVN_LOG_XML = ".svn_log.xml"
FILE_SVN_INFO_XML = ".svn_info.xml"
FILE_SVN_LIST = ".svn_list.txt"
FILE_LIST_DIFF = ".svn_diff.txt"
FILE_LAST_REVISION = ".svn_revision.txt"

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

	def genGitRepo(self):
		if os.path.isdir(".git"):
			return True
		return command_vision("git init")

	def genFileList(self):
		return command_vision("svn list -R | sed '/\/\+\s*$/d' > %s" % FILE_SVN_LIST)

	def saveLastRevision(self, revision):
		return command_vision("echo %s > %s && git add -f %s" % (revision, FILE_LAST_REVISION, FILE_LAST_REVISION))

	def genListDiff(self):
		if command_vision("touch .git/%s && cp .git/%s . -av" % (FILE_SVN_LIST, FILE_SVN_LIST)) == False:
			return False
		if command_vision("git add %s -f" % FILE_SVN_LIST) == False:
			return False
		if self.genFileList() == False:
			return False
		if command_vision("git diff %s | grep '^+[^+]' | sed 's/^+//g' > %s" % (FILE_SVN_LIST, FILE_LIST_DIFF)) == False:
			return False
		return True

	def gitCommit(self, message):
		if (command_vision("git add -f $(cat %s)" % FILE_LIST_DIFF)) == False:
			fp = open(FILE_LIST_DIFF, "r")
			if not fp:
				return False

			lines = fp.readlines()
			fp.close()

			for line in lines:
				if command_vision("git add -f '%s'" % line.strip()) == False:
					return False

		return command_vision("git commit -asm \"%s\" && cp %s .git -av" % (message, FILE_SVN_LIST))

	def svnCheckout(self, reversion, message):
		if os.path.isdir(".svn"):
			cmd = "svn update -r %s" % reversion
		else:
			cmd = "svn checkout -r %s %s ." % (reversion, self.mUrl)

		if command_vision(cmd) == False:
			return False

		if self.genListDiff() == False:
			return False

		if self.saveLastRevision(reversion) == False:
			return False

		return self.gitCommit(message)

	def cmd_init(self):
		if self.genSvnLogXml() == False:
			return False

		if self.genGitRepo() == False:
			return False

		parser = SvnLogParser()
		if parser.loadXml() == False:
			return False

		entryParser = SvnLogEntryParser()
		for entry in parser.getLogEntrys():
			entryParser.setRootElement(entry)
			if self.svnCheckout(entryParser.getRevesion(), entryParser.getMessage()) == False:
				return False

		return True

	def main(self, argv):
		length = len(argv)
		if length < 2:
			stdio.pr_red_info("Too a few argument")
			return False

		subcmd = argv[1]

		if length > 2:
			url = argv[2]
		else:
			url = ""

		if length > 3:
			pathname = argv[3]
		else:
			pathname = os.path.basename(url.rstrip("/"))
		pathname = os.path.abspath(pathname)

		if not os.path.exists(pathname):
			os.makedirs(pathname, 0777)
		os.chdir(pathname)

		print "url = %s, pathname = %s" % (url, pathname)

		self.mUrl = url

		if subcmd in ["init", "clone"]:
			return self.cmd_init()
		else:
			stdio.pr_red_info("unknown subcmd " + subcmd)
			return False

if __name__ == "__main__":
	if len(sys.argv) > 1:
		manager = GitSvnManager()
		manager.main(sys.argv)
