#!/usr/bin/env python

import sys, os
from cavan_xml import CavanXmlBase
from cavan_command import CavanCommandBase

class CavanAlarmTask:
	def __init__(self, tag):
		self.mRootTag = tag

	def getDate(self):
		return self.mRootTag.getAttribute("date")

	def getTime(self):
		return self.mRootTag.getAttribute("time")

	def getRepeat(self):
		return self.mRootTag.getAttribute("repeat")

	def getPath(self):
		return self.mRootTag.getAttribute("path")

	def getCommand(self):
		return self.mRootTag.getAttribute("command")

class CavanAlarmConfigFile(CavanXmlBase):
	def getTaskList(self):
		listTask = []

		for tagTask in self.getElementsByTagName("task"):
			listTask.append(CavanAlarmTask(tagTask))

		return listTask

	def getServicePort(self, defvalue = 8888):
		port = self.getAttribute("port")
		if not port:
			return defvalue
		return int(port)

class CavanAlarmManager(CavanCommandBase):
	def __init__(self, pathCavanMain = None):
		CavanCommandBase.__init__(self)
		if not pathCavanMain or not os.path.isfile(pathCavanMain):
			self.mServiceCommand = "cavan-tcp_dd_server"
			self.mCliendCommand = "cavan-alarm"
			self.mCavanMain = None
		else:
			self.mCavanMain = pathCavanMain

	def startService(self):
		if not self.mCavanMain:
			self.doExecute([self.mServiceCommand, "-p", "%d" % self.mServicePort, "-ds0"], of = "/dev/null", ef = "/dev/null")
		else:
			self.doExecute([self.mCavanMain, "tcp_dd_server", "-p", "%d" % self.mServicePort, "-ds0"], of = "/dev/null", ef = "/dev/null")

	def removeAllTask(self):
		if not self.mCavanMain:
			listCommand = [self.mCliendCommand, "remove", "-lp", "%d" % self.mServicePort, "0"]
		else:
			listCommand = [self.mCavanMain, "alarm", "remove", "-lp", "%d" % self.mServicePort, "0"]

		while True:
			if not self.doExecute(listCommand, of = "/dev/null", ef = "/dev/null", verbose = False):
				break

	def addAlarmTask(self, task):
		if not self.mCavanMain:
			listCommand = [self.mCliendCommand, "add", "-lp", "%d" % self.mServicePort]
		else:
			listCommand = [self.mCavanMain, "alarm", "add", "-lp", "%d" % self.mServicePort]

		command = task.getCommand()
		if not command:
			self.prRedInfo("Please give a command")
			return False

		date = task.getDate()
		if date != "":
			listCommand.append("--date")
			listCommand.append(date)

		time = task.getTime()
		if time != "":
			listCommand.append("--time")
			listCommand.append(time)

		repeat = task.getRepeat()
		if repeat != "":
			listCommand.append("--repeat")
			listCommand.append(repeat)

		path = task.getPath()
		if not path:
			listCommand.append(command)
		else:
			listCommand.append("cd %s && { %s; }" % (self.getAbsPath(path), command))

		return self.doExecute(listCommand)

	def main(self, pathConfigFile):
		fileConfig = CavanAlarmConfigFile()
		if not fileConfig.load(pathConfigFile):
			self.prRedInfo("load xml file", pathConfigFile, "failed")

		self.mServicePort = fileConfig.getServicePort()
		if not self.startService():
			self.removeAllTask()

		for task in fileConfig.getTaskList():
			if not self.addAlarmTask(task):
				self.removeAllTask()
				return False

		return True

if __name__ == "__main__":
	if len(sys.argv) > 2:
		mamager = CavanAlarmManager(sys.argv[2])
	else:
		mamager = CavanAlarmManager()

	if len(sys.argv) > 1:
		mamager.main(sys.argv[1])
