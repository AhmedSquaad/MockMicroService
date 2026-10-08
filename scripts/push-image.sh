#!/usr/bin/env bash
# Construit l'image builder (toolchain + cache Conan pré-rempli) depuis le
# Dockerfile et la pousse dans le registry du projet GitLab, afin que le job
# "build" de la CI (.gitlab-ci.yml) l'utilise directement — sans jamais
# (re)compiler les dépendances.
#
# Usage :
#   scripts/push-image.sh
set -euo pipefail

REGISTRY="registry.gitlab.com"
IMAGE="${REGISTRY}/squaad.io/mockmicroservice"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

echo ">>> [1/3] Connexion au registry ${REGISTRY}"
docker login "${REGISTRY}"

echo ">>> [2/3] Construction de l'image ${IMAGE}"
docker build -t "${IMAGE}" .

echo ">>> [3/3] Push de l'image ${IMAGE}"
docker push "${IMAGE}"

echo ">>> OK"
