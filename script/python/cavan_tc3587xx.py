#!/usr/bin/env python

import sys, os, re
from cavan_file import file_read_lines, file_write_lines
from cavan_xml import CavanXmlBase

MASTER_SEND_ONLY = False
VALUE_BIG_ENDIAN = True
ADDR_BIG_ENDIAN = False

CHIP_NAME = "tc3587xx"
PREFIX = CHIP_NAME + "_init_data"
STRUCT_NAME = PREFIX + "_node"
ARRAY_NAME = PREFIX + "_table"

MACRO_DEBUG = "TC3587XX_DEBUG"
FUNC_WRITE_INIT_DATA = CHIP_NAME + "_write_init_data"
FUNC_INIT_REGISTER = CHIP_NAME + "_init_register"
FUNC_DUMP_MEM = CHIP_NAME + "_dump_mem"

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

	def valToArray(self, value, length, bigEndian = True):
		values = []
		text = value[2:]

		while len(text) < length:
			text = "0" + text

		if not bigEndian:
			index = 0
			while (index < length):
				value = text[index : index + 2]
				values.append("0x" + value)
				index += 2
		else:
			index = length - 2
			while (index >= 0):
				value = text[index : index + 2]
				values.append("0x" + value)
				index -= 2

		return values

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

		if MASTER_SEND_ONLY and not hasMasterSend:
			for node in self.mDataNodes:
				listValue = []

				values = self.valToArray(node.mValues[0], addrLen, ADDR_BIG_ENDIAN)
				if not values:
					return values
				listValue.extend(values)

				values = self.valToArray(node.mValues[1], valLen, VALUE_BIG_ENDIAN)
				if not values:
					return values
				listValue.extend(values)

				node.mValues = listValue

				node.mIsMasterSend = True

			hasMasterSend = True
			valCount = (addrLen + valLen) / 2

		self.mHasMasterSend = hasMasterSend
		self.mHasRead = hasRead
		self.mAddrLen = addrLen
		self.mValLen = valLen
		self.mValCount = valCount

		lines = []

		lines.append("#define %s\t1" % MACRO_DEBUG);
		lines.append("")
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

		if self.mHasMasterSend:
			lines.append("#if %s" % MACRO_DEBUG)
			lines.append("static void %s(struct i2c_client *client, const char *prompt, const u8 *mem, size_t size)" % FUNC_DUMP_MEM)
			lines.append("{")
			lines.append("\tconst u8 *mem_end;")
			lines.append("\tchar buff[%d], *p, *p_end;" % ((self.mValCount * 3 + 4 + 3) / 4 * 4))
			lines.append("")
			lines.append("\tp = buff;")
			lines.append("\tp_end = p + ARRAY_SIZE(buff) - 1;")
			lines.append("")
			lines.append("\tif (p < p_end) {")
			lines.append("\t\t*p++ = '[';")
			lines.append("\t}")
			lines.append("")
			lines.append("\tfor (mem_end = mem + size; mem < mem_end && p < p_end; mem++) {")
			lines.append("\t\tp += snprintf(p, p_end - p, \" %02x\", *mem);")
			lines.append("\t}")
			lines.append("")
			lines.append("\tif (p + 1 < p_end) {")
			lines.append("\t\tp[0] = ' ';")
			lines.append("\t\tp[1] = ']';")
			lines.append("\t\tp += 2;")
			lines.append("\t}")
			lines.append("")
			lines.append("\t*p = 0;")
			lines.append("")
			lines.append("\tif (prompt) {")
			lines.append("\t\tdev_info(&client->dev, \"%s%s\\n\", prompt, buff);")
			lines.append("\t} else {")
			lines.append("\t\tdev_info(&client->dev, \"%s\\n\", buff);")
			lines.append("\t}")
			lines.append("}")
			lines.append("#endif")


		lines.append("")
		lines.append("static int %s(struct i2c_client *client, const struct %s *nodes, size_t count)" % (FUNC_WRITE_INIT_DATA, STRUCT_NAME))
		lines.append("{")
		lines.append("\tint ret = 0;")
		lines.append("\tconst struct tc3587xx_init_data_node *node, *node_end;")
		lines.append("")
		lines.append("\tfor (node = nodes, node_end = node + count; node < node_end; node++) {")

		if self.mHasMasterSend:
			if self.mHasRead:
				lines.append("\t\tif (node->is_read) {")
				lines.append("\t\t\tu8 buff[node->count];")
				lines.append("")
				lines.append("#if %s" % MACRO_DEBUG)
				lines.append("\t\t\tdev_info(&client->dev, \"master_recv: count = %d\\n\", node->count);")
				lines.append("#endif")
				lines.append("")
				lines.append("\t\t\tret = i2c_master_recv(client, buff, node->count);")
				lines.append("\t\t\tif (ret < 0) {")
				lines.append("\t\t\t\tdev_err(&client->dev, \"Failed to i2c_master_recv: %d\\n\", ret);")
				lines.append("\t\t\t\treturn ret;")
				lines.append("\t\t\t}")
				lines.append("")
				lines.append("#if %s" % MACRO_DEBUG)
				lines.append("\t\t\t%s(client, \"master_recv = \", buff, node->count);" % FUNC_DUMP_MEM)
				lines.append("#endif")
				lines.append("\t\t} else {")
				prefix_space = "\t"
			else:
				prefix_space = ""

			lines.append("#if %s" % MACRO_DEBUG)
			lines.append(prefix_space + "\t\t%s(client, \"master_send = \", node->values, node->count);" % FUNC_DUMP_MEM)
			lines.append("#endif")
			lines.append("")
			lines.append(prefix_space + "\t\tret = i2c_master_send(client, node->values, node->count);")
			lines.append(prefix_space + "\t\tif (ret < 0) {")
			lines.append(prefix_space + "\t\t\tdev_err(&client->dev, \"Failed to i2c_master_send: %d\\n\", ret);")
			lines.append(prefix_space + "\t\t\treturn ret;")
			lines.append(prefix_space + "\t\t}")

			if self.mHasRead:
				lines.append("\t\t}")
		else:
			lines.append("#if %s" % MACRO_DEBUG)
			lines.append("\t\tdev_info(&client->dev, \"write: addr = 0x%%0%dx, value = 0x%%0%dx\\n\", node->addr, node->value);" % (self.mAddrLen, self.mValLen))
			lines.append("#endif")
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
		lines.append("static int %s(struct i2c_client *client)" % FUNC_INIT_REGISTER)
		lines.append("{")
		lines.append("\tint ret;")
		lines.append("")
		lines.append("\tret = %s(client, %s, ARRAY_SIZE(%s));" % (FUNC_WRITE_INIT_DATA, ARRAY_NAME, ARRAY_NAME))
		lines.append("\tif (ret < 0) {")
		lines.append("\t\tdev_err(&client->dev, \"Failed to %s: %%d\\n\", ret);" % (FUNC_WRITE_INIT_DATA))
		lines.append("\t\treturn ret;")
		lines.append("\t}")
		lines.append("")
		lines.append("\t// msleep(100);")
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
