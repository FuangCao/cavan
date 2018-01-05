using System;
using System.Windows.Forms;
using System.Reflection;
using NetworkInputMethod.Properties;

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
            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new FormNetworkIme());
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
