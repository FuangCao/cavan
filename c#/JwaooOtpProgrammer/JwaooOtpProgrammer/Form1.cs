using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace JwaooOtpProgrammer {

    public partial class Form1 : Form {

        private static String[] sProgramFiles = {
            "C:\\Program Files (x86)", "C:\\Program Files", "D:\\Program Files (x86)", "D:\\Program Files"
        };

        private static String sBdAddrPath = Path.Combine(Application.StartupPath, "bd_address.txt");
        private static String sProgrammerPath = Path.Combine(Application.StartupPath, "jtag_programmer.bin");
        private static String[] sOtpCommandArgs = { "-type", "otp", "-chip", "DA14580-01", "-jtag", "123456", "-baudrate", "57600", "-firmware", sProgrammerPath };

        private byte[] mBdAddress;
        private String mSmartSnippetsPath;

        delegate void Process_OutputDataReceivedDelegate(object sender, DataReceivedEventArgs e);

        public Form1() {
            InitializeComponent();
            openFileDialogFirmware.InitialDirectory = Application.StartupPath;
            openFileDialogSmartSnippets.InitialDirectory = Application.StartupPath;

            mBdAddress = readBdAddressFile();
            textBoxBdAddress.Text = getBdAddressString(mBdAddress);
        }

        private String getBdAddressString(byte[] bytes) {
            if (bytes == null || bytes.Length != 6) {
                return null;
            }

            StringBuilder builder = new StringBuilder();

            for (int i = bytes.Length - 1; i >= 0; i--) {
                byte value = bytes[i];
                builder.Append(valueToChar(value >> 4));
                builder.Append(valueToChar(value & 0x0F));

                if (i > 0) {
                    builder.Append(':');
                }
            }

            return builder.ToString();
        }

        private byte[] getBdAddressBytes(String text) {
            if (text == null) {
                return null;
            }

            String[] texts = text.Split(':');
            if (texts.Length != 6) {
                return null;
            }

            byte[] bytes = new byte[6];
            int index = bytes.Length;

            foreach (String node in texts) {
                bytes[--index] = Convert.ToByte(node, 16);
            }

            return bytes;
        }

        private byte[] readBdAddressFile() {
            FileStream stream = null;

            try {
                stream = File.OpenRead(sBdAddrPath);

                byte[] buff = new byte[32];
                int length = stream.Read(buff, 0, buff.Length);
                String text = Encoding.ASCII.GetString(buff, 0, length);

                return getBdAddressBytes(text);
            } catch {
                return null;
            } finally {
                if (stream != null) {
                    stream.Close();
                }
            }
        }

        private bool writeBdAddressFile(byte[] bytes) {
            String text = getBdAddressString(bytes);
            if (text == null) {
                return false;
            }

            bytes = Encoding.ASCII.GetBytes(text);

            FileStream stream = null;

            try {
                stream = File.OpenWrite(sBdAddrPath);
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
            if (mSmartSnippetsPath != null && File.Exists(mSmartSnippetsPath)) {
                return mSmartSnippetsPath;
            }

            mSmartSnippetsPath = findSmartSnippetsPath();
            if (mSmartSnippetsPath != null) {
                return mSmartSnippetsPath;
            }

            if (openFileDialogSmartSnippets.ShowDialog() != DialogResult.OK) {
                return null;
            }

            mSmartSnippetsPath = openFileDialogSmartSnippets.FileName;

            return mSmartSnippetsPath;
        }

        private ShellCommandRunner runSmartSnippetsCommand(params String[] args) {
            ShellCommandRunner runner = new ShellCommandRunner(getSmartSnippetsPath());
            runner.addArguments(args);
            if (runner.execute()) {
                return runner;
            }

            return null;
        }

        private ShellCommandRunner runOtpCommand(params String[] args) {
            ShellCommandRunner runner = new ShellCommandRunner(getSmartSnippetsPath());
            runner.addArguments(sOtpCommandArgs);
            runner.addArguments(args);
            if (runner.execute()) {
                return runner;
            }

            return null;
        }

        private bool writeOtpData(String offset, String data) {
            ShellCommandRunner runner = runOtpCommand("-cmd", "write_field", "-offset", offset, "-data", data);
            if (runner == null) {
                return false;
            }

            String line = runner.LastOutputLine;
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

        private bool writeBdAddress(byte[] addr) {
            if (addr == null || addr.Length != 6) {
                return false;
            }

            return writeOtpData("0x7FD4", addr);
        }

        private bool setOtpBootEnable() {
            return writeOtpData("0x7F00", "1234A5A5A5A51234");
        }

        private bool readOtpHeader(String pathname) {
            ShellCommandRunner runner = runOtpCommand("-cmd", "read_header", "-file", pathname);
            if (runner == null) {
                return false;
            }

            String line = runner.LastOutputLine;
            if (line == null) {
                return false;
            }

            textBoxFirmware.Text = line;

            if (line.StartsWith("Failed")) {
                return false;
            }

            return line.StartsWith("Reading is complete");
        }

        private bool readOtpFirmware(String pathname) {
            ShellCommandRunner runner = runOtpCommand("-cmd", "read_custom_code", "-file", pathname);
            if (runner == null) {
                return false;
            }

            String line = runner.LastOutputLine;
            if (line == null) {
                return false;
            }

            textBoxFirmware.Text = line;

            if (line.StartsWith("OTP memory reading has failed")) {
                return false;
            }

            return line.StartsWith("OTP memory reading has finished");
        }

        private bool writeOtpFirmware(String pathname) {
            ShellCommandRunner runner = runOtpCommand("-cmd", "write_custom_code", "-y", "-file", pathname);
            if (runner == null) {
                return false;
            }

            String line = runner.LastOutputLine;
            if (line == null) {
                return false;
            }

            textBoxFirmware.Text = line;

            if (line.StartsWith("OTP Memory burning failed")) {
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
            bool result = false;
            // result = readOtpHeader("C:\\header.txt");
            result = writeBdAddress(new byte[] { 0x13, 0x12, 0x03, 0x04, 0x14, 0x25 });
            // result = writeOtpFirmware("C:\\jwaoo-toy.hex");
            // result = setOtpBootEnable();
            result = readOtpFirmware("C:\\firmware.txt");
            MessageBox.Show("result = " + result);
        }

        private void buttonBurn_Click(object sender, EventArgs e) {

        }
    }

    public class ShellCommandRunner {

        private String mCommand;
        private List<String> mErrLines = new List<string>();
        private List<String> mOutLines = new List<string>();
        private List<String> mArgumants = new List<string>();

        public ShellCommandRunner(String command) {
            mCommand = command;
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

            MessageBox.Show("Arguments = " + process.StartInfo.Arguments);

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

                    if (mOutLines.Count > 0) {
                        MessageBox.Show(linesToString(mOutLines));
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
            mErrLines.Add(e.Data);
        }

        private void Process_OutputDataReceived(object sender, DataReceivedEventArgs e) {
            if (e.Data != null) {
                mOutLines.Add(e.Data);
            }
        }
    }
}
