# Profil spécifique Qt6 : options de compilation du paquet qt nécessaires à
# l'application (QML/Quick). À utiliser en complément du profil hôte :
#   conan install . -pr:h conan/qt6/host.profile -pr:b conan/host.profile --build=missing
include(../host.profile)

# Les options communes (shared, qtdeclarative, qtshadertools, with_dbus,
# with_atspi, ...) sont déjà définies dans conanfile.txt et s'appliquent
# systématiquement : ne redéclarer ici que ce qui est propre à ce profil.
[options]
qt/*:qtsvg=True
