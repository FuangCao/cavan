#!/usr/bin/python

from cavan_xml import CavanXmlBase

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

	def getUrl(self):
		return self.mTagRemote.getAttribute("url")

	def setUrl(self, url):
		return self.mTagRemote.setAttribute("url", url)
	
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
		return self.appendProjectBase("project", name, path)

	def appendFile(self, name, path = None):
		return self.appendProjectBase("file", name, path)
