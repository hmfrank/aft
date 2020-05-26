cmake_minimum_required(VERSION 3.15)
project(Implementierung)

set(CMAKE_CXX_STANDARD 14)

include_directories(include/ libs/Gamma/)
link_directories(libs/Gamma/build/lib/)

add_executable(
        Implementierung
        include/2009_DaPlSt/constants.h
        src/2009_DaPlSt/beat_prediction.cpp include/2009_DaPlSt/beat_prediction.h
        src/2009_DaPlSt/main.cpp include/2009_DaPlSt/main.h
        src/2009_DaPlSt/onset_detection.cpp include/2009_DaPlSt/onset_detection.h
        src/2009_DaPlSt/tempo_induction.cpp include/2009_DaPlSt/tempo_induction.h
        src/main.cpp
        src/shift_register.cpp  include/shift_register.h
)