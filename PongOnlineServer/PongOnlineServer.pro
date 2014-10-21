TEMPLATE = app

TARGET = PongOnlineServer

QT += \
    core \
    network \

QT -= \
    gui \

CONFIG += \
    console \
    c++11 \

CONFIG -= \
    app_bundle \

QMAKE_CXXFLAGS += \
    -Wall \
    -Wextra \
    -Werror \
    -pedantic \
    -pedantic-errors \

debug:DESTDIR = Build/Debug
release:DESTDIR = Build/Release
OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.rcc
UI_DIR = $$DESTDIR/.ui

INCLUDEPATH += \
    Sources \
    ../PongOnlineCommon/Sources \

release:LIBS += -L../PongOnlineCommon/Build/Release
debug:LIBS += -L../PongOnlineCommon/Build/Debug
LIBS += -lPongOnlineCommon

HEADERS += \
    Sources/Game/Game/Game.hpp \
    Sources/Game/GameManager/GameManager.hpp \
    Sources/NetworkManager/NetworkManager.hpp \
    Sources/Peer/Peer.hpp \
    Sources/Server/Server.hpp \

SOURCES += \
    Sources/Main/Main.cpp \
    Sources/Game/Game/Game.cpp \
    Sources/Game/GameManager/GameManager.cpp \
    Sources/NetworkManager/NetworkManager.cpp \
    Sources/Peer/Peer.cpp \
    Sources/Server/Server.cpp \

