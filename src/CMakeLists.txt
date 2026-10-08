# Module applicatif C++/Qt6.
#
# On sépare :
#   - une bibliothèque `orders` (modèle + réseau, sans dépendance UI) que les
#     tests unitaires et de contrat peuvent lier directement ;
#   - l'exécutable `ecommerce` (UI QML) qui s'appuie dessus.

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network)

qt_standard_project_setup()

add_library(orders STATIC
    orders/model/Order.cpp
    orders/model/OrdersListModel.cpp
    orders/network/RestApiClient.cpp
)

# Les en-têtes s'incluent par chemin "orders/..." → racine d'include = ce dossier.
target_include_directories(orders PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(orders PUBLIC Qt6::Core Qt6::Network)

qt_add_executable(ecommerce
    main.cpp
    ApplicationEcommerce.cpp
    qml/qml.qrc
    orders/qml/qml.qrc
)
target_link_libraries(ecommerce PRIVATE orders Qt6::Gui Qt6::Qml Qt6::Quick)
