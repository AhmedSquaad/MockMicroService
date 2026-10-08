#!/usr/bin/env bash
# Build local auto-suffisant.
#
# Arborescence produite (build/ propre : binaires + sous-dossiers) :
#   build/
#   ├── ecommerce                 exécutable de l'application
#   ├── orders_unit_tests         tests unitaires
#   ├── orders_contract_tests     tests de contrat
#   ├── lib/                      toutes les libs partagées (Qt, ICU, gtest, ...)
#   ├── conan/                    fichiers générés par Conan (toolchain, CMakeDeps)
#   └── cmake/                    arbre de build CMake (cache, build.ninja, objets)
#
# Les binaires embarquent un RPATH $ORIGIN/lib : ils s'exécutent sans installer
# aucune librairie système ni définir LD_LIBRARY_PATH (auto-suffisants).
#
# Usage :
#   scripts/build.sh                  # build + tests unitaires
#   scripts/build.sh --with-contract  # + mock Microcks (docker) et tests de contrat
set -euo pipefail

BUILD_DIR="build"
CONAN_DIR="${BUILD_DIR}/conan"
CMAKE_DIR="${BUILD_DIR}/cmake"
LIB_DIR="${BUILD_DIR}/lib"

WITH_CONTRACT=0
[ "${1:-}" = "--with-contract" ] && WITH_CONTRACT=1

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

echo ">>> [1/4] Conan : dépendances (shared) dans ${CONAN_DIR}, libs runtime dans ${LIB_DIR}"
conan install . \
  --lockfile=conan.lock \
  --output-folder="${CONAN_DIR}" \
  --build=missing \
  -s build_type=Release \
  -o "*:shared=True" \
  --deployer=runtime_deploy \
  --deployer-folder="${LIB_DIR}"

# Les outils hôtes de Qt (qmlimportscanner, moc) exécutés pendant le configure/
# build ont besoin des libs déployées : on les expose via LD_LIBRARY_PATH.
# (Uniquement nécessaire à la COMPILATION — les binaires finaux utilisent le RPATH.)
export LD_LIBRARY_PATH="${PWD}/${LIB_DIR}:${LD_LIBRARY_PATH:-}"

echo ">>> [2/4] CMake : configuration (build-tree dans ${CMAKE_DIR}, binaires dans ${BUILD_DIR})"
cmake -S . -B "${CMAKE_DIR}" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="${PWD}/${CONAN_DIR}/conan_toolchain.cmake" \
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${PWD}/${BUILD_DIR}"

echo ">>> [3/4] CMake : compilation"
cmake --build "${CMAKE_DIR}"

echo ">>> [4/4] Tests unitaires"
"${BUILD_DIR}/orders_unit_tests"

if [ "$WITH_CONTRACT" = "1" ]; then
  COMPOSE="tests/contrat/orders/docker-compose.yml"
  echo ">>> Tests de contrat : démarrage du mock Microcks"
  docker compose -f "$COMPOSE" up -d
  trap 'docker compose -f "'"$COMPOSE"'" down -v' EXIT
  docker wait contract-importer >/dev/null 2>&1 || true
  MOCK_BASE_URL="http://localhost:8585/rest/Order+Service+API/1.0.0" \
    "${BUILD_DIR}/orders_contract_tests"
fi

echo ">>> OK"
