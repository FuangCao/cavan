#!/usr/bin/python

import os, sys, getopt
sys.path.append("../../../script/python")
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

class CavanPackageParser(cavan_command.CavanCommandBase):
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

	def GenerateMakeFile(self):
		try:
			opts, args = getopt.getopt(sys.argv[1:], "n:N:d:D:v:V:m:M:f:F:o:O:", ["name=", "depend=", "markdir=", "function=", "output="])
		except:
			ShowUsage()
			sys.exit(-1)

		if len(args) < 1:
			ShowUsage()
			return -1

		NameMakefilePath = None
		DependMakefilePath = None
		MarkDir = None
		InstallFunction = None

		for opt in opts:
			if opt[0] in ["-n", "-N", "--name"]:
				NameMakefilePath = opt[1]
			elif opt[0] in ["-d", "-D", "--depend"]:
				DependMakefilePath = opt[1]
			elif opt[0] in ["-m", "-M", "--markdir"]:
				MarkDir = opt[1]
			elif opt[0] in ["-f", "-F", "--function"]:
				InstallFunction = opt[1]
			elif opt[0] in ["-o", "-O", "--output"]:
				NameMakefilePath = opt[1] + "/name.mk"
				DependMakefilePath = opt[1] + "/depend.mk"
			else:
				print "unknown option: " + opt[0]
				return -1
		try:
			xmlConfig = parse(args[0])
			tagPackages = xmlConfig.getElementsByTagName("packages")
		except:
			print "parse file \"" + args[0] + "\" failed"
			return -1

		if NameMakefilePath == None or DependMakefilePath == None or MarkDir == None or InstallFunction == None:
			ShowUsage()
			return -1

		listAppNames = []
		listAppDepends = []

		for tagPackage in tagPackages[0].getElementsByTagName("package"):
			name = tagPackage.getAttribute("name")
			version = tagPackage.getAttribute("version")
			prefix = name.upper() + "_"
			lastname = prefix + "NAME"
			lastversion = prefix + "VERSION"

			listAppNames.append(lastversion + " ?= " + version + "\n")
			listAppNames.append(lastname + " = " + name + "-$(" + lastversion + ")\n")
			listAppNames.append(prefix + "MARK = " + MarkDir + "/$(" + lastname + ")\n")
			listAppNames.append(prefix + "URL = " + tagPackage.getAttribute("url") + "\n")
			listAppNames.append(prefix + "CONFIG = " + tagPackage.getAttribute("config") + "\n")
			listAppNames.append(prefix + "TYPE = " + tagPackage.getAttribute("type") + "\n")
			depends = prefix + "DEPEND ="
			for depend in tagPackage.getAttribute("depend").split(" "):
				depend = depend.strip()
				if depend:
					depends += " $(" + depend.upper() + "_MARK)"
			listAppNames.append(depends + "\n");
			listAppNames.append("PACKAGES_ALL += $(" + prefix + "MARK)\n\n")

			listAppDepends.append("$(" + prefix + "MARK): " + "$(" + prefix + "DEPEND)\n")
			listAppDepends.append("\t$(call %s,$(%sCONFIG),$(%sURL),$(%sTYPE))\n\n" % (InstallFunction, prefix, prefix, prefix))

		listAppNames.append("all: $(PACKAGES_ALL)")

		try:
			fd = open(NameMakefilePath, "w")
		except:
			print "failed to open file \"" + NameMakefilePath + "\""
			return -1
		fd.writelines(listAppNames)
		fd.close()

		try:
			fd = open(DependMakefilePath, "w")
		except:
			print "failed to open file \"" + DependMakefilePath + "\""
			return -1
		fd.writelines(listAppDepends)
		fd.close()

		return 0

if __name__ == "__main__":
	parser = CavanPackageParser()
	parser.main()
