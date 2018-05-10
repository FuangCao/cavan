using System;
using System.Windows.Forms;
using System.Reflection;
using NetworkInputMethod.Properties;
using System.Threading;

namespace NetworkInputMethod
{
    static class Program
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main()
        {
            bool createdNew;
            Mutex mutex = new Mutex(true, Application.ProductName, out createdNew);

            if (createdNew)
            {
                AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new FormNetworkIme());

                mutex.ReleaseMutex();
            }
            else
            {
                MessageBox.Show("本程序已经在运行了，不要同时打开多个哦！");
                Application.Exit();
            }
        }

        private static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {
            if (args.Name.StartsWith("Newtonsoft.Json"))
            {
                return Assembly.Load(Resources.Newtonsoft_Json);
            }
            else
            {
                return null;
            }
        }
    }
}
