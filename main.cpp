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


#include <QApplication>
#include <QDir>

#include "mainDialog.h"
#include "utils.h"
#include "global.h"
#include "QString"
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QApplication::setOrganizationName(QString("EasyIOT-%1.%2").arg(MAJOR_VERSION).arg(MINOR_VERSION));
    QApplication::setApplicationName("EasyIOT");
    //QApplication::setWindowIcon()

    QDir dir;
    if(!dir.exists(getSettingsDir()))
        dir.mkpath(getSettingsDir());

    mainDialog cw;
    cw.show();
    cw.raise();
    return a.exec();
}
