/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2015 tcet Authors
 *
 * Authors: Adrian
 *          Paul David Callahan
 *          Dolphin Oracle
 *          tcet Linux <http://tcetlinux.org>
 *
 * This file is part of tcet-welcome.
 *
 * tcet-welcome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tcet-welcome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tcet-welcome.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "tcetwelcome.h"
#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QTranslator>
#include <unistd.h>

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("tcet");
    QCoreApplication::setApplicationName("tcet-welcome");
#if QT_VERSION >= 0x050600
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon("/usr/share/pixmaps/tcet-welcome.png"));

    QTranslator qtTran;
    qtTran.load(QString("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QString("tcet-welcome_") + QLocale::system().name(), "/usr/share/tcet-welcome/locale");
    a.installTranslator(&appTran);

    if (getuid() != 0) {
        tcetwelcome w;
        w.show();
        return a.exec();
    } else {
        QApplication::beep();
        QMessageBox::critical(0, QString::null,
            QApplication::tr("You must run this program as normal user."));
        return 1;
    }
}
