using System;
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

namespace FFMpegConvert
{
    public partial class FFMpegConvert : Form
    {
        private static String FILENAME_BACKUP = "ffmpeg-backup";
        private static String[] VIDEO_EXT_LIST = {
            "mkv", "mp4", "rm", "rmvb", "avi", "wmv", "flv", "mov", "m2v", "vob", "3gp", "mpeg", "mpg", "mpe", "ra", "ram", "asf"
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

        public FFMpegConvert()
        {
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

        private void writeLog(String log)
        {
            if (textBoxLog.InvokeRequired)
            {
                Invoke(new WriteLogCallback(writeLog), new object[] { log });
            }
            else
            {
                textBoxLog.AppendText(log);
            }
        }

        private void print(String log)
        {
            writeLog(log);
        }

        private void println(String log)
        {
            print(log + "\n");
        }

        private bool isVideoFile(String ext)
        {
            foreach (String item in VIDEO_EXT_LIST)
            {
                if (String.Compare(ext, "." + item, true) == 0)
                {
                    return true;
                }
            }

            return false;
        }

        private bool doCopyFile(String fileIn, String fileOut)
        {
            println("复制：" + fileIn);
            setConvertState("正在复制：" + fileIn);

            try
            {
                File.Copy(fileIn, fileOut, true);
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }

        private bool doConvertVideo(String fileIn, String fileOut)
        {
            File.Delete(fileOut);

            println("转换: " + fileIn);
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

            MessageBox.Show("arguments = " + arguments);

            try
            {
                ProcessStartInfo info = new ProcessStartInfo("ffmpeg", arguments);

                if (mHiddenCmdline)
                {
                    info.CreateNoWindow = true;
                    info.UseShellExecute = false;
                    info.WindowStyle = ProcessWindowStyle.Hidden;
                }

                Process process = Process.Start(info);
                if (process == null)
                {
                    return false;
                }

                mConvertProcess = process;

                process.WaitForExit();
                if (process.ExitCode != 0)
                {
                    return false;
                }
            }
            catch (Exception)
            {
                return false;
            }
            finally
            {
                if (mConvertProcess != null)
                {
                    if (!mConvertProcess.HasExited)
                    {
                        mConvertProcess.Kill();
                    }

                    mConvertProcess.Close();
                    mConvertProcess = null;
                }
            }

            return true;
        }

        private String doConvertFilename(String filename)
        {
            filename = Path.GetFileNameWithoutExtension(filename);

            if (mVideoCodec.Equals("libx265"))
            {
                filename = Regex.Replace(filename, "x264", "x265", RegexOptions.IgnoreCase);
            }

            filename = Regex.Replace(filename, "dts", "AC3", RegexOptions.IgnoreCase);

            return filename;
        }

        private bool doConvertFile(FileInfo fileInfo, DirectoryInfo dirInfo, String dirOut, String dirBak)
        {
            String fnIn = fileInfo.Name;
            String fnOut, fileOut, fileTmp;
            bool isVideo = isVideoFile(Path.GetExtension(fnIn));

            if (isVideo)
            {
                if (mAudioCodec != null && mAudioBitRate != null) {
                    fnOut = doConvertFilename(fnIn);
                } else {
                    fnOut = fnIn;
                }

                fileOut = Path.Combine(dirOut, fnOut + "." + mOutputFormat);
            }
            else
            {
                fnOut = fnIn;
                fileOut = Path.Combine(dirOut, fnOut);
            }

            if (File.Exists(fileOut))
            {
                if (!mOverride)
                {
                    return true;
                }
            }

            bool success;

            if (isVideo)
            {
                fileTmp = Path.Combine(dirOut, fnOut + ".ffmpeg.cache." + mOutputFormat);
                success = doConvertVideo(fileInfo.FullName, fileTmp);
            }
            else
            {
                fileTmp = Path.Combine(dirOut, fnOut + ".ffmpeg.cache.dat");
                success = doCopyFile(fileInfo.FullName, fileTmp);
            }

            if (!success)
            {
                File.Delete(fileTmp);
                println("失败！");
                return false;
            }

            try
            {
                File.Delete(fileOut);
                File.Move(fileTmp, fileOut);

                String fileBak = Path.Combine(dirBak, fnIn);
                File.Delete(fileBak);
                fileInfo.MoveTo(fileBak);
            }
            catch (Exception)
            {
                return false;
            }

            if (isVideo)
            {
                mStreamWriterLog.WriteLine(fileInfo.FullName + " " + fileOut + " " + mCommandParam);
                mStreamWriterLog.Flush();
            }

            println("成功");

            return true;
        }

        private int doConvertDir(DirectoryInfo dirInfo, String dirOut, String dirBak)
        {
            int failCount = 0;

            if (mStopRequired)
            {
                return -1;
            }

            if (dirInfo.FullName.StartsWith(dirOut))
            {
                return 0;
            }

            Directory.CreateDirectory(dirOut);
            Directory.CreateDirectory(dirBak);

            foreach (FileInfo info in dirInfo.GetFiles())
            {
                if (mStopRequired)
                {
                    return -1;
                }

                if (!doConvertFile(info, dirInfo, dirOut, dirBak))
                {
                    failCount++;
                }
            }

            foreach (DirectoryInfo info in dirInfo.GetDirectories())
            {
                if (info.FullName.Equals(dirBak))
                {
                    continue;
                }

                String basename = info.Name;

                int count = doConvertDir(info, Path.Combine(dirOut, basename), Path.Combine(dirBak, basename));
                if (count < 0)
                {
                    return count;
                }

                failCount += count;
            }

            return failCount;
        }

        private int doConvertDir(String dirIn, String dirOut)
        {
            DirectoryInfo dirInfo = new DirectoryInfo(dirIn);
            if (dirInfo == null)
            {
                return -1;
            }

            String dirBack = Path.Combine(dirIn, FILENAME_BACKUP);

            return doConvertDir(dirInfo, dirOut, dirBack);
        }

        private delegate void SetConvertStateCallback(bool running);

        private void setConvertState(bool running)
        {
            if (buttonStart.InvokeRequired)
            {
                Invoke(new SetConvertStateCallback(setConvertState), new object[] { running });
            }
            else if (running)
            {
                buttonStart.Enabled = false;
                buttonStop.Enabled = true;
                setConvertState("正在转换");
            }
            else
            {
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
            checkBoxShowCmdline.Enabled = enable;
        }

        private delegate void SetConvertStateMessageCallback(String state);

        private void setConvertState(String state)
        {
            if (labelState.InvokeRequired)
            {
                Invoke(new SetConvertStateMessageCallback(setConvertState), new object[] { state });
            }
            else
            {
                labelState.Text = state;
            }
        }

        private String buildCommandParam()
        {
            StringBuilder builder = new StringBuilder();

            if (mAudioCodec != null && mAudioBitRate != null) {
                builder.Append("-acodec " + mAudioCodec);
                builder.Append(" -b:a " + mAudioBitRate);
            }

            builder.Append(" -vcodec " + mVideoCodec);
            builder.Append(" -b:v " + mVideoBitRate);

            if (mVideoCodecParam.Length > 0)
            {
                builder.Append(" " + mVideoCodecParam);
            }

            return builder.ToString();
        }

        private void doConvert()
        {
            int count;

            Directory.CreateDirectory(mDirOutput);

            mStopRequired = false;
            mCommandParam = buildCommandParam();
            MessageBox.Show("mCommandParam = " + mCommandParam);
            mStreamWriterLog = new StreamWriter(mLogFilePath, true);

            setConvertState(true);

            if (File.Exists(mDirInput))
            {
                if (doConvertVideo(mDirInput, mDirOutput))
                {
                    count = 0;
                }
                else
                {
                    count = -1;
                }
            }
            else if (Directory.Exists(mDirInput))
            {
                count = doConvertDir(mDirInput, mDirOutput);
            }
            else
            {
                count = -1;
            }

            mStreamWriterLog.Close();
            mStreamWriterLog = null;

            mConvertThread = null;

            if (!mStopRequired)
            {
                if (count == 0)
                {
                    MessageBox.Show(" 恭喜，转换成功");
                }
                else if (count < 0)
                {
                    MessageBox.Show("转换失败，请检查！");
                }
                else
                {
                    MessageBox.Show("有" + count + "个文件转换失败，请检查！");
                }
            }

            setConvertState(false);
        }

        private void updateStartButtonState()
        {
            if (textBoxInPath.Text.Length > 0 && textBoxOutPath.Text.Length > 0)
            {
                buttonStart.Enabled = true;
                setConvertState("点击“开始”按钮开始转换");
            }
            else
            {
                buttonStart.Enabled = false;

                if (textBoxInPath.Text.Length > 0)
                {
                    setConvertState("请选择输出路径");
                }
                else if (textBoxOutPath.Text.Length > 0)
                {
                    setConvertState("请选择输入路径");
                }
                else
                {
                    setConvertState("请选择输入和输出路径");
                }
            }
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (mConvertThread != null)
            {
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
            mHiddenCmdline = !checkBoxShowCmdline.Checked;

            mDirOutput = Path.Combine(textBoxOutPath.Text, DateTime.Now.ToString("yyyyMMdd"));

            mLogFilePath = textBoxLogFile.Text;
            if (mLogFilePath.Length <= 0)
            {
                mLogFilePath = Path.Combine(mDirOutput, "ffmpeg-convert.log");
                textBoxLogFile.Text = mLogFilePath;
            }

            mConvertThread = new Thread(new ThreadStart(doConvert));
            mConvertThread.Start();
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            mStopRequired = true;

            if (mConvertThread == null)
            {
                return;
            }

            setConvertState("正在停止，请稍候");

            if (mConvertProcess != null)
            {
                mConvertProcess.Kill();
            }
        }

        private void textBoxLogFile_Click(object sender, EventArgs e)
        {
            if (textBoxLogFile.Text.Length > 0)
            {
                saveFileDialogLogFile.FileName = textBoxLogFile.Text;
            }
            else if (textBoxOutPath.Text.Length > 0)
            {
                saveFileDialogLogFile.InitialDirectory = textBoxOutPath.Text;
            }

            if (saveFileDialogLogFile.ShowDialog() == DialogResult.OK)
            {
                textBoxLogFile.Text = saveFileDialogLogFile.FileName;
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
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
                buttonOutFile.Enabled = true;
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
                buttonOutFile.Enabled = false;

                String pathname = textBoxOutPath.Text;
                if (pathname.Length > 0 && !Directory.Exists(pathname)) {
                    textBoxOutPath.Clear();
                }
            }
        }

        private void buttonOutFile_Click(object sender, EventArgs e) {
            String format = comboBoxOutputFormat.Text;

            saveFileDialogOutFile.Filter = format + "文件|*." + format;

            if (saveFileDialogOutFile.ShowDialog() == DialogResult.OK) {
                textBoxOutPath.Text = saveFileDialogOutFile.FileName;
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
