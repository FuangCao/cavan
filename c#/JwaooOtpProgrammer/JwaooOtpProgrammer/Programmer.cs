using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace JwaooOtpProgrammer {

    public partial class Programmer : Form {

        private static String[] sProgramFiles = {
            "C:\\Program Files (x86)", "C:\\Program Files", "D:\\Program Files (x86)", "D:\\Program Files"
        };

        private static String sFileOtpHeaderBin = Path.Combine(Application.LocalUserAppDataPath, "otp_header.bin");
        private static String sFileOtpFirmwareBin = Path.Combine(Application.LocalUserAppDataPath, "otp_firmware.bin");

        private static String sFileBdAddrTxt = Path.Combine(Application.StartupPath, "mac.txt");
        private static String sFileProgrammerBin = Path.Combine(Application.StartupPath, "jtag_programmer.bin");
        private static String[] sOtpCommandArgs = { "-type", "otp", "-chip", "DA14580-01", "-jtag", "123456", "-baudrate", "57600" };

        private static byte[] sOtpBootMagic = { 0x12, 0x34, 0xA5, 0xA5, 0xA5, 0xA5, 0x12, 0x34 };

        private UInt64 mBdAddress;
        private String mFileSmartSnippetsExe;
        private FileStream mFileStreamLog;

        delegate void Process_OutputDataReceivedDelegate(object sender, DataReceivedEventArgs e);

        public Programmer() {
            InitializeComponent();

            mFileStreamLog = File.Open(Path.Combine(Application.StartupPath, "log.txt"), FileMode.Append, FileAccess.Write, FileShare.Read);
            openFileDialogFirmware.InitialDirectory = Application.StartupPath;
            openFileDialogSmartSnippets.InitialDirectory = Application.StartupPath;
            setBdAddress(readBdAddressFile());
        }

        ~Programmer() {
            if (mFileStreamLog != null) {
                mFileStreamLog.Close();
                mFileStreamLog = null;
            }
        }

        public bool writeLogFile(String text) {
            if (mFileStreamLog == null) {
                return false;
            }

            try {
                byte[] bytes = Encoding.UTF8.GetBytes(text);
                mFileStreamLog.Write(bytes, 0, bytes.Length);
                mFileStreamLog.Flush();
                return true;
            } catch {
                return false;
            }
        }

        public bool writeLog(String line) {
            return writeLogFile(line) && writeLogFile("\r\n");
        }

        public void appendLog(String line) {
            textBoxLog.AppendText(line);
            textBoxLog.AppendText("\r\n");
        }

        private void setBdAddress(UInt64 addr) {
            mBdAddress = addr;
            textBoxBdAddressNext.Text = getBdAddressString(addr);
        }

        private bool addBdAddress() {
            UInt64 addr = mBdAddress + 1;

            if (writeBdAddressFile(addr)) {
                setBdAddress(addr);
                return true;
            }

            return false;
        }

        private String getBdAddressString(UInt64 value) {
            StringBuilder builder = new StringBuilder();

            for (int offset = 40; offset >= 0; offset -= 8) {
                builder.Append(valueToChar((int) ((value >> (offset + 4)) & 0x0F)));
                builder.Append(valueToChar((int) ((value >> offset) & 0x0F)));

                if (offset > 0) {
                    builder.Append(':');
                }
            }

            return builder.ToString();
        }

        private String getBdAddressString(byte[] bytes, int offset) {
            StringBuilder builder = new StringBuilder();

            for (int i = offset + 5; i >= offset; i--) {
                byte value = bytes[i];
                builder.Append(valueToChar((value >> 4) & 0x0F));
                builder.Append(valueToChar(value & 0x0F));

                if (i > offset) {
                    builder.Append(':');
                }
            }

            return builder.ToString();
        }

        private UInt64 getBdAddressValue(String text) {
            if (text == null) {
                return 0;
            }

            String[] texts = text.Split(':', '-');
            if (texts.Length != 6) {
                return 0;
            }

            UInt64 value = 0;

            try {
                foreach (String node in texts) {
                    if (node.Length != 2) {
                        return 0;
                    }

                    value = (value << 8) | Convert.ToByte(node, 16);
                }
            } catch {
                return 0;
            }

            return value;
        }

        private byte[] getBdAddressBytes(UInt64 addr) {
            byte[] bytes = new byte[6];

            for (int i = bytes.Length - 1; i >= 0;  i--) {
                bytes[i] = (byte)(addr >> (i * 8));
            }

            return bytes;
        }

        private UInt64 readBdAddressFile() {
            FileStream stream = null;

            try {
                stream = File.OpenRead(sFileBdAddrTxt);

                byte[] buff = new byte[32];
                int length = stream.Read(buff, 0, buff.Length);
                String text = Encoding.ASCII.GetString(buff, 0, length);
                UInt64 addr = getBdAddressValue(text);
                if (addr != 0) {
                    return addr;
                }

                if (text.Length > 0) {
                    MessageBox.Show("MAC地址文件：" + sFileBdAddrTxt + "\r\n格式错误：" + text);
                } else {
                    MessageBox.Show("MAC地址文件为空：" + sFileBdAddrTxt);
                }
            } catch (FileNotFoundException) {
                UInt64 addr = 0x88EA00000000;
                writeBdAddressFile(addr);
                return addr;
            } catch (Exception e) {
                MessageBox.Show("读取MAC地址文件失败：\r\n" + e);
            } finally {
                if (stream != null) {
                    stream.Close();
                }
            }

            return 0;
        }

        private bool writeBdAddressFile(UInt64 addr) {
            String text = getBdAddressString(addr);
            if (text == null) {
                return false;
            }

            byte[] bytes = Encoding.ASCII.GetBytes(text);

            FileStream stream = null;

            try {
                stream = File.OpenWrite(sFileBdAddrTxt);
                stream.Write(bytes, 0, bytes.Length);
                return true;
            } catch (Exception) {
                return false;
            } finally {
                if (stream != null) {
                    stream.Close();
                }
            }
        }

        private String findSmartSnippetsPath() {
            foreach (String path in sProgramFiles) {
                String rootDir = Path.Combine(path, "SmartSnippets");
                if (Directory.Exists(rootDir)) {
                    String binDir = Path.Combine(rootDir, "bin");
                    if (Directory.Exists(binDir)) {
                        String smartSnippetsPath = Path.Combine(binDir, "SmartSnippets.exe");
                        if (File.Exists(smartSnippetsPath)) {
                            return smartSnippetsPath;
                        }
                    }
                }
            }

            return null;
        }

        private String getSmartSnippetsPath() {
            if (mFileSmartSnippetsExe != null && File.Exists(mFileSmartSnippetsExe)) {
                return mFileSmartSnippetsExe;
            }

            mFileSmartSnippetsExe = findSmartSnippetsPath();
            if (mFileSmartSnippetsExe != null) {
                return mFileSmartSnippetsExe;
            }

            if (openFileDialogSmartSnippets.ShowDialog() != DialogResult.OK) {
                return null;
            }

            mFileSmartSnippetsExe = openFileDialogSmartSnippets.FileName;

            return mFileSmartSnippetsExe;
        }

        private String doRunCommand(ShellCommandRunner runner) {
            if (runner.execute()) {
                return runner.LastOutputLine;
            }

            return null;
        }

        private String runSmartSnippetsCommand(params String[] args) {
            ShellCommandRunner runner = new ShellCommandRunner(getSmartSnippetsPath(), this);
            runner.addArguments(args);
            return doRunCommand(runner);
        }

        private String runOtpCommand(bool withFirmware, params String[] args) {
            ShellCommandRunner runner = new ShellCommandRunner(getSmartSnippetsPath(), this);
            runner.addArguments(sOtpCommandArgs);

            if (withFirmware) {
                runner.addArguments("-firmware", sFileProgrammerBin);
            }

            runner.addArguments(args);
            return doRunCommand(runner);
        }

        private bool writeOtpData(String offset, String data) {
            appendLog("写裸数据: " + data + " => " + offset);

            String line = runOtpCommand(false, "-cmd", "write_field", "-offset", offset, "-data", data);
            if (line == null) {
                return false;
            }

            if (line.StartsWith("Failed")) {
                return false;
            }

            return line.StartsWith("Burned");
        }

        private char valueToChar(int value) {
            if (value < 10) {
                return (char)('0' + value);
            } else {
                return (char)('A' - 10 + value);
            }
        }

        private String getBytesHexString(byte[] bytes) {
            StringBuilder builder = new StringBuilder();

            foreach (byte value in bytes) {
                builder.Append(valueToChar(value >> 4));
                builder.Append(valueToChar(value & 0x0F));
            }

            return builder.ToString();
        }

        private bool writeOtpData(String offset, byte[] data) {
            return writeOtpData(offset, getBytesHexString(data));
        }

        private bool writeBdAddress() {
            if (mBdAddress == 0) {
                return false;
            }

            byte[] bytes = getBdAddressBytes(mBdAddress);
            if (!writeOtpData("0x7FD4", bytes)) {
                return false;
            }

            textBoxBdAddressCurrent.Text = getBdAddressString(bytes, 0);

            return addBdAddress();
        }

        private bool setOtpBootEnable() {
            return writeOtpData("0x7F00", "1234A5A5A5A51234");
        }

        private bool readOtpHeader(String pathname) {
            appendLog("读取OTP头部到：" + pathname);

            String line = runOtpCommand(true, "-cmd", "read_header", "-file", pathname);
            if (line == null) {
                return false;
            }

            if (line.StartsWith("Failed")) {
                return false;
            }

            return line.StartsWith("Reading is complete");
        }

        private byte[] readOtpHeader() {
            if (!readOtpHeader(sFileOtpHeaderBin)) {
                return null;
            }

            return File.ReadAllBytes(sFileOtpHeaderBin);
        }

        private bool readOtpFirmware(String pathname) {
            appendLog("从OTP读取固件到：" + pathname);

            String line = runOtpCommand(true, "-cmd", "read_custom_code", "-file", pathname);
            if (line == null) {
                return false;
            }

            if (line.StartsWith("OTP memory reading has failed")) {
                return false;
            }

            return line.StartsWith("OTP memory reading has finished");
        }

        private byte[] readOtpFirmware() {
            if (!readOtpFirmware(sFileOtpFirmwareBin)) {
                return null;
            }

            return File.ReadAllBytes(sFileOtpFirmwareBin);
        }

        private bool writeOtpFirmware(String pathname) {
            appendLog("写固件文件到OTP：" + pathname);

            String line = runOtpCommand(false, "-cmd", "write_custom_code", "-y", "-file", pathname);
            if (line == null) {
                return false;
            }

            if (line.StartsWith("OTP Memory burning failed")) {
                return false;
            }

            return line.StartsWith("OTP Memory burning completed successfully");
        }

        private bool isMemoryEmpty(byte[] bytes, int offset, int length) {
            for (int end = offset + length; offset < end; offset++) {
                if (bytes[offset] != 0x00) {
                    return false;
                }
            }

            return true;
        }

        private bool isMemeoryMatch(byte[] mem1, int off1, byte[] mem2, int off2, int length) {
            for (int i = 0; i < length; i++) {
                if (mem1[off1 + i] != mem2[off2 + i]) {
                    return false;
                }
            }

            return true;
        }

        private bool burnOtpFirmwareAll(String pathname) {
            byte[] bytes = readOtpFirmware();
            if (bytes == null) {
                MessageBox.Show("读取固件失败");
                return false;
            }

            appendLog("成功");

            textBoxBdAddressCurrent.Text = getBdAddressString(bytes, 0x7FD4);

            if (isMemoryEmpty(bytes, 0, 0x7F00)) {
                if (!writeOtpFirmware(pathname)) {
                    MessageBox.Show("写固件失败: " + pathname);
                    return false;
                }

                appendLog("成功");
            } else {
                MessageBox.Show("OTP中的固件不为空，可能已经写过了");
                appendLog("已经写过固件了，直接跳过");
            }

            if (isMemoryEmpty(bytes, 0x7FD4, 6)) {
                if (!writeBdAddress()) {
                    MessageBox.Show("写MAC地址失败");
                    return false;
                }

                appendLog("成功");
            } else {
                MessageBox.Show("OTP中的MAC地址不为空，可能已经写过了");
                appendLog("已经写过MAC地址了，直接跳过");
            }

            if (isMemoryEmpty(bytes, 0x7F00, 8)) {
                if (!setOtpBootEnable()) {
                    MessageBox.Show("设置从OTP启动失败");
                    return false;
                }

                appendLog("成功");
            } else if (isMemeoryMatch(bytes, 0x7F00, sOtpBootMagic, 0, sOtpBootMagic.Length)) {
                appendLog("已经设置从OTP启动了，直接跳过");
            } else {
                MessageBox.Show("OTP启动标志位不匹配！！！");
                return false;
            }

            return true;
        }

        private void buttonFirmware_Click(object sender, EventArgs e) {
            if (openFileDialogFirmware.ShowDialog() == DialogResult.OK) {
                textBoxFirmware.Text = openFileDialogFirmware.FileName;
            }
        }

        private void buttonConnect_Click(object sender, EventArgs e) {
            buttonConnect.Enabled = false;
            buttonBurn.Enabled = false;

            byte[] bytes = readOtpHeader();
            if (bytes != null) {
                MessageBox.Show("连接成功");
                appendLog("连接成功");
                textBoxBdAddressCurrent.Text = getBdAddressString(bytes, 0xD4);
                buttonConnect.Enabled = true;
                buttonBurn.Enabled = true;
            } else {
                MessageBox.Show("连接失败");
                appendLog("连接失败！！！");
                buttonConnect.Enabled = true;
            }
        }

        private void buttonBurn_Click(object sender, EventArgs e) {
            String pathname = textBoxFirmware.Text;
            if (pathname == null || pathname.Length == 0) {
                MessageBox.Show("请选择固件文件");
                return;
            }

            if (!File.Exists(pathname)) {
                MessageBox.Show("固件文件不存在：" + pathname);
                return;
            }

            buttonConnect.Enabled = false;
            buttonBurn.Enabled = false;
            buttonFirmware.Enabled = false;

            if (burnOtpFirmwareAll(pathname)) {
                MessageBox.Show("恭喜，烧录成功");
            } else {
                appendLog("烧录失败！！！");
            }

            buttonConnect.Enabled = true;
            buttonBurn.Enabled = true;
            buttonFirmware.Enabled = true;
        }

        private void buttonClearLog_Click(object sender, EventArgs e) {
            textBoxLog.Clear();
        }
    }

    public class ShellCommandRunner {

        private String mCommand;
        private Programmer mProgrammer;
        private List<String> mErrLines = new List<string>();
        private List<String> mOutLines = new List<string>();
        private List<String> mArgumants = new List<string>();

        public ShellCommandRunner(String command, Programmer programmer) {
            mCommand = command;
            mProgrammer = programmer;
        }

        public void setArguments(List<String> args) {
            if (args == null) {
                mArgumants.Clear();
            } else {
                mArgumants = args;
            }
        }

        public void setArguments(params String[] args) {
            mArgumants = new List<string>(args.Length);
            addArguments(args);
        }

        public void addArgument(String arg) {
            mArgumants.Add(arg);
        }

        public void addArguments(params String[] args) {
            foreach (String arg in args) {
                mArgumants.Add(arg);
            }
        }

        public void addArguments(List<String> args) {
            foreach (String arg in args) {
                mArgumants.Add(arg);
            }
        }

        private String linesToString(List<String> lines) {
            StringBuilder builder = new StringBuilder();

            for (int i = 0; i < lines.Count; i++) {
                builder.Append(i + ". " + lines[i] + "\r\n");
            }

            return builder.ToString();
        }

        public bool execute() {
            mProgrammer.writeLog("================================================================================");
            mProgrammer.writeLog("command = " + mCommand);

            if (mCommand == null) {
                return false;
            }

            Process process = new Process();

            if (mArgumants != null && mArgumants.Count > 0) {
                bool needSpace = false;
                StringBuilder builder = new StringBuilder();

                foreach (String arg in mArgumants) {
                    if (needSpace) {
                        builder.Append(" \"");
                    } else {
                        needSpace = true;
                        builder.Append('"');
                    }

                    builder.Append(arg);
                    builder.Append('"');
                }

                process.StartInfo.Arguments = builder.ToString();
            }

            process.StartInfo.FileName = mCommand;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.OutputDataReceived += Process_OutputDataReceived;
            process.ErrorDataReceived += Process_ErrorDataReceived;

            mProgrammer.writeLog("arguments = " + process.StartInfo.Arguments);

            mOutLines.Clear();
            mErrLines.Clear();

            try {
                if (process.Start()) {
                    process.BeginOutputReadLine();
                    process.BeginErrorReadLine();
                    process.WaitForExit();

                    if (process.ExitCode != 0) {
                        return false;
                    }

                    return true;
                }
            } catch (Exception e) {
                MessageBox.Show("Error: " + e);
            }

            return false;
        }

        public List<String> OutputLines {
            get {
                return mOutLines;
            }
        }

        public int OutputLineCount {
            get {
                return mOutLines.Count;
            }
        }

        public String LastOutputLine {
            get {
                int count = mOutLines.Count;
                if (count > 0) {
                    return mOutLines[count - 1];
                }

                return null;
            }
        }

        public List<String> ErrorLines {
            get {
                return mErrLines;
            }
        }

        public String getOutputLine(int index) {
            return mOutLines[index];
        }

        public String getErrorLine(int index) {
            return mErrLines[index];
        }

        private void Process_ErrorDataReceived(object sender, DataReceivedEventArgs e) {
            if (e.Data != null) {
                mErrLines.Add(e.Data);
                mProgrammer.writeLog(e.Data);
            }
        }

        private void Process_OutputDataReceived(object sender, DataReceivedEventArgs e) {
            if (e.Data != null) {
                mOutLines.Add(e.Data);
                mProgrammer.writeLog(e.Data);
            }
        }
    }
}
