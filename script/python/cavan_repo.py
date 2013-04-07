#!/usr/bin/python

import sys, os
from git_svn import GitSvnManager
from cavan_xml import CavanXmlBase
from cavan_stdio import pr_red_info
from cavan_command import popen_to_list, command_vision, single_arg

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
			return False

		node.setAttribute("name", name)
		if path != None:
			node.setAttribute("path", path)
		self.appendChild(node)

		return True

	def appendProject(self, name, path = None):
		if path == None:
			path = name
		return self.appendProjectBase("project", name, path)

	def appendFile(self, name, path = None):
		return self.appendProjectBase("file", name, path)

class CavanGitSvnRepoManager:
	def __init__(self):
		self.mRootPath = os.path.abspath(".")
		self.mPathSvnRepo = ".svn_repo"
		self.mFileManifest = os.path.join(self.mPathSvnRepo, "manifest.xml")

	def genProjectNode(self, depth = 0, path = ""):
		url = os.path.join(self.mUrl, path)
		lines = popen_to_list("svn list %s" % single_arg(url))
		if not lines:
			return lines != None

		if depth > 2:
			self.mManifest.appendProject(path)
			return True

		listDir = []
		listFile = []

		for line in lines:
			line = os.path.join(path, line.rstrip("\r\n"))
			if line.endswith("/"):
				listDir.append(line.rstrip("/"))
			elif depth <= 0:
				listFile.append(line)
			else:
				self.mManifest.appendProject(path)
				return True

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

		if not os.path.isdir(self.mPathSvnRepo):
			os.makedirs(self.mPathSvnRepo)

		return self.genManifest(url)

	def doSync(self):
		if not self.loadManifest():
			return False

		self.mUrl = self.mManifest.getUrl()

		dictProject = self.mManifest.getProjects()
		for node in dictProject:
			url = os.path.join(self.mUrl, node)
			pathname = dictProject[node]
			if not pathname:
				pathname = node

			pathname = os.path.join(self.mRootPath, pathname)
			if not os.path.isdir(pathname):
				os.makedirs(pathname)
			os.chdir(pathname)

			manager = GitSvnManager()
			if manager.isInitialized():
				if not manager.doSync(url):
					return False
			elif not manager.doInitBase(url, None):
					return False

		os.chdir(self.mRootPath)

		dictFile = self.mManifest.getFiles()
		for node in dictFile:
			url = os.path.join(self.mUrl, node)
			pathname = dictFile[node]
			if not pathname:
				pathname = node
			pathname = pathname
			if not command_vision("svn export --force %s %s > /dev/null" % (single_arg(url), single_arg(pathname))):
				return False

		return True

	def doCommand(self, argv):
		return False

	def doBackup(self, argv):
		if not self.loadManifest():
			return False

		if len(argv) < 1:
			self.mBackupPath = self.mManifest.getBackup()
			if not self.mBackupPath:
				pr_red_info("Please give backup path")
				return False
		else:
			self.mBackupPath = os.path.abspath(argv[0])
			self.mManifest.setBackup(self.mBackupPath)
			self.mManifest.save(self.mFileManifest)

		if not os.path.isdir(self.mBackupPath):
			os.makedirs(self.mBackupPath)

		dictProject = self.mManifest.getProjects()
		for node in dictProject:
			localPath = dictProject[node]
			if not localPath:
				localPath = node
			localPath = os.path.join(self.mRootPath, localPath)
			if not os.path.isdir(localPath):
				return False

			backupPath = os.path.join(self.mBackupPath, node)
			if not os.path.exists(os.path.join(backupPath, "HEAD")):
				if not os.path.isdir(backupPath):
					os.makedirs(backupPath)
				os.chdir(backupPath)
				if not command_vision("git init --shared --bare"):
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
		if subcmd in ["init", "clone"]:
			return self.doInit(argv[2:])
		elif subcmd in ["update", "sync"]:
			return self.doSync()
		elif subcmd in ["command", "cmd"]:
			return self.doCommand(argv[2:])
		elif subcmd in ["backup"]:
			return self.doBackup(argv[2:])
		else:
			pr_red_info("unknown subcmd " + subcmd)
			return False
