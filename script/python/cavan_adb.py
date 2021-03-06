#!/usr/bin/env python

import sys, os
from cavan_command import CavanCommandBase

class AdbManager(CavanCommandBase):
	def __init__(self, pathname = None, verbose = True):
		CavanCommandBase.__init__(self, pathname, verbose)
		self.setAdbDevice()
		self.mHost = self.getEnv("ADB_HOST")
		if not self.mHost:
			self.mHost = None

		self.mPort = self.getEnv("ADB_PORT")
		if not self.mPort:
			self.mPort = "9999"

		self.TempPath = "/data/local/tmp"
		self.ApkTempPath = os.path.join(self.TempPath, "cavan.apk")

	def setAdbDevice(self, device = None):
		self.mDevice = device

	def doAdbCommand(self, args):
		if not self.mHost:
			command = ["adb"]
			if self.mDevice != None:
				command.extend(["-s", self.mDevice])
		else:
			if self.mHost in ["local", "localhost"]:
				option = ["-a"]
			else:
				option = ["-i", self.mHost, "-p", self.mPort]

			subcmd = args.pop(0)
			if subcmd in ["push"]:
				command = ["cavan-tcp_dd", "-w"]
			elif subcmd in ["shell"]:
				command = ["cavan-tcp_exec"]
			else:
				return True

			command.extend(option)

		command.extend(args)

		return self.doExecute(command)

	def doAdbShell(self, command):
		if isinstance(command, list):
			command = " ".join(command)
		args = ["shell", command]
		return self.doAdbCommand(args)

	def doWaitForDevice(self):
		return self.doAdbCommand(["wait-for-device"])

	def doRoot(self):
		if not self.doWaitForDevice():
			return False

		if not self.doAdbCommand(["root"]):
			return False

		return self.doWaitForDevice()

        def disableVerity(self):
            return self.doAdbCommand(["disable-verity"])

	def doPushOnce(self, srcFile, destFile, destDir = None):
		if not destDir:
			destDir = os.path.dirname(destFile)
		else:
			destFile = os.path.join(destDir, destFile)

		if not self.doAdbShell("mkdir -p '%s'" % destDir):
			return False

		return self.doAdbCommand(["push", srcFile, destFile])

	def doPush(self, listFile, devPath = None):
		if not listFile:
			return True

		if not isinstance(listFile, list):
			if not devPath:
				return False
			return self.doPushOnce(listFile, devPath)

		if not devPath:
			devPath = listFile.pop()

		for pathname in listFile:
			if not self.doPushOnce(pathname, os.path.basename(pathname), devPath):
				return False

		return True

	def doInstall(self, listFile):
		if not listFile:
			return True

		if not isinstance(listFile, list):
			listFile = [ listFile ]

		for pathname in listFile:
			if not self.doPush(pathname, self.ApkTempPath):
				return False

			res = self.doAdbShell(["pm install -r", self.ApkTempPath])
			self.doAdbShell(["rm -f", self.ApkTempPath])
			if not res:
				return False

		return True

	def doRemount(self, listDir = None):
		# if not self.mHost:
			# self.mHost = "localhost"

		if not self.doRoot():
		    return False

                self.disableVerity()

		if not listDir:
		    return self.doAdbCommand(["remount"])

	        if not isinstance(listDir, list):
			listDir = [ listDir ]

		for pathname in listDir:
			if not self.doAdbShell("mount -o remount,rw " + pathname):
				return False

		return True
