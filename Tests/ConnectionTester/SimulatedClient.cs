using System;
using System.Net.Sockets;
using System.Threading.Tasks;
using Google.FlatBuffers;
using MMO.Protocol;
using System.Threading;
using System.Security.Cryptography;
using System.Text;
using System.Collections.Generic;

namespace ConnectionTester
{
    class SimulatedClient
    {
        private string _host;
        private int _port;
        private string _username;
        private string _password;
        private TcpClient _client;
        private NetworkStream _stream;
        private bool _running = true;
        private ulong _accountId;
        private ulong _characterId;
        private bool _inWorld = false;

        public SimulatedClient(string host, int port, string username, string password = "password123")
        {
            _host = host;
            _port = port;
            _username = username;
            _password = password;
        }

        public async Task Run(int durationSeconds = 10)
        {
            try
            {
                _client = new TcpClient();
                Console.WriteLine($"[{_username}] Connecting to {_host}:{_port}...");
                await _client.ConnectAsync(_host, _port);
                _stream = _client.GetStream();

                await SendLogin();
                
                _ = Task.Run(ListenLoop);

                var startTime = DateTime.UtcNow;
                while (_running && (DateTime.UtcNow - startTime).TotalSeconds < durationSeconds)
                {
                    await Task.Delay(1000);
                    if (_inWorld && _characterId > 0)
                    {
                        await SendMovement(100.0f + (float)startTime.Second, 100.0f, 10.0f, 0.0f);
                    }
                }
                _running = false;
                _client.Close();
                Console.WriteLine($"[{_username}] Test ended.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[{_username}] Global Error: {ex.Message}");
            }
        }

        private async Task SendLogin()
        {
            var fbb = new FlatBufferBuilder(1024);
            var userOffset = fbb.CreateString(_username);
            var versionOffset = fbb.CreateString("0.1.0");
            var token = fbb.CreateString(ComputeSha256Hash("salt" + _password));

            LoginRequest.StartLoginRequest(fbb);
            LoginRequest.AddUsername(fbb, userOffset);
            LoginRequest.AddClientVersion(fbb, versionOffset);
            LoginRequest.AddToken(fbb, token);
            var loginReq = LoginRequest.EndLoginRequest(fbb);

            MasterPacket.StartMasterPacket(fbb);
            MasterPacket.AddDataType(fbb, PacketType.LoginRequest);
            MasterPacket.AddData(fbb, loginReq.Value);
            var master = MasterPacket.EndMasterPacket(fbb);
            fbb.Finish(master.Value);

            await SendBuffer(fbb);
        }

        private async Task SendRegister()
        {
            Console.WriteLine($"[{_username}] Account not found. Registering...");
            var fbb = new FlatBufferBuilder(1024);
            var userOffset = fbb.CreateString(_username);
            var emailOffset = fbb.CreateString(_username + "@test.com");
            var pwOffset = fbb.CreateString(ComputeSha256Hash("salt" + _password));

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

            await SendBuffer(fbb);
        }

        private async Task SendRealmListRequest()
        {
            var fbb = new FlatBufferBuilder(1024);
            RealmListRequest.StartRealmListRequest(fbb);
            var req = RealmListRequest.EndRealmListRequest(fbb);
            MasterPacket.StartMasterPacket(fbb);
            MasterPacket.AddDataType(fbb, PacketType.RealmListRequest);
            MasterPacket.AddData(fbb, req.Value);
            var master = MasterPacket.EndMasterPacket(fbb);
            fbb.Finish(master.Value);
            await SendBuffer(fbb);
        }

        private async Task SendJoinRealm(uint realmId)
        {
            var fbb = new FlatBufferBuilder(1024);
            JoinRealmRequest.StartJoinRealmRequest(fbb);
            JoinRealmRequest.AddRealmId(fbb, realmId);
            var req = JoinRealmRequest.EndJoinRealmRequest(fbb);
            MasterPacket.StartMasterPacket(fbb);
            MasterPacket.AddDataType(fbb, PacketType.JoinRealmRequest);
            MasterPacket.AddData(fbb, req.Value);
            var master = MasterPacket.EndMasterPacket(fbb);
            fbb.Finish(master.Value);
            await SendBuffer(fbb);
        }

