cmake_minimum_required(VERSION 3.16)

project(SmartPersonalDashboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appSmartPersonalDashboard
    main.cpp
    Backend/metricsservice.cpp
)

qt_add_qml_module(appSmartPersonalDashboard
    URI SmartPersonalDashboard
    VERSION 1.0
    QML_FILES
        Main.qml
        UI/HomeComponent.qml
        UI/RightComponent.qml
        UI/LeftComponent.qml
        UI/CenterComponent.qml
)

# Include Backend directory
target_include_directories(appSmartPersonalDashboard PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Backend)

# Link required Qt modules
target_link_libraries(appSmartPersonalDashboard
    PRIVATE Qt6::Quick Qt6::Core
)

include(GNUInstallDirs)
install(TARGETS appSmartPersonalDashboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
