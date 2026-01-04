# Briefing: Blank MMO Backend (TrinityCore 3.3.5 Legacy)

This document provides the technical specifications required to implement a custom client capable of authenticating with this backend.

## 1. Connection Overview
- **IP Address:** `127.0.0.1`
- **Auth Port:** `3724` (Raw TCP)
- **World Port:** `8085` (Raw TCP + SRP6 Encryption)
- **WoW Version:** `3.3.5a` (Build `12340`)

---

## 2. Authentication Protocol (SRP6)
The server uses the **SRP6 (Secure Remote Password)** protocol for the handshake.

### Credentials
- **Username:** `ADMIN` (Must be sent as uppercase in the hash)
- **Password:** `ADMIN`

### Key Algorithms
1. **Password Hash (I):** `SHA1(UPPERCASE(username) + ":" + UPPERCASE(password))`
2. **SRP6 Parameters:**
   - **G (Generator):** `7`
   - **N (Large Safe Prime):** `0x894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7`
   - **k:** `3`

---

## 3. Packet Flow (Auth Server)

### Step 1: Client Challenge (`AUTH_LOGON_CHALLENGE` - 0x00)
The client connects to `3724` and sends its identity.
- **Opcode:** `0x00`
- **Data:** Build version, Platform, OS, Locale, Timezone, IP, and Username length/string.

### Step 2: Server Response
The server responds with the **Salt (s)** and the **Public Key (B)**.

### Step 3: Client Proof (`AUTH_LOGON_PROOF` - 0x01)
The client calculates its Proof (**M1**) using the server's public key and salt.
- **Opcode:** `0x01`
- **Data:** `A` (Client Public Key), `M1` (Proof), `CRC`.

---

## 4. World Server Handshake
Once authenticated, the client connects to `8085`. 

### Header Encryption
After the initial handshake, the World Server requires **ARC4-based header encryption**.
- The key for the ARC4 cipher is derived from the **Session Key (K)** generated during the Auth phase.
- **Header Size:** 
  - Client -> Server: 6 bytes (Size [2], Opcode [4]).
  - Server -> Client: 4 bytes (Size [2], Opcode [2]).

---

## 5. Implementation Notes
- All integers are **Little Endian**.
- **Critical (BigInt Unsigned Conversion):** In managed languages like C#, `new BigInteger(bytes)` expects a signed integer. Because the MSB (last byte in Little Endian) of `N` and potentially `B` has the high bit set, they will be interpreted as negative numbers, breaking the SRP6 math. 
    - **Fix:** You must append a trailing `0x00` byte to any 32-byte array received from the server before passing it to a BigInteger constructor to force it to be positive.
- **Critical (Fixed-Length Hashing):** When calculating `u = SHA1(A | B)` and `M1`, the server expects `A` and `B` to be hashed as **exactly 32 bytes**. C# `BigInteger.ToByteArray()` returns variable-length arrays (e.g. 33 bytes with sign, or 31 bytes if leading zeros exist). You **must** pad or truncate these to exactly 32 bytes before hashing.
- **Critical (SHA1 Interleave Quirk):** The Session Key `K` generation (`SHA1Interleave(S)`) has a legacy quirk: it skips leading zero bytes in `S` before splitting and hashing. If your `S` ends in `00` (Little Endian), standard implementations will mismatch.
    - **Important:** The result is formed by interleaving the two 20-byte hashes **byte-by-byte** (`K[2*i] = h0[i]`, `K[2*i+1] = h1[i]`), **NOT** by concatenating them.
- BigInts (like N, B, A) are used throughout the SRP6 calculation.
- Authentication at port 3724 does **not** use SSL/TLS; it is a raw TCP stream.
- After a successful `AUTH_LOGON_PROOF`, the server provides a `Realm List`. The `Address` field in the realm list should point back to `127.0.0.1:8085`.

---

## 6. Current Backend State
- **Status:** All services (Auth, World, Database) are currently **RUNNING**.
- **Data:** Game data (DBC/Maps) is successfully loaded.
- **Ready for Connection.**