using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Configuration;

namespace FFMpegConvert {
    public partial class FFMpegConvert : Form {

        private const String CONFIG_INPUT_PATH = "inPath";
        private const String CONFIG_OUTPUT_PATH = "outPath";
        private const String CONFIG_LOG_PATH = "logPath";
        private const String CONFIG_VIDEO_CODEC = "video_codec";
        private const String CONFIG_VIDEO_BITRATE = "video_bitrate";
        private const String CONFIG_AUDIO_CODEC = "audio_codec";
        private const String CONFIG_AUDIO_BITRATE = "audio_bitrate";
        private const String CONFIG_FORMAT = "format";
        private const String CONFIG_PARAMS = "params";

        private static String[] VIDEO_EXT_LIST = {
#if false
            "mkv", "mp4", "rm", "rmvb", "avi", "wmv", "flv", "mov", "m2v", "vob", "3gp", "mpeg", "mpg", "mpe", "ra", "ram", "asf"
#else
            "mp4"
#endif
        };

        private bool mHiddenCmdline;
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
        private String mInPath;
        private String mOutPath;
        private String mLogFilePath;

        public FFMpegConvert() {
            InitializeComponent();

            String text = ConfigurationManager.AppSettings[CONFIG_INPUT_PATH];
            if (text != null && text.Length > 0 && (File.Exists(text) || Directory.Exists(text))) {
                textBoxInPath.Text = text;
            }

            text = ConfigurationManager.AppSettings[CONFIG_OUTPUT_PATH];
            if (text != null && text.Length > 0) {
                textBoxOutPath.Text = text;
            }

            text = ConfigurationManager.AppSettings[CONFIG_LOG_PATH];
            if (text != null && text.Length > 0 && File.Exists(text)) {
                textBoxLogFile.Text = text;
            }

            text = ConfigurationManager.AppSettings[CONFIG_VIDEO_CODEC];
            if (text != null && text.Length > 0) {
                comboBoxVideoCodec.Text = text;
            } else {
                comboBoxVideoCodec.SelectedIndex = 0;
            }

            text = ConfigurationManager.AppSettings[CONFIG_VIDEO_BITRATE];
            if (text != null && text.Length > 0) {
                comboBoxVideoBitRate.Text = text;
            } else {
                comboBoxVideoBitRate.SelectedIndex = 0;
            }

            text = ConfigurationManager.AppSettings[CONFIG_AUDIO_CODEC];
            if (text != null && text.Length > 0) {
                comboBoxAudioCodec.Text = text;
            } else {
                comboBoxAudioCodec.SelectedIndex = 0;
            }

            text = ConfigurationManager.AppSettings[CONFIG_AUDIO_BITRATE];
            if (text != null && text.Length > 0) {
                comboBoxAudioBitRate.Text = text;
            } else {
                comboBoxAudioBitRate.SelectedIndex = 0;
            }

            text = ConfigurationManager.AppSettings[CONFIG_FORMAT];
            if (text != null && text.Length > 0) {
                comboBoxOutputFormat.Text = text;
            } else {
                comboBoxOutputFormat.SelectedIndex = 0;
            }

            text = ConfigurationManager.AppSettings[CONFIG_PARAMS];
            if (text != null && text.Length > 0) {
                textBoxParam.Text = text;
            }

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

            FileInfo tmpInfo = new FileInfo(Path.Combine(outInfo.DirectoryName, Path.GetFileNameWithoutExtension(outInfo.Name) + ".ffmpeg.temp" + outInfo.Extension));

            if (tmpInfo.Exists) {
                println("Delete: " + tmpInfo.FullName);
                tmpInfo.Delete();
            }

            if (!doConvertVideo(inInfo.FullName, tmpInfo.FullName)) {
                tmpInfo.Delete();
                return false;
            }

            try {
                println("Move: " + tmpInfo.FullName + " -> " + outInfo.FullName);
                tmpInfo.MoveTo(outInfo.FullName);
            } catch (Exception) {
                tmpInfo.Delete();
                return false;
            }

            mStreamWriterLog.WriteLine(inInfo.FullName + " " + outInfo.FullName + " " + mCommandParam);
            mStreamWriterLog.Flush();

            return true;
        }

        private bool doConvertFile(FileInfo inInfo, String outPath) {
            FileInfo outInfo;

            if (Directory.Exists(outPath)) {
                outInfo = new FileInfo(Path.Combine(outPath, Path.GetFileNameWithoutExtension(inInfo.Name) + "." + mOutputFormat));
            } else {
                outInfo = new FileInfo(outPath);
            }

            return doConvertFile(inInfo, outInfo);
        }

