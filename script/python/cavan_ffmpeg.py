#!/usr/bin/python
# coding: utf-8

import sys, os, re, time, subprocess
from getopt import getopt

REAL_CONVERT = True

def pr_info(message):
	try:
		sys.stdout.write(message + "\n")
	except:
		return None

def pr_err_info(message):
	try:
		sys.stderr.write("Error: " + message + "\n")
	except:
		return None

class FFMpegConvert:
	def __init__(self):
		self.mForceOverride = False
		self.mSourceDir = None
		self.mDestDir = None
		self.mAudioCodec = "ac3"
		self.mAudioBitrate = "500k"
		self.mVideoCodec = "libx265"
		self.mVideoBitrate = "2000k"
		self.mVideoCodecParam = None
		self.mLogFile = None
		self.mConvertParam = []

	def showUsage(self):
		command_name = os.path.basename(sys.argv[0])
		pr_info("Usage %s:" %  command_name)
		pr_info("%s [options] srcDir dstDir" % command_name)
		pr_info("-f, --force\t\t可选项，如果dstDir已经有一个同名的文件，强制覆盖，如果没有选项，会跳过这个文件")
		pr_info("-s srcDir\t\t需要转码的视频所在的文件夹，扫描文件夹下的所有目录和子目录，找到后一个一个转或者srcDir也可以指定一个文件进行转码")
		pr_info("-d dstDir\t\t转码后的文件存储位置，文件存储在dstDir/日期时间目录下，比如dstDir/20160304/")
		pr_info("-acodec acodec\t\t指定音频的编码器，这个参数需要传给ffmpeg的 -acodec 参数")
		pr_info("-ba abitrate\t\t指定音频编码的码率，这个参数需要传给ffmpeg的 -b:a 参数")
		pr_info("-vcodec vcodec\t\t指定视频编码器，这里只能是libx265或libx264,这个参数需要传递给ffmpeg的 -vcodec 参数")
		pr_info("-bv vbitrate\t\t指定视频编码器的码率，这个参数需要传递给ffmpegde的 -b:v 参数")
		pr_info("-vp vcodecparam\t\t指定视频编码的额外的参数，这个参数直接跟在ffmpeg的-vcodec 参数后面")
		pr_info("-l log\t\t\t生成log文件，指明从哪个文件转化，时间，转化后的大小，log的具体格式以一行为一个转换记录")

	def getDestFilename(self, filename):
		filename = os.path.splitext(filename)[0]

		if self.mVideoCodec == "libx265":
			filename = re.sub("x264", "x265", filename, flags = re.I)

		filename = re.sub("dts", "AC3", filename, flags = re.I)

		return filename

	def doConvertFile(self, srcPath, destDir, filename):
		destFilename = self.getDestFilename(filename)
		destPath = os.path.join(destDir, destFilename + ".mp4")

		if os.path.exists(destPath) and not self.mForceOverride:
			return True

		destPathTemp = os.path.join(destDir, destFilename + "-temp.mp4")

		pr_info("ConvertFile: %s => %s" % (srcPath, destPathTemp))

		command = ["ffmpeg", "-i" , srcPath]
		command.extend(self.mConvertParam)
		command.append(destPathTemp)

		print command

		if REAL_CONVERT:
			process = subprocess.Popen(command)
			if not process:
				return False

			if process.wait() != 0:
				return False

			try:
				os.rename(destPathTemp, destPath)
			except:
				return False

		message = "%s %s.mp4 %s\n" % (filename, destFilename, " ".join(self.mConvertParam))
		self.mLogFp.write(message)

		return True

	def doConvertDir(self, srcPath, destPath):
		pr_info("ConvertDir: %s => %s" % (srcPath, destPath))

		try:
			os.makedirs(self.mDestDir)
		except:
			if not os.path.isdir(self.mDestDir):
				return False

		for filename in os.listdir(srcPath):
			pathname = os.path.join(srcPath, filename)
			if not os.path.isdir(pathname):
				if not self.doConvertFile(pathname, destPath, filename):
					return False
			elif not self.doConvertDir(pathname, os.path.join(destPath, filename)):
				return False

		return True

	def doConvert(self):
		pr_info("Convert: %s => %s" % (self.mSourceDir, self.mDestDir))

		try:
			os.makedirs(self.mDestDir)
		except:
			if not os.path.isdir(self.mDestDir):
				return False

		if self.mLogFile == None:
			self.mLogFile = os.path.join(self.mDestDir, "log.txt")

		self.mLogFp = open(self.mLogFile, "a")
		if not self.mLogFp:
			pr_err_info("Failed to open log file: " + self.mLogFile)
			return False

		self.mConvertParam.extend(["-acodec", self.mAudioCodec])
		self.mConvertParam.extend(["-b:a", self.mAudioBitrate])
		self.mConvertParam.extend(["-vcodec", self.mVideoCodec])

		if (self.mVideoCodecParam != None):
			self.mConvertParam.extend(re.split("\\s+", self.mVideoCodecParam))

		self.mConvertParam.extend(["-b:v", self.mVideoBitrate])

		if not os.path.isdir(self.mSourceDir):
			filename = os.path.basename(self.mSourceDir)
			return self.doConvertFile(self.mSourceDir, self.mDestDir, filename)
		else:
			return self.doConvertDir(self.mSourceDir, self.mDestDir)

	def getopt(self, argv):
		opts = []
		args = []
		optName = None

		for arg in argv:
			if optName != None:
				opts.append((optName, arg))
				optName = None
			elif arg in ["-acodec", "-ba", "-vcodec", "-bv", "-vp"]:
				optName = arg
			else:
				args.append(arg)

		std_opts, args = getopt(args, "fs:d:l:", ["acodec=", "ba=", "vcodec=", "bv=", "log=", "force"])
		opts.extend(std_opts)

		return opts, args

	def main(self, argv):
		opts, args = self.getopt(argv)

		for opt in opts:
			if opt[0] in ["-f", "--force"]:
				self.mForceOverride = True
				pr_info("mForceOverride = %d" % self.mForceOverride)
			elif opt[0] in ["-s"]:
				self.mSourceDir = opt[1]
				pr_info("mSourceDir = " + self.mSourceDir)
			elif opt[0] in ["-d"]:
				self.mDestDir = opt[1]
				pr_info("mDestDir = " + self.mDestDir)
			elif opt[0] in ["-acodec", "--acodec"]:
				self.mAudioCodec = opt[1]
				pr_info("mAudioCodec = " + self.mAudioCodec)
			elif opt[0] in ["-ba", "--ba"]:
				self.mAudioBitrate = opt[1]
				pr_info("mAudioBitrate = " + self.mAudioBitrate)
			elif opt[0] in ["-vcodec", "--vcodec"]:
				if opt[1] in ["libx264", "libx265"]:
					self.mVideoCodec = opt[1]
					pr_info("mVideoCodec = " + self.mVideoCodec)
				else:
					pr_info("invalid video codec: " + opt[1])
					return False
			elif opt[0] in ["-bv", "--bv"]:
				self.mVideoBitrate = opt[1]
				pr_info("mVideoBitrate = " + self.mVideoBitrate)
			elif opt[0] in ["-l", "--log"]:
				self.mLogFile = opt[1]
				pr_info("mLogFile = " + self.mLogFile)
			elif opt[0] in ["-vp"]:
				self.mVideoCodecParam = opt[1]
				pr_info("mVideoCodecParam = " + self.mVideoCodecParam)
			else:
				pr_err_info("unknown option %s" % opt[0])
				self.showUsage()
				return False

		remain = len(args)

		if not self.mSourceDir:
			if not self.mDestDir:
				if remain < 1:
					pr_err_info("Too a few argument")
					self.showUsage()
					return False

				self.mSourceDir = args[0]

				if remain < 2:
					self.mDestDir = "."
				elif remain < 3:
					self.mDestDir = args[1]
				else:
					pr_err_info("Too much argument")
					self.showUsage()
					return False
			elif remain < 1:
				self.mSourceDir = "."
			elif remain < 2:
				self.mSourceDir = args[0]
			else:
				pr_err_info("Too much argument")
				self.showUsage()
				return False
		elif not self.mDestDir:
			if remain < 1:
				self.mDestDir = "."
			elif remain < 2:
				self.mDestDir = args[0]
			else:
				pr_err_info("Too much argument")
				self.showUsage()
				return False

		self.mDestDir = os.path.join(self.mDestDir, time.strftime("%Y%m%d"))

		if not self.doConvert():
			pr_info("Failed");
		else:
			pr_info("OK");

if __name__ == "__main__":
	convert = FFMpegConvert()
	convert.main(sys.argv[1:])
