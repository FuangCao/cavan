#!/usr/bin/env python

import sys, os, re
from cavan_file import file_read_lines
from cavan_xml import CavanXmlBase

class TC3587XX_DataNode():
	def __init__(self, values, title, comment, isWrite = True, count = 0):
		self.mDelay = 0;

		if not title:
			self.mTitle = None
		else:
			self.mTitle = [line.strip() for line in title]

		self.mMasterSend = True
		self.mValues = []

		if not values:
			self.mValues = []
		else:
			for value in values.strip().split():
				value = value.strip().upper()
				if len(value) > 2:
					self.mMasterSend = False
				self.mValues.append("0x" + value)

		if len(self.mValues) > 2:
			self.mMasterSend = True

		if not comment:
			self.mComment = None
		else:
			self.mComment = comment.strip()

		self.mIsWrite = isWrite
		self.mCount = count

	def setDelay(self, delay):
		self.mDelay = delay

	def setComment(self, comment):
		comment = comment.strip()
		if not comment:
			return;
		if not self.mComment:
			self.mComment = comment
		else:
			self.mComment += ", " + comment

	def toString(self):
		text = ""

		if self.mTitle != None:
			if len(self.mTitle) > 1:
				text += "/*\n"
				for line in self.mTitle:
					if not line:
						text += " *\n"
					else:
						text += " * %s\n" % line
				text += " */\n"
			else:
				text += "/* %s */\n" % self.mTitle[0]

		if not self.mValues:
			values = None
		else:
			values = ", ".join(self.mValues)

		if self.mMasterSend:
			if not values:
				values = "%d, {}" % self.mCount
			else:
				values = "%d, { %s }" % (len(self.mValues), values)

		text += "{ %d, %s, %d }," % (self.mIsWrite, values, self.mDelay)

		if self.mComment != None:
			text += " // " + self.mComment

		return text

class TC3587XX_Converter(CavanXmlBase):
	def doConvertXml(self, pathname):
		if not self.load(pathname):
			return False

		listData = []

		for node in self.getChildNodes():
			if node.nodeType == node.COMMENT_NODE:
				title = node.nodeValue
			elif node.nodeType == node.ELEMENT_NODE:
				isWrite = (node.tagName == "i2c_write")
				if isWrite or node.tagName == "i2c_read":
					if title != None:
						title = title.split("\n")
					if not node.firstChild:
						nodeValue = None
					else:
						nodeValue = node.firstChild.nodeValue
					count = int(node.getAttribute("count").strip())
					dataNode = TC3587XX_DataNode(nodeValue, title, None, isWrite, count)
					listData.append(dataNode)
					title = None
				elif node.tagName == "sleep":
					delay = node.getAttribute("ms").strip()
					listData[-1].setDelay(int(delay))
				else:
					print "skipping tag: " + node.tagName

		self.mDataNodes = listData

		return True

	def doConvertText(self, pathname):
		self.mPatternWrite = re.compile('^\s*WR\s+(\S+\s+\S+)\s*(.*)\s*$')
		self.mPatternDelay = re.compile('^\s*delay\s+(\S+)\s*(.*)\s*$')

		lines = file_read_lines(pathname)
		if not lines:
			return False

		title = []
		listData = []

		for line in lines:
			line = line.strip()
			if not line:
				continue

			if line.startswith("REM"):
				if line.startswith("REMnd"):
					line = line[5:]
				else:
					line = line[3:]
				title.append(line)
			elif line.startswith("delay"):
				match = self.mPatternDelay.match(line)
				if not match:
					print "Invalid line " + line
					return False

				delay = int(match.group(1))
				if delay > 1000:
					delay /= 1000
				else:
					delay = 1

				listData[-1].setDelay(delay)
				listData[-1].setComment(match.group(2))
			elif line.startswith("WR"):
				match = self.mPatternWrite.match(line)
				if not match:
					print "Invalid line " + line
					return False

				node = TC3587XX_DataNode(match.group(1), title, match.group(2))
				listData.append(node)
				title = []

		self.mDataNodes = listData

		return True

	def save(self, pathname):
		fp = open(pathname, "w")
		if not fp:
			return False

		for node in self.mDataNodes:
			line = node.toString()
			fp.write(line + "\n")

		fp.close()

		return True
