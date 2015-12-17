#!/usr/bin/env python

import sys, os
from cavan_command import CavanCommandBase

class AdbManager(CavanCommandBase):
	def __init__(self, pathname = None, verbose = False):
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
			subcmd = args.pop(0)
			if subcmd in ["push"]:
				command = ["cavan-tcp_dd", "-wi", self.mHost, "-p", self.mPort]
			elif subcmd in ["shell"]:
				command = ["cavan-tcp_exec", "-i", self.mHost, "-p", self.mPort]
			else:
				return True

		command.extend(args)
		print command

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

	def doRemount(self):
		if not self.doRoot():
			return False

		return self.doAdbCommand(["remount"])

	def doPush(self, listFile, devPath = None):
		if not listFile:
			return True

		if not isinstance(listFile, list):
			if not devPath:
				return False
			return self.doAdbCommand(["push", listFile, devPath])

		if not devPath:
			devPath = listFile.pop()

		for pathname in listFile:
			target = os.path.join(devPath, os.path.basename(pathname))
			if not self.doAdbCommand(["push", pathname, target]):
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
