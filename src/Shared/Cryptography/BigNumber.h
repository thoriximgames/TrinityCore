#pragma once

#include <openssl/bn.h>
#include <vector>
#include <cstdint>
#include <memory>
#include <string>

namespace MMO::Cryptography {

/**
 * @brief Modern wrapper for OpenSSL BigNumber math.
 */
class BigNumber {
public:
    BigNumber();
    BigNumber(uint32_t val);
    BigNumber(const std::vector<uint8_t>& bytes);
    BigNumber(const BigNumber& other);
    ~BigNumber();

    BigNumber& operator=(const BigNumber& other);

    // Math operations
    static BigNumber ModExp(const BigNumber& base, const BigNumber& exp, const BigNumber& mod);
    static BigNumber ModMul(const BigNumber& a, const BigNumber& b, const BigNumber& mod);
    
    std::vector<uint8_t> ToByteArray() const;
    std::string ToHexString() const;

private:
    BIGNUM* _bn;
};

} // namespace MMO::Cryptography
