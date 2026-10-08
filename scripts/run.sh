#!/usr/bin/env bash
# Lancement local de l'application GUI (./build/ecommerce).
#
# Pourquoi ce script ?
#   Les binaires embarquent un RPATH ($ORIGIN/lib) : ils trouvent toutes les
#   *librairies* partagées dans build/lib. MAIS une appli Qt/QML a besoin, EN
#   PLUS des .so, de deux arborescences que `runtime_deploy` n'installe pas :
#     - les PLUGINS de plateforme (platforms/libqxcb.so, wayland, ...) : Qt les
#       cherche dans un sous-dossier "platforms/", absent de build/lib (à plat) ;
#     - les MODULES QML (QtQuick, QtQuick.Controls) : ce sont des dossiers avec
#       des fichiers "qmldir", jamais copiés par runtime_deploy.
#   -> D'où les erreurs "Could not find the Qt platform plugin xcb/wayland" puis
#      "module QtQuick is not installed".
#
#   On réutilise donc l'environnement d'exécution généré par Conan
#   (build/conan/conanrun.sh : QT_PLUGIN_PATH, LD_LIBRARY_PATH, FONTCONFIG_PATH,
#   OPENSSL_MODULES, ICU_DATA...) qui pointe vers le cache Conan, et on ajoute la
#   seule variable que Conan n'exporte pas : QML_IMPORT_PATH (dossier qml de Qt).
#
# Usage :
#   scripts/run.sh                 # lance l'appli (affichage X11/Wayland natif)
#   QT_QPA_PLATFORM=offscreen scripts/run.sh   # sans écran (CI, headless)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

CONAN_RUN="build/conan/conanrun.sh"
BIN="build/ecommerce"

[ -f "$CONAN_RUN" ] || { echo "ERREUR : $CONAN_RUN introuvable. Lancez d'abord scripts/build.sh." >&2; exit 1; }
[ -x "$BIN" ]       || { echo "ERREUR : $BIN introuvable. Lancez d'abord scripts/build.sh." >&2; exit 1; }

# Environnement runtime Conan (expose notamment QT_PLUGIN_PATH).
# shellcheck disable=SC1090
source "$CONAN_RUN"

# Conan n'exporte PAS le chemin des modules QML : il se déduit de QT_PLUGIN_PATH
# (.../p/plugins -> .../p/qml), dans le même paquet Qt du cache.
export QML_IMPORT_PATH="${QT_PLUGIN_PATH%/plugins}/qml"
export QML2_IMPORT_PATH="$QML_IMPORT_PATH"

exec "$BIN" "$@"
