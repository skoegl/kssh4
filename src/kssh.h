/***************************************************************************
                          kssh.h  -  description
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

#ifndef KSSH_H
#define KSSH_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kconfig.h>
#include <qwidget.h>
#include "ui_ksshdialog.h"

class KCompletion;

/** KSSH is the base class of the project */
class KSSH : public QDialog, private Ui::kssh
{
    Q_OBJECT 
public:
    KSSH(QWidget* parent=0);
    virtual ~KSSH() {};

    QString userathost();
    QString cmd();
    QStringList parameters();

private:
    bool opt;
    bool mopt;
    bool m_user_editor;
    bool m_host_editor;
    KApplication *app;
    KCompletion *compUser;
    KCompletion *compHost;

    void saveLists();
    void loadHosts();
    void saveOptions(QString group);
    bool loadOptions(QString group);
private slots:
    void userFor(const QString&);
    void options();
    void saveAsDefault();
    void moreOptions();
    void ssh();
    void about();
    void userEditor();
    void hostEditor();
    void okEditor();
    void cancelEditor();
    void exitHandler();
};

#endif