        private int doConvertDir(DirectoryInfo dirInfo, String dirOut) {
            int failCount = 0;

            if (backgroundWorkerConvert.CancellationPending) {
                return -1;
            }

            if (dirInfo.FullName.StartsWith(dirOut)) {
                return 0;
            }

            Directory.CreateDirectory(dirOut);

            foreach (FileInfo info in dirInfo.GetFiles()) {
                if (backgroundWorkerConvert.CancellationPending) {
                    return -1;
                }

                if (!doConvertFile(info, dirOut)) {
                    failCount++;
                }
            }

            foreach (DirectoryInfo info in dirInfo.GetDirectories()) {
                String basename = info.Name;

                int count = doConvertDir(info, Path.Combine(dirOut, basename));
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

            return doConvertDir(dirInfo, dirOut);
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
            buttonInDir.Enabled = enable;
            buttonInFile.Enabled = enable;
            buttonOutDir.Enabled = enable;
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

            Directory.CreateDirectory(mOutPath);

            mCommandParam = buildCommandParam();
            mStreamWriterLog = new StreamWriter(mLogFilePath, true);

            if (File.Exists(mInPath)) {
                if (doConvertFile(new FileInfo(mInPath), mOutPath)) {
                    count = 0;
                } else {
                    count = -1;
                }
            } else if (Directory.Exists(mInPath)) {
                count = doConvertDir(mInPath, mOutPath);
            } else {
                count = -1;
            }

            mStreamWriterLog.Close();
            mStreamWriterLog = null;

            if (!backgroundWorkerConvert.CancellationPending) {
                if (count == 0) {
                    MessageBox.Show(" 恭喜，转换成功");
                } else if (count < 0) {
                    MessageBox.Show("转换失败，请检查！");
                } else {
                    MessageBox.Show("有" + count + "个文件转换失败，请检查！");
                }
            }
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
            if (backgroundWorkerConvert.IsBusy) {
                return;
            }

            backgroundWorkerConvert.RunWorkerAsync();
        }

        private void buttonStop_Click(object sender, EventArgs e) {
            if (backgroundWorkerConvert.IsBusy) {
                backgroundWorkerConvert.CancelAsync();

                setConvertState("正在停止，请稍候");

                if (mConvertProcess != null) {
                    mConvertProcess.Kill();
                }
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

        private void backgroundWorkerConvert_DoWork(object sender, DoWorkEventArgs args) {
            backgroundWorkerConvert.ReportProgress(0);

            try {
                doConvert();
            } catch (Exception e) {
                MessageBox.Show(e.ToString());
            }
        }

        private void backgroundWorkerConvert_ProgressChanged(object sender, ProgressChangedEventArgs e) {
            switch (e.ProgressPercentage) {
            case 0:
                Cursor = Cursors.WaitCursor;

                setConvertState(true);

                mInPath = textBoxInPath.Text;
                mOutPath = textBoxOutPath.Text;

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

                mLogFilePath = textBoxLogFile.Text;
                if (mLogFilePath.Length <= 0) {
                    mLogFilePath = Path.Combine(mOutPath, "ffmpeg-convert.log");
                    textBoxLogFile.Text = mLogFilePath;
                }
                break;
            }
        }

        private void backgroundWorkerConvert_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e) {
            setConvertState(false);
            Cursor = Cursors.Default;
        }

        private void FFMpegConvert_FormClosing(object sender, FormClosingEventArgs args) {
            buttonStop_Click(null, null);

            try {
                Configuration config = ConfigurationManager.OpenExeConfiguration(Application.ExecutablePath);
                if (config == null) {
                    return;
                }

                KeyValueConfigurationCollection settings = config.AppSettings.Settings;
                settings.Clear();

                String text = textBoxInPath.Text;
                if (text.Length > 0) {
                    settings.Add(CONFIG_INPUT_PATH, text);
                }

                text = textBoxOutPath.Text;
                if (text.Length > 0) {
                    settings.Add(CONFIG_OUTPUT_PATH, text);
                }

                text = textBoxLogFile.Text;
                if (text.Length > 0) {
                    settings.Add(CONFIG_LOG_PATH, text);
                }

                settings.Add(CONFIG_VIDEO_CODEC, comboBoxVideoCodec.Text);
                settings.Add(CONFIG_VIDEO_BITRATE, comboBoxVideoBitRate.Text);
                settings.Add(CONFIG_FORMAT, comboBoxOutputFormat.Text);
                settings.Add(CONFIG_PARAMS, textBoxParam.Text);

                if (comboBoxAudioCodec.SelectedIndex > 0) {
                    settings.Add(CONFIG_AUDIO_CODEC, comboBoxAudioCodec.Text);
                }

                if (comboBoxAudioBitRate.SelectedIndex > 0) {
                    settings.Add(CONFIG_AUDIO_BITRATE, comboBoxAudioBitRate.Text);
                }

                config.Save(ConfigurationSaveMode.Full);
            } catch (Exception e) {
                MessageBox.Show(e.ToString());
            }
        }
    }
}
