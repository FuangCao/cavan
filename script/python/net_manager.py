#!/usr/bin/python

import sys, os, re

from cavan_file import file_read_line, file_read_lines, \
		file_write_line, file_write_lines, file_append_line, file_append_lines

from cavan_command import CavanCommandBase

class CavanNetInfo:
	def __init__(self, ifName):
		self.mIfName = ifName
		self.mHwAddr = None
		self.mMacAddr = None
		self.mIpAddr = None
		self.mIpAddrV6 = None
		self.mNetMask = None
		self.mBroadcastAddr = None
		self.mNetType = None
		self.mScope = None
		self.mMtu = None

	def getIfName(self):
		return self.mIfName

	def getHwAddr(self):
		return self.mHwAddr

	def getMacAddr(self):
		return self.mMacAddr

	def getIpAddr(self):
		return self.mIpAddr

	def getIpAddrV6(self):
		return self.mIpAddrV6

	def getNetMask(self):
		return self.mNetMask

	def getBroadcastAddr(self):
		return self.mBroadcastAddr

	def getNetType(self):
		return self.mNetType

	def getScope(self):
		return self.mScope

	def getMtu(self):
		return self.mMtu

	def setIfName(self, ifName):
		self.mIfName = ifName

	def setHwAddr(self, hwAddr):
		self.mHwAddr = hwAddr

	def setMacAddr(self, macAddr):
		self.mMacAddr = macAddr

	def setIpAddr(self, ipAddr):
		self.mIpAddr = ipAddr

	def setIpAddrV6(self, ipAddrV6):
		self.mIpAddrV6 = ipAddrV6

	def setNetMask(self, netMask):
		self.mNetMask = netMask

	def setBroadcastAddr(self, broadcastAddr):
		self.mBroadcastAddr = broadcastAddr

	def setNetType(self, netType):
		self.mNetType = netType

	def setScope(self, scope):
		self.mScope = scope

	def setMtu(self, mtu):
		self.mMtu = mtu

	def toString(self):
		builder = []

		if self.mIfName != None:
			builder.append("mIfName = " + self.mIfName)

		if self.mHwAddr != None:
			builder.append("mHwAddr = " + self.mHwAddr)

		if self.mMacAddr != None:
			builder.append("mMacAddr = " + self.mMacAddr)

		if self.mIpAddr != None:
			builder.append("mIpAddr = " + self.mIpAddr)

		if self.mIpAddrV6 != None:
			builder.append("mIpAddrV6 = " + self.mIpAddrV6)

		if self.mNetMask != None:
			builder.append("mNetMask = " + self.mNetMask)

		if self.mBroadcastAddr != None:
			builder.append("mBroadcastAddr = " + self.mBroadcastAddr)

		if self.mNetType != None:
			builder.append("mNetType = " + self.mNetType)

		if self.mScope != None:
			builder.append("mScope = " + self.mScope)

		if self.mMtu != None:
			builder.append("mMtu = " + self.mMtu)

		return ", ".join(builder)

class CavanNetManager(CavanCommandBase):
	def __init__(self, verbose = False):
		CavanCommandBase.__init__(self, verbose = verbose)
		self.mPatternIfName = re.compile("(^[^\\s]+[0-9]*)\\s+Link encap:([^\\s]+)")
		self.mPatternInetAddr = re.compile("^\\s+inet addr:([0-9\\.]+).*Mask:([0-9\\.]+)")
		self.mPatternHwaddr = re.compile(".*HWaddr\\s+([0-9a-z:]+)")
		self.mPatternBroadcast = re.compile(".*Bcast:([^\\s]+)")
		self.mPatternIpaddrV6 = re.compile("^\\s+inet6 addr:\\s+([^\\s]+)\\s+Scope:([^\\s]+)")
		self.mPatternMtu = re.compile(".*MTU:([0-9]+)")

	def getNetInfoList(self):
		listNetInfo = []

		lines = self.doPopen(["ifconfig"])
		if not lines:
			if lines == None:
				return None
			return listNetInfo

		for line in lines:
			match = self.mPatternIfName.match(line)
			if match != None:
				info = CavanNetInfo(match.group(1))
				listNetInfo.append(info)
				info.setNetType(match.group(2))
				match = self.mPatternHwaddr.match(line)
				if match != None:
					info.setHwAddr(match.group(1))
				continue

			match = self.mPatternInetAddr.match(line)
			if match != None:
				info.setIpAddr(match.group(1))
				info.setNetMask(match.group(2))
				match = self.mPatternBroadcast.match(line)
				if match != None:
					info.setBroadcastAddr(match.group(1))
				continue

			match = self.mPatternIpaddrV6.match(line)
			if match != None:
				info.setIpAddrV6(match.group(1))
				info.setScope(match.group(2))
				continue

			match = self.mPatternMtu.match(line)
			if match != None:
				info.setMtu(match.group(1))
				continue

			if self.mVerbose:
				print "can't match line" + line.rstrip("\n")

		return listNetInfo

	def getLinkIpAddrList(self):
		listIpAddr = []
		listNetInfo = self.getNetInfoList()
		if not listNetInfo:
			return listIpAddr

		for info in listNetInfo:
			ipAddr = info.getIpAddr()
			if not ipAddr:
				continue

			if info.getScope() == "Link":
				listIpAddr.append(ipAddr)

		return listIpAddr

if __name__ == "__main__":
	manager = CavanNetManager()
	print manager.getLinkIpAddrList()