        private async Task SendCharacterListRequest()
        {
            var fbb = new FlatBufferBuilder(1024);
            CharacterListRequest.StartCharacterListRequest(fbb);
            var req = CharacterListRequest.EndCharacterListRequest(fbb);
            MasterPacket.StartMasterPacket(fbb);
            MasterPacket.AddDataType(fbb, PacketType.CharacterListRequest);
            MasterPacket.AddData(fbb, req.Value);
            var master = MasterPacket.EndMasterPacket(fbb);
            fbb.Finish(master.Value);
            await SendBuffer(fbb);
        }

        private async Task SendCreateCharacter(string name)
        {
            Console.WriteLine($"[{_username}] Creating character: {name}");
            var fbb = new FlatBufferBuilder(1024);
            var nameOffset = fbb.CreateString(name);
            CreateCharacterRequest.StartCreateCharacterRequest(fbb);
            CreateCharacterRequest.AddName(fbb, nameOffset);
            CreateCharacterRequest.AddRace(fbb, 1);
            CreateCharacterRequest.AddCharacterClass(fbb, 1);
            var req = CreateCharacterRequest.EndCreateCharacterRequest(fbb);
            MasterPacket.StartMasterPacket(fbb);
            MasterPacket.AddDataType(fbb, PacketType.CreateCharacterRequest);
            MasterPacket.AddData(fbb, req.Value);
            var master = MasterPacket.EndMasterPacket(fbb);
            fbb.Finish(master.Value);
            await SendBuffer(fbb);
        }

        private async Task SendSelectCharacter(ulong charId)
        {
            var fbb = new FlatBufferBuilder(1024);
            SelectCharacterRequest.StartSelectCharacterRequest(fbb);
            SelectCharacterRequest.AddCharacterId(fbb, charId);
            var req = SelectCharacterRequest.EndSelectCharacterRequest(fbb);
            MasterPacket.StartMasterPacket(fbb);
            MasterPacket.AddDataType(fbb, PacketType.SelectCharacterRequest);
            MasterPacket.AddData(fbb, req.Value);
            var master = MasterPacket.EndMasterPacket(fbb);
            fbb.Finish(master.Value);
            await SendBuffer(fbb);
        }

        private async Task SendMovement(float x, float y, float z, float rot)
        {
            var fbb = new FlatBufferBuilder(1024);
            MovementUpdate.StartMovementUpdate(fbb);
            MovementUpdate.AddEntityId(fbb, _characterId);
            MovementUpdate.AddPosition(fbb, Vec3.CreateVec3(fbb, x, y, z));
            MovementUpdate.AddRotation(fbb, rot);
            MovementUpdate.AddTimestamp(fbb, (ulong)DateTimeOffset.UtcNow.ToUnixTimeMilliseconds());
            var moveUpdate = MovementUpdate.EndMovementUpdate(fbb);
            MasterPacket.StartMasterPacket(fbb);
            MasterPacket.AddDataType(fbb, PacketType.MovementUpdate);
            MasterPacket.AddData(fbb, moveUpdate.Value);
            var master = MasterPacket.EndMasterPacket(fbb);
            fbb.Finish(master.Value);
            await SendBuffer(fbb);
        }

        private async Task SendBuffer(FlatBufferBuilder fbb)
        {
            byte[] data = fbb.SizedByteArray();
            byte[] sizePrefix = BitConverter.GetBytes((uint)data.Length);
            if (!BitConverter.IsLittleEndian) Array.Reverse(sizePrefix);
            await _stream.WriteAsync(sizePrefix, 0, 4);
            await _stream.WriteAsync(data, 0, data.Length);
        }

