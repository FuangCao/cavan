#!/usr/bin/env python

import sys, os, re
from cavan_file import file_read_lines, file_write_lines
from cavan_xml import CavanXmlBase

CHIP_NAME = "tc3587xx"
PREFIX = CHIP_NAME + "_init_data"
STRUCT_NAME = PREFIX + "_node"
ARRAY_NAME = PREFIX + "_table"
FUNC_NAME = CHIP_NAME + "_write_init_data"

class TC3587XX_DataNode():
	def __init__(self, values, title, comment, isRead = False, count = 0):
		self.mDelay = 0;

		if not title:
			self.mTitle = None
		else:
			self.mTitle = [line.strip() for line in title]

		self.mIsMasterSend = True
		self.mValues = []

		if not values:
			self.mValues = []
		else:
			for value in values.strip().split():
				value = value.strip().upper()
				if len(value) > 2:
					self.mIsMasterSend = False
				self.mValues.append("0x" + value)

		if len(self.mValues) > 2:
			self.mIsMasterSend = True

		if not comment:
			self.mComment = None
		else:
			self.mComment = comment.strip()

		self.mIsRead = isRead
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

	def toStringLines(self, hasRead):
		lines = []

		if self.mTitle != None:
			if len(self.mTitle) > 1:
				lines.append("/*")
				for line in self.mTitle:
					if not line:
						lines.append(" *")
					else:
						lines.append(" * " + line)
				lines.append(" */")
			else:
				lines.append("/* %s */" % self.mTitle[0])

		if not self.mValues:
			values = None
			count = self.mCount
		else:
			values = ", ".join(self.mValues)
			count = len(self.mValues)

		if self.mIsMasterSend:
			if self.mIsRead and not values:
				values = "{ }"
			else:
				values = "{ " + values + " }"

			values = "%d, %s" % (count, values)

		text = ""

		if hasRead:
			text += "{ %d, %s, %d }," % (self.mIsRead, values, self.mDelay)
		else:
			text += "{ %s, %d }," % (values, self.mDelay)

		if self.mComment != None:
			text += " // " + self.mComment

		lines.append(text)

		return lines

