cmake_minimum_required(VERSION 3.10)

# Project
project(CAN_Monitor_qt LANGUAGES CXX C)
message(STATUS "Project: ${PROJECT_NAME}")

# Qt
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    SerialPort
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Enable automoc/autouic/autorcc so .ui/.qrc and Q_OBJECT are handled automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Default to Debug if not provided
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Build type not set, defaulting to Debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Option to exclude building PCAN adapter (matching .pro conditional)
option(EXCLUDE_PCAN "Exclude building PCAN adapter" OFF)

# Source files (translated from CAN-Monitor-qt.pro)
set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    TransmitWidget.cpp
    Tree/TreeModel.cpp
    Tree/TreeNode.cpp
    CanTree/CanTreeModel.cpp
    CanTree/HeaderTreeNode.cpp
    CanTree/MessageTreeNode.cpp
    CanAdapter/CanAdapter.cpp
    CanAdapter/CanAdapterLawicel.cpp
    CanAdapter/SlcanControlWidget.cpp
    CanAdapter/ChinaControlWidget.cpp
    CanAdapter/SerialPortComboBox.cpp
    WidgetUtils/HexStringValidator.cpp
    WidgetUtils/CustomLineEdit.cpp
    lib-slcan/slcan.c
    CanAdapter/CanAdapterLoopback.cpp
    CanAdapter/CanAdapterFactory.cpp
    CanAdapter/CanAdapterTesting.cpp
    WidgetUtils/MenuOpenKeeper.cpp
    CanAdapter/TestingControlWidget.cpp
    Commanders/CommanderDialog.cpp
    Tree/XmlTreeNode.cpp
    CanTree/CanTreeNodeFactory.cpp
    ParameterTree/ParameterTreeModel.cpp
    ParameterTree/ParameterGroupNode.cpp
    ParameterTree/ParameterNode.cpp
    Commanders/ButtonEditDialog.cpp
    ParameterTree/ParameterTreeNodeFactory.cpp
    Commanders/CommanderPropertiesDialog.cpp
    CanHub/CanHub.cpp
    CanAdapter/PollingCanAdapter.cpp
    CanAdapter/CanAdapterTritium.cpp
    Tree/ComboBoxDelegate.cpp
    Format/format.cpp
    CanAdapter/TritiumControlWidget.cpp
    Trace/TraceWindow.cpp
    CanTable/CanTableModel.cpp
    CanTable/CanTableNode.cpp
    util/canMessageUtil.cpp
    util/rangeParse.cpp
    Simulator/TritiumSimulatorWindow.cpp
    CanAdapter/CanAdapterChina.cpp
    Cansole/Cansole.cpp
    Cansole/CansoleIdDialog.cpp
    util/DialogUtil.cpp
)

# Conditional: PCAN adapter
if(NOT EXCLUDE_PCAN)
    message(STATUS "Including PCAN adapter (BUILD_PCAN)")
    list(APPEND PROJECT_SOURCES CanAdapter/CanAdapterPCAN.cpp)
    add_compile_definitions(BUILD_PCAN)
else()
    message(STATUS "Excluding PCAN adapter")
endif()

# Headers and UI files (not strictly required, but useful for IDEs)
set(PROJECT_HEADERS
    MainWindow.h
    TransmitWidget.h
    Tree/TreeModel.h
    Tree/TreeNode.h
    CanTree/CanTreeModel.h
    CanTree/HeaderTreeNode.h
    CanTree/MessageTreeNode.h
    CanAdapter/CanAdapter.h
    CanAdapter/CanAdapterLawicel.h
    CanAdapter/SlcanControlWidget.h
    CanAdapter/ChinaControlWidget.h
    CanAdapter/SerialPortComboBox.h
    WidgetUtils/HexStringValidator.h
    WidgetUtils/CustomLineEdit.h
    lib-slcan/can_message.h
    lib-slcan/slcan.h
    CanAdapter/CanAdapterLoopback.h
    CanAdapter/CanAdapterFactory.h
    CanAdapter/CanAdapterTesting.h
    WidgetUtils/MenuOpenKeeper.h
    CanAdapter/TestingControlWidget.h
    Commanders/CommanderDialog.h
    Tree/XmlTreeNode.h
    CanTree/CanTreeNode.h
    CanTree/CanTreeNodeFactory.h
    ParameterTree/ParameterTreeModel.h
    ParameterTree/ParameterGroupNode.h
    ParameterTree/ParameterTreeNode.h
    ParameterTree/ParameterNode.h
    Commanders/ButtonEditDialog.h
    ParameterTree/ParameterTreeNodeFactory.h
    Commanders/CommanderPropertiesDialog.h
    CanHub/CanHub.h
    CanAdapter/PollingCanAdapter.h
    CanAdapter/CanAdapterTritium.h
    Tree/ComboBoxDelegate.h
    Format/format.h
    CanAdapter/TritiumControlWidget.h
    Trace/TraceWindow.h
    CanTable/CanTableModel.h
    CanTable/CanTableNode.h
    util/canMessageUtil.h
    util/rangeParse.h
    Simulator/TritiumSimulatorWindow.h
    CanAdapter/CanAdapterChina.h
    Cansole/Cansole.h
    Cansole/CansoleIdDialog.h
    util/DialogUtil.h
)

set(PROJECT_FORMS
    MainWindow.ui
    CanAdapter/SlcanControlWidget.ui
    CanAdapter/ChinaControlWidget.ui
    TransmitWidget.ui
    CanAdapter/TestingControlWidget.ui
    Commanders/CommanderDialog.ui
    Commanders/ButtonEditDialog.ui
    Commanders/CommanderPropertiesDialog.ui
    CanAdapter/TritiumControlWidget.ui
    Trace/TraceWindow.ui
    Simulator/TritiumSimulatorWindow.ui
    Cansole/CansoleIdDialog.ui
)

# Create executable target
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_FORMS})

# Set output name to match original TARGET if desired
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "CAN-Monitor-qt")

# Include directories (project subdirs)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    CanAdapter
    CanHub
    Cansole
    CanTable
    CanTree
    Commanders
    Format
    lib-slcan
    ParameterTree
    Simulator
    Trace
    Tree
    util
    WidgetUtils
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::SerialPort
)

# Link local libPCBUSB if present in project root and bundle it on macOS
set(LOCAL_PCB_LIB ${CMAKE_CURRENT_SOURCE_DIR}/third_party/peak_system/PCANBasic.dylib)
if(EXISTS ${LOCAL_PCB_LIB})
    message(STATUS "Found local libPCBUSB: ${LOCAL_PCB_LIB}")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LOCAL_PCB_LIB})
    if(APPLE)
        # Copy the dylib into the app bundle Contents/Frameworks
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LOCAL_PCB_LIB}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks"
            COMMENT "Bundling libPCBUSB into app bundle Contents/Frameworks"
        )
    endif()
endif()

# Deployment notes: on Windows users may use windeployqt; on macOS bundle is created automatically when MACOSX_BUNDLE is used

message(STATUS "CMake configuration complete. To build: mkdir build && cd build && cmake .. && cmake --build .")

