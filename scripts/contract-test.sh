#!/usr/bin/env bash
# Lance un test de conformité Microcks (vérifie qu'un µService réel respecte
# bien son contrat d'interface) via l'API REST /api/tests, et attend/affiche
# le résultat. Générique : pilote n'importe quel nombre de µServices depuis la
# CLI (local ou pipeline CI), sans jamais modifier le contrat ni l'image mock.
#
# Usage:
#   ./scripts/contract-test.sh <serviceId:version> <testEndpoint> [runnerType] [timeoutMs]
#
# Exemples:
#   ./scripts/contract-test.sh "Order Service API:1.0.0" http://order-service:8080 OPEN_API_SCHEMA 10000
#   ./scripts/contract-test.sh "Order Notifications API:1.0.0" ws://order-service:9000/ws ASYNC_API_SCHEMA 10000
#
# Variables d'environnement:
#   MICROCKS_URL     URL de l'instance Microcks (défaut: http://localhost:8585)
#   POLL_INTERVAL_S  intervalle de scrutation du résultat, en secondes (défaut: 2)
set -euo pipefail

MICROCKS_URL="${MICROCKS_URL:-http://localhost:8585}"
POLL_INTERVAL_S="${POLL_INTERVAL_S:-2}"

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <serviceId:version> <testEndpoint> [runnerType] [timeoutMs]" >&2
  echo "  runnerType par défaut: OPEN_API_SCHEMA (autres valeurs: ASYNC_API_SCHEMA, POSTMAN, HTTP, SOAP_HTTP, GRPC_PROTOBUF, GRAPHQL_SCHEMA)" >&2
  exit 1
fi

SERVICE_ID="$1"
TEST_ENDPOINT="$2"
RUNNER_TYPE="${3:-OPEN_API_SCHEMA}"
TIMEOUT_MS="${4:-10000}"

echo "Lancement du test de contrat: service='${SERVICE_ID}' endpoint='${TEST_ENDPOINT}' runner='${RUNNER_TYPE}'"

RESPONSE=$(curl -sf -X POST "${MICROCKS_URL}/api/tests" \
  -H "Content-Type: application/json" \
  -d "{\"serviceId\":\"${SERVICE_ID}\",\"testEndpoint\":\"${TEST_ENDPOINT}\",\"runnerType\":\"${RUNNER_TYPE}\",\"timeout\":${TIMEOUT_MS}}")

TEST_ID=$(echo "${RESPONSE}" | grep -o '"id"[[:space:]]*:[[:space:]]*"[^"]*"' | head -1 | sed -E 's/.*"([^"]*)"$/\1/')

if [[ -z "${TEST_ID}" ]]; then
  echo "Echec du lancement du test, réponse Microcks inattendue:" >&2
  echo "${RESPONSE}" >&2
  exit 1
fi

echo "Test lancé (id=${TEST_ID}), attente du résultat..."

DEADLINE=$(( $(date +%s) + (TIMEOUT_MS / 1000) + 15 ))
RESULT=""
while :; do
  RESULT=$(curl -sf "${MICROCKS_URL}/api/tests/${TEST_ID}")
  IN_PROGRESS=$(echo "${RESULT}" | grep -o '"inProgress"[[:space:]]*:[[:space:]]*[a-z]*' | sed -E 's/.*:[[:space:]]*//')

  if [[ "${IN_PROGRESS}" == "false" ]]; then
    break
  fi
  if (( $(date +%s) > DEADLINE )); then
    echo "Timeout: le test ${TEST_ID} est toujours en cours après ${TIMEOUT_MS}ms." >&2
    exit 1
  fi
  sleep "${POLL_INTERVAL_S}"
done

SUCCESS=$(echo "${RESULT}" | grep -o '"success"[[:space:]]*:[[:space:]]*[a-z]*' | sed -E 's/.*:[[:space:]]*//')

echo "Résultat détaillé: ${MICROCKS_URL}/#/tests/${TEST_ID}"

if [[ "${SUCCESS}" == "true" ]]; then
  echo "Test de contrat REUSSI pour ${SERVICE_ID}"
  exit 0
else
  echo "Test de contrat ECHOUE pour ${SERVICE_ID}" >&2
  echo "${RESULT}" >&2
  exit 1
fi
