#!/usr/bin/python

import sys, os, threading
from git_svn import GitSvnManager
from cavan_xml import CavanXmlBase
from cavan_stdio import pr_red_info, pr_bold_info, pr_green_info
from cavan_command import CavanCommandBase, single_arg

MAX_THREAD_COUNT = 8

class AndroidManifest(CavanXmlBase):
	def load(self, pathname):
		if not CavanXmlBase.load(self, pathname):
			return False

		node = self.getFirstElement("default")
		if not node:
			return False
		self.mTagDefault = node

		node = self.getTagRemote()
		if not node:
			return False
		self.mTagRemote = node

		return True

	def create(self):
		if not CavanXmlBase.create(self):
			return False

		node = self.createElement("remote")
		if not node:
			return False
		self.appendChild(node)
		self.mTagRemote = node

		node = self.createElement("default")
		if not node:
			return False
		self.appendChild(node)
		self.mTagDefault = node

		self.setRemoteName("cavan-svn")
		self.setRevision("master")

		return True

	def getTagRemote(self, name = None):
		if not name:
			name = self.getRemoteName()
			if not name:
				return None

		tags = self.getElementsByTagName("remote")
		if not tags:
			return None

		for tag in tags:
			attrName = tag.getAttribute("name")
			if attrName == name:
				return tag
		return None

	def getTagDefault(self):
		node = self.getFirstElement("default")
		if not node:
			return None

		self.mTagDefault = node
		return node

	def getRevision(self):
		return self.mTagDefault.getAttribute("revision")

	def setRevision(self, revision):
		self.mTagDefault.setAttribute("revision", revision)

	def getRemoteName(self):
		return self.mTagDefault.getAttribute("remote")

	def setRemoteName(self, name):
		self.mTagRemote.setAttribute("name", name)
		self.mTagDefault.setAttribute("remote", name)

	def getFetch(self):
		return self.mTagRemote.getAttribute("fetch")

	def setFetch(self, fetch):
		return self.mTagRemote.setAttribute("fetch", fetch)

	def getReview(self):
		return self.mTagRemote.getAttribute("review")

	def setReview(self, review):
		return self.mTagRemote.setAttribute("review", review)

	def getUrl(self):
		return self.mTagRemote.getAttribute("url")

	def setUrl(self, url):
		return self.mTagRemote.setAttribute("url", url)

	def getBackup(self):
		return self.mTagDefault.getAttribute("backup")

	def setBackup(self, pathname):
		return self.mTagDefault.setAttribute("backup", pathname)

	def getProjectBase(self, typeName):
		listProject = []
		tagProject = self.getElementsByTagName(typeName)
		if not tagProject:
			return listProject

		for tag in tagProject:
			listProject.append((tag.getAttribute("name"), tag.getAttribute("path")))

		return listProject

	def getProjects(self):
		return self.getProjectBase("project")

	def getFiles(self):
		return self.getProjectBase("file")

	def appendProjectBase(self, typeName, name, path = None):
		node = self.createElement(typeName)
		if not node:
			return None

		node.setAttribute("name", name)
		if path != None:
			node.setAttribute("path", path)
		self.appendChild(node)

		return node

	def appendProject(self, name, path = None):
		if path == None:
			path = name
		return self.appendProjectBase("project", name, path)

	def appendFile(self, name, path = None):
		return self.appendProjectBase("file", name, path)

class CavanCheckoutThread(threading.Thread):
	def __init__(self, index, manager):
		threading.Thread.__init__(self)
		self.mIndex = index
		self.mRepoManager = manager
		self.mExitStatus = True

	def getExitStatus(self):
		return self.mExitStatus

	def run(self):
		while True:
			pr_green_info("Thread %d running" % self.mIndex)

			res = self.mRepoManager.syncProject()
			if res <= 0:
				if res < 0:
					self.mExitStatus = False
					pr_red_info("Thread %s fault" % self.mIndex)
				else:
					self.mExitStatus = True

				break

			pr_green_info("Thread %s complete" % self.mIndex)

		pr_bold_info("Thread %d exit" % self.mIndex)

