TARGET = EasyIOT
TEMPLATE = app
INCLUDEPATH += src\client
INCLUDEPATH += src\base
INCLUDEPATH += src\server
SOURCES += main.cpp \
    chatMsgGraphicsItem.cpp \
    chatGraphicsScene.cpp \
    chatGraphicsView.cpp \
    chatDialog.cpp \
    mainDialog.cpp \
    rosterItemModel.cpp \
    rosterItem.cpp \
    rosterItemSortFilterProxyModel.cpp \
    utils.cpp \
    rosterListView.cpp \
    searchLineEdit.cpp \
    statusWidget.cpp \
    signInStatusLabel.cpp \
    statusAvatarWidget.cpp \
    statusTextWidget.cpp \
    statusToolButton.cpp \
    vCardCache.cpp \
    profileDialog.cpp \
    capabilitiesCache.cpp \
    accountsCache.cpp \
    xmlConsoleDialog.cpp \
    aboutDialog.cpp \
    tcpserver.cpp \
    tcpsetdialog.cpp \
    iniconfig.cpp \
    qxmpptranslator.cpp \
    qxmpptranslatordelegate.cpp \
    qxmpptranslatorjsondelegate.cpp \
    webchannel/websocketclientwrapper.cpp \
    webchannel/websockettransport.cpp \
    otherplatformssignin.cpp \
    debugdialog.cpp \
    messagelog.cpp \
    label.cpp \
    sqlhelper.cpp \
    at/atparse.cpp \
    at/conv_hex_str.cpp \
    configdialog.cpp

HEADERS += chatMsgGraphicsItem.h \
    chatGraphicsScene.h \
    chatGraphicsView.h \
    chatDialog.h \
    mainDialog.h \
    rosterItemModel.h \
    rosterItem.h \
    rosterItemSortFilterProxyModel.h \
    utils.h \
    rosterListView.h \
    searchLineEdit.h \
    statusWidget.h \
    signInStatusLabel.h \
    statusAvatarWidget.h \
    statusTextWidget.h \
    statusToolButton.h \
    vCardCache.h \
    profileDialog.h \
    capabilitiesCache.h \
    accountsCache.h \
    xmlConsoleDialog.h \
    aboutDialog.h \
    tcpserver.h \
    tcpsetdialog.h \
    iniconfig.h \
    qxmpptranslator.h \
    qxmpptranslatordelegate.h \
    qxmpptranslatorjsondelegate.h \
    webchannel/websocketclientwrapper.h \
    webchannel/websockettransport.h \
    otherplatformssignin.h \
    debugdialog.h \
    messagelog.h \
    label.h \
    global.h \
    sqlhelper.h \
    at/atparse.h \
    at/conv_hex_str.h \
    configdialog.h

FORMS += mainDialog.ui \
    chatDialog.ui \
    statusWidget.ui \
    profileDialog.ui \
    xmlConsoleDialog.ui \
    aboutDialog.ui \
    tcpsetdialog.ui \
    debugwindow.ui \
    configdialog.ui

QT += network \
    xml \
    widgets \
    webchannel \
    websockets  \
    sql

RESOURCES += resources.qrc
CONFIG(debug, debug|release) {
    LIBS += -LC:\qxmpplib -lqxmpp_d0
} else {
    LIBS += -LC:\qxmpplib -lqxmpp0
}
