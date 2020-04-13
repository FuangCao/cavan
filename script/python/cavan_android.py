#!/usr/bin/env python

import sys, os
from cavan_adb import AdbManager

class AndroidManager(AdbManager):
	def __init__(self, verbose = True):
		buildTop = self.getEnv("ANDROID_BUILD_TOP")
		productOut = self.getEnv("ANDROID_PRODUCT_OUT")
		if not buildTop or not productOut:
			self.doRaise("please run 'source build/envsetup.sh && lunch'")

		self.mTargetProduct = self.getEnv("TARGET_PRODUCT")

		self.mBuildTop = os.path.realpath(buildTop)
		self.mProductOut = os.path.realpath(productOut)
		self.mProductOutLen = len(self.mProductOut)
		self.mPathSystem = os.path.join(self.mProductOut, "system")
                self.mPathVendor = os.path.join(self.mProductOut, "vendor")

		AdbManager.__init__(self, self.mBuildTop, verbose)

		if self.mVerbose:
			self.prStdInfo("mBuildTop = " + self.mBuildTop)
			self.prStdInfo("mProductOut = " + self.mProductOut)

        def chroot(self):
            try:
                os.chdir(self.mBuildTop)
            except:
                return False

            return True

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
			pathname = pathname[self.mProductOutLen:]
		elif self.mTargetProduct != None:
			index = pathname.find("/" + self.mTargetProduct + "/")
			if (index < 0):
				return pathname
			pathname = pathname[index + len(self.mTargetProduct) + 1:]
		else:
			index = pathname.find("/system/")
			if index < 0:
				return pathname
			pathname = pathname[index:]

		if pathname.startswith("/symbols"):
			return pathname[8:]

		return pathname

        def IsPushAble(self, pathname):
            for name in ["/system/", "/vendor/", "/data/", "/cache/"]:
                if pathname.startswith(name):
                    return True

            return False

	def checkFileList(self, listFile):
		if not listFile:
			return []

		dictFile = {}

		for pathname in listFile:
			pathname = os.path.realpath(pathname)

                        if not pathname.startswith(self.mProductOut):
                            self.prRedInfo("skipping: " + pathname)
                            continue

                        filename = pathname[len(self.mProductOut):]

                        if not self.IsPushAble(filename):
                            self.prStdInfo("skipping: " + filename)
                            continue

			if dictFile.has_key(filename):
				self.prBrownInfo("Override: %s <= %s" % (dictFile[filename], pathname))
			dictFile[filename] = pathname

		return dictFile.values()

	def push(self, listFile):
                if not self.chroot():
                    return False

		if not self.doRemount():
			return False

		lastDir = None

		for pathname in self.checkFileList(listFile):
			hostPath = self.getHostPath(pathname)
			if not os.path.exists(hostPath) and lastDir != None:
				hostPath = os.path.join(lastDir, pathname)

			devPath = self.getDevicePath(hostPath)

			if not self.doPush(hostPath, devPath):
				return False

			# self.doSymlink(hostPath)

			lastDir = os.path.dirname(hostPath)

		return True
