#pragma once
#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <iostream>
#include <optional>
#include <vector>

namespace MMO::Engine {

struct AccountData {
    uint64_t id;
    std::string username;
    std::string password_hash;
    std::string salt;
};

struct CharacterData {
    uint64_t id;
    uint64_t account_id;
    std::string name;
    uint8_t race;
    uint8_t char_class;
    uint32_t level;
    float x, y, z, rotation;
};

class DatabaseManager {
public:
    static DatabaseManager& Instance() {
        static DatabaseManager instance;
        return instance;
    }

    bool Connect(const std::string& host, int port, const std::string& user, const std::string& pass, const std::string& dbname) {
        try {
            std::string conn_str = "host=" + host + " port=" + std::to_string(port) + 
                                  " user=" + user + " password=" + pass + " dbname=" + dbname;
            _connection = std::make_unique<pqxx::connection>(conn_str);
            
            if (_connection->is_open()) {
                std::cout << "[Database] Connected to PostgreSQL at " << host << ":" << port << std::endl;
                return true;
            }
        } catch (const std::exception& e) {
            std::cerr << "[Database] Connection failed: " << e.what() << std::endl;
        }
        return false;
    }

    std::optional<AccountData> GetAccount(const std::string& username) {
        try {
            pqxx::work W(*_connection);
            pqxx::result R = W.exec_params("SELECT id, username, password_hash, salt FROM accounts WHERE username = $1", username);
            
            if (R.empty()) return std::nullopt;

            AccountData account;
            account.id = R[0][0].as<uint64_t>();
            account.username = R[0][1].as<std::string>();
            account.password_hash = R[0][2].as<std::string>();
            account.salt = R[0][3].as<std::string>();
            return account;

        } catch (const std::exception& e) {
            std::cerr << "[Database] Query failed: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    bool CreateAccount(const std::string& username, const std::string& email, const std::string& password_hash, const std::string& salt) {
        try {
            pqxx::work W(*_connection);
            W.exec_params("INSERT INTO accounts (username, email, password_hash, salt) VALUES ($1, $2, $3, $4)", 
                         username, email, password_hash, salt);
            W.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Database] Account creation failed: " << e.what() << std::endl;
            return false;
        }
    }

    bool IsUsernameTaken(const std::string& username) {
        try {
            pqxx::work W(*_connection);
            pqxx::result R = W.exec_params("SELECT 1 FROM accounts WHERE username = $1", username);
            return !R.empty();
        } catch (...) { return true; }
    }

    bool IsEmailTaken(const std::string& email) {
        try {
            pqxx::work W(*_connection);
            pqxx::result R = W.exec_params("SELECT 1 FROM accounts WHERE email = $1", email);
            return !R.empty();
        } catch (...) { return true; }
    }

    // Character Management
    std::vector<CharacterData> GetCharacters(uint64_t account_id) {
        std::vector<CharacterData> chars;
        try {
            pqxx::work W(*_connection);
            pqxx::result R = W.exec_params("SELECT id, name, race, class, level, pos_x, pos_y, pos_z, rotation FROM characters WHERE account_id = $1", account_id);
            
            for (auto row : R) {
                chars.push_back({
                    row[0].as<uint64_t>(),
                    account_id,
                    row[1].as<std::string>(),
                    static_cast<uint8_t>(row[2].as<int>()),
                    static_cast<uint8_t>(row[3].as<int>()),
                    row[4].as<uint32_t>(),
                    row[5].as<float>(),
                    row[6].as<float>(),
                    row[7].as<float>(),
                    row[8].as<float>()
                });
            }
        } catch (const std::exception& e) {
            std::cerr << "[Database] GetCharacters failed: " << e.what() << std::endl;
        }
        return chars;
    }

    bool IsCharacterNameTaken(const std::string& name) {
        try {
            pqxx::work W(*_connection);
            pqxx::result R = W.exec_params("SELECT 1 FROM characters WHERE name = $1", name);
            return !R.empty();
        } catch (...) { return true; }
    }

    bool CreateCharacter(uint64_t account_id, const std::string& name, uint8_t race, uint8_t char_class) {
        try {
            pqxx::work W(*_connection);
            W.exec_params("INSERT INTO characters (account_id, name, race, class) VALUES ($1, $2, $3, $4)", 
                         account_id, name, (int)race, (int)char_class);
            W.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Database] CreateCharacter failed: " << e.what() << std::endl;
            return false;
        }
    }

    bool DeleteCharacter(uint64_t char_id, uint64_t account_id) {
        try {
            pqxx::work W(*_connection);
            W.exec_params("DELETE FROM characters WHERE id = $1 AND account_id = $2", char_id, account_id);
            W.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Database] DeleteCharacter failed: " << e.what() << std::endl;
            return false;
        }
    }

    bool UpdateCharacterPosition(uint64_t char_id, float x, float y, float z, float rotation) {
        try {
            pqxx::work W(*_connection);
            W.exec_params("UPDATE characters SET pos_x = $1, pos_y = $2, pos_z = $3, rotation = $4 WHERE id = $5", 
                         x, y, z, rotation, char_id);
            W.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Database] UpdateCharacterPosition failed: " << e.what() << std::endl;
            return false;
        }
    }

    std::optional<CharacterData> GetCharacter(uint64_t char_id, uint64_t account_id) {
        try {
            pqxx::work W(*_connection);
            pqxx::result R = W.exec_params("SELECT name, race, class, level, pos_x, pos_y, pos_z, rotation FROM characters WHERE id = $1 AND account_id = $2", char_id, account_id);
            
            if (R.empty()) return std::nullopt;

            CharacterData data;
            data.id = char_id;
            data.account_id = account_id;
            data.name = R[0][0].as<std::string>();
            data.race = static_cast<uint8_t>(R[0][1].as<int>());
            data.char_class = static_cast<uint8_t>(R[0][2].as<int>());
            data.level = R[0][3].as<uint32_t>();
            data.x = R[0][4].as<float>();
            data.y = R[0][5].as<float>();
            data.z = R[0][6].as<float>();
            data.rotation = R[0][7].as<float>();
            return data;
        } catch (...) { return std::nullopt; }
    }

private:
    DatabaseManager() = default;
    std::unique_ptr<pqxx::connection> _connection;
};

} // namespace MMO::Engine