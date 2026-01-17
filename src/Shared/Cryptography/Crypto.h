#pragma once
#include <vector>
#include <string>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

namespace MMO::Cryptography {

class Crypto {
public:
    /**
     * @brief Generates a cryptographically secure random session key.
     */
    static std::string GenerateSessionKey(size_t length = 32) {
        std::vector<unsigned char> buffer(length);
        if (RAND_bytes(buffer.data(), (int)length) != 1) {
            return "";
        }
        return ToHex(buffer);
    }

    /**
     * @brief Verifies a token (Simple SHA-256 hash check for MVP).
     * In a real app, this would verify a JWT signature.
     */
    static bool VerifyToken(const std::string& username, const std::string& token) {
        // MVP: The valid token is SHA256("salt" + username)
        std::string expected = HashSHA256("salt" + username);
        return token == expected;
    }

    static std::string HashSHA256(const std::string& input) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int lengthOfHash = 0;

        EVP_MD_CTX* context = EVP_MD_CTX_new();
        if (context != nullptr) {
            if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr)) {
                if (EVP_DigestUpdate(context, input.c_str(), input.length())) {
                    EVP_DigestFinal_ex(context, hash, &lengthOfHash);
                }
            }
            EVP_MD_CTX_free(context);
        }

        return ToHex(std::vector<unsigned char>(hash, hash + lengthOfHash));
    }

private:
    static std::string ToHex(const std::vector<unsigned char>& data) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int c : data) {
            ss << std::setw(2) << c;
        }
        return ss.str();
    }
};

} // namespace MMO::Cryptography
