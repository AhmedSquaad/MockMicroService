# Profil Conan hôte par défaut (Linux x86_64, GCC 11, C++17).
# Adaptez compiler.version / libcxx à votre toolchain, ou générez-en un avec
# `conan profile detect`.
include(profiles/common)

[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=11
compiler.cppstd=17
compiler.libcxx=libstdc++11
build_type=Release
