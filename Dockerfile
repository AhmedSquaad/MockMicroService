# Image "builder" : toolchain C++ (gcc, cmake, ninja) + Conan, avec le cache des
# dépendances du projet DÉJÀ rempli (Qt6, GTest et leurs transitives).
#
# But : compiler l'application sans jamais re-télécharger ni recompiler les
# dépendances — `conan install` les trouve dans le cache embarqué (CONAN_HOME).
#
# Construire l'image (une fois) :
#   docker build -f Dockerfile -t ecommerce-builder .
#
# Compiler le projet avec (le cache rend `conan install` quasi instantané) :
#   docker run --rm -v "$PWD":/workspace -w /workspace ecommerce-builder scripts/build.sh
#
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive \
    # Cache Conan à un emplacement fixe, embarqué dans l'image.
    CONAN_HOME=/opt/conan2

# Toolchain de compilation + Python (pour Conan).
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build git curl ca-certificates \
      python3 python3-pip python3-venv pkg-config \
 && rm -rf /var/lib/apt/lists/*

# Conan 2 (runtime_deploy nécessite >= 2.2).
RUN pip3 install --break-system-packages --no-cache-dir "conan>=2.2"

# Profil par défaut (gcc du système).
RUN conan profile detect --force

# Pré-remplissage du cache : on copie uniquement la déclaration des dépendances
# (+ le lockfile, pour figer exactement les mêmes versions que le projet) et on
# les installe/compile une fois (figées dans cette couche d'image).
#   - --lockfile : versions identiques à celles utilisées par scripts/build.sh,
#     garantissant la réutilisation du cache (pas de recompilation ensuite) ;
#   - package_manager:mode=install : autorise Conan à installer les dépendances
#     SYSTÈME de Qt (OpenGL, X11...) via apt pendant la construction de l'image.
WORKDIR /opt/deps
COPY conanfile.txt conan.lock ./
RUN conan install . --lockfile=conan.lock --build=missing -s build_type=Release -o "*:shared=True" \
      -c tools.system.package_manager:mode=install \
      -c tools.system.package_manager:sudo=False \
 && rm -rf /opt/deps/*

WORKDIR /workspace
