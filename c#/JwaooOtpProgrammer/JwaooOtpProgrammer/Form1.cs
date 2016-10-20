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

        delegate void Process_OutputDataReceivedDelegate(object sender, DataReceivedEventArgs e);

        private const String CMD_NAME = "SmartSnippets.exe";
        private const String CMD_PATH = "C:\\SmartSnippets\\bin\\" + CMD_NAME;

        public Form1() {
            InitializeComponent();
        }

        private bool runShellCommand(String command, String name, String[] args) {
            Process process = new Process();
            process.StartInfo.FileName = command;
            process.StartInfo.Arguments = String.Join(" ", args);
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.RedirectStandardInput = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.ErrorDataReceived += Process_OutputDataReceived;
            process.OutputDataReceived += Process_OutputDataReceived;

            if (!process.Start()) {
                return false;
            }

            process.BeginOutputReadLine();
            process.BeginErrorReadLine();

            return true;
        }

        private void Process_OutputDataReceived(object sender, DataReceivedEventArgs e) {
            if (textBoxLog.InvokeRequired) {
                Process_OutputDataReceivedDelegate handler = new Process_OutputDataReceivedDelegate(Process_OutputDataReceived);
                Invoke(handler, sender, e);
            } else {
                textBoxLog.AppendText(e.Data + "\r\n");
            }
        }

        private bool runSmartSnippets(String[] args) {
            return runShellCommand(CMD_PATH, CMD_NAME, args);
        }

        private bool writeOtp(String offset, String value) {
            return run
        }

        private void buttonFirmware_Click(object sender, EventArgs e) {

        }

        private void buttonConnect_Click(object sender, EventArgs e) {
            runSmartSnippets(new String[] { @"-help" });
        }

        private void buttonBurn_Click(object sender, EventArgs e) {

        }

        private void buttonLogClean_Click(object sender, EventArgs e) {
            textBoxLog.Clear();
        }
    }
}
