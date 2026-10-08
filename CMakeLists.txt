cmake_minimum_required(VERSION 3.21)

project(EcommerceMock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# AUTOMOC/AUTORCC : moc pour les QObject, rcc pour les .qrc (QML embarqué).
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sortie des exécutables : CMAKE_BINARY_DIR par défaut, mais surchargeable via
# -DCMAKE_RUNTIME_OUTPUT_DIRECTORY. Le script/CI place l'arbre de build dans
# build/cmake et sort les binaires dans build/. Les libs partagées vont dans build/lib.
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()

# Binaires relocalisables et auto-suffisants : RPATH = $ORIGIN/lib, gravé dès la
# compilation (BUILD_WITH_INSTALL_RPATH) → aucun LD_LIBRARY_PATH ni installation
# de librairie requis pour exécuter les binaires.
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
