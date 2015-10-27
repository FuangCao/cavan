#!/usr/bin/env python

import sys, os
from cavan_xml import CavanXmlBase

class TC3587XX_DataNode():
	def __init__(self, values, comment):
		self.mDelay = 0;

		if not comment:
			self.mComment = None
		else:
			self.mComment = comment.strip();

		listVale = []

		for value in values.strip().split():
			listVale.append("0x" + value.strip().upper())

		self.mValues = listVale;

	def setDelay(self, delay):
		self.mDelay = int(delay.strip())

	def toString(self):
		text = ""
		if self.mComment != None:
			text += "/* %s */\n" % self.mComment

		text += "{ %d, { %s }, %d }," % (len(self.mValues), ", ".join(self.mValues), self.mDelay)

		return text

class TC3587XX_Converter(CavanXmlBase):
	def doConvertXml(self, xml):
		if not self.load(xml):
			return False

		listData = []

		for node in self.getChildNodes():
			if node.nodeType == node.COMMENT_NODE:
				comment = node.nodeValue
			elif node.nodeType == node.ELEMENT_NODE:
				if node.tagName == "i2c_write":
					dataNode = TC3587XX_DataNode(node.firstChild.nodeValue, comment)
					listData.append(dataNode)
					comment = None
				elif node.tagName == "sleep":
					listData[-1].setDelay(node.getAttribute("ms"))

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
