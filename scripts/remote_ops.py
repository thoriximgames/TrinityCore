import paramiko
import os
import sys

def create_ssh_client(server, port, user, password):
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(server, port, user, password)
    return client

def sftp_put_recursive(sftp, localpath, remotepath):
    if os.path.isfile(localpath):
        sftp.put(localpath, remotepath)
    else:
        try:
            sftp.mkdir(remotepath)
        except OSError:
            pass # Directory might exist
        for item in os.listdir(localpath):
            sftp_put_recursive(sftp, os.path.join(localpath, item), remotepath + "/" + item)

def run_sudo_command(ssh, command, password):
    transport = ssh.get_transport()
    chan = transport.open_session()
    chan.get_pty()
    chan.exec_command(f"sudo -S {command}")
    chan.send(password + "\n")
    
    while not chan.exit_status_ready():
        if chan.recv_ready():
            print(chan.recv(1024).decode('utf-8', errors='ignore'), end="")
        if chan.recv_stderr_ready():
            print(chan.recv_stderr(1024).decode('utf-8', errors='ignore'), end="", file=sys.stderr)
    
    while chan.recv_ready():
        print(chan.recv(1024).decode('utf-8', errors='ignore'), end="")
    
    return chan.recv_exit_status()

def sync_files(server, port, user, password, local_root, remote_root):
    ssh = create_ssh_client(server, port, user, password)
    sftp = ssh.open_sftp()
    
    try:
        ssh.exec_command(f"mkdir -p {remote_root}")
        items = ["src", "Shared", "CMakeLists.txt", "Dockerfile", "docker-compose.yml"]
        for item in items:
            local_path = os.path.join(local_root, item)
            remote_path = remote_root + "/" + item
            if os.path.exists(local_path):
                print(f"Syncing {item}...")
                sftp_put_recursive(sftp, local_path, remote_path)
    finally:
        sftp.close()
        ssh.close()

if __name__ == "__main__":
    HOST = "192.168.0.101"
    USER = "trader2"
    PASS = "nominativ159357"
    REMOTE_DIR = "/home/trader2/mmo-server"

    if len(sys.argv) < 2:
        print("Usage: python remote_ops.py [sync|build|install-deps|start-services|run-server|setup-firewall]")
        sys.exit(1)

    action = sys.argv[1]

    if action == "sync":
        sync_files(HOST, 22, USER, PASS, os.getcwd(), REMOTE_DIR)
        print("Sync complete.")
    
    elif action == "install-deps":
        ssh = create_ssh_client(HOST, 22, USER, PASS)
        inner_cmd = (
            "apt-get update && apt-get install -y "
            "cmake clang make libssl-dev libasio-dev "
            "flatbuffers-compiler libflatbuffers-dev "
            "libpq-dev libpqxx-dev libhiredis-dev pkg-config"
        )
        deps_cmd = f"bash -c '{inner_cmd}'"
        print(f"Installing dependencies on {HOST}...")
        status = run_sudo_command(ssh, deps_cmd, PASS)
        ssh.close()
        sys.exit(status)

    elif action == "build":
        ssh = create_ssh_client(HOST, 22, USER, PASS)
        inner_cmd = (
            f"cd {REMOTE_DIR}/src/Shared/Protocol && "
            "flatc --cpp --csharp --gen-onefile MMO.fbs && "
            f"cd {REMOTE_DIR} && "
            "mkdir -p build && cd build && "
            "cmake .. -DCMAKE_BUILD_TYPE=Release && "
            "make -j$(nproc)"
        )
        build_cmd = f"bash -c '{inner_cmd}'"
        print(f"Executing build on {HOST}...")
        status = run_sudo_command(ssh, build_cmd, PASS)
        ssh.close()
        sys.exit(status)

    elif action == "start-services":
        ssh = create_ssh_client(HOST, 22, USER, PASS)
        inner_cmd = f"cd {REMOTE_DIR} && docker compose down && docker compose up -d db redis"
        cmd = f"bash -c '{inner_cmd}'"
        print(f"Starting DB and Redis on {HOST}...")
        status = run_sudo_command(ssh, cmd, PASS)
        ssh.close()
        sys.exit(status)

    elif action == "run-server":
        ssh = create_ssh_client(HOST, 22, USER, PASS)
        inner_cmd = f"cd {REMOTE_DIR}/build && DB_PORT=5433 nohup ./MMOServer > server.log 2>&1 &"
        cmd = f"bash -c '{inner_cmd}'"
        print(f"Starting MMOServer in background on {HOST}...")
        status = run_sudo_command(ssh, cmd, PASS)
        ssh.close()
        sys.exit(status)

    elif action == "stop-server":
        ssh = create_ssh_client(HOST, 22, USER, PASS)
        cmd = "pkill MMOServer"
        print(f"Stopping MMOServer on {HOST}...")
        status = run_sudo_command(ssh, cmd, PASS)
        ssh.close()
        sys.exit(status)

    elif action == "setup-firewall":
        ssh = create_ssh_client(HOST, 22, USER, PASS)
        cmd = "ufw allow 8085/tcp && ufw allow 5433/tcp"
        print(f"Configuring firewall on {HOST}...")
        status = run_sudo_command(ssh, cmd, PASS)
        ssh.close()
        sys.exit(status)

    elif action == "tail-log":
        ssh = create_ssh_client(HOST, 22, USER, PASS)
        cmd = f"tail -n 50 {REMOTE_DIR}/build/server.log"
        print(f"Fetching logs from {HOST}...")
        status = run_sudo_command(ssh, cmd, PASS)
        ssh.close()
        sys.exit(status)

    elif action == "check-ports":
        ssh = create_ssh_client(HOST, 22, USER, PASS)
        cmd = "ss -tulpn | grep -E '8085|5433|6379'"
        print(f"Checking ports on {HOST}...")
        status = run_sudo_command(ssh, cmd, PASS)
        ssh.close()
        sys.exit(status)
