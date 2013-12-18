#!/usr/bin/python

import sys, os, threading, errno
from git_manager import CavanGitManager
from git_svn import GitSvnManager
from net_manager import CavanNetManager
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

	def findProjectTag(self, typeName, name):
		tagProject = self.getElementsByTagName(typeName)
		if not tagProject:
			return None

		for tag in tagProject:
			if tag.getAttribute("name") == name:
				return tag

		return None

	def removeProjectTag(self, typeName, name):
		count = 0
		tagProject = self.getElementsByTagName(typeName)
		if not tagProject:
			return count

		for tag in tagProject:
			if tag.getAttribute("name") == name:
				self.removeChild(tag)
				count = count + 1;

		return count

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

	def removeAllProject(self):
		self.removeAllChildByName("project")

	def removeAllFile(self):
		self.removeAllChildByName("file")

	def removeProject(self, name):
		return self.removeProjectTag("project", name)

	def removeFile(self, name):
		return self.removeProjectTag("file", name)

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

		self.mDepthMap = {}
		self.mDepthMap["build"] = 0
		self.mDepthMap["libcore"] = 0
		self.mDepthMap["pdk"] = 0
		self.mDepthMap["abi"] = 0
		self.mDepthMap["development"] = 0
		self.mDepthMap["bionic"] = 0
		self.mDepthMap["cts"] = 0
		self.mDepthMap["dalvik"] = 0
		self.mDepthMap["gdk"] = 0
		self.mDepthMap["sdk"] = 0
		self.mDepthMap["tools"] = 1
		self.mDepthMap["system"] = 1
		self.mDepthMap["external"] = 1
		self.mDepthMap["packages"] = 2
		self.mDepthMap["hardware"] = 2
		self.mDepthMap["device"] = 2
		self.mDepthMap["bootable"] = 2
		self.mDepthMap["frameworks"] = 2
		self.mDepthMap["frameworks/base"] = 0
		self.mDepthMap["prebuilts"] = 4

	def setRootPath(self, pathname):
		CavanCommandBase.setRootPath(self, pathname)

		self.mPathSvnRepo = self.getAbsPath(".svn_repo")
		if os.path.exists(self.mPathSvnRepo):
			self.mkdirSafe(self.mPathSvnRepo)

		self.mPathProjects = os.path.join(self.mPathSvnRepo, "projects")
		self.mFileManifest = os.path.join(self.mPathSvnRepo, "manifest.xml")
		self.mFileFailed = os.path.join(self.mPathSvnRepo, "failed.txt")
		self.mPathManifestRepo = os.path.join(self.mPathProjects, "platform/manifest.git")
		self.mPathFileRepo = os.path.join(self.mPathProjects, "platform/copyfile.git")

	def genProjectNode(self, depth = 3, path = ""):
		url = os.path.join(self.mUrl, path)
		lines = self.doPopen(["svn", "list", url])
		if not lines:
			return lines != None

		if self.mDepthMap.has_key(path):
			depth = self.mDepthMap[path]
			self.prGreenInfo("Depth of ", path, " is %d" % depth)
		elif depth > 0:
			basename = os.path.basename(path)
			if basename.startswith("lib"):
				depth = 0
				self.prGreenInfo(path, " is a library, set depth to %d" % depth)

		if depth < 1:
			return self.mManifest.appendProject(path) != None

		listDir = []
		listFile = []
		hasDotGit = False

		for line in lines:
			line = line.rstrip("\r\n")
			if line in [".git/"]:
				hasDotGit = True
				continue

			line = os.path.join(path, line)
			if line.endswith("/"):
				listDir.append(line.rstrip("/"))
			elif not path:
				listFile.append(line)
			else:
				return self.mManifest.appendProject(path) != None

		if hasDotGit and len(listDir) > 0:
			return self.mManifest.appendProject(path) != None

		for line in listFile:
			if not self.mManifest.appendFile(line):
				return False

		for line in listDir:
			if not self.genProjectNode(depth - 1, line):
				return False

		return True

	def genManifest(self, url):
		if os.path.exists(self.mFileManifest):
			if not self.loadManifest():
				return False
		else:
			self.mManifest = AndroidManifest()
			if not self.mManifest.create():
				return False

		if not url:
			url = self.mManifest.getUrl()
			if not url:
				self.prRedInfo("Url not found")
				return False
		else:
			self.mManifest.setUrl(url)

		self.mUrl = url
		self.mManifest.removeAllProject()
		self.mManifest.removeAllFile()

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
		self.setVerbose(True)

		length = len(argv)
		if length > 1:
			self.setRootPath(argv[1])

		if not os.path.isdir(self.mPathProjects):
			os.makedirs(self.mPathProjects)

		if length > 0:
			url = argv[0].rstrip("/")
		else:
			url = None

		return self.genManifest(url)

	def getTempProjectPath(self):
		for index in range(100):
			if self.mErrorCount > 0:
				break

			pathname = os.path.join(self.mPathTempProject, "%02d" % index)
			if not os.path.isdir(pathname):
				return pathname

		return None

	def fetchProjectBase(self, manager, name):
		url = os.path.join(self.mUrl, name)

		for count in range(2):
			if self.mErrorCount > 0:
				return -1

			if (manager.isInitialized() or manager.doInitBase(url)) and (manager.doSync(url) or manager.doDcommit(url)):
				self.addProgress()
				return 1

			if not self.doExecute(["svn", "info", url], of = "/dev/null"):
				return -1

			self.prRedInfo("Retry count = %d" % count)
			manager.removeSelf()

		self.mLockProject.acquire()
		tmpPathname = self.getTempProjectPath()
		self.mLockProject.release()

		if tmpPathname != None:
			if self.mVerbose:
				self.prRedInfo(url, " => ", tmpPathname)

			managerTemp = GitSvnManager(tmpPathname, self.mVerbose)
			if managerTemp.doInitBase(url) and managerTemp.doSync(url) and self.doExecute(["mv", managerTemp.getRootPath(), manager.getRootPath]):
				self.addProgress()
				return 1

			managerTemp.removeSelf()

		self.prRedInfo("Checkout ", manager.getRootPath(), " Failed")

		self.mLockProject.acquire()
		self.mErrorCount = self.mErrorCount + 1
		file_append_line(self.mFileFailed, "%s => %s" % (manager.getRootPath(), url))
		self.mLockProject.release()

		return -1

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

		relPath = self.getProjectRelPath(node)
		srcPath = self.getAbsPath(relPath)
		manager = GitSvnManager(srcPath, self.mVerbose)

		iResult = self.fetchProjectBase(manager, node[0])
		if iResult < 1:
			return iResult

		if not manager.isInitialized():
			self.mListEmpty.append(node)
			return iResult

		destPath = os.path.join(self.mPathProjects, relPath + ".git")
		relDestPath = self.getRelPath(destPath)
		relDestPath = os.path.join(self.getRelRoot(relPath), relDestPath)

		if not manager.doBackup(destPath):
			return -1

		return iResult

	def doSync(self):
		if not self.loadManifest():
			return False

		if os.path.exists(self.mFileFailed):
			os.remove(self.mFileFailed)

		self.mUrl = self.mManifest.getUrl()
		self.mListFile = self.mManifest.getFiles()
		self.mListProject = self.mManifest.getProjects()

		self.mListEmpty = []
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

		if len(self.mListEmpty) > 0:
			for node in self.mListEmpty:
				self.prBrownInfo("Remove empty project ", node[0], " from manifest.xml")
				self.mManifest.removeProject(node[0])
			self.mManifest.save(self.mFileManifest)

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

	def doCleanup(self):
		if not self.loadManifest():
			return False

		for node in self.mManifest.getProjects():
			pathname = self.getProjectAbsPath(node)
			self.prBrownInfo("Cleaning ", pathname)
			if not self.doGitClean(pathname):
				return False

		return True

	def genManifestRepo(self):
		manager = CavanGitManager(self.mPathManifestRepo, self.mVerbose, name = None)
		if not manager.genGitRepo():
			return False

		if not self.mManifest.save(os.path.join(self.mPathManifestRepo, "default.xml")):
			return False

		return manager.gitAutoCommit()

	def genFileRepo(self):
		manager = CavanGitManager(self.mPathFileRepo, self.mVerbose, name = None)
		if not manager.genGitRepo():
			return False

		listFile = self.mManifest.getFiles()
		if not listFile:
			return listFile != None

		nodeProject = self.mManifest.appendProject("platform/copyfile", self.getRelPath(self.mPathFileRepo))
		if not nodeProject:
			return False

		for node in listFile:
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

		return manager.gitAutoCommit()

	def gitPushProject(self, localPath, backupPath):
		managerLocal = CavanGitManager(localPath, self.mVerbose)
		managerBackup = CavanGitManager(backupPath, self.mVerbose, bare = True)
		for index in range(2):
			if not managerBackup.genGitRepo():
				return False

			if managerLocal.doPush([backupPath]):
				return True

			self.prRedInfo("Remove git repo ", backupPath)
			managerBackup.removeSelf();

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

	def getDefaultBackupPath(self):
		if os.access("/git", os.W_OK):
			gitRepoPath = "/git"
		else:
			homePath = os.getenv("HOME", None)
			if not homePath:
				self.prRedInfo("get HOME failed")
				return None
			gitRepoPath = os.path.join(homePath, "git")

		return os.path.join(gitRepoPath, os.path.basename(self.mPathRoot))

	def getDefaultGitRepoUrl(self):
		manager = CavanNetManager();
		ipList = manager.getLinkIpAddrList()
		if not ipList:
			self.prRedInfo("getLinkIpAddrList failed")
			return None
		return "git://%s:7777/%s" % (ipList[0], os.path.basename(self.mPathRoot))

	def doSymlink(self, argv):
		if not self.loadManifest():
			return False

		length = len(argv)

		if length > 0:
			self.mPathBackup = os.path.abspath(argv[0])
			self.mManifest.setBackup(self.mPathBackup)
		else:
			self.mPathBackup = self.mManifest.getBackup()

		if not self.mPathBackup:
			self.mPathBackup = self.getDefaultBackupPath()
			if not self.mPathBackup:
				self.prRedInfo("Please give symlink path")
				return False
			self.mManifest.setBackup(self.mPathBackup)
			self.prBrownInfo("Use default backup path ", self.mPathBackup)

		if length > 1:
			self.mUrlFetch = argv[1]
			self.mManifest.setFetch(self.mUrlFetch)
		else:
			self.mUrlFetch = self.mManifest.getFetch()

		if not self.mUrlFetch:
			self.mUrlFetch = self.getDefaultGitRepoUrl()
			if not self.mUrlFetch:
				self.prRedInfo("Please give fetch url")
				return False
			self.mManifest.setFetch(self.mUrlFetch)
			self.prBrownInfo("Use default fetch url ", self.mUrlFetch)

		self.setVerbose(True)
		self.mManifest.save(self.mFileManifest)

		if not os.path.isdir(self.mPathBackup):
			os.makedirs(self.mPathBackup)

		if not self.genFileRepo():
			self.prRedInfo("generate ", self.mPathFileRepo, " failed")
			return False

		if not self.genManifestRepo():
			self.prRedInfo("generate ", self.mPathManifestRepo, " failed")
			return False

		if os.path.exists(self.mPathBackup):
			self.removeSafe(self.mPathBackup)
		else:
			self.mkdirSafe(os.path.dirname(self.mPathBackup))

		os.symlink(self.mPathProjects, self.mPathBackup)

		return True

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
		elif subcmd in ["update", "sync"]:
			return self.doSync()
		elif subcmd in ["command", "cmd"]:
			return self.doCommand(argv[2:])
		elif subcmd in ["clean", "cleanup"]:
			return self.doCleanup()
		elif subcmd in ["backup", "ln", "link", "symlink"]:
			return self.doSymlink(argv[2:])
		else:
			self.prRedInfo("unknown subcmd ", subcmd)
			return False
