cmake_minimum_required(VERSION 4.2)
project(bitboard_chess)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b25ba")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(bitboard_chess main.cpp
        BitBoard.hpp
        types.hpp
        Gameboard.hpp
        MoveGen.cpp
        MoveGen.hpp
        move.hpp
        SlidingAttacks.hpp
        SlidingAttacks.cpp
        Search.cpp
        Search.hpp
        Attacks.cpp
        Attacks.hpp
        Uci.hpp)