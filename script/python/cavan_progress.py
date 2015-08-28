#!/usr/bin/env python

import sys, threading

class CavanProgressBar:
	def __init__(self, length = 60, stdout = None):
		self.mLock = threading.Lock()
		self.setProgressStdout(stdout)
		self.setProgressBarLength(length)

	def setProgressStdout(self, stdout = None):
		self.mLock.acquire()

		if stdout != None:
			self.mStdout = stdout
		else:
			self.mStdout = sys.stdout

		self.mLock.release()

	def setProgressBarLength(self, length):
		self.mLock.acquire()

		self.mHalfLength = length >> 1
		self.mLength = self.mHalfLength << 1

		self.mLock.release()

	def initProgress(self, total, current = 0):
		self.mLock.acquire()

		self.mTotal = total
		self.mCurrent = current

		self.mLock.release()

		self.updateProgress()

	def updateProgress(self):
		self.mLock.acquire()

		if self.mTotal == 0:
			fill = PROGRESS_BAR_LENGTH
			percent = 100
		else:
			fill = self.mCurrent * self.mLength / self.mTotal
			percent = self.mCurrent * 100 / self.mTotal

		left = right = ""

		for weight in range(0, self.mHalfLength):
			if weight < fill:
				left += "H"
			else:
				left += "="

		for weight in range(self.mHalfLength, self.mLength):
			if weight < fill:
				right += "H"
			else:
				right += "="

		text = "[%s %d%% %s] [%d/%d]" % (left, percent, right, self.mCurrent, self.mTotal)
		self.mStdout.write(text + "\r")
		self.mStdout.flush()

		self.mLock.release()

	def addProgress(self, count = 1):
		self.mLock.acquire()
		self.mCurrent += count
		self.mLock.release()

		self.updateProgress()

	def finishProgress(self):
		self.mLock.acquire()

		if self.mCurrent < self.mTotal:
			self.mCurrent = self.mTotal
			self.mLock.release()
			self.updateProgress()
		else:
			self.mLock.release()

		print >> self.mStdout
