#!/bin/sh
# Injecte tous les contrats (OpenAPI) présents sous /contracts dans Microcks.
# Entrypoint du conteneur contract-importer (voir docker-compose.yml).
set -e

MICROCKS_URL="${MICROCKS_URL:-http://microcks:8080}"

until curl -sf "${MICROCKS_URL}/api/keycloak/config" > /dev/null 2>&1 \
  || curl -sf "${MICROCKS_URL}/" > /dev/null 2>&1; do
  echo "En attente de Microcks..."
  sleep 2
done

find /contracts -type f \( -name '*.yaml' -o -name '*.yml' \) | sort | while read -r contract; do
  echo "Injection de ${contract}..."
  curl -sf -X POST "${MICROCKS_URL}/api/artifact/upload?mainArtifact=true" -F "file=@${contract}"
  echo
done

echo "Contrats injectés avec succès."
