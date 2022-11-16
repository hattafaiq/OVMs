QT -= gui
QT += network
QT += core
QT += websockets
QT += sql

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG += sanitizer sanitize_address

QMAKE_CXXFLAGS+= -fsanitize=address -fno-omit-frame-pointer -g3
QMAKE_CFLAGS+= -fsanitize=address -static-libasan -fno-omit-frame-pointer
QMAKE_LFLAGS+= -fsanitize=address -static-libasan


SOURCES += \
        data.cpp \
        database.cpp \
        main.cpp \
        save_database.cpp \
        threader.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    data.h \
    database.h \
    save_database.h \
    setting.h \
    threader.h
