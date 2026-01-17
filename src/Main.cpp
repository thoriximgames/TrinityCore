#include <iostream>
#include <csignal>
#include <thread>
#include <cstdlib>
#include "Host/NetworkHost.h"
#include "Engine/DatabaseManager.h"
#include "Engine/RedisManager.h"
#include "Engine/EntityManager.h"
#include "Engine/PersistenceWorker.h"

// Global pointer for signal handling
std::unique_ptr<MMO::Host::NetworkHost> gHost = nullptr;

/**
 * @brief Signal handler to ensure graceful shutdown on Ctrl+C
 */
void SignalHandler(int signal) {
    if (signal == SIGINT && gHost) {
        std::cout << "\n[Main] Shutdown signal received." << std::endl;
        gHost->Stop();
        MMO::Engine::EntityManager::Instance().StopPersistenceThread();
        MMO::Engine::PersistenceWorker::Instance().Stop();
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   MODERN MMO SERVER v0.1.0 (C++20)     " << std::endl;
    std::cout << "========================================" << std::endl;

    // Register signal handlers
    std::signal(SIGINT, SignalHandler);

    try {
        // Initialize Database from Environment
        const char* dbHost = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "127.0.0.1";
        const char* dbPort = std::getenv("DB_PORT") ? std::getenv("DB_PORT") : "5432";
        const char* dbUser = std::getenv("DB_USER") ? std::getenv("DB_USER") : "mmo_admin";
        const char* dbPass = std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "mmo_password";
        const char* dbName = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "mmo_world";

        if (!MMO::Engine::DatabaseManager::Instance().Connect(dbHost, std::atoi(dbPort), dbUser, dbPass, dbName)) {
            std::cerr << "[Main] CRITICAL: Could not connect to database. Check your environment/compose settings." << std::endl;
            return 1;
        }

        // Initialize Redis from Environment
        const char* redisHost = std::getenv("REDIS_HOST") ? std::getenv("REDIS_HOST") : "127.0.0.1";
        const char* redisPort = std::getenv("REDIS_PORT") ? std::getenv("REDIS_PORT") : "6379";

        if (!MMO::Engine::RedisManager::Instance().Connect(redisHost, std::atoi(redisPort))) {
            std::cerr << "[Main] CRITICAL: Could not connect to Redis. Check your environment/compose settings." << std::endl;
            return 1;
        }

        // Start Engine Threads
        MMO::Engine::EntityManager::Instance().StartPersistenceThread();
        MMO::Engine::PersistenceWorker::Instance().Start();

        // Initialize the Host
        // Defaulting to 8085 (WoW World Port)
        gHost = std::make_unique<MMO::Host::NetworkHost>(8085);
        
        // Start the engine
        gHost->Run();
        
    } catch (const std::exception& e) {
        std::cerr << "[Main] FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[Main] Server exited cleanly." << std::endl;
    return 0;
}