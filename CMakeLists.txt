cmake_minimum_required(VERSION 3.15)
project(kok-1976 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Kamoy Doktrini Derleme Bayrakları (Sıfır Dış Bağımlılık, Katı Hata Denetimi)
if(MSVC)
    add_compile_options(/W4 /WX /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Temel KÖK-1976 Çekirdek Kütüphanesi
add_library(kok_core STATIC
    core/src/math.cpp
    core/src/fec.cpp
    core/src/crypto.cpp
    radio/modulation/fsk.cpp
    radio/modulation/fhss.cpp
    radio/modulation/dsss.cpp
    radio/telemetry/frame.cpp
    radio/telemetry/protocol.cpp
    radio/audio/cvsd.cpp
    radio/audio/squelch.cpp
    radio/network/mesh_router.cpp
    radio/sdr/dsp_filter.cpp
)

target_include_directories(kok_core PUBLIC
    core/include
    radio/modulation
    radio/telemetry
    radio/audio
    radio/network
    radio/sdr
)

# C++ Test Koşturucusu
add_executable(kok_cpp_test
    tests/test_cpp_core.cpp
)

target_link_libraries(kok_cpp_test PRIVATE kok_core)

enable_testing()
add_test(NAME KokCppCoreTest COMMAND kok_cpp_test)
