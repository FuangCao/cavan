using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Threading;
using System.IO;
using System.Resources;
using System.Reflection;
using JwaooOtpProgrammer.Properties;

namespace JwaooOtpProgrammer {
    static class Program {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main() {
            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;

            bool createdNew;
            Mutex mutex = new Mutex(true, Application.ProductName, out createdNew);

            if (createdNew) {
                releaseResourceFile(Resources.jtag_programmer, "jtag_programmer.bin");

                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new JwaooOtpProgrammer());

                mutex.ReleaseMutex();
            } else {
                MessageBox.Show("本程序已经在运行了，不要同时打开多个哦！");
                Application.Exit();
            }
        }

        private static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args) {
            if (args.Name.StartsWith("zxing")) {
                return Assembly.Load(Resources.zxing);
            }

            return null;
        }

        public static String releaseResourceFile(byte[] bytes, String fileName) {
            if (bytes == null) {
                return null;
            }

            try {
                String pathname = Path.Combine(Application.StartupPath, fileName);
                File.WriteAllBytes(pathname, bytes);
                return pathname;
            } catch (Exception) {
                return null;
            }
        }
    }
}
