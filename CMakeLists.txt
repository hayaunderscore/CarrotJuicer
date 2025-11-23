cmake_minimum_required(VERSION 3.31)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
# ALWAYS use MinGW triplet
set(VCPKG_TARGET_TRIPLET "x64-mingw-static")
# Annoyingly would compile an .so without this
set(TARGET "x86_64-w64-mingw32") 
set(CMAKE_C_COMPILER_TARGET ${TARGET})
set(CMAKE_CXX_COMPILER_TARGET ${TARGET})
set(CMAKE_SYSTEM_NAME "Windows")

project(CarrotJuicer)

# Add unicode definition
add_compile_definitions(UNICODE)
add_compile_definitions(_UNICODE)

# Libraries obv
# For networking and request handling
# TODO make this optional
find_package(ASIO CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(ViaHttpLib CONFIG REQUIRED)
# Discord RPC doesn't wanna work under CMake with vcpkg for now :(
# find_package(discord-rpc CONFIG REQUIRED) # TODO make this optional
find_package(minhook CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)

# Compared to the original CarrotJuicer, this doesn't export has a version.dll hijack at all!
# It's best to use this with Hachimi and not directly as such, as that is detected by anticheat
add_library(CarrotJuicer MODULE)

add_subdirectory(CarrotJuicer)

target_link_libraries(CarrotJuicer CURL::libcurl minhook::minhook nlohmann_json::nlohmann_json SQLiteCpp asio::asio ViaHttpLib::via-httplib)
