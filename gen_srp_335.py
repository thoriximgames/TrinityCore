import hashlib
import os

def sha1_hash(*args):
    h = hashlib.sha1()
    for arg in args:
        if isinstance(arg, str):
            h.update(arg.encode())
        else:
            h.update(arg)
    return h.digest()

N_hex = "894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7"
N = int(N_hex, 16)
g = 7

user = "ADMIN"
password = "ADMIN"
salt = os.urandom(32)

# x = H(s, H(U, ":", P))
h_up = sha1_hash(user, ":", password)
h_s_hup = sha1_hash(salt, h_up)
# SRP6 uses Big-Endian for 3.3.5
x = int.from_bytes(h_s_hup, byteorder='big')

v = pow(g, x, N)
verifier = v.to_bytes(32, byteorder='big')

print(f"SALT:{salt.hex().upper()}")
print(f"VERIFIER:{verifier.hex().upper()}")
