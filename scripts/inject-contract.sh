#!/usr/bin/env bash
# Injecte manuellement un ou plusieurs contrats (OpenAPI/AsyncAPI) dans une instance Microcks déjà démarrée.
# Usage:
#   ./scripts/inject-contract.sh contracts/openapi/order-service.yaml
#   ./scripts/inject-contract.sh contracts/asyncapi/order-notifications.yaml
set -euo pipefail

MICROCKS_URL="${MICROCKS_URL:-http://localhost:8585}"

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <chemin-du-fichier-contrat> [...]" >&2
  exit 1
fi

for contract in "$@"; do
  echo "Injection de ${contract} sur ${MICROCKS_URL} ..."
  curl -sf -X POST "${MICROCKS_URL}/api/artifact/upload?mainArtifact=true" \
    -F "file=@${contract}" \
    && echo "OK: ${contract}" \
    || { echo "Echec de l'injection: ${contract}" >&2; exit 1; }
done