class TC3587XX_Converter(CavanXmlBase):
	def doConvertXml(self, pathname):
		if not self.load(pathname):
			return False

		listData = []

		for node in self.getChildNodes():
			if node.nodeType == node.COMMENT_NODE:
				title = node.nodeValue
			elif node.nodeType == node.ELEMENT_NODE:
				isRead = (node.tagName == "i2c_read")
				if isRead or node.tagName == "i2c_write":
					if title != None:
						title = title.split("\n")
					if not node.firstChild:
						nodeValue = None
					else:
						nodeValue = node.firstChild.nodeValue
					count = int(node.getAttribute("count").strip())
					dataNode = TC3587XX_DataNode(nodeValue, title, None, isRead, count)
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

	def toStringLines(self):
		lines = []
		for node in self.mDataNodes:
			nodeLines = node.toStringLines(self.mHasRead)
			if not nodeLines:
				return None
			lines.extend(nodeLines)

		return lines

	def toString(self):
		lines = self.toStringLines()
		if not lines:
			return None
		return "\n".join(lines)

	def genStructLines(self):
		hasMasterSend = False
		hasRead = False
		addrLen = 0
		valLen = 0
		valCount = 0

		for node in self.mDataNodes:
			if node.mIsMasterSend:
				hasMasterSend = True
			if node.mIsRead:
				hasRead = True

			values = node.mValues
			count = len(values)
			if count > valCount:
				valCount = count

			if count > 1:
				length = len(values[0]) - 2
				if addrLen < length:
					addrLen = length

				length = len(values[1]) - 2
				if valLen < length:
					valLen = length

		self.mHasMasterSend = hasMasterSend
		self.mHasRead = hasRead
		self.mAddrLen = addrLen
		self.mValLen = valLen
		self.mValCount = valCount

		lines = []
		lines.append("struct %s {" % STRUCT_NAME)

		print "hasMasterSend = %d" % hasMasterSend
		print "hasRead = %d" % hasRead
		print "addrLen = %d" % addrLen
		print "valLen = %d" % valLen
		print "valCount = %d" % valCount

		if hasRead:
			lines.append("\tbool is_read;")

		if hasMasterSend:
			lines.append("\tint count;")
			lines.append("\tu8 values[%d];" % valCount)
		else:
			lines.append("\tu%d addr;" % (addrLen * 4))
			lines.append("\tu%d value;" % (valLen * 4))

		lines.append("\tu32 delay;")
		lines.append("};")

		return lines

	def genFunctionLines(self):
		lines = []

		lines.append("static int %s(struct i2c_client *client, const struct %s *nodes, size_t count)" % (FUNC_NAME, STRUCT_NAME))
		lines.append("{")
		lines.append("\tint ret;")
		lines.append("\tconst struct tc3587xx_init_data_node *node, *node_end;")
		lines.append("")
		lines.append("\tfor (node = nodes, node_end = node + count; node < node_end; node++) {")

		if self.mHasMasterSend:
			lines.append("\t\tif (node->is_read) {")
			lines.append("\t\t\tu8 buff[node->count]")
			lines.append("")
			lines.append("\t\t\tdev_info(&client->dev, \"master_recv: count = %d\\n\", node->count);")
			lines.append("")
			lines.append("\t\t\tret = i2c_master_recv(client, buff, node->count);")
			lines.append("\t\t\tif (ret < 0) {")
			lines.append("\t\t\t\tdev_err(&client->dev, \"Failed to i2c_master_recv: %d\\n\", ret);")
			lines.append("\t\t\t\treturn ret;")
			lines.append("\t\t\t}")
			lines.append("")
			lines.append("\t\t\tif (strncmp(buff, node->value, node->count)) {")
			lines.append("\t\t\t\tdev_err(&client->dev, \"i2c_master_recv data is invalid\\n\");")
			lines.append("\t\t\t\treturn -EINVAL;")
			lines.append("\t\t\t}")
			lines.append("\t\t} else {")
			lines.append("\t\t\tdev_info(&client->dev, \"master_send: count = %d\\n\", node->count);")
			lines.append("")
			lines.append("\t\t\tret = i2c_master_send(client, node->values, node->count);")
			lines.append("\t\t\tif (ret < 0) {")
			lines.append("\t\t\t\tdev_err(&client->dev, \"Failed to i2c_master_send: %d\\n\", ret);")
			lines.append("\t\t\t\treturn ret;")
			lines.append("\t\t\t}")
			lines.append("\t\t}")
		else:
			lines.append("\t\tdev_info(&client->dev, \"write: addr = 0x%%0%dx, value = 0x%%0%dx\\n\", node->addr, node->value);" % (self.mAddrLen, self.mValLen))
			lines.append("")
			lines.append("\t\tret = tc3587xx_write_register%d(client, node->addr, node->value);" % (self.mValLen * 4))
			lines.append("\t\tif (ret < 0) {")
			lines.append("\t\t\tdev_err(&client->dev, \"Failed to tc3587xx_write_register%d: %%d\\n\", ret);" % (self.mValLen * 4))
			lines.append("\t\t\treturn ret;")
			lines.append("\t\t}")

		lines.append("")
		lines.append("\t\tif (node->delay) {")
		lines.append("\t\t\tmsleep(node->delay);")
		lines.append("\t\t}")
		lines.append("\t}")
		lines.append("")
		lines.append("\treturn ret;")
		lines.append("}")

		lines.append("")
		lines.append("static int %s_init_register(struct i2c_client *client)" % CHIP_NAME)
		lines.append("{")
		lines.append("\tint ret;")
		lines.append("")
		lines.append("\tret = %s(client, %s, ARRAY_SIZE(%s));" % (FUNC_NAME, ARRAY_NAME, ARRAY_NAME))
		lines.append("\tif (ret < 0) {")
		lines.append("\t\tdev_err(&client->dev, \"Failed to %s: %%d\\n\", ret);" % (FUNC_NAME))
		lines.append("\t\treturn ret;")
		lines.append("\t}")
		lines.append("")
		lines.append("\tmsleep(100);")
		lines.append("")
		lines.append("\treturn 0;")
		lines.append("}")

		return lines


	def save(self, pathname):
		lines = self.genStructLines()
		if not lines:
			return False

		valLines = self.toStringLines()
		if not valLines:
			return False

		lines.append("")
		lines.append("static struct %s %s[] = {" % (STRUCT_NAME, ARRAY_NAME))

		valLines = ["\t" + line for line in valLines]
		lines.extend(valLines)
		lines.append("};")

		lines.append("")
		funcLines = self.genFunctionLines()
		if not funcLines:
			return False
		lines.extend(funcLines)

		lines = [line + "\n" for line in lines]

		return file_write_lines(pathname, lines)
