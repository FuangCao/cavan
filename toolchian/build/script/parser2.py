#!/usr/bin/python

import os, sys, getopt
import cavan_xml, cavan_command

class CavanPackage(cavan_xml.CavanXmlBase):
	def __init__(self, element):
		self.mRootElement = element

	def getName(self):
		return self.getAttribute("name")

	def getUpperName(self):
		return self.getName().upper()

	def getVersion(self):
		return self.getAttribute("version")

	def getType(self):
		return self.getAttributeSafe("type")

	def getUrl(self):
		return self.getAttributeSafe("url")

	def getDepend(self):
		depends = self.getAttribute("depend")
		if not depends:
			return None

		dependList = None

		for depend in depends.split(" "):
			depend = "$(%s_MARK)" % depend.upper()
			if not dependList:
				dependList = depend
			else:
				dependList = dependList + " " + depend

		return dependList

	def getAction(self):
		node = self.mRootElement.firstChild
		if not node or node.nodeType != node.TEXT_NODE:
			return None

		text = node.wholeText.strip()
		if not text:
			return None

		listAction = []
		for action in text.split("\n"):
			action = action.strip()
			if not action:
				continue
			listAction.append(action)

		return listAction

	def getConfig(self):
		return self.getAttributeSafe("config")

class CavanPackageXml(cavan_xml.CavanXmlBase):
	def __init__(self, pathname):
		self.load(pathname)

	def getPackageList(self):
		packages = self.getElementsByTagName("package")
		if not packages:
			return []
		listPackage = []
		for package in packages:
			listPackage.append(CavanPackage(package))
		return listPackage

	def getMarkPath(self):
		return self.getAttribute("mark_path")

	def getMarkName(self):
		return self.getAttributeSafe("mark")

	def getOutPath(self):
		return self.getAttribute("out")

	def getSourcePath(self):
		return self.getAttributeSafe("source")

	def getBuildMethod(self):
		return self.getAttribute("build")

	def getName(self):
		return self.getAttribute("name")

	def getUpperName(self):
		return self.getName().upper()

class CavanPackageParser:
	def showUsage(self, command):
		print "Usage: %s [package | depend] [pathname]" % command
		return False

	def write(self, data):
		return self.mFp.write(data)

	def writelines(self, lines):
		for line in lines:
			self.mFp.write(line + "\n")

	def genPackageMakeFile(self):
		for package in self.mXml.getPackageList():
			upperName = package.getUpperName()
			self.write("%s_VERSION ?= %s\n" % (upperName, package.getVersion()))
			self.write("%s_NAME = %s-$(%s_VERSION)\n" % (upperName, package.getName(), upperName))
			self.write("%s_TYPE = %s\n" % (upperName, package.getType()))
			self.write("%s_URL = %s\n\n" % (upperName, package.getUrl()))

		return True

	def genDownloadMakeFile(self):
		for package in self.mXml.getPackageList():
			upperName = package.getUpperName()
			self.write("%s_VERSION ?= %s\n" % (upperName, package.getVersion()))
			self.write("%s_NAME = %s-$(%s_VERSION)\n" % (upperName, package.getName(), upperName))
			self.write("%s_TYPE = %s\n" % (upperName, package.getType()))
			self.write("%s_URL = %s\n" % (upperName, package.getUrl()))
			self.write("%s_MARK = $(MARK_DOWNLOAD)/$(%s_NAME)\n" % (upperName, upperName))
			self.write("$(%s_MARK):\n\t$(call download_package,$(%s_NAME),$(%s_URL),$(%s_TYPE))\n\n" % (upperName, upperName, upperName, upperName))

		return True

	def genBuildMakeFile(self):
		self.write("OUT_PATH := %s\n" % self.mXml.getOutPath())
		self.write("MARK_PATH := %s\n" % self.mXml.getMarkPath())

		source = self.mXml.getSourcePath()
		if not not source:
			self.write("SOURCE_PATH := %s\n" % source)

		self.write("BUILD := %s\n" % self.mXml.getBuildMethod())

		markName = self.mXml.getMarkName()
		if not markName:
			markName = ready
		self.write("MARK_READY := $(MARK_PATH)/%s\n\n" % markName)

		listMark = []
		listDepend = []

		for package in self.mXml.getPackageList():
			upperName = package.getUpperName()
			markName = upperName + "_MARK"
			self.write("%s := $(MARK_PATH)/$(%s_NAME)\n" % (markName, upperName))
			self.write("%s_CONFIG := %s\n" % (upperName, package.getConfig()))

			depend = package.getDepend()
			if not not depend:
				listDepend.append("$(%s): %s" % (markName, depend))

			self.write("$(%s):\n" % markName)
			actions = package.getAction()
			if not actions:
				self.write("\t$(Q)$(call $(BUILD))\n")
			else:
				for action in actions:
					self.write("\t$(Q)" + action + "\n")
			self.write("\n")

			listMark.append(markName)

		self.writelines(listDepend)

		self.write("\nall $(MARK_READY):")
		for mark in listMark:
			self.write(" $(%s)" % mark)
		self.write("\n")

	def main(self):
		argv = sys.argv
		argc = len(argv)
		if argc < 4:
			return self.showUsage(argv[0])

		subcmd = argv[1]
		if subcmd in ["package"]:
			action = self.genPackageMakeFile
		elif subcmd in ["download"]:
			action = self.genDownloadMakeFile
		elif subcmd in ["build"]:
			action = self.genBuildMakeFile
		else:
			return self.showUsage(argv[0])

		self.mFp = open(argv[3], "w")
		if not self.mFp:
			return False

		self.mXml = CavanPackageXml(argv[2])
		result = action()

		self.mFp.close()

		return result

if __name__ == "__main__":
	parser = CavanPackageParser()
	parser.main()
