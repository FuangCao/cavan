#!/usr/bin/env python

from cavan_command import CavanCommandBase

class AdbManager(CavanCommandBase):
	def __init__(self, pathname, verbose = False):
		CavanCommandBase.__init__(self, pathname, verbose)
		self.setAdbDevice()

	def setAdbDevice(self, device = None):
		self.mDevice = device

	def doAdbCommand(self, args):
		command = ["adb"]
		if self.mDevice != None:
			command.extend(["-s", self.mDevice])

		command.extend(args)

		return self.doExecute(command)

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

	def doPush(self, hostPath, devPath):
		return self.doAdbCommand(["push", hostPath, devPath])

