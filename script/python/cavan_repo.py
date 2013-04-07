#!/usr/bin/python

import sys, os, threading
from cavan_xml import CavanXmlBase
from cavan_stdio import pr_red_info, pr_bold_info, pr_green_info
from cavan_command import popen_to_list, command_vision, single_arg

MAX_THREAD_COUNT = 10

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

	def getProjects(self):
		dictProject = {}
		tagProject = self.getElementsByTagName("project")
		if not tagProject:
			return dictProject

		for tag in tagProject:
			dictProject[tag.getAttribute("name")] = tag.getAttribute("path")

		return dictProject

	def getFiles(self):
		dictFile = {}
		tagFile = self.getElementsByTagName("file")
		if not tagFile:
			return dictFile

		for tag in tagFile:
			dictFile[tag.getAttribute("name")] = tag.getAttribute("path")

		return dictFile

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

class CavanProjectDict:
	def __init__(self, dictProject):
		self.mDictProject = dictProject
		self.mLock = threading.Lock()

	def pop(self):
		self.mLock.acquire()

		try:
			node = self.mDictProject.popitem()
		except:
			node = None

		self.mLock.release()
		return node

class CavanCheckoutThread(threading.Thread):
	def __init__(self, index, manager, project):
		threading.Thread.__init__(self)
		self.mIndex = index
		self.mRepoManager = manager
		self.mDictProject = project
		self.mExitStatus = True

	def run(self):
		while True:
			node = self.mDictProject.pop()
			if not node:
				break

			name = node[0]
			if len(node) < 2:
				pathname = name
			else:
				pathname = node[1]

			pr_green_info("Thread %d checkout project %s" % (self.mIndex, name))

			pathname = self.mRepoManager.getAbsPath(pathname)
			url = os.path.join(self.mRepoManager.mUrl, name)

			if os.path.isdir(pathname):
				if not command_vision("cd %s && cavan-git-svn sync %s" % (single_arg(pathname), single_arg(url))):
					self.mExitStatus = False
					break
			else:
				if not command_vision("cavan-git-svn clone %s %s" % (single_arg(url), single_arg(pathname))):
					self.mExitStatus = False
					break

		if self.mExitStatus:
			pr_green_info("Thread %d exit" % self.mIndex)
		else:
			pr_red_info("Thread %d exit" % self.mIndex)

class CavanGitSvnRepoManager:
	def __init__(self):
		self.mPathRoot = os.path.abspath(".")
		self.mPathSvnRepo = ".svn_repo"
		self.mFileManifest = os.path.join(self.mPathSvnRepo, "manifest.xml")
		self.mPathManifestRepo = os.path.join(self.mPathSvnRepo, "manifest")
		self.mPathFileRepo = os.path.join(self.mPathSvnRepo, "copyfile")

	def getAbsPath(self, pathname):
		return os.path.join(self.mPathRoot, pathname)

	def genProjectNode(self, depth = 0, path = ""):
		url = os.path.join(self.mUrl, path)
		lines = popen_to_list("svn list %s" % single_arg(url))
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
			pathname = argv[1]
			if not os.path.exists(pathname):
				os.makedirs(pathname, 0777)

			os.chdir(pathname)
			self.mPathRoot = os.path.abspath(".")

		if not os.path.isdir(self.mPathSvnRepo):
			os.makedirs(self.mPathSvnRepo)

		return self.genManifest(url)

	def doSync(self):
		if not self.loadManifest():
			return False

		self.mUrl = self.mManifest.getUrl()

		listThread = []
		dictProject = CavanProjectDict(self.mManifest.getProjects())

		for index in range(MAX_THREAD_COUNT):
			thread = CavanCheckoutThread(index, self, dictProject)
			if not thread:
				return False

			listThread.append(thread)

			thread.setDaemon(True)
			thread.start()

		for thread in listThread:
			thread.join()

		for thread in listThread:
			if thread.mExitStatus == False:
				return False

		dictFile = self.mManifest.getFiles()
		for node in dictFile:
			url = os.path.join(self.mUrl, node)
			pathname = dictFile[node]
			if not pathname:
				pathname = node
			pathname = self.getAbsPath(pathname)
			if not command_vision("svn export --force %s %s > /dev/null" % (single_arg(url), single_arg(pathname))):
				return False

		return True

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

		os.chdir(pathname)

		if command_vision("git branch > /dev/null"):
			return True

		if not command_vision("git init %s" % option):
			return False

		if not command_vision("git config user.name Fuang.Cao"):
			return False

		if not command_vision("git config user.email cavan.cfa@gmail.com"):
			return False

		return True

	def gitAutoCommit(self):
		return command_vision("git add -f . && git commit -asm 'auto commit by Fuang.Cao'")

	def genManifestRepo(self):
		if not self.genGitRepo(self.getAbsPath(self.mPathManifestRepo)):
			return False

		if not self.mManifest.save("default.xml"):
			return False

		self.gitAutoCommit()
		return True

	def genFileRepo(self):
		nodeProject = self.mManifest.appendProject("platform/copyfile", self.mPathFileRepo)
		if not nodeProject:
			return False

		if not self.genGitRepo(self.getAbsPath(self.mPathFileRepo)):
			return False

		dictFile = self.mManifest.getFiles()
		for node in dictFile:
			pathname = dictFile[node]
			if not pathname:
				pathname = node

			dirname = os.path.dirname(pathname)
			if not dirname:
				dirname = "."
			elif not os.path.isdir(dirname):
				os.makedirs(dirname)

			if not command_vision("cp -a %s %s" % (single_arg(self.getAbsPath(pathname)), single_arg(dirname))):
				return False

			copyfile = self.mManifest.createElement("copyfile")
			if not copyfile:
				return False

			copyfile.setAttribute("dest", pathname)
			copyfile.setAttribute("src", pathname)
			nodeProject.appendChild(copyfile)

		self.gitAutoCommit()
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

		dictProject = self.mManifest.getProjects()
		dictProject["platform/manifest"] = self.mPathManifestRepo
		for node in dictProject:
			localPath = dictProject[node]
			if not localPath:
				localPath = node
			localPath = self.getAbsPath(localPath)
			if not os.path.isdir(localPath):
				return False

			backupPath = os.path.join(self.mPathBackup, node + ".git")
			if not self.genGitRepo(backupPath, "--shared --bare"):
				return False

			os.chdir(localPath)
			if not command_vision("git push --all %s" % single_arg(backupPath)):
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