class CavanGitSvnRepoManager(CavanCommandBase):
	def __init__(self, pathname = "."):
		CavanCommandBase.__init__(self, pathname)

	def setRootPath(self, pathname):
		CavanCommandBase.setRootPath(self, pathname)

		self.mPathSvnRepo = self.getAbsPath(".svn_repo")
		if not os.path.isdir(self.mPathSvnRepo):
			os.makedirs(self.mPathSvnRepo)

		self.mFileManifest = os.path.join(self.mPathSvnRepo, "manifest.xml")
		self.mPathManifestRepo = os.path.join(self.mPathSvnRepo, "manifest")
		self.mPathFileRepo = os.path.join(self.mPathSvnRepo, "copyfile")

	def genProjectNode(self, depth = 0, path = ""):
		url = os.path.join(self.mUrl, path)
		lines = self.doPathPopen("svn list %s" % single_arg(url))
		if not lines:
			return lines != None

		if depth > 2:
			return self.mManifest.appendProject(path) != None

		listDir = []
		listFile = []

		for line in lines:
			line = os.path.join(path, line.rstrip("\r\n"))
			if line.endswith("/"):
				listDir.append(line.rstrip("/"))
			elif depth <= 0:
				listFile.append(line)
			else:
				return self.mManifest.appendProject(path) != None

		for line in listFile:
			if not self.mManifest.appendFile(line):
				return False

		for line in listDir:
			if not self.genProjectNode(depth + 1, line):
				return False

		return True

	def genManifest(self, url):
		self.mManifest = AndroidManifest()
		if not self.mManifest.create():
			return False

		self.mManifest.setUrl(url)
		self.mUrl = url

		if not self.genProjectNode():
			return False

		return self.mManifest.save(self.mFileManifest)

	def loadManifest(self):
		self.mManifest = AndroidManifest()
		return self.mManifest.load(self.mFileManifest)

	def getManifest(self):
		return self.mManifest

	def doInit(self, argv):
		length = len(argv)
		if length > 0:
			url = argv[0].rstrip("/")
		elif os.path.exists(self.mFileManifest):
			if not self.loadManifest():
				return False
			url = self.mManifest.getUrl()
			if not url:
				pr_red_info("Url not found")
				return False
		else:
			pr_red_info("Please give repo url")
			return False

		if length > 1:
			self.setRootPath(argv[1])

		return self.genManifest(url)

	def syncProject(self):
		self.mLockProject.acquire()

		if len(self.mListProject) > 0:
			node = self.mListProject.pop(0)
		else:
			node = None

		self.mLockProject.release()

		if not node:
			return 0

		name = node[0]
		if len(node) < 2:
			pathname = name
		else:
			pathname = node[1]

		pathname = self.getAbsPath(pathname)
		url = os.path.join(self.mUrl, name)

		manager = GitSvnManager(pathname)
		if not manager.isInitialized() and not manager.doInitBase(url):
			return -1

		if not manager.doSync(url):
			return -1

		return 1

	def doSync(self):
		if not self.loadManifest():
			return False

		self.mUrl = self.mManifest.getUrl()

		listThread = []
		self.mListProject = self.mManifest.getProjects()
		self.mLockProject = threading.Lock()

		for index in range(MAX_THREAD_COUNT):
			thread = CavanCheckoutThread(index, self)
			if not thread:
				return False

			listThread.append(thread)

			thread.setDaemon(True)
			thread.start()

		for node in self.mManifest.getFiles():
			if not node[1]:
				pathname = node[0]
			else:
				pathname = node[1]

			pathname = self.getAbsPath(pathname)

			dirname = os.path.dirname(pathname)
			if not os.path.isdir(dirname):
				os.makedirs(dirname)

			url = os.path.join(self.mUrl, node[0])

			if not self.doPathExecute("svn export --force %s %s" % (single_arg(url), single_arg(pathname)), "/dev/null"):
				return False

		while True:
			res = self.syncProject()
			if res <= 0:
				break

		for thread in listThread:
			thread.join()

		for thread in listThread:
			if thread.getExitStatus() == False:
				return False

		return res == 0

	def doClone(self, argv):
		if len(argv) == 1:
			argv.append(os.path.basename(argv[0].rstrip("/")))

		if not self.doInit(argv):
			return False

		return self.doSync()

	def doCommand(self, argv):
		return False

	def genGitRepo(self, pathname, option = ""):
		if not os.path.isdir(pathname):
			os.makedirs(pathname)

		if self.doPathExecute("git branch", "/dev/null", pathname):
			return True

		if not self.doPathExecute("git init %s" % option, None, pathname):
			return False

		if not self.doPathExecute("git config user.name Fuang.Cao", None, pathname):
			return False

		if not self.doPathExecute("git config user.email cavan.cfa@gmail.com", None, pathname):
			return False

		return True

	def gitAutoCommit(self, pathname = None):
		return self.doPathExecute("git add -f . && git commit -asm 'auto commit by Fuang.Cao'", None, pathname)

	def genManifestRepo(self):
		if not self.genGitRepo(self.mPathManifestRepo):
			return False

		if not self.mManifest.save(os.path.join(self.mPathManifestRepo, "default.xml")):
			return False

		self.gitAutoCommit(self.mPathManifestRepo)
		return True

	def genFileRepo(self):
		nodeProject = self.mManifest.appendProject("platform/copyfile", self.getRelPath(self.mPathFileRepo))
		if not nodeProject:
			return False

		if not self.genGitRepo(self.mPathFileRepo):
			return False

		for node in self.mManifest.getFiles():
			if not node[1]:
				pathname = node[0]
			else:
				pathname = node[1]

			dirname = os.path.dirname(pathname)
			if not dirname:
				dirname = self.mPathFileRepo
			else:
				dirname = os.path.join(self.mPathFileRepo, dirname)
				if not os.path.isdir(dirname):
					os.makedirs(dirname)

			if not self.doExecute("cp -a %s %s" % (single_arg(self.getAbsPath(pathname)), single_arg(dirname))):
				return False

			copyfile = self.mManifest.createElement("copyfile")
			if not copyfile:
				return False

			copyfile.setAttribute("dest", pathname)
			copyfile.setAttribute("src", pathname)
			nodeProject.appendChild(copyfile)

		self.gitAutoCommit(self.mPathFileRepo)
		return True

	def doBackup(self, argv):
		if not self.loadManifest():
			return False

		length = len(argv)

		if length > 0:
			self.mPathBackup = os.path.abspath(argv[0])
			self.mManifest.setBackup(self.mPathBackup)
		else:
			self.mPathBackup = self.mManifest.getBackup()

		if not self.mPathBackup:
			pr_red_info("Please give backup path")
			return False

		if length > 1:
			self.mUrlFetch = argv[1]
			self.mManifest.setFetch(self.mUrlFetch)
		else:
			self.mUrlFetch = self.mManifest.getFetch()

		if not self.mUrlFetch:
			pr_red_info("Please give fetch url")
			return False

		self.mManifest.save(self.mFileManifest)

		if not os.path.isdir(self.mPathBackup):
			os.makedirs(self.mPathBackup)

		if not self.genFileRepo():
			return False

		if not self.genManifestRepo():
			return False

		listProject = self.mManifest.getProjects()
		listProject.append(("platform/manifest", self.mPathManifestRepo))
		for node in listProject:
			if not node[1]:
				localPath = node[0]
			else:
				localPath = node[1]

			localPath = self.getAbsPath(localPath)
			if not os.path.isdir(localPath):
				return False

			backupPath = os.path.join(self.mPathBackup, node[0] + ".git")
			if not self.genGitRepo(backupPath, "--shared --bare"):
				return False

			if not self.doPathExecute("git push --all %s" % single_arg(backupPath), None, localPath):
				return False

		return True

	def main(self, argv):
		length = len(argv)
		if length < 2:
			stdio.pr_red_info("Please give a subcmd")
			return False

		subcmd = argv[1]
		if subcmd in ["init"]:
			return self.doInit(argv[2:])
		if subcmd in ["clone"]:
			return self.doClone(argv[2:])
		elif subcmd in ["update", "sync"]:
			return self.doSync()
		elif subcmd in ["command", "cmd"]:
			return self.doCommand(argv[2:])
		elif subcmd in ["backup"]:
			return self.doBackup(argv[2:])
		else:
			pr_red_info("unknown subcmd " + subcmd)
			return False
