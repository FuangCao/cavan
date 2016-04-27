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

        private bool mNeedStop;
        private Thread mConvertThread;
        private Process mConvertProcess;
        private StreamWriter mStreamWriterLog;

        private bool mOverride;
        private String mVideoCodec;
        private String mVideoBitRate;
        private String mVideoCodecParam;
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
        }

        private void textBoxInputDir_Click(object sender, EventArgs e)
        {
            if (textBoxInDir.Text.Length > 0)
            {
                folderBrowserDialogInDir.SelectedPath = textBoxInDir.Text;
            }
            else
            {
                folderBrowserDialogInDir.SelectedPath = "D:\\test-input"; //"F:\\epan\\media";
            }

            if (folderBrowserDialogInDir.ShowDialog() == DialogResult.OK)
            {
                textBoxInDir.Text = folderBrowserDialogInDir.SelectedPath;

                if (textBoxOutDir.Text.Length > 0)
                {
                    buttonStart.Enabled = true;
                }
            }
        }

        private void textBoxOutputDir_Click(object sender, EventArgs e)
        {
            if (textBoxOutDir.Text.Length > 0)
            {
                folderBrowserDialogOutDir.SelectedPath = textBoxOutDir.Text;
            }
            else
            {
                folderBrowserDialogOutDir.SelectedPath = "D:\\test-output";
            }

            if (folderBrowserDialogOutDir.ShowDialog() == DialogResult.OK)
            {
                textBoxOutDir.Text = folderBrowserDialogOutDir.SelectedPath;

                if (textBoxInDir.Text.Length > 0)
                {
                    buttonStart.Enabled = true;
                }
            }
        }

        private bool doConvertFile(String fileIn, String fileOut)
        {
            File.Delete(fileOut);

            Console.WriteLine(fileIn + " => " + fileOut);

            StringBuilder builder = new StringBuilder();
            builder.Append("-i \"");
            builder.Append(fileIn);
            builder.Append("\" ");
            builder.Append(mCommandParam);
            builder.Append(" \"");
            builder.Append(fileOut);
            builder.Append("\"");
            String arguments = builder.ToString();

            Console.WriteLine("argument = " + arguments);

            try
            {
                mConvertProcess = Process.Start("ffmpeg.exe", arguments);
                mConvertProcess.WaitForExit();

                if (mConvertProcess.ExitCode != 0)
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
                    mConvertProcess.Close();
                    mConvertProcess = null;
                }
            }

            mStreamWriterLog.WriteLine(fileIn + " " + fileOut + " " + mCommandParam);
            mStreamWriterLog.Flush();

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
            String fnOut = doConvertFilename(fnIn);
            String fileTmp = Path.Combine(dirOut, fnOut + ".ffmpeg.cache.mp4");
            String fileOut = Path.Combine(dirOut, fnOut + ".mp4");

            if (File.Exists(fileOut))
            {
                if (!mOverride)
                {
                    return true;
                }
            }

            if (!doConvertFile(fileInfo.FullName, fileTmp))
            {
                return false;
            }

            File.Move(fileTmp, fileOut);
            fileInfo.MoveTo(Path.Combine(dirBak, fnIn));

            return true;
        }

        private int doConvertDir(DirectoryInfo dirInfo, String dirOut, String dirBak)
        {
            int failCount = 0;

            if (mNeedStop)
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
                if (mNeedStop)
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
                String basename = info.Name;

                if (basename.Equals(FILENAME_BACKUP))
                {
                    continue;
                }

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

        delegate void SetConvertStateCallback(bool running);

        private void setConvertState(bool running)
        {
            if (running)
            {
                buttonStart.Enabled = false;
                buttonStop.Enabled = true;
            }
            else
            {
                buttonStart.Enabled = true;
                buttonStop.Enabled = false;
            }
        }

        private String buildCommandParam()
        {
            StringBuilder builder = new StringBuilder();

            builder.Append("-acodec " + mAudioCodec);
            builder.Append(" -b:a " + mAudioBitRate);
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

            mNeedStop = false;
            mCommandParam = buildCommandParam();
            mStreamWriterLog = new StreamWriter(mLogFilePath);

            SetConvertStateCallback callback = new SetConvertStateCallback(setConvertState);
            Invoke(callback, new object[] { true });

            if (File.Exists(mDirInput))
            {
                if (doConvertFile(mDirInput, mDirOutput))
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
            Invoke(callback, new object[] { false });
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (mConvertThread != null)
            {
                return;
            }

            mDirInput = textBoxInDir.Text;
            mVideoCodec = comboBoxVideoCodec.Text;
            mVideoBitRate = comboBoxVideoBitRate.Text;
            mVideoCodecParam = textBoxVidecCodecParam.Text;
            mAudioCodec = comboBoxAudioCodec.Text;
            mAudioBitRate = comboBoxAudioBitRate.Text;
            mOverride = checkBoxOverride.Checked;

            mDirOutput = Path.Combine(textBoxOutDir.Text, DateTime.Now.ToString("yyyyMMdd"));

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
            mNeedStop = true;

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
            else if (textBoxOutDir.Text.Length > 0)
            {
                saveFileDialogLogFile.InitialDirectory = textBoxOutDir.Text;
            }

            if (saveFileDialogLogFile.ShowDialog() == DialogResult.OK)
            {
                textBoxLogFile.Text = saveFileDialogLogFile.FileName;
            }
        }
    }
}
