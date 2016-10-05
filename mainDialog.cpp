/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */


#include "mainDialog.h"
#include "ui_mainDialog.h"

#include "utils.h"
#include "profileDialog.h"
#include "aboutDialog.h"
#include "chatDialog.h"

#include "QXmppPresence.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"
#include "QXmppVCardManager.h"

#include <QMovie>
#include <QCompleter>
#include <QInputDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDir>
#include <QSettings>
#include "global.h"
mainDialog::mainDialog(QWidget *parent): QDialog(parent, Qt::Window),
    ui(new Ui::mainDialogClass), m_rosterItemModel(this),
    m_rosterItemSortFilterModel(this), m_vCardCache(&m_xmppClient),
    m_capabilitiesCache(&m_xmppClient), m_accountsCache(this),
    m_tcpServer(&m_xmppClient,this),
#ifndef QT_NO_SYSTEMTRAYICON
    m_trayIcon(this), m_trayIconMenu(this),
#endif

    m_quitAction(tr("退出"), this),
    m_signOutAction(tr("注销"), this),
    m_tcpSetDlg(this),
    m_configDlg(&m_xmppClient),
    m_settingsMenu(0),
    openAdvancedSetting(false)

{
    bool check;
    Q_UNUSED(check);

    ui->setupUi(this);
    createTrayIconAndMenu();
    createSettingsMenu();

    ui->pushButton_cancel->setDisabled(true);
    ui->label_throbber->setMovie(new QMovie(":/icons/resource/ajax-loader.gif"));
    ui->label_throbber->movie()->start();
    showSignInPage();
    loadAccounts();

    check = connect(ui->lineEdit_userName->completer(),SIGNAL(activated(QString)),this, SLOT(userNameCompleter_activated(QString)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient.rosterManager(),SIGNAL(rosterReceived()),this, SLOT(rosterReceived()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient.rosterManager(),SIGNAL(itemChanged(QString)),this, SLOT(rosterChanged(QString)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient,SIGNAL(error(QXmppClient::Error)),this, SLOT(errorClient(QXmppClient::Error)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient,SIGNAL(presenceReceived(QXmppPresence)),this, SLOT(presenceReceived(QXmppPresence)));
    Q_ASSERT(check);

    QXmppLogger::getLogger()->setLoggingType(QXmppLogger::SignalLogging);


    check = connect(&m_xmppClient.rosterManager(),SIGNAL(presenceChanged(QString,QString)),this, SLOT(presenceChanged(QString,QString)));
    Q_ASSERT(check);

    check = connect(ui->lineEdit_filter, SIGNAL(textChanged(QString)),this, SLOT(filterChanged(QString)));
    Q_ASSERT(check);

    check = connect(ui->listView, SIGNAL(showChatDialog(QString)),this, SLOT(showChatDialog(QString)));
    Q_ASSERT(check);

    check = connect(ui->listView, SIGNAL(showProfile(QString)),this, SLOT(showProfile(QString)));
    Q_ASSERT(check);

    check = connect(ui->listView, SIGNAL(removeContact(QString)),this, SLOT(action_removeContact(QString)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(messageReceived(QXmppMessage)),SLOT(messageReceived(QXmppMessage)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient,SIGNAL(messageReceived(QXmppMessage)),&m_tcpServer,SLOT(sendRecvMessage(QXmppMessage)));

    check = connect(ui->pushButton_signIn, SIGNAL(clicked(bool)), SLOT(signIn()));
    Q_ASSERT(check);

    check = connect(ui->pushButton_cancel, SIGNAL(clicked(bool)),SLOT(cancelSignIn()));
    Q_ASSERT(check);

    check = connect(ui->pushButton_wechatSignIn,SIGNAL(clicked(bool)),SLOT(wechatSignIn()));
    Q_ASSERT(check);

    check = connect(&m_otherPlatformsSignIn,SIGNAL(signIn(QString,QString)),SLOT(signIn(QString,QString)));
    Q_ASSERT(check);

    m_rosterItemSortFilterModel.setSourceModel(&m_rosterItemModel);
    ui->listView->setModel(&m_rosterItemSortFilterModel);
    m_rosterItemSortFilterModel.sort(0);

    rosterItemDelegate *delegate = new rosterItemDelegate();
    ui->listView->setItemDelegate(delegate);
    ui->listView->setFocus();
    ui->verticalLayout_3->insertWidget(0, &m_statusWidget);

    check = connect(&m_statusWidget, SIGNAL(statusTextChanged(QString)),SLOT(statusTextChanged(QString)));
    Q_ASSERT(check);

    check = connect(&m_statusWidget, SIGNAL(presenceTypeChanged(QXmppPresence::Type)),SLOT(presenceTypeChanged(QXmppPresence::Type)));
    Q_ASSERT(check);

    check = connect(&m_statusWidget,SIGNAL(presenceStatusTypeChanged(QXmppPresence::AvailableStatusType)),SLOT(presenceStatusTypeChanged(QXmppPresence::AvailableStatusType)));
    Q_ASSERT(check);

    check = connect(&m_statusWidget,SIGNAL(avatarChanged(QImage)),SLOT(avatarChanged(QImage)));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(connected()), SLOT(updateStatusWidget()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(connected()), SLOT(showRosterPage()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(connected()), SLOT(addAccountToCache()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient,SIGNAL(connected()),SLOT(loadUserConfig()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient, SIGNAL(disconnected()), SLOT(showSignInPageAfterUserDisconnection()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient,SIGNAL(disconnected()),SLOT(stopTcpServer()));
    Q_ASSERT(check);

    check = connect(&m_xmppClient.vCardManager(),SIGNAL(vCardReceived(QXmppVCardIq)), &m_vCardCache,SLOT(vCardReceived(QXmppVCardIq)));
    Q_ASSERT(check);

    check = connect(&m_vCardCache,SIGNAL(vCardReadyToUse(QString)),SLOT(updateVCard(QString)));
    Q_ASSERT(check);

    check = connect(ui->pushButton_addContact, SIGNAL(clicked()), SLOT(action_addContact()));
    Q_ASSERT(check);

    check = connect(QXmppLogger::getLogger(),SIGNAL(message(QXmppLogger::MessageType,QString)),&m_consoleDlg,SLOT(message(QXmppLogger::MessageType,QString)));
    Q_ASSERT(check);

    check = connect(ui->pushButton_settings,SIGNAL(pressed()),SLOT(action_settingsPressed()));
    Q_ASSERT(check);

    check = connect(&m_tcpServer,SIGNAL(sendMessage(QString,QString)),&m_xmppClient,SLOT(sendMessage(QString,QString)));
    Q_ASSERT(check);

    check = connect(&m_tcpServer,SIGNAL(setPresenceStatus(QString)),SLOT(statusTextChanged(QString)));
    Q_ASSERT(check);

    addAdvancedSettingsLabel();
    check = connect(m_lab_advancedSetting,SIGNAL(clicked()),SLOT(pressAdvancedSettingLabel()));
    Q_ASSERT(check);
}

void mainDialog::rosterChanged(const QString& bareJid)
{

    m_rosterItemModel.updateRosterEntry(bareJid, m_xmppClient.rosterManager().
                                        getRosterEntry(bareJid));

    // if available in cache, update it else based on presence it will request if not available
    if(m_vCardCache.isVCardAvailable(bareJid))
        updateVCard(bareJid);
}

void mainDialog::rosterReceived()
{
    QStringList list = m_xmppClient.rosterManager().getRosterBareJids();
    QString bareJid;
    foreach(bareJid, list)
        rosterChanged(bareJid);
}

void mainDialog::presenceChanged(const QString& bareJid, const QString& resource)
{
    if(bareJid == m_xmppClient.configuration().jidBare())
        return;

    if(!m_rosterItemModel.getRosterItemFromBareJid(bareJid))
        return;

    QString jid = bareJid + "/" + resource;
    QMap<QString, QXmppPresence> presences = m_xmppClient.rosterManager().
                                             getAllPresencesForBareJid(bareJid);
    //m_rosterItemModel.updatePresence(bareJid, presences);
    if(!bareJid.isEmpty())
        getDebugDialog(bareJid)->setResource(m_xmppClient.rosterManager().getResources(bareJid));

    if(presences.contains(resource))
    {
        QXmppPresence& presence = presences[resource];

        if(!onlineMap.contains(jid))
        {
            onlineMap.insert(jid,false);
        }

        if(presence.type() == QXmppPresence::Available && onlineMap.value(jid,true) == false)//上线
        {
            m_tcpServer.sendOnline(jid,true);
            if(!bareJid.isEmpty())
                getDebugDialog(bareJid)->setBareJidOnline(true);
            onlineMap[jid] = true;
            m_rosterItemModel.updatePresence(bareJid, presences);
        }
        else if(presence.type() == QXmppPresence::Unavailable && onlineMap.value(jid,false) == true)//下线
        {
            m_tcpServer.sendOnline(jid,false);
            if(!bareJid.isEmpty())
                getDebugDialog(bareJid)->setBareJidOnline(false);
            onlineMap[jid] = false;
            m_rosterItemModel.updatePresence(bareJid, presences);
        }
        else
        {
            if(getDebugDialog(bareJid))
                getDebugDialog(bareJid)->presenceReceived(presence);
        }

        m_tcpServer.sendRecvPresence(jid,presence);
    }
    else if(onlineMap.contains(jid) && onlineMap.value(jid,false) == true)
    {
        m_tcpServer.sendOnline(jid,false);
        if(!bareJid.isEmpty())
            getDebugDialog(bareJid)->setBareJidOnline(false);
        onlineMap[jid] = false;
        m_rosterItemModel.updatePresence(bareJid, presences);
    }

    QXmppPresence& pre = presences[resource];

    if(pre.type() == QXmppPresence::Available)
    {
        QString node = pre.capabilityNode();
        QString ver = pre.capabilityVer().toBase64();
        QStringList exts = pre.capabilityExt();

        QString nodeVer = node + "#" + ver;
        if(!m_capabilitiesCache.isCapabilityAvailable(nodeVer))
            m_capabilitiesCache.requestInfo(jid, nodeVer);

        foreach(QString ext, exts)
        {
            nodeVer = node + "#" + ext;
            if(!m_capabilitiesCache.isCapabilityAvailable(nodeVer))
                m_capabilitiesCache.requestInfo(jid, nodeVer);
        }

        switch(pre.vCardUpdateType())
        {
        case QXmppPresence::VCardUpdateNone:
            if(!m_vCardCache.isVCardAvailable(bareJid))
                m_vCardCache.requestVCard(bareJid);
        case QXmppPresence::VCardUpdateNotReady:
            break;
        case QXmppPresence::VCardUpdateNoPhoto:
        case QXmppPresence::VCardUpdateValidPhoto:
            if(m_vCardCache.getPhotoHash(bareJid) != pre.photoHash())
                m_vCardCache.requestVCard(bareJid);
            break;
        }
    }

//    QXmppPresence::Type presenceType = presences.begin().value().getType();

//    if(!m_vCardCache.isVCardAvailable(bareJid) &&
//       presenceType == QXmppPresence::Available)
//    {
//        m_rosterItemModel.updateAvatar(bareJid,
//                                   m_vCardCache.getVCard(bareJid).image);
//    }
}

void mainDialog::filterChanged(const QString& filter)
{
    m_rosterItemSortFilterModel.setFilterRegExp(filter);

    // follow statement selects the first row
    ui->listView->selectionModel()->select(ui->listView->model()->index(0, 0),
                                           QItemSelectionModel::ClearAndSelect);
}

void mainDialog::keyPressEvent(QKeyEvent* event1)
{
    if(ui->stackedWidget->currentIndex() == 0) // roster page
    {
        if(event1->matches(QKeySequence::Find) ||(
           event1->key() <= Qt::Key_9 && event1->key() >= Qt::Key_1) ||
           (event1->key() <= Qt::Key_Z && event1->key() >= Qt::Key_At) ||
           event1->key() == Qt::Key_Backspace)
        {
            ui->lineEdit_filter->setFocus();
            ui->lineEdit_filter->event(event1);
        }
        else if(event1->key() == Qt::Key_Escape)
        {
            ui->lineEdit_filter->clear();
            ui->listView->setFocus();
        }
        else if(event1->key() == Qt::Key_Up ||
                event1->key() == Qt::Key_Down ||
                event1->key() == Qt::Key_PageUp ||
                event1->key() == Qt::Key_PageDown)
        {
            ui->listView->setFocus();
            ui->listView->event(event1);
        }
        else if(event1->key() == Qt::Key_Return && ui->listView->hasFocus())
        {
            ui->listView->event(event1);
        }
    }

// don't close on escape
    if(event1->key() == Qt::Key_Escape)
    {
        event1->ignore();
        return;
    }
// FIXME: I'm not sure what this is supposed to do, but it does not compile.
#if 0
    else if(minimize && e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
    {
        event1->ignore();
        return;
    }
#endif
// don't close on escape

    if(ui->stackedWidget->currentIndex() == 1) // sign in page
    {
        QDialog::keyPressEvent(event1);
        return;
    }
}

chatDialog* mainDialog::getChatDialog(const QString& bareJid)
{
    if(!m_chatDlgsList.contains(bareJid))
    {
        m_chatDlgsList[bareJid] = new chatDialog();
        m_chatDlgsList[bareJid]->setBareJid(bareJid);

        if(!m_rosterItemModel.getRosterItemFromBareJid(bareJid))
            return 0;

        if(!m_rosterItemModel.getRosterItemFromBareJid(bareJid)->
           getName().isEmpty())
            m_chatDlgsList[bareJid]->setDisplayName(m_rosterItemModel.
                                                getRosterItemFromBareJid(bareJid)->getName());
        else
            m_chatDlgsList[bareJid]->setDisplayName(QXmppUtils::jidToUser(bareJid));

        m_chatDlgsList[bareJid]->setQXmppClient(&m_xmppClient);
    }

    return m_chatDlgsList[bareJid];
}

DebugDialog *mainDialog::getDebugDialog(const QString &bareJid)
{
    if(!m_debugDlgList.contains(bareJid))
    {
        if(!m_rosterItemModel.getRosterItemFromBareJid(bareJid))
            return 0;
        m_debugDlgList[bareJid] = new DebugDialog;
        m_debugDlgList[bareJid]->setBareJid(bareJid);
        if(!m_rosterItemModel.getRosterItemFromBareJid(bareJid)->getName().isEmpty())
            m_debugDlgList[bareJid]->setDisplayName(m_rosterItemModel.getRosterItemFromBareJid(bareJid)->getName());
        else
            m_debugDlgList[bareJid]->setDisplayName(QXmppUtils::jidToUser(bareJid));
        m_debugDlgList[bareJid]->setQXmppClient(&m_xmppClient);
        m_debugDlgList[bareJid]->setResource(m_xmppClient.rosterManager().getResources(bareJid));
        connect(m_debugDlgList[bareJid],SIGNAL(updateSignal(QString,int,int)),SLOT(updateSignal(QString,int,int)));
    }
    return m_debugDlgList[bareJid];
}

void mainDialog::showChatDialog(const QString& bareJid)
{
    if(!bareJid.isEmpty())
    {
        getDebugDialog(bareJid)->hide();
        getDebugDialog(bareJid)->show();
    }
}

void mainDialog::messageReceived(const QXmppMessage& msg)
{
    if (msg.body().isEmpty())
        return;

    /*chatDialog *dialog = getChatDialog(QXmppUtils::jidToBareJid(msg.from()));
    if (dialog)
    {
        dialog->show();
        dialog->messageReceived(msg.body());
    }*/

    DebugDialog *debugDialog = getDebugDialog(QXmppUtils::jidToBareJid(msg.from()));
    if(debugDialog)
    {
        debugDialog->messageReceived(msg);
    }
}

void mainDialog::statusTextChanged(const QString& status)
{
    QXmppPresence presence = m_xmppClient.clientPresence();
    presence.setStatusText(status);
    addPhotoHash(presence);
    m_xmppClient.setClientPresence(presence);
    m_statusWidget.setStatusText(status);
}

void mainDialog::presenceTypeChanged(QXmppPresence::Type presenceType)
{
    if(presenceType == QXmppPresence::Unavailable)
    {
        m_xmppClient.disconnectFromServer();
    }
    else if(presenceType == QXmppPresence::Available)
    {
        QXmppPresence newPresence = m_xmppClient.clientPresence();
        newPresence.setType(presenceType);
        newPresence.setAvailableStatusType(QXmppPresence::Online);
        addPhotoHash(newPresence);
        m_xmppClient.setClientPresence(newPresence);
    }
    m_statusWidget.setStatusText(
            presenceToStatusText(m_xmppClient.clientPresence()));
}

void mainDialog::presenceStatusTypeChanged(QXmppPresence::AvailableStatusType statusType)
{
    QXmppPresence presence = m_xmppClient.clientPresence();
    presence.setType(QXmppPresence::Available);
    presence.setAvailableStatusType(statusType);
    addPhotoHash(presence);
    m_xmppClient.setClientPresence(presence);
    m_statusWidget.setStatusText(
    presenceToStatusText(m_xmppClient.clientPresence()));
}

void mainDialog::avatarChanged(const QImage& image)
{
    QXmppVCardIq vcard;
    vcard.setType(QXmppIq::Set);

    QByteArray ba;
    QBuffer buffer(&ba);
    if(buffer.open(QIODevice::WriteOnly))
    {
        if(image.save(&buffer, "PNG"))
        {
            vcard.setPhoto(ba);
            m_xmppClient.sendPacket(vcard);
            m_statusWidget.setAvatar(image);

            m_vCardCache.getVCard(m_xmppClient.configuration().jidBare()) = vcard;
            // update photo hash
            QXmppPresence presence = m_xmppClient.clientPresence();
            addPhotoHash(presence);
            m_xmppClient.setClientPresence(presence);
        }
    }
}

void mainDialog::updateStatusWidget()
{
    const QString bareJid = m_xmppClient.configuration().jidBare();
    qDebug()<<bareJid;
    // initialise status widget
    updateVCard(bareJid);
    m_statusWidget.setStatusText(presenceToStatusText(m_xmppClient.clientPresence()));
    m_statusWidget.setPresenceAndStatusType(m_xmppClient.clientPresence().type(),
                                            m_xmppClient.clientPresence().availableStatusType());

    // fetch own vCard
    m_vCardCache.requestVCard(bareJid);
}

void mainDialog::signIn()
{
   /* ui->label_throbber->show();
    ui->pushButton_signIn->setDisabled(true);
    ui->pushButton_cancel->setDisabled(false);
    ui->lineEdit_userName->setDisabled(true);
    ui->lineEdit_password->setDisabled(true);
    ui->checkBox_rememberPasswd->setDisabled(true);
    showLoginStatusWithProgress(tr("连接中"));

    QString bareJid = ui->lineEdit_userName->text();
    QString passwd = ui->lineEdit_password->text();

    m_xmppClient.configuration().setJid(bareJid);
    m_xmppClient.configuration().setPassword(passwd);

    m_xmppClient.configuration().setHost(ui->lineEdit_host->text());//"jareymobile.com.tw"
    m_xmppClient.configuration().setPort(ui->lineEdit_port->text().toInt());//20030

    m_rosterItemModel.clear();

    m_vCardCache.loadFromFile();
    m_capabilitiesCache.loadFromFile();

    startConnection();*/
    signIn(ui->lineEdit_userName->text(),ui->lineEdit_password->text());
}

void mainDialog::signIn(const QString &userName, const QString &password)
{
    ui->label_throbber->show();
    ui->pushButton_signIn->setDisabled(true);
    ui->pushButton_cancel->setDisabled(false);
    ui->lineEdit_userName->setDisabled(true);
    ui->lineEdit_password->setDisabled(true);
    ui->checkBox_rememberPasswd->setDisabled(true);
    showLoginStatusWithProgress(tr("连接中"));
    ui->lineEdit_userName->setText(userName);
    ui->lineEdit_password->setText(password);
    QString bareJid = userName;
    QString passwd = password;

    m_xmppClient.configuration().setJid(bareJid);
    m_xmppClient.configuration().setPassword(passwd);

    m_xmppClient.configuration().setHost(/*"jareymobile.com.tw"*/ui->lineEdit_host->text());
    m_xmppClient.configuration().setPort(/*20030*/ui->lineEdit_port->text().toInt());

    m_rosterItemModel.clear();

    m_vCardCache.loadFromFile();
    m_capabilitiesCache.loadFromFile();

    startConnection();
}

void mainDialog::wechatSignIn()
{
    m_otherPlatformsSignIn.signIn(OtherPlatformsSignIn::QQ);
}

void mainDialog::cancelSignIn()
{
    if(!ui->checkBox_rememberPasswd->isChecked())
        ui->lineEdit_password->setText("");

    ui->label_throbber->hide();
    m_xmppClient.disconnectFromServer();
    showSignInPage();
    showLoginStatus(tr("登录已经取消"));
    addAccountToCache();
}

void mainDialog::showSignInPage()
{
    ui->label_throbber->hide();
    ui->pushButton_signIn->setDisabled(false);
    ui->pushButton_cancel->setDisabled(true);
    ui->lineEdit_userName->setDisabled(false);
    ui->lineEdit_password->setDisabled(false);
    ui->checkBox_rememberPasswd->setDisabled(false);
    ui->stackedWidget->setCurrentIndex(1);
}

void mainDialog::showSignInPageAfterUserDisconnection()
{
    if(!ui->checkBox_rememberPasswd->isChecked())
        ui->lineEdit_password->setText("");

    QMap<QString,DebugDialog*>::iterator m_iterator;
    for(m_iterator = m_debugDlgList.begin();m_iterator !=  m_debugDlgList.end();++m_iterator)
    {
       delete m_iterator.value();
    }
    m_debugDlgList.clear();
    ui->label_throbber->hide();

    showLoginStatus(tr("未连接"));
    showSignInPage();

    onlineMap.clear();
}

void mainDialog::showRosterPage()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void mainDialog::startConnection()
{
//    m_xmppClient.setClientPresence(QXmppPresence());
    m_xmppClient.connectToServer(m_xmppClient.configuration());
}

void mainDialog::showLoginStatus(const QString& msg)
{
    ui->label_status->setCustomText(msg, signInStatusLabel::None);
}

void mainDialog::showLoginStatusWithProgress(const QString& msg)
{
    ui->label_status->setCustomText(msg, signInStatusLabel::WithProgressEllipsis);
}

void mainDialog::showLoginStatusWithCounter(const QString& msg, int time)
{
    ui->label_status->setCustomText(msg, signInStatusLabel::CountDown, time);
}

void mainDialog::updateVCard(const QString& bareJid)
{
    // determine full name
    const QXmppVCardIq vCard = m_vCardCache.getVCard(bareJid);
    QString fullName = vCard.fullName();
    if (fullName.isEmpty())
        fullName = bareJid;

    // determine avatar
    QImage avatar = m_vCardCache.getAvatar(bareJid);
    if (avatar.isNull())
        avatar = QImage(":/icons/resource/avatar.png");

    if (bareJid == m_xmppClient.configuration().jidBare()) {
        // update our own information
        m_statusWidget.setAvatar(avatar);
        m_statusWidget.setDisplayName(fullName);
    } else {
        // update roster information
        m_rosterItemModel.updateAvatar(bareJid, avatar);
        m_rosterItemModel.updateName(bareJid, fullName);
    }
}

void mainDialog::showProfile(const QString& bareJid)
{
    if(bareJid.isEmpty())
        return;

    profileDialog dlg(this, bareJid, m_xmppClient, m_capabilitiesCache);
    dlg.setBareJid(bareJid);
    // TODO use original image
    if(!m_vCardCache.getAvatar(bareJid).isNull())
        dlg.setAvatar(m_vCardCache.getAvatar(bareJid));
    QStringList resources = m_xmppClient.rosterManager().getResources(bareJid);

    dlg.setFullName(m_vCardCache.getVCard(bareJid).fullName());

    if(m_vCardCache.getVCard(bareJid).fullName().isEmpty())
        dlg.setFullName(m_xmppClient.rosterManager().getRosterEntry(bareJid).name());

    dlg.exec();
}

void mainDialog::loadAccounts()
{
    m_accountsCache.loadFromFile();
    QStringList list = m_accountsCache.getBareJids();
    QCompleter *completer = new QCompleter(list, this);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->lineEdit_userName->setCompleter(completer);

    if(!list.isEmpty())
    {
        ui->lineEdit_userName->setText(list.last());
        QString passwd = m_accountsCache.getPassword(list.last());
        QString host = m_accountsCache.getHost(list.last());
        QString port = m_accountsCache.getPort(list.last());

        ui->lineEdit_password->setText(passwd);
        ui->lineEdit_host->setText(host);
        ui->lineEdit_port->setText(port);

        if(!passwd.isEmpty())
            ui->checkBox_rememberPasswd->setChecked(true);
    }else{
        //如果第一次登陆，则写入一个缺省信息
        ui->lineEdit_host->setText("easy-iot.cc");
        ui->lineEdit_port->setText("5222");
    }
}

void mainDialog::userNameCompleter_activated(const QString& user)
{
    QString passwd = m_accountsCache.getPassword(user);
    ui->lineEdit_password->setText(passwd);
    if(!passwd.isEmpty())
        ui->checkBox_rememberPasswd->setChecked(true);
}

void mainDialog::addAccountToCache()
{
    QString bareJid = ui->lineEdit_userName->text();
    QString passwd = ui->lineEdit_password->text();
    if(!ui->checkBox_rememberPasswd->isChecked())
        passwd = "";
    m_accountsCache.addAccount(bareJid, passwd,ui->lineEdit_host->text(),ui->lineEdit_port->text());
}

void mainDialog::action_signOut()
{
    m_xmppClient.disconnectFromServer();

    // update widget
    m_statusWidget.setStatusText(
            presenceToStatusText(m_xmppClient.clientPresence()));
}

void mainDialog::action_quit()
{
    m_xmppClient.disconnectFromServer();
    QApplication::quit();
}

void mainDialog::createTrayIconAndMenu()
{
    bool check;
    Q_UNUSED(check);

    check = connect(&m_quitAction, SIGNAL(triggered()), SLOT(action_quit()));
    Q_ASSERT(check);

    check = connect(&m_signOutAction, SIGNAL(triggered()), SLOT(action_signOut()));
    Q_ASSERT(check);


#ifndef QT_NO_SYSTEMTRAYICON
    m_trayIcon.setIcon(QIcon(":/icons/resource/icon.png"));

    check = connect(&m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                    SLOT(action_trayIconActivated(QSystemTrayIcon::ActivationReason)));
    Q_ASSERT(check);

    m_trayIconMenu.addAction(&m_signOutAction);
    m_trayIconMenu.addSeparator();
    m_trayIconMenu.addAction(&m_quitAction);

    m_trayIcon.setContextMenu(&m_trayIconMenu);
    m_trayIcon.show();
#endif
}

void mainDialog::createSettingsMenu()
{
    m_settingsMenu = new QMenu(ui->pushButton_settings);
//    ui->pushButton_settings->setMenu(m_settingsMenu);

    QAction* aboutDlg = new QAction(tr("关于EasyIOT"), ui->pushButton_settings);
    connect(aboutDlg, SIGNAL(triggered()), SLOT(action_aboutDlg()));
    m_settingsMenu->addAction(aboutDlg);

    m_settingsMenu->addSeparator();

    QAction* showXml = new QAction(tr("显示 XML 控制台..."), ui->pushButton_settings);
    connect(showXml, SIGNAL(triggered()), SLOT(action_showXml()));
    m_settingsMenu->addAction(showXml);

    QAction* tcpServer = new QAction(tr("TcpServer 设置"),ui->pushButton_settings);
    connect(tcpServer,SIGNAL(triggered()),SLOT(action_tcpServerSet()));
    m_settingsMenu->addAction(tcpServer);

    QAction* configAction = new QAction(tr("配置"),ui->pushButton_settings);
    connect(configAction,SIGNAL(triggered(bool)),SLOT(action_configParam()));
    m_settingsMenu->addAction(configAction);

    QMenu* viewMenu = new QMenu(tr("视图"), ui->pushButton_settings);
    m_settingsMenu->addMenu(viewMenu);


    QAction* showOfflineContacts = new QAction(tr("显示离线设备"), ui->pushButton_settings);
    showOfflineContacts->setCheckable(true);
    showOfflineContacts->setChecked(true);
    connect(showOfflineContacts, SIGNAL(triggered(bool)),
            &m_rosterItemSortFilterModel, SLOT(setShowOfflineContacts(bool)));
    viewMenu->addAction(showOfflineContacts);

    QAction* sortByName = new QAction(tr("按名称排序"), ui->pushButton_settings);
    sortByName->setCheckable(true);
    sortByName->setChecked(false);
    connect(sortByName, SIGNAL(triggered(bool)),
            &m_rosterItemSortFilterModel, SLOT(sortByName(bool)));
    viewMenu->addAction(sortByName);

    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(&m_quitAction);
}

void mainDialog::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void mainDialog::action_trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        show();
        break;
    default:
        ;
    }
}

void mainDialog::action_addContact()
{
    bool ok;
    QString bareJid = QInputDialog::getText(this,tr("添加一个设备"),
                                            tr("设备 ID:"), QLineEdit::Normal, "", &ok);
    if(!bareJid.contains("@easy-iot.cc"))
        bareJid += "@easyio-iot.cc";

    if(!ok)
        return;

    if(!isValidBareJid(bareJid))
    {
        QMessageBox::information(this, tr("无效 ID"), tr("指定的 ID <I>") + bareJid + tr(" </I> 是无效的"));
        return;
    }

    if(ok && !bareJid.isEmpty())
    {
        QXmppPresence subscribe;
        subscribe.setTo(bareJid);
        subscribe.setType(QXmppPresence::Subscribe);
        m_xmppClient.sendPacket(subscribe);
    }
}

void mainDialog::presenceReceived(const QXmppPresence& presence)
{
    QString from = presence.from();

    QString bareJid = QXmppUtils::jidToBareJid(from);
    //if(getDebugDialog(bareJid))
        //getDebugDialog(bareJid)->presenceReceived(presence);

    QString message;
    switch(presence.type())
    {
    case QXmppPresence::Subscribe:
        {
            message = tr("<B>%1</B> 请求订阅");

            int retButton = QMessageBox::question(
                    this, tr("设备订阅申请"), message.arg(from),
                    QMessageBox::Yes, QMessageBox::No);

            switch(retButton)
            {
            case QMessageBox::Yes:
                {
                    //先同意对方的订阅请求
                    QXmppPresence subscribed;
                    subscribed.setTo(from);
                    subscribed.setType(QXmppPresence::Subscribed);
                    m_xmppClient.sendPacket(subscribed);

                    //在请求订阅对方
                    // reciprocal subscription
                    QXmppPresence subscribe;
                    subscribe.setTo(from);
                    subscribe.setType(QXmppPresence::Subscribe);
                    m_xmppClient.sendPacket(subscribe);
                }
                break;
            case QMessageBox::No:
                {
                    //拒绝对方的订阅请求也不订阅对方
                    QXmppPresence unsubscribed;
                    unsubscribed.setTo(from);
                    unsubscribed.setType(QXmppPresence::Unsubscribed);
                    m_xmppClient.sendPacket(unsubscribed);
                }
                break;
            default:
                break;
            }

            return;
        }
        break;
    case QXmppPresence::Subscribed:
        message = tr("<B>%1</B> 通过了你的申请");
        break;
    case QXmppPresence::Unsubscribe:
        message = tr("<B>%1</B> 取消订阅");
        break;
    case QXmppPresence::Unsubscribed:
        message = tr("<B>%1</B> 拒绝订阅");
        break;
    default:
        return;
        break;
    }

    if(message.isEmpty())
        return;

    QMessageBox::information(this, tr("设备订阅"), message.arg(from),
            QMessageBox::Ok);
}

void mainDialog::action_removeContact(const QString& bareJid)
{
    if(!isValidBareJid(bareJid))
        return;

    int answer = QMessageBox::question(this,tr("移除设备"),
                            QString("确定要移除设备 <I>%1</I> 吗？").arg(bareJid),
                            QMessageBox::Yes, QMessageBox::No);
    if(answer == QMessageBox::Yes)
    {
        QXmppRosterIq remove;
        remove.setType(QXmppIq::Set);
        QXmppRosterIq::Item itemRemove;
        itemRemove.setSubscriptionType(QXmppRosterIq::Item::Remove);
        itemRemove.setBareJid(bareJid);
        remove.addItem(itemRemove);
        m_xmppClient.sendPacket(remove);

        m_rosterItemModel.removeRosterEntry(bareJid);
    }
}

void mainDialog::errorClient(QXmppClient::Error error)
{
    ui->label_throbber->hide();

    showSignInPage();

    switch(error)
    {
    case QXmppClient::SocketError:
        showLoginStatus(tr("Socket error"));
        break;
    case QXmppClient::KeepAliveError:
        showLoginStatus(tr("Keep alive error"));
        break;
    case QXmppClient::XmppStreamError:
        switch(m_xmppClient.xmppStreamError())
        {
        case QXmppStanza::Error::NotAuthorized:
            showLoginStatus(tr("Invalid password"));
            break;
        default:
            showLoginStatus(tr("Stream error"));
            break;
        }
        break;
    default:
        break;
    }
}

void mainDialog::loadUserConfig()
{
    m_tcpServer.setTcpPort(IniConfig::getTcpServerPort(m_xmppClient.configuration().jidBare()));
    SqlHelper::initMySqlDataBase(m_xmppClient.configuration().jidBare());
}

void mainDialog::addAdvancedSettingsLabel()
{
    m_lab_advancedSetting = new Label(this);
    m_lab_advancedSetting->setText(tr("高级设置"));
    m_lab_advancedSetting->setStyleSheet("color: blue;text-decoration: underline;");
    ui->horizontalLayout_5->insertWidget(0,m_lab_advancedSetting);
    openAdvancedSetting = false;
    hideAdvancedSetting();
    m_lab_advancedSetting->setCursor(Qt::PointingHandCursor);
}

void mainDialog::pressAdvancedSettingLabel()
{
    openAdvancedSetting = !openAdvancedSetting;
    if(openAdvancedSetting)
    {
        showAdvanceSettings();
    }
    else
    {
        hideAdvancedSetting();
    }
}

void mainDialog::hideAdvancedSetting()
{
    ui->label_4->hide();
    ui->label_5->hide();
    ui->lineEdit_host->hide();
    ui->lineEdit_port->hide();
    //this->pos();
    this->resize(203,415);
    //QRect r = this->geometry();
    //r.setHeight(403);
    //this->setGeometry(r);
}

void mainDialog::showAdvanceSettings()
{
    ui->label_4->show();
    ui->label_5->show();
    ui->lineEdit_host->show();
    ui->lineEdit_port->show();
    this->resize(203,503);
    //QRect r = this->geometry();
    //r.setHeight(503);
    //this->setGeometry(r);
}

void mainDialog::setTcpServerPort(quint16 port)
{
    m_tcpServer.setTcpPort(port);
    IniConfig::setTcpServerPort(port,m_xmppClient.configuration().jidBare());
}

bool mainDialog::startTcpServer()
{
    bool ret =  m_tcpServer.startAccept();
    if(ret)
        setWindowTitle(tr("EasyIOT-%1.%2-TCP监听中").arg(MAJOR_VERSION).arg(MINOR_VERSION));
    return ret;
}

void mainDialog::stopTcpServer()
{
    m_tcpServer.stopAccept();
    setWindowTitle(tr("EasyIOT-%1.%2").arg(MAJOR_VERSION).arg(MINOR_VERSION));
}

void mainDialog::action_showXml()
{
    m_consoleDlg.show();
}

void mainDialog::addPhotoHash(QXmppPresence& pre)
{
    QString clientBareJid = m_xmppClient.configuration().jidBare();

    if(m_vCardCache.isVCardAvailable(clientBareJid))
    {
        QByteArray hash = m_vCardCache.getPhotoHash(clientBareJid);
        if(hash.isEmpty())
            pre.setVCardUpdateType(QXmppPresence::VCardUpdateNoPhoto);
        else
            pre.setVCardUpdateType(QXmppPresence::VCardUpdateValidPhoto);
        pre.setPhotoHash(hash);
    }
    else
    {
        pre.setVCardUpdateType(QXmppPresence::VCardUpdateNone);
        pre.setPhotoHash(QByteArray());
    }
}

void mainDialog::action_aboutDlg()
{
    aboutDialog abtDlg(this);
    abtDlg.exec();
}

void mainDialog::action_settingsPressed()
{
    m_settingsMenu->exec(ui->pushButton_settings->mapToGlobal(QPoint(0, ui->pushButton_settings->height())));
}

void mainDialog::action_tcpServerSet()
{
    m_tcpSetDlg.setShowTcpServerPort(m_tcpServer.tcpPort());
    m_tcpSetDlg.setTcpServerStatus(m_tcpServer.isAccept());
    m_tcpSetDlg.show();

}

void mainDialog::action_configParam()
{
    m_configDlg.initConfigUIText();
    m_configDlg.show();
}

void mainDialog::updateSignal(const QString &bareJid, int signal , int version)
{
    char strbuf[32];
    snprintf(strbuf,sizeof(strbuf),"Sig:%d Ver:%d",signal,version);
    m_rosterItemModel.updatePresence(bareJid, QString(strbuf));
}
