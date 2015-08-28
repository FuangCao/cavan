#!/usr/bin/env python

import sys, os
from cavan_adb import AdbManager

class AndroidManager(AdbManager):
	def __init__(self, verbose = True):
		buildTop = self.getEnv("ANDROID_BUILD_TOP")
		productOut = self.getEnv("ANDROID_PRODUCT_OUT")
		if not buildTop or not productOut:
			self.doRaise("please run 'source build/envsetup.sh && lunch'")

		self.mBuildTop = os.path.realpath(buildTop)
		self.mProductOut = os.path.realpath(productOut)
		self.mProductOutLen = len(self.mProductOut)

		AdbManager.__init__(self, self.mBuildTop, verbose)

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
		if not self.doRemount():
			return False

		lastDir = None

		for pathname in listFile:
			hostPath = self.getHostPath(pathname)
			if not os.path.exists(hostPath) and lastDir != None:
				hostPath = os.path.join(lastDir, pathname)

			devPath = self.getDevicePath(hostPath)

			if not self.doPush(hostPath, devPath):
				return False

			lastDir = os.path.dirname(hostPath)

		return True
