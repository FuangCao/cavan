#!/usr/bin/python
# coding: utf-8

import sys, os, re, time
import subprocess, chardet
from getopt import getopt

# REAL_CONVERT = True
REAL_CONVERT = False

def decodeAuto(text):
	if isinstance(text, unicode):
		return text

	try:
		dictEncode = chardet.detect(text)
		if not dictEncode:
			return text

		encoding = dictEncode["encoding"]
		if not encoding:
			return text

		return text.decode(encoding)
	except:
		print(u"无法识别编码方式")

	for encoding in ["ascii", "utf-8", "gbk"]:
		print(u"尝试编码方式: " + encoding)

		try:
			return text.decode(encoding)
		except:
			print(u"编码方式不是: " + encoding)

	return text

def pr_info(message):
	try:
		sys.stdout.write(message + "\n")
	except:
		print(u"打印Log失败")
		return None

def pr_err_info(message):
	try:
		sys.stderr.write(u"[出错了] " + message + "\n")
	except:
		print(u"打印Log失败")
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
		pr_info(u"%s [options] 源文件夹 目标文件夹" % command_name)
		pr_info(u"-f, --force\t\t可选项，如果目标文件夹已经有一个同名的文件，强制覆盖，如果没有选项，会跳过这个文件")
		pr_info(u"-s 文件夹\t\t需要转码的视频所在的文件夹，扫描文件夹下的所有目录和子目录，找到后一个一个转或者源文件夹也可以指定一个文件进行转码")
		pr_info(u"-d 文件夹\t\t转码后的文件存储位置，文件存储在dstDir/日期时间目录下，比如dstDir/20160304/")
		pr_info(u"-acodec acodec\t\t指定音频的编码器，这个参数需要传给ffmpeg的 -acodec 参数")
		pr_info(u"-ba abitrate\t\t指定音频编码的码率，这个参数需要传给ffmpeg的 -b:a 参数")
		pr_info(u"-vcodec vcodec\t\t指定视频编码器，这里只能是libx265或libx264,这个参数需要传递给ffmpeg的 -vcodec 参数")
		pr_info(u"-bv vbitrate\t\t指定视频编码器的码率，这个参数需要传递给ffmpegde的 -b:v 参数")
		pr_info(u"-vp vcodecparam\t\t指定视频编码的额外的参数，这个参数直接跟在ffmpeg的-vcodec 参数后面")
		pr_info(u"-l log\t\t\t生成log文件，指明从哪个文件转化，时间，转化后的大小，log的具体格式以一行为一个转换记录")

	def isVideoFile(self, filename):
		file_ext = os.path.splitext(filename)[1]
		if not file_ext:
			return False

		for ext in ["mkv", "mp4", "rm", "rmvb", "avi", "mov", "flv", "wmv", "3gp", "ra", "ram", "mpg", "mpe", "mpeg", "m2v", "vob", "asf"]:
			if re.match("." + ext, file_ext, flags = re.I) != None:
				return True

		return False

	def mkdirs(self, pathname):
		try:
			os.makedirs(pathname)
			return True
		except:
			if os.path.isdir(pathname):
				return True

		pr_err_info(u"无法创建文件夹: " + pathname)

		return False

	def joinPath(self, dirPath, relPath):
		if not relPath:
			return dirPath
		elif not dirPath:
			return relPath
		else:
			return os.path.join(dirPath, relPath)

	def getSourcePath(self, relPath = None):
		return self.joinPath(self.mSourceDir, relPath)

	def getSourceReadyPath(self, relPath = None):
		return self.joinPath(self.mSourceDir + "-ffmpeg-converted", relPath)

	def getDestPath(self, relPath = None):
		return self.joinPath(self.mDestDir, relPath)

	def getDestFilename(self, filename):
		filename = os.path.splitext(filename)[0]

		if self.mVideoCodec == "libx265":
			filename = re.sub("x264", "x265", filename, flags = re.I)

		filename = re.sub("dts", "AC3", filename, flags = re.I)

		return filename

	def doConvertFile(self, srcPath, destDir, filename):
		if not self.isVideoFile(filename):
			if os.path.getsize(srcPath) > (1 << 20):
				return True

			destPath = os.path.join(destDir, filename)
			pr_info(u"文件拷贝: %s => %s" % (decodeAuto(srcPath), decodeAuto(destPath)))

			srcFp = open(srcPath, "rb")
			if not srcFp:
				return False

			destFp = open(destPath, "wb")
			if not destFp:
				srcFp.close()
				return False

			destFp.write(srcFp.read())

			destFp.close()
			srcFp.close()
		else:
			destFilename = self.getDestFilename(filename)
			destPath = os.path.join(destDir, destFilename + ".mp4")

			if os.path.exists(destPath):
				if not self.mForceOverride:
					return True
				os.unlink(destPath)

			destPathTemp = os.path.join(destDir, "." + destFilename + ".cache.mp4")

			try:
				os.unlink(destPathTemp)
			except:
				if os.path.exists(destPathTemp):
					return False

			pr_info(u"转换文件: %s => %s" % (decodeAuto(srcPath), decodeAuto(destPath)))

			command = ["ffmpeg", "-i" , srcPath]
			command.extend(self.mConvertParam)
			command.append(destPathTemp)

			print(command)

			if not REAL_CONVERT:
				fp = open(destPath, "w")
				if not fp:
					return False
				fp.write(destPath)
				fp.close()
			else:
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
			self.mLogFp.flush()

		return True

	def doConvertDir(self, relPath):
		destPath = self.getDestPath(relPath)
		if not self.mkdirs(destPath):
			return -1

		readyPath = self.getSourceReadyPath(relPath)
		if not self.mkdirs(readyPath):
			return -1

		srcPath = self.getSourcePath(relPath)
		pr_info(u"转换文件夹: %s => %s" % (decodeAuto(srcPath), decodeAuto(destPath)))

		failed = 0

		for filename in os.listdir(srcPath):
			pathname = os.path.join(srcPath, filename)
			if not os.path.isdir(pathname):
				try:
					if not self.doConvertFile(pathname, destPath, filename):
						failed += 1
					else:
						os.rename(pathname, os.path.join(readyPath, filename))
				except:
					failed += 1
			else:
				count = self.doConvertDir(self.joinPath(relPath, filename))
				if count < 0:
					return count

				failed += count

		if failed != 0:
			return failed

		os.rmdir(srcPath)

		return 0

	def doConvert(self):
		self.mSourceDir = os.path.realpath(self.mSourceDir)
		self.mDestDir = os.path.realpath(self.mDestDir)

		pr_info(u"格式转换: %s => %s" % (self.mSourceDir, self.mDestDir))

		if self.mDestDir.startswith(self.mSourceDir):
			if self.mDestDir[len(self.mSourceDir)] in ["\0", os.path.sep]:
				pr_err_info(u"目标文件不能放在源文件夹里")
				return -1

		if not self.mkdirs(self.mDestDir):
			return -1

		if self.mLogFile == None:
			self.mLogFile = os.path.join(self.mDestDir, "ffmpeg-log.txt")

		self.mLogFp = open(self.mLogFile, "a")
		if not self.mLogFp:
			pr_err_info(u"无法打开Log文件: " + self.mLogFile)
			return -1

		self.mConvertParam.extend(["-acodec", self.mAudioCodec])
		self.mConvertParam.extend(["-b:a", self.mAudioBitrate])
		self.mConvertParam.extend(["-vcodec", self.mVideoCodec])

		if (self.mVideoCodecParam != None):
			self.mConvertParam.extend(re.split("\\s+", self.mVideoCodecParam))

		self.mConvertParam.extend(["-b:v", self.mVideoBitrate])

		if not os.path.isdir(self.mSourceDir):
			if not os.path.exists(self.mSourceDir):
				pr_err_info(u"文件或目录 \"%s\" 不存在" % self.mSourceDir)
				return -1
			filename = os.path.basename(self.mSourceDir)
			if not self.doConvertFile(self.mSourceDir, self.mDestDir, filename):
				return 1
			else:
				return 0
		else:
			return self.doConvertDir(None)

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
					pr_info(u"不支持的视频编码: " + opt[1])
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
				pr_err_info(u"无法识别命令参数 %s" % opt[0])
				self.showUsage()
				return False

		remain = len(args)

		if not self.mSourceDir:
			if not self.mDestDir:
				if remain < 1:
					pr_err_info(u"请提供源文件路径和目标文件路径")
					self.showUsage()
					return False

				self.mSourceDir = args[0]

				if remain < 2:
					self.mDestDir = "."
				elif remain < 3:
					self.mDestDir = args[1]
				else:
					pr_err_info(u"存在多余的参数，请检查输入是否正确")
					self.showUsage()
					return False
			elif remain < 1:
				self.mSourceDir = "."
			elif remain < 2:
				self.mSourceDir = args[0]
			else:
				pr_err_info(u"存在多余的参数，请检查输入是否正确")
				self.showUsage()
				return False
		elif not self.mDestDir:
			if remain < 1:
				self.mDestDir = "."
			elif remain < 2:
				self.mDestDir = args[0]
			else:
				pr_err_info(u"存在多余的参数，请检查输入是否正确")
				self.showUsage()
				return False

		self.mDestDir = os.path.join(self.mDestDir, time.strftime("%Y%m%d"))

		count = self.doConvert()
		if count == 0:
			pr_info(u"恭喜，转换成功");
		else:
			if count < 0:
				pr_err_info(u"很遗憾，转换失败");
			else:
				pr_err_info(u"转换失败的文件有: %d 个" % count)

			pr_info(u"转换失败的文件放在: " + self.getSourcePath())

		pr_info(u"转换成功的文件放在: " + self.getSourceReadyPath())
		pr_info(u"转换后的文件放在: " + self.getDestPath())

if __name__ == "__main__":
	convert = FFMpegConvert()
	convert.main(sys.argv[1:])
