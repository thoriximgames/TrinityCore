cmake_minimum_required(VERSION 3.20)
project(ModernMMO VERSION 0.1.0)

# Enforce C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common compiler flags
if(MSVC)
    add_compile_options(/W4 /WX /MP /utf-8)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# Define project structure
include_directories(src)
include_directories(src/Shared)

# Dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(PQ REQUIRED libpq)
pkg_check_modules(HIREDIS REQUIRED hiredis)

add_executable(MMOServer
    src/Main.cpp
    src/Host/NetworkHost.cpp
    src/Host/NetworkHost.h
    src/Host/Session.cpp
    src/Host/Session.h
    src/Host/SessionManager.h
    src/Engine/Entity.h
    src/Engine/EntityManager.h
    src/Engine/DatabaseManager.h
    src/Modules/Auth/Handlers/LoginHandler.cpp
    src/Modules/Auth/Handlers/RegisterHandler.cpp
    src/Modules/Character/Handlers/CharacterListHandler.cpp
    src/Modules/Character/Handlers/CreateCharacterHandler.cpp
    src/Modules/Character/Handlers/DeleteCharacterHandler.cpp
    src/Modules/Character/Handlers/SelectCharacterHandler.cpp
    src/Modules/World/Handlers/RealmListHandler.cpp
    src/Modules/World/Handlers/JoinRealmHandler.cpp
    src/Modules/World/Handlers/MovementHandler.cpp
)

# Link dependencies
target_link_libraries(MMOServer PRIVATE 
    Threads::Threads 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ${PQXX_LIBRARIES}
    ${PQ_LIBRARIES}
    ${HIREDIS_LIBRARIES}
)

target_include_directories(MMOServer PRIVATE
    ${PQXX_INCLUDE_DIRS}
    ${PQ_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}
)

# Target-specific settings
target_compile_definitions(MMOServer PRIVATE 
    MODERN_MMO_SERVER
    ASIO_HAS_CO_AWAIT
    ASIO_HAS_STD_COROUTINE
)
