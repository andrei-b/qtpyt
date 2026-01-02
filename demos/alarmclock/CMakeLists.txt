project(alarmclock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_standard_project_setup()

qt_add_executable(alarmclock
    main.cpp
    alarmmodel.h
    alarmmodel.cpp
)

qt_add_qml_module(alarmclock
    URI AlarmApp
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(alarmclock
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
)