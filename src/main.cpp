/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : gio mar 14 18:37:43 CET 2002
    copyright            : (C) 2002 by Andrea Rizzi
    email                : rizzi@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <vector>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include "kssh.h"

static const char *description = "KDE SSH - A KDE front end for ssh";


int main(int argc, char *argv[])
{
    KCmdLineOptions options;
    options.add("+[user@host]", ki18n("Connect to \"host\" as \"user\""),"0");
    options.add("die", ki18n("Use this for konsole sessions (ignore --keepalive)"), "0");
    options.add("keepalive", ki18n("Do not close the dialog after \"Connect\""), "0");

    KAboutData aboutData(
        "kssh",
        "kssh",
        ki18n("KDE Secure Shell"),
        "1.0",
        ki18n(description),
        KAboutData::License_GPL,
        ki18n("(c) 2000-2002, Andrea Rizzi"),
        ki18n(""),
        "www.kde.org",
        "rizzi@kde.org"
    );
    aboutData.addAuthor(ki18n("Andrea Rizzi"),ki18n("coding"), "rizzi@kde.org");
    aboutData.addAuthor(ki18n("Stefan Kögl"), ki18n("porting to kde4"), "hotshelf@free.de");
    aboutData.addCredit(ki18n("OpenSSH"),ki18n("Documentation of ssh functions is taken\nfrom the OpenSSH man page"), "", "man:/ssh");
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication a;
    KSSH *kssh = new KSSH();
    a.setTopWidget(kssh);
    kssh->show();
    int ret=a.exec();

    if(ret==1)  //Go ssh...
    {
        QString uah = kssh->userathost();
        std::vector<char *> vec;
        int n;

        QStringList para=kssh->parameters();

        vec.resize(n=(para.count()+3));
        vec[0] = QByteArray("ssh").data();
        vec[1] = uah.toLocal8Bit().data();
        for(int i=2;i<n-1;i++) {
            vec[i] = para[i-2].toLocal8Bit().data();
        }
        vec[n-1] = 0;

        return execvp("/usr/bin/ssh", vec.data());
    }
    return 0;
}
