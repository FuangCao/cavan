using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Sockets;
using System.Text;

namespace NetworkInputMethod
{
    public class CavanAdbClient : CavanTcpClient
    {
        public static int[] PORTS = new int[] { 5037, 5038 };

        public CavanAdbClient(TcpClient client) : base(client) { }

        public static TcpClient ConnectLocal(string host)
        {
            var client = new TcpClient();

            for (int i = 0; i < 2; i++)
            {
                foreach (var port in PORTS)
                {
                    try
                    {
                        client.Connect(host, port);
                        return client;
                    }
                    catch (Exception)
                    {
                        continue;
                    }
                }

                try
                {
                    var process = new Process();
                    var info = process.StartInfo;

                    info.FileName = "adb.exe";
                    info.Arguments = "start-server";

                    process.Start();
                    process.WaitForExit();

                    var code = process.ExitCode;
                    process.Close();

                    if (code != 0)
                    {
                        break;
                    }
                }
                catch (Exception)
                {
                    break;
                }
            }

            return null;
        }

        public static TcpClient ConnectLocal(string host, string service)
        {
            var client = ConnectLocal(host);
            if (client == null)
            {
                return null;
            }

            bool success = false;

            try
            {
                var stream = client.GetStream();

                if (SendCommand(stream, "host:transport-any") == null)
                {
                    return null;
                }

                if (SendCommand(stream, service) == null)
                {
                    return null;
                }

                success = true;

                return client;
            }
            catch (Exception)
            {
                return null;
            }
            finally
            {
                if (!success)
                {
                    client.Close();
                }
            }
        }

        public static bool WaitForDevice(string host)
        {
            var client = ConnectLocal(host, "host:wait-for-any-device");
            if (client == null)
            {
                return false;
            }

            client.Close();

            return true;
        }

        public static TcpClient ConnectRemote(string host, int port, bool wait)
        {
            if (wait == false || WaitForDevice(host))
            {
                return ConnectLocal(host, string.Format("tcp:{0:d}", port));
            }

            return null;
        }

        public static void Send(NetworkStream stream, string text)
        {
            var bytes = Encoding.UTF8.GetBytes(text);
            stream.Write(bytes, 0, bytes.Length);
        }

        public static byte[] ReadReply(NetworkStream stream)
        {
            var bytes = new byte[8];
            var length = stream.Read(bytes, 0, bytes.Length);
            if (length < 4)
            {
                return null;
            }

            var status = Encoding.UTF8.GetString(bytes, 0, 4);
            if (!status.Equals("OKAY"))
            {
                return null;
            }

            if (length == 8)
            {
                bytes = new byte[Convert.ToInt32(Encoding.UTF8.GetString(bytes, 4, 4), 16)];
                stream.Read(bytes, 0, bytes.Length);
            }
            else
            {
                bytes = new byte[0];
            }

            return bytes;
        }

        public static byte[] SendCommand(NetworkStream stream, byte[] bytes, int index, int length)
        {
            Send(stream, string.Format("{0:x4}", length));
            stream.Write(bytes, index, length);
            return ReadReply(stream);
        }

        public static byte[] SendCommand(NetworkStream stream, string command)
        {
            var bytes = Encoding.UTF8.GetBytes(command);
            return SendCommand(stream, bytes, 0, bytes.Length);
        }
    }
}
