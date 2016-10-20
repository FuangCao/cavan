using System;
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

        private const String FILE_HEADER = "C:\\header.txt";
        public const String CMD_DIR = "C:\\SmartSnippets\\bin";
        public const String CMD_PATH = CMD_DIR + "\\SmartSnippets.exe";
        public const String FIRMWARE_PATH = "C:\\jtag_programmer.bin";
        public static String[] CMD_OTP_ARGS = { "-type", "otp", "-chip", "DA14580-01", "-jtag", "123456", "-baudrate", "57600", "-firmware", FIRMWARE_PATH };

        delegate void Process_OutputDataReceivedDelegate(object sender, DataReceivedEventArgs e);

        public Form1() {
            InitializeComponent();
        }

        private void Process_OutputDataReceived(object sender, DataReceivedEventArgs e) {
            if (textBoxLog.InvokeRequired) {
                Process_OutputDataReceivedDelegate handler = new Process_OutputDataReceivedDelegate(Process_OutputDataReceived);
                Invoke(handler, sender, e);
            } else {
                textBoxLog.AppendText(e.Data + "\r\n");
            }
        }

        private bool runSmartSnippetsCommand(params String[] args) {
            ShellCommandRunner runner = new ShellCommandRunner(CMD_PATH, Process_OutputDataReceived, Process_OutputDataReceived);
            runner.addArguments(args);
            return runner.execute();
        }

        private bool runOtpCommand(params String[] args) {
            ShellCommandRunner runner = new ShellCommandRunner(CMD_PATH, Process_OutputDataReceived, Process_OutputDataReceived);
            runner.addArguments(CMD_OTP_ARGS);
            runner.addArguments(args);
            return runner.execute();
        }

        private bool writeOtpData(String offset, String data) {
            return runOtpCommand("-cmd", "write_field", "-offset", offset, "-data", data);
        }

        private char valueToChar(int value) {
            if (value < 10) {
                return (char) ('0' + value);
            } else {
                return (char) ('A' - 10 + value);
            }
        }

        private String bytesToString(byte[] bytes) {
            StringBuilder builder = new StringBuilder();

            foreach (byte value in bytes) {
                builder.Append(valueToChar(value & 0x0F));
                builder.Append(valueToChar((value >> 4) & 0x0F));
            }

            return builder.ToString();
        }

        private bool writeOtpData(String offset, byte[] data) {
            return writeOtpData(offset, bytesToString(data));
        }

        private bool writeBdAddress(byte[] addr) {
            if (addr == null || addr.Length != 6) {
                return false;
            }

            return writeOtpData("0x7FD4", addr);
        }

        private bool setOtpBootEnable() {
            return writeOtpData("0x7F00", "A5A534123412A5A5");
        }

        private bool readOtpHeader(String pathname) {
            return runOtpCommand("-cmd", "read_header", "-file", pathname);
        }

        private bool readOtpFirmware(String pathname) {
            return runOtpCommand("-cmd", "read_custom_code", "-file", pathname);
        }

        private bool writeOtpFirmware(String pathname) {
            return runOtpCommand("-cmd", "write_custom_code", "-y", "-file", pathname);
        }

        private void buttonFirmware_Click(object sender, EventArgs e) {

        }

        private void buttonConnect_Click(object sender, EventArgs e) {
            // readOtpFirmware("C:\\firmware.txt");
            // writeBdAddress(new byte[] { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 });
            writeOtpFirmware("C:\\jwaoo-toy.hex");
        }

        private void buttonBurn_Click(object sender, EventArgs e) {

        }

        private void buttonLogClean_Click(object sender, EventArgs e) {
            textBoxLog.Clear();
        }
    }

    public class ShellCommandRunner {

        private String mCommand;
        public DataReceivedEventHandler mErrorDataHander;
        public DataReceivedEventHandler mOutputDataHandler;
        private List<String> mArgumants = new List<string>();

        public ShellCommandRunner(String command, DataReceivedEventHandler errHandler, DataReceivedEventHandler outHandler) {
            mCommand = command;
            mErrorDataHander = errHandler;
            mOutputDataHandler = outHandler;
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

        public bool execute() {
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
            process.StartInfo.RedirectStandardInput = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;

            if (mErrorDataHander != null) {
                process.ErrorDataReceived += mErrorDataHander;
            }

            if (mOutputDataHandler != null) {
                process.OutputDataReceived += mOutputDataHandler;
            }

            try {
                MessageBox.Show("Arguments = " + process.StartInfo.Arguments);
                if (process.Start()) {
                    if (mOutputDataHandler != null) {
                        process.BeginOutputReadLine();
                    }

                    if (mOutputDataHandler != null) {
                        process.BeginErrorReadLine();
                    }

                    // process.WaitForExit();
                    MessageBox.Show("code = " + process.ExitCode);
                    return true;
                }
            } catch {
            }

            return false;
        }
    }
}