        private async Task ListenLoop()
        {
            byte[] sizeBuffer = new byte[4];
            try
            {
                while (_running)
                {
                    int read = await _stream.ReadAsync(sizeBuffer, 0, 4);
                    if (read == 0) break;
                    uint size = BitConverter.ToUInt32(sizeBuffer, 0);
                    byte[] data = new byte[size];
                    int totalRead = 0;
                    while (totalRead < size)
                    {
                        read = await _stream.ReadAsync(data, totalRead, (int)size - totalRead);
                        if (read == 0) break;
                        totalRead += read;
                    }
                    var master = MasterPacket.GetRootAsMasterPacket(new ByteBuffer(data));
                    await HandlePacket(master);
                }
            }
            catch {}
        }

        private async Task HandlePacket(MasterPacket master)
        {
            switch (master.DataType)
            {
                case PacketType.LoginResponse:
                    var loginRes = master.Data<LoginResponse>().Value;
                    if (loginRes.Success) {
                        _accountId = loginRes.PlayerId;
                        Console.WriteLine($"[{_username}] Login Success. AccountID: {_accountId}");
                        await SendRealmListRequest();
                    } else {
                        await SendRegister();
                    }
                    break;

                case PacketType.RegisterResponse:
                    var regRes = master.Data<RegisterResponse>().Value;
                    Console.WriteLine($"[{_username}] Register Result: {regRes.Success}");
                    if (regRes.Success) await SendLogin();
                    break;

                case PacketType.RealmListResponse:
                    var realms = master.Data<RealmListResponse>().Value;
                    if (realms.RealmsLength > 0) {
                        var r = realms.Realms(0).Value;
                        Console.WriteLine($"[{_username}] Joining Realm {r.Name}...");
                        await SendJoinRealm(r.Id);
                        await SendCharacterListRequest();
                    }
                    break;

                case PacketType.CharacterListResponse:
                    var chars = master.Data<CharacterListResponse>().Value;
                    Console.WriteLine($"[{_username}] Found {chars.CharactersLength} characters.");
                    if (chars.CharactersLength > 0) {
                        var c = chars.Characters(0).Value;
                        _characterId = c.Id;
                        Console.WriteLine($"[{_username}] Selecting character: {c.Name} (ID: {_characterId})");
                        await SendSelectCharacter(_characterId);
                        _inWorld = true;
                    } else {
                        await SendCreateCharacter(_username + "_Hero");
                        await SendCharacterListRequest();
                    }
                    break;

                case PacketType.CreateCharacterResponse:
                    var creRes = master.Data<CreateCharacterResponse>().Value;
                    Console.WriteLine($"[{_username}] Create Char Result: {creRes.Success}");
                    break;

                case PacketType.MovementUpdate:
                    var move = master.Data<MovementUpdate>().Value;
                    if (move.EntityId != _characterId)
                        Console.WriteLine($"[{_username}] BROADCAST: Entity {move.EntityId} moved.");
                    break;
            }
        }

        private string ComputeSha256Hash(string rawData)
        {
            using (SHA256 sha256Hash = SHA256.Create())
            {
                byte[] bytes = sha256Hash.ComputeHash(Encoding.UTF8.GetBytes(rawData));
                StringBuilder builder = new StringBuilder();
                for (int i = 0; i < bytes.Length; i++) builder.Append(bytes[i].ToString("x2"));
                return builder.ToString();
            }
        }
    }

    class Program
    {
        static async Task Main(string[] args)
        {
            string host = args.Length > 0 ? args[0] : "127.0.0.1";
            Console.WriteLine($"\n=== MMO Full Life-Cycle Persistence Test ({host}) ===");
            
            // Client 1 (Alice)
            var c1 = new SimulatedClient(host, 8085, "Alice");
            // Client 2 (Bob)
            var c2 = new SimulatedClient(host, 8085, "Bob");

            var t1 = c1.Run(10);
            await Task.Delay(1000);
            var t2 = c2.Run(10);

            await Task.WhenAll(t1, t2);
            Console.WriteLine("=== All tests finished ===");
        }
    }
}
