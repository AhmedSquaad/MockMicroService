# Tests GTest/GMock : unitaires (avec mock de l'API) et de contrat (contre le
# mock Microcks réel). Les deux lient la bibliothèque `orders`.

find_package(Qt6 REQUIRED COMPONENTS Core Network)
find_package(GTest REQUIRED)

# --- Tests unitaires -------------------------------------------------------
# Rapides, hors-ligne : le réseau est remplacé par MockAPIClient (GMock).
add_executable(orders_unit_tests
    unit/orders/main.cpp
    unit/orders/OrderTest.cpp
    unit/orders/OrdersListModelTest.cpp
)
target_include_directories(orders_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/unit/orders)
target_link_libraries(orders_unit_tests
    PRIVATE orders GTest::gtest GTest::gmock Qt6::Core)
add_test(NAME orders_unit COMMAND orders_unit_tests)
set_tests_properties(orders_unit PROPERTIES LABELS "unit")

# --- Tests de contrat ------------------------------------------------------
# Exercent RestApiClient contre le mock Microcks. L'URL est fournie via la
# variable d'environnement MOCK_BASE_URL (défaut : http://localhost:8585/...).
add_executable(orders_contract_tests
    contrat/orders/tests/main.cpp
    contrat/orders/tests/listOrdersTest.cpp
    contrat/orders/tests/CreateOrderTest.cpp
)
target_link_libraries(orders_contract_tests
    PRIVATE orders GTest::gtest Qt6::Core Qt6::Network)
add_test(NAME orders_contract COMMAND orders_contract_tests)
set_tests_properties(orders_contract PROPERTIES LABELS "contract")
