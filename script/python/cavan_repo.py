#!/usr/bin/python

import sys, os, threading
from git_svn import GitSvnManager
from cavan_file import file_append_line
from cavan_xml import CavanXmlBase
from cavan_command import CavanCommandBase
from cavan_progress import CavanProgressBar

MAX_THREAD_COUNT = 5

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

	def run(self):
		iResult = 1
		manager = self.mRepoManager

		while iResult > 0:
			if manager.mVerbose:
				manager.prGreenInfo("Thread %d running" % self.mIndex)
			iResult = self.mRepoManager.fetchProject()

		if manager.mVerbose:
			if iResult < 0:
				manager.prRedInfo("Thread %s fault" % self.mIndex)
			else:
				manager.prGreenInfo("Thread %s complete" % self.mIndex)

			manager.prBoldInfo("Thread %d exit" % self.mIndex)

		return iResult

class CavanGitSvnRepoManager(CavanCommandBase, CavanProgressBar):
	def __init__(self, pathname = ".", verbose = False):
		CavanCommandBase.__init__(self, pathname, verbose)
		CavanProgressBar.__init__(self)

		self.mErrorCount = 0
		self.mLockProject = threading.Lock()
		self.mPathTempProject = "/tmp/cavan-git-svn-repo"

	def setRootPath(self, pathname):
		CavanCommandBase.setRootPath(self, pathname)

		self.mPathSvnRepo = self.getAbsPath(".svn_repo")
		self.mFileManifest = os.path.join(self.mPathSvnRepo, "manifest.xml")
		self.mFileFailed = os.path.join(self.mPathSvnRepo, "failed.txt")
		self.mPathManifestRepo = os.path.join(self.mPathSvnRepo, "manifest")
		self.mPathFileRepo = os.path.join(self.mPathSvnRepo, "copyfile")

	def genProjectNode(self, depth = 0, path = ""):
		url = os.path.join(self.mUrl, path)
		lines = self.doPopen(["svn", "list", url])
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

	def getProjectRelPath(self, nodeProject):
		if not nodeProject[1]:
			return nodeProject[0]
		else:
			return nodeProject[1]

	def getProjectAbsPath(self, nodeProject):
		pathname = self.getProjectRelPath(nodeProject)
		return self.getAbsPath(pathname)

	def doInit(self, argv):
		length = len(argv)
		if length > 0:
			url = argv[0].rstrip("/")
		elif os.path.exists(self.mFileManifest):
			if not self.loadManifest():
				return False
			url = self.mManifest.getUrl()
			if not url:
				self.prRedInfo("Url not found")
				return False
		else:
			self.prRedInfo("Please give repo url")
			return False

		if length > 1:
			self.setRootPath(argv[1])

		if not os.path.isdir(self.mPathSvnRepo):
			os.makedirs(self.mPathSvnRepo)

		self.setVerbose(True)

		return self.genManifest(url)

	def getTempProjectPath(self):
		for index in range(100):
			if self.mErrorCount > 0:
				break

			pathname = os.path.join(self.mPathTempProject, "%02d" % index)
			if not os.path.isdir(pathname):
				return pathname

		return None

	def fetchProject(self):
		self.mLockProject.acquire()
		length = len(self.mListProject)
		if self.mErrorCount == 0 and length > 0:
			node = self.mListProject.pop(0)
		else:
			node = None
		self.mLockProject.release()

		if not node:
			return 0

		pathname = self.getProjectAbsPath(node)
		url = os.path.join(self.mUrl, node[0])

		for count in range(2):
			if self.mErrorCount > 0:
				return -1

			if self.mVerbose:
				self.prBoldInfo(url, " => ", pathname)

			manager = GitSvnManager(pathname, self.mVerbose)
			if (manager.isInitialized() or manager.doInitBase(url)) and manager.doSync(url):
				self.addProgress()
				return 1

			self.prRedInfo("Retry count = %d" % count)
			self.doExecute(["rm", "-rf", pathname])

		self.mLockProject.acquire()
		tmpPathname = self.getTempProjectPath()
		self.mLockProject.release()

		if tmpPathname != None:
			if self.mVerbose:
				self.prRedInfo(url, " => ", tmpPathname)

			manager = GitSvnManager(tmpPathname, self.mVerbose)
			if manager.doInitBase(url) and manager.doSync(url) and self.doExecute(["mv", tmpPathname, pathname]):
				self.addProgress()
				return 1
			self.doExecute(["rm", "-rf", tmpPathname])

		self.prRedInfo("Checkout ", pathname, " Failed")

		self.mLockProject.acquire()
		self.mErrorCount = self.mErrorCount + 1
		file_append_line(self.mFileFailed, "%s => %s" % (pathname, url))
		self.mLockProject.release()

		return -1

	def doSync(self):
		if not self.loadManifest():
			return False

		if os.path.exists(self.mFileFailed):
			os.remove(self.mFileFailed)

		self.mUrl = self.mManifest.getUrl()
		self.mListFile = self.mManifest.getFiles()
		self.mListProject = self.mManifest.getProjects()

		listThread = []

		for index in range(MAX_THREAD_COUNT):
			thread = CavanCheckoutThread(index, self)
			if not thread:
				return False
			listThread.append(thread)

		self.initProgress(len(self.mListProject) + len(self.mListFile))

		for thread in listThread:
			thread.setDaemon(True)
			thread.start()

		iResult = 1

		for node in self.mListFile:
			pathname = self.getProjectAbsPath(node)
			dirname = os.path.dirname(pathname)
			if not os.path.isdir(dirname):
				os.makedirs(dirname)

			url = os.path.join(self.mUrl, node[0])

			if not self.doExecute(["svn", "export", "--force", url, pathname], of = "/dev/null"):
				iResult = -1
				break

			self.addProgress()

		while iResult > 0:
			iResult = self.fetchProject()

		for thread in listThread:
			thread.join()

		if iResult < 0 or self.mErrorCount > 0:
			return False

		self.finishProgress()

		return True

	def doClone(self, argv):
		if len(argv) == 1:
			argv.append(os.path.basename(argv[0].rstrip("/")))

		if not self.doInit(argv):
			return False

		return self.doSync()

	def doCommand(self, argv):
		if len(argv) < 1:
			self.prBlueInfo("Too a few argument")
			return False

		if not self.loadManifest():
			return False

		commandRaw = " ".join(argv)

		for node in self.mManifest.getProjects():
			pathname = self.getProjectAbsPath(node)
			command = commandRaw.replace("<path>", pathname).replace("<name>", node[0]).replace("<all>", "*")
			self.prBrownInfo(command, " => ", pathname)
			if not self.doSystemExec(command, cwd = pathname):
				return False

		return True

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
			pathname = self.getProjectRelPath(node)
			dirname = os.path.dirname(pathname)
			if not dirname:
				dirname = self.mPathFileRepo
			else:
				dirname = os.path.join(self.mPathFileRepo, dirname)
				if not os.path.isdir(dirname):
					os.makedirs(dirname)

			if not self.doExecute(["cp", "-a", pathname, dirname]):
				return False

			copyfile = self.mManifest.createElement("copyfile")
			if not copyfile:
				return False

			copyfile.setAttribute("dest", pathname)
			copyfile.setAttribute("src", pathname)
			nodeProject.appendChild(copyfile)

		self.gitAutoCommit(self.mPathFileRepo)
		return True

	def gitPushProject(self, localPath, backupPath):
		for index in range(2):
			if not self.genGitRepo(backupPath, ["--shared", "--bare"]):
				return False

			if self.doExecute(["git", "push", "--all", backupPath], cwd = localPath):
				return True

			self.prRedInfo("Remove git repo ", backupPath)
			self.doExecute(["rm", "-rf", backupPath])

		return False

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
			self.prRedInfo("Please give backup path")
			return False

		if length > 1:
			self.mUrlFetch = argv[1]
			self.mManifest.setFetch(self.mUrlFetch)
		else:
			self.mUrlFetch = self.mManifest.getFetch()

		if not self.mUrlFetch:
			self.prRedInfo("Please give fetch url")
			return False

		self.setVerbose(True)
		self.mManifest.save(self.mFileManifest)

		if not os.path.isdir(self.mPathBackup):
			os.makedirs(self.mPathBackup)

		if not self.genFileRepo():
			return False

		if not self.genManifestRepo():
			return False

		listProject = self.mManifest.getProjects()
		listProject.insert(0, ("platform/manifest", self.mPathManifestRepo))
		for node in listProject:
			localPath = self.getProjectAbsPath(node)
			if not os.path.isdir(localPath):
				return False

			backupPath = os.path.join(self.mPathBackup, node[0] + ".git")
			if not self.gitPushProject(localPath, backupPath):
				return False

		return True

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
		elif subcmd in ["update", "sync"]:
			return self.doSync()
		elif subcmd in ["command", "cmd"]:
			return self.doCommand(argv[2:])
		elif subcmd in ["backup"]:
			return self.doBackup(argv[2:])
		else:
			self.prRedInfo("unknown subcmd ", subcmd)
			return False
