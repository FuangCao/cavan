﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace FFMpegConvert {
    public partial class FFMpegConvert : Form {
        private static String FILENAME_BACKUP = "ffmpeg-backup";
        private static String[] VIDEO_EXT_LIST = {
#if false
            "mkv", "mp4", "rm", "rmvb", "avi", "wmv", "flv", "mov", "m2v", "vob", "3gp", "mpeg", "mpg", "mpe", "ra", "ram", "asf"
#else
            "mp4"
#endif
        };

        private bool mHiddenCmdline;
        private bool mStopRequired;
        private Thread mConvertThread;
        private Process mConvertProcess;
        private StreamWriter mStreamWriterLog;

        private bool mOverride;
        private String mVideoCodec;
        private String mVideoBitRate;
        private String mVideoCodecParam;
        private String mOutputFormat;
        private String mAudioCodec;
        private String mAudioBitRate;
        private String mCommandParam;
        private String mDirInput;
        private String mDirOutput;
        private String mLogFilePath;

        public FFMpegConvert() {
            InitializeComponent();

            comboBoxVideoCodec.SelectedIndex = 0;
            comboBoxVideoBitRate.SelectedIndex = 0;
            comboBoxAudioCodec.SelectedIndex = 0;
            comboBoxAudioBitRate.SelectedIndex = 0;
            comboBoxOutputFormat.SelectedIndex = 0;

            StringBuilder builder = new StringBuilder("视频文件|");

            for (int i = 0; i < VIDEO_EXT_LIST.Length; i++) {
                if (i > 0) {
                    builder.Append(';');
                }

                builder.Append("*.").Append(VIDEO_EXT_LIST[i]);
            }

            openFileDialogInFile.Filter = builder.ToString();

            updateStartButtonState();
        }

        private delegate void WriteLogCallback(String line);

        private void writeLog(String log) {
            if (textBoxLog.InvokeRequired) {
                Invoke(new WriteLogCallback(writeLog), new object[] { log });
            } else {
                textBoxLog.AppendText(log);
            }
        }

        private void print(String log) {
            writeLog(log);
        }

        private void println(String log) {
            print(log + "\n");
        }

        private bool isVideoFile(String ext) {
            foreach (String item in VIDEO_EXT_LIST) {
                if (String.Compare(ext, "." + item, true) == 0) {
                    return true;
                }
            }

            return false;
        }

        private bool doCopyFile(String fileIn, String fileOut) {
            println("复制：" + fileIn);
            setConvertState("正在复制：" + fileIn);

            try {
                File.Copy(fileIn, fileOut, true);
            } catch (Exception) {
                return false;
            }

            return true;
        }

        private bool doConvertVideo(String fileIn, String fileOut) {
            setConvertState("正在转换：" + fileIn);

            StringBuilder builder = new StringBuilder();
            builder.Append("-i \"");
            builder.Append(fileIn);
            builder.Append("\" ");
            builder.Append(mCommandParam);
            builder.Append(" \"");
            builder.Append(fileOut);
            builder.Append("\"");
            String arguments = builder.ToString();

            println("ffmpeg " + arguments);

            try {
                ProcessStartInfo info = new ProcessStartInfo("ffmpeg", arguments);

                if (mHiddenCmdline) {
                    info.CreateNoWindow = true;
                    info.UseShellExecute = false;
                    info.WindowStyle = ProcessWindowStyle.Hidden;
                }

                Process process = Process.Start(info);
                if (process == null) {
                    MessageBox.Show("启动ffmpeg失败！");
                    return false;
                }

                mConvertProcess = process;
                process.WaitForExit();

                if (process.ExitCode != 0) {
                    return false;
                }
            } catch (Exception e) {
                MessageBox.Show(e.ToString());
                return false;
            } finally {
                if (mConvertProcess != null) {
                    if (!mConvertProcess.HasExited) {
                        mConvertProcess.Kill();
                    }

                    mConvertProcess.Close();
                    mConvertProcess = null;
                }
            }

            return true;
        }

        private String doConvertFilename(String filename) {
            filename = Path.GetFileNameWithoutExtension(filename);

            if (mVideoCodec.Equals("libx265")) {
                filename = Regex.Replace(filename, "x264", "x265", RegexOptions.IgnoreCase);
            }

            filename = Regex.Replace(filename, "dts", "AC3", RegexOptions.IgnoreCase);

            return filename;
        }

        private bool doConvertFile(FileInfo inInfo, FileInfo outInfo) {
            if (!isVideoFile(Path.GetExtension(inInfo.Name))) {
                println(inInfo.FullName + " is not video file.");
                return true;
            }

            if (outInfo.Exists) {
                if (mOverride) {
                    println(outInfo.FullName + " is exists.");
                    return true;
                }

                println("Delete: " + outInfo.FullName);
                outInfo.Delete();
            }

            FileInfo cacheInfo = new FileInfo(Path.Combine(outInfo.DirectoryName, outInfo.Name + ".ffmpeg.cache" + outInfo.Extension));

            if (cacheInfo.Exists) {
                println("Delete: " + cacheInfo.FullName);
                cacheInfo.Delete();
            }

            if (!doConvertVideo(inInfo.FullName, cacheInfo.FullName)) {
                cacheInfo.Delete();
                return false;
            }

            try {
                println("Move: " + cacheInfo.FullName + " -> " + outInfo.FullName);
                cacheInfo.MoveTo(outInfo.FullName);
            } catch (Exception) {
                cacheInfo.Delete();
                return false;
            }

            mStreamWriterLog.WriteLine(inInfo.FullName + " " + outInfo.FullName + " " + mCommandParam);
            mStreamWriterLog.Flush();

            return true;
        }

        private bool doConvertFile(String inPath, String outPath) {
            FileInfo inInfo = new FileInfo(inPath);
            FileInfo outInfo;

            if (Directory.Exists(outPath)) {
                outInfo = new FileInfo(Path.Combine(outPath, Path.GetFileNameWithoutExtension(inInfo.Name) + "." + mOutputFormat));
            } else {
                outInfo = new FileInfo(outPath);
            }

            return doConvertFile(inInfo, outInfo);
        }

        private bool doConvertFile(FileInfo inInfo, String outDir, String bakDir) {
            FileInfo outInfo = new FileInfo(Path.Combine(outDir, Path.GetFileNameWithoutExtension(inInfo.Name) + "." + mOutputFormat));

            if (!doConvertFile(inInfo, outInfo)) {
                return false;
            }

            try {
                String bakFile = Path.Combine(bakDir, inInfo.Name);
                File.Delete(bakFile);
                inInfo.MoveTo(bakFile);
            } catch (Exception) {
                return false;
            }

            return true;
        }

        private int doConvertDir(DirectoryInfo dirInfo, String dirOut, String dirBak) {
            int failCount = 0;

            if (mStopRequired) {
                return -1;
            }

            if (dirInfo.FullName.StartsWith(dirOut)) {
                return 0;
            }

            Directory.CreateDirectory(dirOut);
            Directory.CreateDirectory(dirBak);

            foreach (FileInfo info in dirInfo.GetFiles()) {
                if (mStopRequired) {
                    return -1;
                }

                if (!doConvertFile(info, dirOut, dirBak)) {
                    failCount++;
                }
            }

            foreach (DirectoryInfo info in dirInfo.GetDirectories()) {
                if (info.FullName.Equals(dirBak)) {
                    continue;
                }

                String basename = info.Name;

                int count = doConvertDir(info, Path.Combine(dirOut, basename), Path.Combine(dirBak, basename));
                if (count < 0) {
                    return count;
                }

                failCount += count;
            }

            return failCount;
        }

        private int doConvertDir(String dirIn, String dirOut) {
            DirectoryInfo dirInfo = new DirectoryInfo(dirIn);
            if (dirInfo == null) {
                return -1;
            }

            String dirBack = Path.Combine(dirIn, FILENAME_BACKUP);

            return doConvertDir(dirInfo, dirOut, dirBack);
        }

        private delegate void SetConvertStateCallback(bool running);

        private void setConvertState(bool running) {
            if (buttonStart.InvokeRequired) {
                Invoke(new SetConvertStateCallback(setConvertState), new object[] { running });
            } else if (running) {
                buttonStart.Enabled = false;
                buttonStop.Enabled = true;
                setConvertState("正在转换");
            } else {
                buttonStart.Enabled = true;
                buttonStop.Enabled = false;
                setConvertState("已停止转换");
            }

            bool enable = !running;

            textBoxInPath.Enabled = enable;
            textBoxOutPath.Enabled = enable;
            comboBoxOutputFormat.Enabled = enable;
            comboBoxVideoCodec.Enabled = enable;
            comboBoxVideoBitRate.Enabled = enable;
            textBoxParam.Enabled = enable;
            comboBoxAudioCodec.Enabled = enable;
            comboBoxAudioBitRate.Enabled = enable;
            textBoxLogFile.Enabled = enable;
            checkBoxOverride.Enabled = enable;
            checkBoxHiddenCmdline.Enabled = enable;
        }

        private delegate void SetConvertStateMessageCallback(String state);

        private void setConvertState(String state) {
            if (labelState.InvokeRequired) {
                Invoke(new SetConvertStateMessageCallback(setConvertState), new object[] { state });
            } else {
                labelState.Text = state;
            }
        }

        private String buildCommandParam() {
            StringBuilder builder = new StringBuilder();

            if (mAudioCodec != null && mAudioBitRate != null) {
                builder.Append("-acodec " + mAudioCodec);
                builder.Append(" -b:a " + mAudioBitRate);
            }

            builder.Append(" -vcodec " + mVideoCodec);
            builder.Append(" -b:v " + mVideoBitRate);

            if (mVideoCodecParam.Length > 0) {
                builder.Append(" " + mVideoCodecParam);
            }

            return builder.ToString();
        }

        private void doConvert() {
            int count;

            Directory.CreateDirectory(mDirOutput);

            mStopRequired = false;
            mCommandParam = buildCommandParam();
            mStreamWriterLog = new StreamWriter(mLogFilePath, true);

            setConvertState(true);

            if (File.Exists(mDirInput)) {
                if (doConvertFile(mDirInput, mDirOutput)) {
                    count = 0;
                } else {
                    count = -1;
                }
            } else if (Directory.Exists(mDirInput)) {
                count = doConvertDir(mDirInput, mDirOutput);
            } else {
                count = -1;
            }

            mStreamWriterLog.Close();
            mStreamWriterLog = null;

            mConvertThread = null;

            if (!mStopRequired) {
                if (count == 0) {
                    MessageBox.Show(" 恭喜，转换成功");
                } else if (count < 0) {
                    MessageBox.Show("转换失败，请检查！");
                } else {
                    MessageBox.Show("有" + count + "个文件转换失败，请检查！");
                }
            }

            setConvertState(false);
        }

        private void updateStartButtonState() {
            if (textBoxInPath.Text.Length > 0 && textBoxOutPath.Text.Length > 0) {
                buttonStart.Enabled = true;
                setConvertState("点击“开始”按钮开始转换");
            } else {
                buttonStart.Enabled = false;

                if (textBoxInPath.Text.Length > 0) {
                    setConvertState("请选择输出路径");
                } else if (textBoxOutPath.Text.Length > 0) {
                    setConvertState("请选择输入路径");
                } else {
                    setConvertState("请选择输入和输出路径");
                }
            }
        }

        private void buttonStart_Click(object sender, EventArgs e) {
            if (mConvertThread != null) {
                return;
            }

            mDirInput = textBoxInPath.Text;
            mVideoCodec = comboBoxVideoCodec.Text;
            mOutputFormat = comboBoxOutputFormat.Text;
            mVideoBitRate = comboBoxVideoBitRate.Text;
            mVideoCodecParam = textBoxParam.Text;

            if (comboBoxAudioCodec.SelectedIndex > 0) {
                mAudioCodec = comboBoxAudioCodec.Text;
            } else {
                mAudioCodec = null;
            }

            if (comboBoxAudioBitRate.SelectedIndex > 0) {
                mAudioBitRate = comboBoxAudioBitRate.Text;
            } else {
                mAudioBitRate = null;
            }

            mOverride = checkBoxOverride.Checked;
            mHiddenCmdline = checkBoxHiddenCmdline.Checked;

            mDirOutput = Path.Combine(textBoxOutPath.Text, DateTime.Now.ToString("yyyyMMdd"));

            mLogFilePath = textBoxLogFile.Text;
            if (mLogFilePath.Length <= 0) {
                mLogFilePath = Path.Combine(mDirOutput, "ffmpeg-convert.log");
                textBoxLogFile.Text = mLogFilePath;
            }

            mConvertThread = new Thread(new ThreadStart(doConvert));
            mConvertThread.Start();
        }

        private void buttonStop_Click(object sender, EventArgs e) {
            mStopRequired = true;

            if (mConvertThread == null) {
                return;
            }

            setConvertState("正在停止，请稍候");

            if (mConvertProcess != null) {
                mConvertProcess.Kill();
            }
        }

        private void textBoxLogFile_Click(object sender, EventArgs e) {
            if (textBoxLogFile.Text.Length > 0) {
                saveFileDialogLogFile.FileName = textBoxLogFile.Text;
            } else if (textBoxOutPath.Text.Length > 0) {
                saveFileDialogLogFile.InitialDirectory = textBoxOutPath.Text;
            }

            if (saveFileDialogLogFile.ShowDialog() == DialogResult.OK) {
                textBoxLogFile.Text = saveFileDialogLogFile.FileName;
            }
        }

        private void buttonClear_Click(object sender, EventArgs e) {
            textBoxLog.Clear();
        }

        private void comboBoxOutputFormat_SelectedIndexChanged(object sender, EventArgs e) {
            switch (comboBoxOutputFormat.SelectedIndex) {
            case 1:
                textBoxParam.Text = "-x264-params \"level = 5.0\" -bf 0";
                break;

            default:
                textBoxParam.Text = "-r 30";
                break;
            }
        }

        private void buttonInFile_Click(object sender, EventArgs e) {
            if (openFileDialogInFile.ShowDialog() == DialogResult.OK) {
                textBoxInPath.Text = openFileDialogInFile.FileName;
                updateStartButtonState();
            }
        }

        private void buttonInDir_Click(object sender, EventArgs e) {
            if (textBoxInPath.Text.Length > 0) {
                folderBrowserDialogInDir.SelectedPath = textBoxInPath.Text;
            } else {
                folderBrowserDialogInDir.SelectedPath = "D:\\test-input";
            }

            if (folderBrowserDialogInDir.ShowDialog() == DialogResult.OK) {
                textBoxInPath.Text = folderBrowserDialogInDir.SelectedPath;
                updateStartButtonState();
            }
        }

        private void buttonOutDir_Click(object sender, EventArgs e) {
            if (textBoxOutPath.Text.Length > 0) {
                folderBrowserDialogOutDir.SelectedPath = textBoxOutPath.Text;
            } else {
                folderBrowserDialogOutDir.SelectedPath = "D:\\test-output";
            }

            if (folderBrowserDialogOutDir.ShowDialog() == DialogResult.OK) {
                textBoxOutPath.Text = folderBrowserDialogOutDir.SelectedPath;
                updateStartButtonState();
            }
        }
    }
}
