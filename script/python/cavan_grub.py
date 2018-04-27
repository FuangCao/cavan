#!/usr/bin/python

import sys, os, glob
from cavan_command import CavanCommandBase

class CavanGrub(CavanCommandBase):
	def __init__(self):
		CavanCommandBase.__init__(self, verbose = False)

	def globVmlinuz(self, dname):
		return self.globOne("*vmlinuz*", dname)

	def globInitrd(self, dname):
		return self.globOne("*initrd*", dname)

	def buildUbuntuIsoEnv(self, fpath, dpath):
		mpath = "/mnt/cavan-grub-iso"
		if not self.doMountFile(fpath, mpath):
			return False

		if not self.mkdirSafe(dpath):
			self.doUmount(mpath)
			return False

		casper = os.path.join(mpath, "casper")
		vmlinuz = self.globVmlinuz(casper)
		initrd = self.globInitrd(casper)
		if not vmlinuz or not initrd:
			self.doUmount(mpath)
			return False

		if not self.doExecute(["cp", vmlinuz, initrd, dpath]):
			self.doUmount(mpath)
			return False

		self.doUmount(mpath)

		return True

	def scanUbuntuIsoDir(self, dev, dpath, hd):
		part = "(hd%d,msdos%s)" % (hd, dev[-1])

		for fn in os.listdir(dpath):
			fpath = os.path.join(dpath, fn)
			if os.path.isdir(fpath):
				self.scanUbuntuIsoDir(dev, fpath, hd)
			elif fn.startswith("ubuntu-") and fn.endswith(".iso"):
				vmlinuz = self.globVmlinuz(dpath)
				initrd = self.globInitrd(dpath)
				if not vmlinuz or not initrd:
					print >> sys.stderr, "Config ubuntu image: " + fpath
					names = os.path.splitext(fpath)
					diso = names[0]

					if not self.buildUbuntuIsoEnv(fpath, diso):
						continue

					vmlinuz = self.globVmlinuz(diso)
					initrd = self.globInitrd(diso)
					if not vmlinuz or not initrd:
						continue

					fiso = os.path.join(diso, fn)
					os.rename(fpath, fiso)
				else:
					fiso = fpath

				print >> sys.stderr, "Found ubuntu image: " + fiso

				print "menuentry '" + fn + "' {"
				print "\tset root='" + part + "'"
				print "\tlinux /" + vmlinuz + " boot=casper iso-scan/filename=/" + fiso
				print "\tinitrd /" + initrd
				print "}"

	def genUbuntuIsoMenus(self, disk = "sda"):
		mounts = self.parseMounts()
		if not mounts:
			return False

		hd = ord(disk[-1]) - ord('a')

		for dev in glob.glob("/dev/" + disk + "[0-9]*"):
			if mounts.has_key(dev):
				continue

			target = os.path.join("/mnt", os.path.basename(dev))

			if self.doMount(dev, target):
				mounts[dev] = target

		for dev, target in mounts.items():
			if dev.startswith("/dev/" + disk):
				dpath = "os"
				os.chdir(target)
				self.setRootPath(target)

				if os.path.isdir(dpath):
					self.scanUbuntuIsoDir(dev, dpath, hd)

if __name__ == "__main__":
	grub = CavanGrub()
	grub.genUbuntuIsoMenus()
