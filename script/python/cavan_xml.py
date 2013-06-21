#!/usr/bin/python

import sys, xml.dom.minidom
from cavan_file import file_write_line

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

class CavanXmlBase:
	def __init__(self):
		reload(sys)
		sys.setdefaultencoding("utf-8")

	def load(self, pathname):
		dom = xml.dom.minidom.parse(pathname)
		if not dom:
			return False

		self.mDocument = dom
		self.mRootElement = dom.documentElement

		return True

	def create(self):
		dom = xml.dom.minidom.Document()
		if not dom:
			return False

		self.mDocument = dom

		node = self.createElement("manifest")
		if not node:
			return False

		dom.appendChild(node)

		self.mRootElement = node

		return True

	def save(self, pathname):
		content = self.mDocument.toprettyxml(encoding = "utf-8")
		if not content:
			return False

		fp = open(pathname, "w")
		if not fp:
			return False

		for line in content.split("\n"):
			line = line.rstrip()
			if not line:
				continue
			fp.write(line + "\n")

		fp.close()

		return True

	def getElementsByTagName(self, name, parent = None):
		if not parent:
			parent = self.mRootElement
		return parent.getElementsByTagName(name)

	def getFirstElement(self, name, parent = None):
		tags = self.getElementsByTagName(name, parent)
		if not tags:
			return None
		return tags[0]

	def getAttribute(self, name):
		return self.mRootElement.getAttribute(name)

	def createElement(self, name):
		return self.mDocument.createElement(name)

	def appendChild(self, node, parent = None):
		if not parent:
			parent = self.mRootElement
		parent.appendChild(node)

	def removeChild(self, node, parent = None):
		if not parent:
			parent = self.mRootElement
		parent.removeChild(node)

	def removeAllChildByName(self, name, parent = None):
		if not parent:
			parent = self.mRootElement

		for node in parent.getElementsByTagName(name):
			parent.removeChild(node)
