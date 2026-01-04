import hashlib
import os

def sha256_hex(data):
    return hashlib.sha256(data).hexdigest().upper()

N_hex = "86A7F6DEEB306CE519770FE37D556F29944132554DED0BD68205E27F3231FEF5A10108238A3150C59CAF7B0B6478691C13A6ACF5E1B5ADAFD4A943D4A21A142B800E8A55F8BFBAC700EB77A7235EE5A609E350EA9FC19F10D921C2FA832E4461B7125D38D254A0BE873DFC27858ACB3F8B9F258461E4373BC3A6C2A9634324AB"
N = int(N_hex, 16)
g = 2

email = "admin@local".upper()
password = "admin".upper()
salt = os.urandom(32)

# TrinityCore BNet SRP uses a hex string of the hash of the email as the "username" for SRP
srp_username = sha256_hex(email.encode()).upper()

# x = H(s, H(U | ":" | P))
# NOTE: TrinityCore modern uses SHA256 for this
h_up = hashlib.sha256(f"{srp_username}:{password}".encode()).digest()
h_s_hup = hashlib.sha256(salt + h_up).digest()
x = int.from_bytes(h_s_hup, byteorder='little')

v = pow(g, x, N)
verifier = v.to_bytes(128, byteorder='little')

print(f"SALT:{salt.hex().upper()}")
print(f"VERIFIER:{verifier.hex().upper()}")
print(f"DEBUG_SRP_USER:{srp_username}")