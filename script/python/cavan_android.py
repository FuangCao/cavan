#!/usr/bin/env python

import sys, os
from cavan_command import CavanCommandBase

class AndroidManager(CavanCommandBase):
	def __init__(self, verbose = True):
		pathBuildTop = self.getEnv("ANDROID_BUILD_TOP")
		pathProductOut = self.getEnv("ANDROID_PRODUCT_OUT")
		if not pathBuildTop or not pathProductOut:
			self.doRaise("please run 'source build/envsetup.sh && lunch'")

		self.mBuildTop = os.path.realpath(pathBuildTop)
		self.mProductOut = os.path.realpath(pathProductOut)
		self.mProductOutLen = len(self.mProductOut)

		CavanCommandBase.__init__(self, self.mBuildTop, verbose)

		if self.mVerbose:
			self.prStdInfo("mBuildTop = " + self.mBuildTop)
			self.prStdInfo("mProductOut = " + self.mProductOut)

	def getHostPath(self, pathname):
		if not os.path.exists(pathname) and not os.path.isabs(pathname):
			if pathname.startswith("out"):
				dirname = self.mBuildTop
			else:
				dirname = self.mProductOut

			pathname = os.path.join(dirname, pathname)

		return os.path.realpath(pathname)

	def getDevicePath(self, pathname):
		if pathname.startswith(self.mProductOut):
			return pathname[self.mProductOutLen:]

		return pathname

	def push(self, listFile):
		for pathname in listFile:
			pathHost = self.getHostPath(pathname)
			pathDevice = self.getDevicePath(pathHost)

			if not self.doExecute(["adb", "push", pathHost, pathDevice]):
				return False

		return True
