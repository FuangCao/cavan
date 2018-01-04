using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace NetworkInputMethod
{
    public class CavanHttpSender
    {
        private Thread mThread;
        private CavanHttpReq mReq;
        private FormHttpSender mForm;

        public CavanHttpSender(FormHttpSender form, CavanHttpReq req)
        {
            mForm = form;
            mReq = req;
        }

        public void start()
        {
            if (mThread == null)
            {
                mThread = new Thread(new ThreadStart(mainLoop));
                mThread.Start();
            }
        }

        public void stop()
        {
            if (mThread != null)
            {
                mThread.Abort();
            }

            mReq.closeTcpClient();
        }

        private void mainLoop()
        {
            try
            {
                Console.WriteLine("started");
                mReq.mainLoop(true);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
            finally
            {
                mThread = null;
                mReq.closeTcpClient();
                Console.WriteLine("stopped");
            }
        }
    }
}
