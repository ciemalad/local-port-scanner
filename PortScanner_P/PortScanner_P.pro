QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    helpdialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    helpdialog.h \
    mainwindow.h

FORMS += \
    helpdialog.ui \
    mainwindow.ui

TRANSLATIONS += \
    PortScanner_P_en_150.ts
CONFIG += lrelease
CONFIG += embed_translations

INCLUDEPATH += D:\SFML\SFML-2.5.1\include
DEPENDPATH += D:\SFML\SFML-2.5.1\include
LIBS += -LD:\SFML\PORTSCANNER\LIBS

CONFIG(debug, debug|release): LIBS += -lsfml-audio-d -lsfml-system-d -lsfml-network-d -lsfml-main-d -lsfml-window-d -lsfml-graphics-d
CONFIG(release, debug|release): LIBS += -lsfml-audio -lsfml-system -lsfml-network -lsfml-main -lsfml-window -lsfml-graphics

RC_ICONS = favicon.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
