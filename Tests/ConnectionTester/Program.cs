using System;
using System.Net.Sockets;
using System.Threading.Tasks;
using Google.FlatBuffers;
using MMO.Protocol;

namespace ConnectionTester
{
    class Program
    {
        static async Task Main(string[] args)
        {
            Console.WriteLine("=== Modern MMO Connection Tester ===");
            string host = "192.168.0.101";
            int port = 8085;

            try
            {
                using var client = new TcpClient();
                Console.WriteLine($"Connecting to {host}:{port}...");
                await client.ConnectAsync(host, port);
                Console.WriteLine("Connected!");

                using var stream = client.GetStream();

                // 1. Build Login Request
                var fbb = new FlatBufferBuilder(1024);
                var username = fbb.CreateString("TestPlayer");
                var version = fbb.CreateString("0.1.0");
                var token = fbb.CreateString("debug-token");

                LoginRequest.StartLoginRequest(fbb);
                LoginRequest.AddUsername(fbb, username);
                LoginRequest.AddClientVersion(fbb, version);
                LoginRequest.AddToken(fbb, token);
                var loginReq = LoginRequest.EndLoginRequest(fbb);

                MasterPacket.StartMasterPacket(fbb);
                MasterPacket.AddDataType(fbb, PacketType.LoginRequest);
                MasterPacket.AddData(fbb, loginReq.Value);
                var master = MasterPacket.EndMasterPacket(fbb);
                fbb.Finish(master.Value);

                // 2. Send Size + Data
                byte[] data = fbb.SizedByteArray();
                byte[] sizePrefix = BitConverter.GetBytes((uint)data.Length);
                
                // Ensure Little Endian for the server
                if (!BitConverter.IsLittleEndian) Array.Reverse(sizePrefix);

                Console.WriteLine($"Sending LoginRequest ({data.Length} bytes)...");
                await stream.WriteAsync(sizePrefix, 0, 4);
                await stream.WriteAsync(data, 0, data.Length);

                // 3. Read Response Size
                byte[] resSizeBuffer = new byte[4];
                await stream.ReadAsync(resSizeBuffer, 0, 4);
                uint resSize = BitConverter.ToUInt32(resSizeBuffer, 0);

                // 4. Read Response Data
                byte[] resData = new byte[resSize];
                int totalRead = 0;
                while (totalRead < resSize)
                {
                    int read = await stream.ReadAsync(resData, totalRead, (int)resSize - totalRead);
                    if (read == 0) break;
                    totalRead += read;
                }

                // 5. Parse Response
                var resPacket = MasterPacket.GetRootAsMasterPacket(new ByteBuffer(resData));
                if (resPacket.DataType == PacketType.LoginResponse)
                {
                    var loginRes = resPacket.Data<LoginResponse>().Value;
                    Console.WriteLine("=== LOGIN SUCCESS ===");
                    Console.WriteLine($"Message: {loginRes.ErrorMessage}");
                    Console.WriteLine($"PlayerID: {loginRes.PlayerId}");
                    var pos = loginRes.SpawnPos.Value;
                    Console.WriteLine($"Spawn Position: ({pos.X}, {pos.Y}, {pos.Z})");
                }
                else
                {
                    Console.WriteLine($"Received unexpected packet type: {resPacket.DataType}");
                }

            }
            catch (Exception ex)
            {
                Console.WriteLine($"ERROR: {ex.Message}");
            }

            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
    }
}