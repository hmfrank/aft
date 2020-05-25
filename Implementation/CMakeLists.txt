cmake_minimum_required(VERSION 3.15)
project(Implementierung)

set(CMAKE_CXX_STANDARD 14)

include_directories(include/ libs/Gamma/)
link_directories(libs/Gamma/build/lib/)

add_executable(
        Implementierung
        include/constants.h
        src/main.cpp
        src/onset_detection.cpp include/onset_detection.h
        src/shift_register.cpp  include/shift_register.h
        src/tempo_induction.cpp include/tempo_induction.h
)