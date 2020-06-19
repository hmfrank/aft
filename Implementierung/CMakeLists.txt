cmake_minimum_required(VERSION 3.15)
project(Implementierung)

set(CMAKE_CXX_STANDARD 14)

include_directories(
    include/
    libs/Gamma/
    libs/simple2d/include/
)

link_directories(
    libs/Gamma/build/lib/
    libs/simple2d/build/
)

link_libraries(
    Gamma portaudio simple2d SDL2 GL m SDL2_image SDL2_mixer SDL2_ttf pthread
)

set(
    SOURCE_FILES
    src/2001_ChNaUpVe/beat_this.cpp include/2001_ChNaUpVe/beat_this.h
    include/2001_ChNaUpVe/constants.h
    include/2009_DaPlSt/constants.h
    src/2009_DaPlSt/2009_DaPlSt.cpp include/2009_DaPlSt/2009_DaPlSt.h
    src/2009_DaPlSt/beat_prediction.cpp include/2009_DaPlSt/beat_prediction.h
    src/2004_BeDaDuSa/onset_detection.cpp include/2004_BeDaDuSa/onset_detection.h
    src/2009_DaPlSt/tempo_induction.cpp include/2009_DaPlSt/tempo_induction.h
    src/2011_PlRoSt/2011_PlRoSt.cpp include/2011_PlRoSt/2011_PlRoSt.h
    include/2011_PlRoSt/constants.h
    src/misc.cpp include/misc.h
    src/shift_register.cpp include/shift_register.h
)

add_executable(2001_CaNaUpVe ${SOURCE_FILES} src/2001_ChNaUpVe/main.cpp)
add_executable(2009_DaPlSt ${SOURCE_FILES} src/2009_DaPlSt/main.cpp)
add_executable(2011_PlRoSt ${SOURCE_FILES} src/2011_PlRoSt/main.cpp)
add_executable(playground ${SOURCE_FILES} src/main.cpp)
