#!/usr/bin/python

import os, sys, getopt
from xml.dom.minidom import parse

def ShowUsage():
	print "Usage:"
	print "config option xml file path"
	print "-n, -N, --name: name.mk output path"
	print "-v, -V, --version: version.mk output path"
	print "-d, -D, --depend: depend.mk output path"
	print "-m, -M, --depend: mark path"
	print "-f, -F, --depend: install function"

def GenerateMakeFile():
	try:
		opts, args = getopt.getopt(sys.argv[1:], "n:N:d:D:v:V:m:M:f:F:", ["name=", "depend=", "version=", "markdir=", "function="])
	except:
		ShowUsage()
		sys.exit(-1)

	if len(args) < 1:
		ShowUsage()
		return -1

	NameMakefilePath = None
	VersionMakefilePath = None
	DependMakefilePath = None
	MarkDir = None
	InstallFunction = None

	for opt in opts:
		if opt[0] in ["-n", "-N", "--name"]:
			NameMakefilePath = opt[1]
		elif opt[0] in ["-d", "-D", "--depend"]:
			DependMakefilePath = opt[1]
		elif opt[0] in ["-v", "-V", "--version"]:
			VersionMakefilePath = opt[1]
		elif opt[0] in ["-m", "-M", "--markdir"]:
			MarkDir = opt[1]
		elif opt[0] in ["-f", "-F", "--function"]:
			InstallFunction = opt[1]
		else:
			print "unknown option: " + opt[0]
			return -1
	try:
		xmlConfig = parse(args[0])
		tagPackages = xmlConfig.getElementsByTagName("packages")
	except:
		print "parse file \"" + args[0] + "\" failed"
		return -1

	if NameMakefilePath == None or VersionMakefilePath == None or DependMakefilePath == None or MarkDir == None or InstallFunction == None:
		ShowUsage()
		return -1

	listAppNames = []
	listAppVersions = []
	listAppDepends = []

	for tagPackage in tagPackages[0].getElementsByTagName("package"):
		name = tagPackage.getAttribute("name")
		version = tagPackage.getAttribute("version")
		prefix = name.upper() + "_"
		lastname = prefix + "NAME"
		lastversion = prefix + "VERSION"

		listAppVersions.append(lastversion + " = " + version + "\n")

		listAppNames.append(lastname + " = " + name + "-$(" + lastversion + ")\n")
		listAppNames.append(prefix + "MARK = " + MarkDir + "/$(" + lastname + ")\n")
		listAppNames.append(prefix + "URL = " + tagPackage.getAttribute("url") + "\n")
		listAppNames.append(prefix + "CONFIG = " + tagPackage.getAttribute("config") + "\n")
		depends = prefix + "DEPEND ="
		for depend in tagPackage.getAttribute("depend").split(" "):
			depend = depend.strip()
			if depend:
				depends += " $(" + depend.upper() + "_MARK)"
		listAppNames.append(depends + "\n");
		listAppNames.append("PACKAGES_ALL += $(" + prefix + "MARK)\n\n")

		listAppDepends.append("$(" + prefix + "MARK): " + "$(" + prefix + "DEPEND)\n")
		listAppDepends.append("\t$(call " + InstallFunction + ",$(" + prefix + "CONFIG),$(" + prefix + "URL))\n\n")

	try:
		fd = open(NameMakefilePath, "w")
	except:
		print "failed to open file"
		return -1
	fd.writelines(listAppNames)
	fd.close()

	try:
		fd = open(VersionMakefilePath, "w")
	except:
		print "failed to open file"
		return -1
	fd.writelines(listAppVersions)
	fd.close()

	try:
		fd = open(DependMakefilePath, "w")
	except:
		print "failed to open file"
		return -1
	fd.writelines(listAppDepends)
	fd.close()

	return 0

if __name__ == "__main__":
	if GenerateMakeFile() < 0:
		sys.exit(-1)
