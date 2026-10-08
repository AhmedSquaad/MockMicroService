# Profil spécifique Qt6 : options de compilation du paquet qt nécessaires à
# l'application (QML/Quick). À utiliser en complément du profil hôte :
#   conan install . -pr:h conan/qt6/host.profile -pr:b conan/host.profile --build=missing
include(../host.profile)

[options]
qt/*:shared=True
qt/*:qtdeclarative=True
qt/*:qtshadertools=True
qt/*:qtsvg=True
