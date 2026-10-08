# Ecommerce (C++/Qt6) — tests unitaires & de contrat pilotés par Microcks

Application **C++/Qt6** (CMake + Conan) dont les classes d'accès au µService de
commandes sont validées par :

- des **tests unitaires** (GTest/GMock) — hors-ligne, l'API est mockée ;
- des **tests de contrat** (GTest) — `RestApiClient` est exercé contre un mock
  **Microcks** généré à partir du contrat OpenAPI.

La communication HTTP et la (dé)sérialisation JSON utilisent exclusivement Qt
(`QNetworkAccessManager`, `QJsonDocument`).

## 1. Structure

```
.
├── CMakeLists.txt                # racine : add_subdirectory(src) + tests
├── conanfile.txt                 # dépendances : qt/6, gtest
├── conan/                        # profils Conan (host.profile, qt6/, profiles/common)
├── .gitlab-ci.yml                # CI : build + unit + contrat (Microcks en service GitLab)
├── scripts/
│   └── build.sh                  # build local auto-suffisant + tests
├── src/
│   ├── main.cpp                  # point d'entrée (QGuiApplication)
│   ├── ApplicationEcommerce.*    # composition : client REST + modèle + QML
│   ├── CMakeLists.txt            # lib `orders` + exécutable `ecommerce`
│   ├── qml/Main.qml
│   └── orders/                   # module "orders"
│       ├── model/Order.*         # DTO (de)sérialisable JSON
│       ├── model/OrdersListModel.*
│       ├── network/IAPIClient.h  # interface (méthodes virtuelles, mockable)
│       ├── network/RestApiClient.* # implémentation Qt Network
│       └── qml/OrdersPanel.qml
└── tests/
    ├── CMakeLists.txt
    ├── unit/orders/              # GTest/GMock, hors-ligne
    │   ├── OrderTest.cpp
    │   ├── OrdersListModelTest.cpp
    │   └── mocks/MockAPIClient.h # mock GMock d'IApiClient
    └── contrat/orders/           # GTest contre le mock Microcks
        ├── docker-compose.yml    # mock Microcks pour exécution LOCALE
        ├── upload-contracts.sh
        ├── contracts/openapi/order-service.yaml
        └── tests/
            ├── listOrdersTest.cpp
            └── CreateOrderTest.cpp
```

## 2. Prérequis

- CMake ≥ 3.21, un compilateur C++17, Ninja.
- [Conan 2](https://conan.io) (`pip install conan`).
- Docker + Docker Compose (pour lancer le mock en local).

## 3. Construire

Script tout-en-un (Conan + CMake + tests), build auto-suffisant dans `build/` :

```bash
scripts/build.sh                  # build + tests unitaires
scripts/build.sh --with-contract  # + mock Microcks (docker) et tests de contrat
```

Le dossier `build/` produit est **auto-suffisant** :

```
build/
├── ecommerce                 # exécutable de l'application
├── orders_unit_tests
├── orders_contract_tests
└── lib/                      # TOUTES les libs partagées (Qt, ICU, gtest, ...)
```

Toutes les dépendances sont **dynamiques** et déployées dans `build/lib` par le
deployer Conan `runtime_deploy`. Les binaires embarquent un RPATH `$ORIGIN/lib` :
ils s'exécutent **sans installer aucune librairie** ni définir `LD_LIBRARY_PATH`.

Étapes détaillées (équivalent du script) :

```bash
conan install . --output-folder=build --build=missing -s build_type=Release \
  -o "*:shared=True" --deployer=runtime_deploy --deployer-folder=build/lib
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/build/conan_toolchain.cmake"
cmake --build build
```

> Conan télécharge/compile Qt6 et GTest. La première compilation de Qt peut être
> longue ; configurez un remote Conan avec binaires pré-compilés pour l'accélérer.
> (Qt ≥ 2.2 requis pour `runtime_deploy`.)

## 4. Tests unitaires

Aucune dépendance externe (l'API est remplacée par `MockAPIClient`) :

```bash
./build/orders_unit_tests
```

## 5. Tests de contrat (local)

1. Démarrer le mock Microcks (REST) et injecter le contrat `orders` :
   ```bash
   docker compose -f tests/contrat/orders/docker-compose.yml up -d
   ```
2. Lancer la suite de contrat en pointant vers le mock :
   ```bash
   MOCK_BASE_URL="http://localhost:8585/rest/Order+Service+API/1.0.0" \
     ./build/orders_contract_tests
   ```
3. Arrêter le mock :
   ```bash
   docker compose -f tests/contrat/orders/docker-compose.yml down -v
   ```

> `scripts/build.sh --with-contract` enchaîne automatiquement ces trois étapes.

Les tests couvrent : `GET /orders` (liste), `POST /orders` conforme (**201**) et
non conforme — `customerId` manquant → dispatcher `JSON_BODY` → **400**.

## 6. Intégration continue

[.gitlab-ci.yml](.gitlab-ci.yml) — **2 stages, 3 jobs** :

| Stage | Job | Rôle |
|---|---|---|
| `build` | `build` | compile l'app + les tests, publie `build/` en artefact |
| `test` | `test:unit` | lance `orders_unit_tests` (hors-ligne) |
| `test` | `test:contrat` | lance `orders_contract_tests` contre Microcks |

Les jobs de test réutilisent l'artefact `build/` et exécutent **directement** les
binaires — aucune librairie à installer (RPATH → `build/lib`). Microcks est lancé
comme **service GitLab** (`services:`), joint via l'alias `microcks` ; les tests
de contrat tapent `http://microcks:8080/rest/Order+Service+API/1.0.0`
(`MOCK_BASE_URL`).

> Avantage : **aucun Docker-in-Docker ni mode `privileged`** requis — le pipeline
> fonctionne sur n'importe quel runner, y compris auto-hébergé/VPS.

## 7. Image Docker builder (dépendances pré-compilées)

[Dockerfile](Dockerfile) produit une image **builder** dont le cache Conan est
**déjà rempli** (Qt6, GTest et transitives, versions figées par
[conan.lock](conan.lock)). Elle permet de compiler le projet **sans jamais
re-télécharger ni recompiler les dépendances** : `conan install` les trouve
dans le cache embarqué.

```bash
# Construire l'image une fois (longue : compile/télécharge toutes les deps)
docker build -t ecommerce-builder .

# Compiler + tester le projet (conan install = cache, quasi instantané)
docker run --rm -v "$PWD":/workspace -w /workspace ecommerce-builder scripts/build.sh
```

Le lockfile garantit que l'image, `scripts/build.sh` et la CI utilisent **les
mêmes versions** de dépendances (cohérence du cache).

## 8. Ajouter un µService / une opération à mocker

1. Décrire l'opération dans le contrat OpenAPI
   ([tests/contrat/orders/contracts/openapi/order-service.yaml](tests/contrat/orders/contracts/openapi/order-service.yaml)),
   avec des `examples` pour chaque réponse.
2. Étendre `IApiClient` + `RestApiClient` (et le DTO si besoin).
3. Ajouter un test de contrat sous `tests/contrat/orders/tests/` et un test
   unitaire sous `tests/unit/orders/`.
