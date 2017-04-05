using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Threading;

namespace JwaooOtpProgrammer {
    static class Program {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main() {
            bool createdNew;
            Mutex mutex = new Mutex(true, Application.ProductName, out createdNew);

            if (createdNew) {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new Programmer());

                mutex.ReleaseMutex();
            } else {
                MessageBox.Show("本程序已经在运行了，不要同时打开多个哦！");
                Application.Exit();
            }
        }
    }
}
