cmake_minimum_required(VERSION 3.15)
project(Implementierung)

set(CMAKE_CXX_STANDARD 14)

include_directories(libs/Gamma/)
link_directories(libs/Gamma/build/lib/)
add_executable(Implementierung main.cpp onset_detection.cpp onset_detection.h tempo_induction.cpp tempo_induction.h shift_register.cpp shift_register.h)