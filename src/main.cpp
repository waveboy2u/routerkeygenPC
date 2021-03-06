/*
 * Copyright 2013 Rui Araújo, Luís Fonseca
 *
 * This file is part of Router Keygen.
 *
 * Router Keygen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Router Keygen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Router Keygen.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QVector>
#include <iostream>
#include "RouterKeygen.h"
#include "wifi/QScanResult.h"
#include "WirelessMatcher.h"
#include "algorithms/Keygen.h"
#include "qcmdlineparser/qcmdlineparser.h"
#include "version.h"

int main(int argc, char * argv[]) {
    if (argc > 1) {
        QCoreApplication app(argc, argv);
        QCmdLineParser parser;
        parser.addOption("-s", QCmdLineArgument::StoreValue, QObject::tr("SSID"), "--ssid", "network_name");
        parser.addOption("-m", QCmdLineArgument::StoreValue, QObject::tr("MAC address"), "--mac", "mac_address");
        parser.addOption("-v", QCmdLineArgument::StoreTrue, QObject::tr("Version"), "--version");
        parser.addOption("-k", QCmdLineArgument::StoreTrue, QObject::tr("Print keygen in form kgname:candidate"), "--kg");
        parser.addOption("-q", QCmdLineArgument::StoreTrue, QObject::tr("Print only calculated keys"), "--quiet");
        QString error = "";
        QVariantMap options = parser.parse(QCoreApplication::arguments(), &error);
        if ( !error.isEmpty() ) {
            std::cout << error.toUtf8().data() << std::endl <<  parser.help().toUtf8().data();
            return -1;
        }
        if ( options.value("v", false).toBool() ) {
            std::cout << QObject::tr("Version: %1").arg(PROJECT_VERSION).toUtf8().data() << std::endl;
        }
        if ( options.contains("s") || options.contains("m") ) {
            WirelessMatcher m;
            QString mac = options.value("m", "").toString().toUpper();
            if ( mac.length()>0 && mac.count(QRegExp("^([0-9A-F]{2}[:-]){5}([0-9A-F]{2})$")) == 0 ) {
                mac = "";
                if ( !options.value("q", false).toBool() )
                    std::cout << QObject::tr("Invalid MAC. It will not be used.").toUtf8().data() << std::endl;
            }
            QString ssid = options.value("s", "").toString();
            QScanResult wifi(ssid, mac);
            wifi.checkSupport(m);
            QVector<Keygen *> * keygens = wifi.getKeygens();
            if (keygens == NULL ) {
                if ( !options.value("q", false).toBool() )
                    std::cout << QObject::tr("Out of memory.").toUtf8().data() << std::endl;
                return -100;
            }
            if ( keygens->size() == 0) {
                if ( !options.value("q", false).toBool() )
                    std::cout << QObject::tr("Unsupported network. Check the MAC address and the SSID.").toUtf8().data() << std::endl;
                return -2;
            }
            if ( !options.value("q", false).toBool() )
                std::cout << QObject::tr("Calculating keys. This can take a while.").toUtf8().data() << std::endl;
            QVector<QString> results;
            bool kg = options.value("k", false).toBool();
            for ( int i = 0; i < keygens->size(); ++i ){
                try {
                    QVector<QString> r = keygens->at(i)->getResults();
                    if (kg) {
                        foreach (QString s, r) {
                             results.append(keygens->at(i)->kgname + ":" + s);
                         }
                    } else {
                        results +=r;
                    }
                } catch (int e) {
                    if ( !options.value("q", false).toBool() )
                        std::cout << QObject::tr("Errors while calculating.").toUtf8().data() << std::endl;
                    continue;
                }
            }
            if (results.isEmpty()) {
                if ( !options.value("q", false).toBool() )
                    std::cout << QObject::tr("No keys were calculated.").toUtf8().data() << std::endl;
                return -1;
            }
            if ( !options.value("q", false).toBool() )
                std::cout << QObject::tr("Calculated Passwords for %1").arg(wifi.getSsidName()).toUtf8().data() << std::endl;
            for (int i = 0; i < results.size(); ++i)
                std::cout <<  results.at(i).toLatin1().data() << std::endl;
        }
        return 0;
    } else {
        /* Run main application */
        QApplication app(argc, argv);
        QTranslator translator;
        QApplication::setOrganizationName("com.doublecheck");
        QApplication::setApplicationName(PROJECT_NAME);
        QApplication::setApplicationVersion(PROJECT_VERSION);
        QString qmFile = app.applicationName().toLower() + "_" + QLocale::system().name();
        if ( translator.load(qmFile,":/lang") )
            app.installTranslator(&translator);

        RouterKeygen w;
        w.showWithDialog();
        return app.exec();
    }
}
