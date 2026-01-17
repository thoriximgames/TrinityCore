using System;
using System.Net.Sockets;
using System.Threading.Tasks;
using Google.FlatBuffers;
using MMO.Protocol;
using System.Security.Cryptography;
using System.Text;

namespace ConnectionTester
{
    class RegistrationTester
    {
        private string _host;
        private int _port;

        public RegistrationTester(string host, int port)
        {
            _host = host;
            _port = port;
        }

        public async Task Run()
        {
            Console.WriteLine($"=== Registration Test Started ({_host}:{_port}) ===");

            // Test 1: Successful Registration
            string testUser = "User_" + Guid.NewGuid().ToString().Substring(0, 8);
            await DoRegister(testUser, testUser + "@example.com", "pass123", "SUCCESS (New User)");

            // Test 2: Duplicate Username (should fail)
            await DoRegister(testUser, "another@example.com", "pass123", "FAILURE (Duplicate User)");

            // Test 3: Invalid Email (should fail)
            await DoRegister("ValidUser", "invalid-email", "pass123", "FAILURE (Invalid Email)");

            Console.WriteLine("=== Registration Test Finished ===");
        }

        private async Task DoRegister(string user, string email, string pass, string label)
        {
            try
            {
                using var client = new TcpClient();
                await client.ConnectAsync(_host, _port);
                using var stream = client.GetStream();

                var fbb = new FlatBufferBuilder(1024);
                var userOffset = fbb.CreateString(user);
                var emailOffset = fbb.CreateString(email);
                
                // Hash as client would
                using var sha256 = SHA256.Create();
                string pwHash = BitConverter.ToString(sha256.ComputeHash(Encoding.UTF8.GetBytes("salt" + pass))).Replace("-", "").ToLower();
                var pwOffset = fbb.CreateString(pwHash);

                RegisterRequest.StartRegisterRequest(fbb);
                RegisterRequest.AddUsername(fbb, userOffset);
                RegisterRequest.AddEmail(fbb, emailOffset);
                RegisterRequest.AddPasswordHash(fbb, pwOffset);
                var reg = RegisterRequest.EndRegisterRequest(fbb);

                MasterPacket.StartMasterPacket(fbb);
                MasterPacket.AddDataType(fbb, PacketType.RegisterRequest);
                MasterPacket.AddData(fbb, reg.Value);
                var master = MasterPacket.EndMasterPacket(fbb);
                fbb.Finish(master.Value);

                byte[] data = fbb.SizedByteArray();
                byte[] sizePrefix = BitConverter.GetBytes((uint)data.Length);
                if (!BitConverter.IsLittleEndian) Array.Reverse(sizePrefix);

                await stream.WriteAsync(sizePrefix, 0, 4);
                await stream.WriteAsync(data, 0, data.Length);

                // Read Response
                byte[] resSizeBuf = new byte[4];
                await stream.ReadAsync(resSizeBuf, 0, 4);
                uint resSize = BitConverter.ToUInt32(resSizeBuf, 0);
                byte[] resData = new byte[resSize];
                await stream.ReadAsync(resData, 0, (int)resSize);

                var resPacket = MasterPacket.GetRootAsMasterPacket(new ByteBuffer(resData));
                if (resPacket.DataType == PacketType.RegisterResponse)
                {
                    var result = resPacket.Data<RegisterResponse>().Value;
                    Console.WriteLine($"[{label}] Result: {(result.Success ? "OK" : "FAILED")} | Msg: {result.ErrorMessage}");
                }
                else
                {
                    Console.WriteLine($"[{label}] Error: Received {resPacket.DataType} instead of RegisterResponse");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[{label}] Exception: {ex.Message}");
            }
        }
    }
}
