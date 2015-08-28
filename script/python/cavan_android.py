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
		self.mPathSystem = os.path.join(self.mProductOut, "system")

		AdbManager.__init__(self, self.mBuildTop, verbose)

		if self.mVerbose:
			self.prStdInfo("mBuildTop = " + self.mBuildTop)
			self.prStdInfo("mProductOut = " + self.mProductOut)

	def getHostPath(self, pathname):
		if not os.path.exists(pathname):
			if os.path.isabs(pathname):
				self.doRaise("file %s is not exists" % pathname)
			else:
				if pathname.startswith("out"):
					dirname = self.mBuildTop
				else:
					dirname = self.mProductOut

				newPath = os.path.join(dirname, pathname)

				if os.path.exists(newPath):
					pathname = newPath
				elif not os.path.dirname(pathname):
					if self.mVerbose:
						self.prStdInfo("Try find from: ", self.mPathSystem)

					files = self.doFindFile(self.mPathSystem, pathname)
					if not files:
						self.doRaise("file %s is not exists" % pathname)
					elif len(files) > 1:
						for fn in files:
							self.prRedInfo("pathname = " + fn)
						self.doRaise("too much file named %s" % pathname)
					else:
						pathname = files[0]
				else:
					self.doRaise("file %s is not exists" % pathname)

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

			self.doSymlink(hostPath)

			lastDir = os.path.dirname(hostPath)

		return True
