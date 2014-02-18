/***************************************************************************
                          kssh.cpp  -  description
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

#include "kssh.h"

#include <kaboutapplicationdialog.h>
#include <kcmdlineargs.h>
#include <krun.h>
#include <QDesktopWidget>
#include <QStringBuilder>

KSSH::KSSH(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QLayout *lay;
    lay = layout();
    if (lay)
        lay->setSizeConstraint(QLayout::SetFixedSize);

    opt = false;
    mopt = false;

    editorF->hide();

    QSize s = size();

    QPoint p(s.width(), s.height());
    QPoint po = pos();
    QDesktopWidget *d = QApplication::desktop();
    int w = d->width();                   // returns desktop width
    int h = d->height ();                  // returns desktop height
    int x = 0, y = 0;
    if ((p + po).x()>w)
        po.setX(x = w-p.x());
    if ((p + po).y()>h)
        po.setY(y = h-p.y());

    if (x<0)
        po.setX(0);
    if (y<0)
        po.setY(0);

    move(po);
    optionsGB->hide();
    moreF->hide();

    adjustSize();

    compUser = new KCompletion();
    userCB->setCompletionObject(compUser);

    compHost = new KCompletion();

    hostCB->setCompletionObject(compHost);

    hostCB->setFocus();

    connect(hostCB, SIGNAL(editTextChanged(const QString&)), this, SLOT(userFor(const QString&)));

    connect(compHost, SIGNAL(match(const QString&)), this, SLOT(userFor(const QString&)));

    userCB->insertItem(1, "");
    hostCB->insertItem(2, "");

    loadHosts();

    loadOptions("DefaultConfig");

    connect(aboutPB, SIGNAL(clicked()), this, SLOT(about()));
    connect(optionsPB, SIGNAL(clicked()), this, SLOT(options()));
    connect(morePB, SIGNAL(clicked()), this, SLOT(moreOptions()));

    connect(hostTB, SIGNAL(clicked()), this, SLOT(hostEditor()));
    connect(userTB, SIGNAL(clicked()), this, SLOT(userEditor()));
    connect(cancelPB, SIGNAL(clicked()), this, SLOT(cancelEditor()));
    connect(okPB, SIGNAL(clicked()), this, SLOT(okEditor()));

    connect(connectPB, SIGNAL(clicked()), this, SLOT(ssh()));
    connect(savePB, SIGNAL(clicked()), this, SLOT(saveAsDefault()));
    connect(quitPB, SIGNAL(clicked()), this, SLOT(exitHandler()));

    KConfigGroup general_group = KGlobal::config()->group("General");

    int fi = hostCB->findText(general_group.readEntry("LastHost"));
    if (fi)
        hostCB->setCurrentIndex(fi);

    int def = KGlobalSettings::completionMode();
    int mode = general_group.readEntry("HostCompletionMode", def);
    hostCB->setCompletionMode((KGlobalSettings::Completion) mode);

    mode = general_group.readEntry("UserCompletionMode", def);
    userCB->setCompletionMode((KGlobalSettings::Completion)mode);
    setWindowIcon(KIcon("kssh.png"));
}


QString KSSH::userathost()
{
    return QString(userCB->currentText() % QChar('@') % hostCB->currentText());
}

QString KSSH::cmd()
{
    int n;
    QString ret;
    QStringList para = parameters();
    n = para.count();
    ret = "ssh " % userCB->currentText() % QChar('@') % hostCB->currentText() % QChar(' ');
    for(int i = 0;i<n; i++)
      ret += para[i] % QChar(' ');
    ret = ret % (";");
    return ret;
}

void KSSH::options()
{
    opt = !opt;
    if (opt) {
        optionsPB->setText(i18n("Hide options"));
        optionsGB->show();
    }
    else {
        optionsPB->setText(i18n("Show options"));
        optionsGB->hide();
    }
}

void KSSH::moreOptions()
{
    mopt = !mopt;
    if (mopt)
    {
        morePB->setText(i18n("Less..."));
        moreF->show();
    }
    else
    {
        morePB->setText(i18n("More.."));
        moreF->hide();
    }
 }


void KSSH::about()
{
     KAboutApplicationDialog* aa = new KAboutApplicationDialog(KCmdLineArgs::aboutData(), this);
     aa->show();
}

void KSSH::ssh()
{
    KConfig config;
    KConfigGroup conf_group = config.group("General");
    conf_group.writeEntry("LastHost", hostCB->currentText());
    conf_group.writeEntry("HostCompletionMode", (int) compHost->completionMode());
    conf_group.writeEntry("UserCompletionMode", (int) compUser->completionMode());

    config.sync();

    compUser->addItem(userCB->currentText());
    compHost->addItem(hostCB->currentText());

    if (saveCB->isChecked())
        saveOptions(hostCB->currentText() % QString("-Options"));

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("die")) {
        qApp->exit(1);
    }
    else {
        QString caption = "KSSH: %1" ;

        QString terminal = conf_group.readEntry("TerminalApplication", "konsole");

        QString ex(terminal % QString(" -e ") % cmd());
        KRun::runCommand(ex, this);
        if (!args->isSet("keepalive"))
            qApp->quit();
    }
}


void KSSH::loadHosts()
{
    KConfig config;
    KConfigGroup g = config.group("Host List");

    QStringList data = g.readEntry("Host", QStringList());
    compHost->setItems(data);
    hostCB->insertItems(hostCB->count(), data);
}

void KSSH::saveAsDefault()
{
    KConfig config;
    KConfigGroup def_config = config.group("DefaultConfig");
    def_config.writeEntry("-X", XCB->isChecked());
    def_config.writeEntry("-x", xCB->isChecked());
    def_config.writeEntry("-p", pCB->isChecked());
    def_config.writeEntry("-L", LCB->isChecked());
    def_config.writeEntry("-R", RCB->isChecked());
    def_config.writeEntry("-P", PCB->isChecked());
    def_config.writeEntry("-1", ssh1CB->isChecked());
    def_config.writeEntry("-2", ssh2CB->isChecked());
    def_config.writeEntry("-a", aCB->isChecked());
    def_config.writeEntry("-A", ACB->isChecked());
    def_config.writeEntry("-c", cCB->isChecked());
    def_config.writeEntry("-C", CCB->isChecked());
    def_config.writeEntry("-F", FCB->isChecked());
    def_config.writeEntry("-4", IPV4CB->isChecked());
    def_config.writeEntry("-6", IPV6CB->isChecked());
    def_config.writeEntry("-b", bCB->isChecked());
    def_config.writeEntry("-c1", c1CB->isChecked());
    def_config.writeEntry("-e", eCB->isChecked());
    def_config.writeEntry("-f", fCB->isChecked());
    def_config.writeEntry("-g", gCB->isChecked());
    def_config.writeEntry("-i", iCB->isChecked());
    def_config.writeEntry("-I", ICB->isChecked());
    def_config.writeEntry("-k", kCB->isChecked());
    def_config.writeEntry("-m", mCB->isChecked());
    def_config.writeEntry("-n", nCB->isChecked());
    def_config.writeEntry("-N", NCB->isChecked());
    def_config.writeEntry("-q", qCB->isChecked());
    def_config.writeEntry("-v", vCB->isChecked());
    def_config.writeEntry("-s", sCB->isChecked());
    def_config.writeEntry("-T", TCB->isChecked());
    def_config.writeEntry("-t", tCB->isChecked());
    def_config.writeEntry("-o", oCB->isChecked());

    def_config.writeEntry("Port", portSB->value());
    def_config.writeEntry("VerboseLevel", vSB->value());
    def_config.writeEntry("Cipher", ccCB->currentText());

    def_config.writeEntry("LLE", LLE->text());
    def_config.writeEntry("FLE", FLE->lineEdit()->text());
    def_config.writeEntry("RLE", RLE->text());
    def_config.writeEntry("cLE", cLE->text());

    def_config.writeEntry("eLE", eLE->text());
    def_config.writeEntry("bLE", bLE->text());
    def_config.writeEntry("iLE", iLE->lineEdit()->text());

    def_config.writeEntry("ILE1", ILE->lineEdit()->text());
    def_config.writeEntry("mLE", mLE->text());
    def_config.writeEntry("oLE", oLE->text());
    def_config.writeEntry("Command", commandLE->text());
}

QStringList KSSH::parameters()
{
    QStringList ret;
    int count = 0;

    if (XCB->isChecked())
    {
        ret.append("-X");
        count ++;
    }

    if (xCB->isChecked())
    {
        ret.append("-x");
        count ++;
    }
    if (pCB->isChecked())
    {
        ret.append("-p");
        ret.append(QString::number(portSB->value()));
        count += 2;
    }


    if (LCB->isChecked())
    {
        ret.append("-L");
        ret.append(LLE->text());
        count += 2;
    }
    if (RCB->isChecked())
    {
        ret.append("-R");
        ret.append(RLE->text());
        count += 2;
    }
    if (PCB->isChecked())
    {
        ret.append("-P");
        count ++;
    }
    if (ssh1CB->isChecked())
    {
        ret.append("-1");
        count ++;
    }
    if (ssh2CB->isChecked())
    {
        ret.append("-2");
        count ++;
    }
    if (aCB->isChecked())
    {
        ret.append("-a");
        count ++;
    }
    if (ACB->isChecked())
    {
        ret.append("-A");
        count ++;
    }
    if (cCB->isChecked())
    {
        ret.append("-c");
        ret.append(ccCB->currentText());
        count += 2;
    }
    if (CCB->isChecked())
        {
        ret.append("-C");
        count ++;
    }
    if (FCB->isChecked())
        {
        ret.append("-F");
        ret.append(FLE->lineEdit()->text());
        count += 2;
    }
    if (IPV4CB->isChecked())
    {
        ret.append("-4");
        count ++;
    }
    if (IPV6CB->isChecked())
    {
        ret.append("-6");
        count ++;
    }
    if (bCB->isChecked())
        {
        ret.append("-b");
        ret.append(bLE->text());
        count += 2;
    }
    if (c1CB->isChecked())
    {
        ret.append("-c");
        ret.append(cLE->text());
        count += 2;
    }
    if (eCB->isChecked())
        {
        ret.append("-e");
        ret.append(eLE->text());
        count += 2;
    }
    if (fCB->isChecked())
    {
        ret.append("-f");
        count ++;
    }
    if (gCB->isChecked())
    {
        ret.append("-g");
        count ++;
    }
    if (iCB->isChecked())
    {
        ret.append("-i");
        ret.append(iLE->lineEdit()->text());
        count += 2;
    }
    if (ICB->isChecked())
    {
        ret.append("-I");
        ret.append(ILE->lineEdit()->text());
        count += 2;
    }
    if (kCB->isChecked())
    {
        ret.append("-k");
        count ++;
    }
    if (mCB->isChecked())
    {
        ret.append("-m");
        ret.append(mLE->text());
        count += 2;
    }
    if (nCB->isChecked())
        {
        ret.append("-n");
        count ++;
    }
    if (NCB->isChecked())
        {
        ret.append("-N");
        count ++;
    }
    if (qCB->isChecked())
        {
        ret.append("-q");
        count ++;
    }
    if (vCB->isChecked())
    {
        QString v = "-";
        for(int i = 0;i<vSB->value();i ++)
            v+= "v";
        ret.append(v);
        count ++;
    }
    if (sCB->isChecked())
        {
        ret.append("-s");
        count ++;
    }
    if (TCB->isChecked())
    {
        ret.append("-T");
        count ++;
    }
    if (tCB->isChecked())
    {
        ret.append("-t");
        count ++;
    }
    if (oCB->isChecked())
    {
        ret.append("-o");
        ret.append(oLE->text());
        count += 2;
    }

    QString com = commandLE->text();
    if (!com.isEmpty()) ret.append(com);

    return ret;
}


void KSSH::saveLists()
{
    KConfig config;
    KConfigGroup host_list = config.group("Host List");
    host_list.writeEntry("Host", compHost->items());
    KConfigGroup group_host_list = config.group(hostCB->currentText() % QString("-User List"));
    group_host_list.writeEntry("User", compUser->items());
    config.sync();
}

void KSSH::exitHandler()
{
    KConfig config;
    KConfigGroup def_config = config.group("DefaultConfig");
//     def_config.writeEntry("Size", size());
//     def_config.writeEntry("Position", pos());
    qApp->quit();
}

void KSSH::saveOptions(QString group)
{
    bool setFlag;

    saveLists();
    KConfig config;
    KConfigGroup host_list = config.group(hostCB->currentText() % QString("-User List"));
    KConfigGroup opt_config = config.group(group);
    KConfigGroup def_config = config.group("DefaultConfig");

    host_list.writeEntry("LastUsed", userCB->currentText());

    setFlag = def_config.readEntry("-X", false);
    if (XCB->isChecked() != setFlag)
        opt_config.writeEntry("-X", XCB->isChecked());
    else
        opt_config.deleteEntry("-X");

    setFlag = def_config.readEntry("-x", false);
    if (xCB->isChecked() != setFlag)
        opt_config.writeEntry("-x", xCB->isChecked());
    else
        opt_config.deleteEntry("-x");

    setFlag = def_config.readEntry("-p", false);
    if (pCB->isChecked() != setFlag)
        opt_config.writeEntry("-p", pCB->isChecked());
    else
        opt_config.deleteEntry("-p");

    int pp = setFlag = def_config.readEntry("Port", 22);
    if (portSB->value() != pp)
        opt_config.writeEntry("Port", portSB->value());
    else
        opt_config.deleteEntry("Port");

    setFlag = def_config.readEntry("-L", false);
    if (LCB->isChecked() != setFlag)
        opt_config.writeEntry("-L", LCB->isChecked());
    else
        opt_config.deleteEntry("-L");

    QString  Lstr = def_config.readEntry("LLE");
    if (LLE->text() != Lstr)
        opt_config.writeEntry("LLE", LLE->text());
    else
        opt_config.deleteEntry("LLE");

    setFlag = def_config.readEntry("-R", false);
    if (RCB->isChecked() != setFlag)
        opt_config.writeEntry("-R", RCB->isChecked());
    else
        opt_config.deleteEntry("-R");

    QString str = def_config.readEntry("RLE");
    if (RLE->text() != str)
        opt_config.writeEntry("RLE", RLE->text());
    else
        opt_config.deleteEntry("RLE");

    setFlag = def_config.readEntry("-P", false);

    if (PCB->isChecked() != setFlag)
    opt_config.writeEntry("-P", PCB->isChecked());
        else
    opt_config.deleteEntry("-P");


    setFlag = def_config.readEntry("-1", false);
    if (ssh1CB->isChecked() != setFlag)
        opt_config.writeEntry("-1", ssh1CB->isChecked());
    else
        opt_config.deleteEntry("-1");

    setFlag = def_config.readEntry("-2", false);
    if (ssh2CB->isChecked() != setFlag)
        opt_config.writeEntry("-2", ssh2CB->isChecked());
    else
        opt_config.deleteEntry("-2");

    setFlag = def_config.readEntry("-a", false);
    if (aCB->isChecked() != setFlag)
        opt_config.writeEntry("-a", aCB->isChecked());
    else
        opt_config.deleteEntry("-a");

    setFlag = def_config.readEntry("-A", false);
    if (ACB->isChecked() != setFlag)
        opt_config.writeEntry("-A", ACB->isChecked());
    else
        opt_config.deleteEntry("-A");

    setFlag = def_config.readEntry("-c", false);
    if (cCB->isChecked() != setFlag)
        opt_config.writeEntry("-c", cCB->isChecked());
    else
        opt_config.deleteEntry("-c");

    int ii = def_config.readEntry("Cipher", false);
    if (ccCB->currentIndex() != ii)
        opt_config.writeEntry("Cipher", ccCB->currentIndex());
    else
        opt_config.deleteEntry("Cipher");

    setFlag = def_config.readEntry("-C", false);
    if (CCB->isChecked() != setFlag)
        opt_config.writeEntry("-C", CCB->isChecked());
    else
        opt_config.deleteEntry("-C");

    str = def_config.readEntry("cLE");
    if (cLE->text() != str)
        opt_config.writeEntry("cLE", cLE->text());
    else
        opt_config.deleteEntry("cLE");

    str = def_config.readEntry("FLE");
    if (FLE->lineEdit()->text() != str)
        opt_config.writeEntry("FLE", FLE->lineEdit()->text());
    else
        opt_config.deleteEntry("FLE");

    setFlag = def_config.readEntry("-F", false);
    if (FCB->isChecked() != setFlag)
        opt_config.writeEntry("-F", FCB->isChecked());
    else
        opt_config.deleteEntry("-F");

    setFlag = def_config.readEntry("-4", false);
    if (IPV4CB->isChecked() != setFlag)
        opt_config.writeEntry("-4", IPV4CB->isChecked());
    else
        opt_config.deleteEntry("-4");

    setFlag = def_config.readEntry("-6", false);
    if (IPV6CB->isChecked() != setFlag)
        opt_config.writeEntry("-6", IPV6CB->isChecked());
    else
        opt_config.deleteEntry("-6");

    setFlag = def_config.readEntry("-b", false);
    if (bCB->isChecked() != setFlag)
        opt_config.writeEntry("-b", bCB->isChecked());
    else
        opt_config.deleteEntry("-b");

    str = def_config.readEntry("bLE");
    if (bLE->text() != str)
        opt_config.writeEntry("bLE", bLE->text());
    else
        opt_config.deleteEntry("bLE");

    setFlag = def_config.readEntry("-c1", false);
    if (c1CB->isChecked() != setFlag)
        opt_config.writeEntry("-c1", c1CB->isChecked());
    else
        opt_config.deleteEntry("-c1");

    setFlag = def_config.readEntry("-e", false);
    if (eCB->isChecked() != setFlag)
        opt_config.writeEntry("-e", eCB->isChecked());
    else
        opt_config.deleteEntry("-e");

    str = def_config.readEntry("eLE");
    if (eLE->text() != str)
        opt_config.writeEntry("eLE", eLE->text());
    else
        opt_config.deleteEntry("eLE");

    setFlag = def_config.readEntry("-f", false);
    if (fCB->isChecked() != setFlag)
        opt_config.writeEntry("-f", fCB->isChecked());
    else
        opt_config.deleteEntry("-f");

    setFlag = def_config.readEntry("-g", false);
    if (gCB->isChecked() != setFlag)
        opt_config.writeEntry("-g", gCB->isChecked());
    else
        opt_config.deleteEntry("-g");

    setFlag = def_config.readEntry("-i", false);
    if (iCB->isChecked() != setFlag)
        opt_config.writeEntry("-i", iCB->isChecked());
    else
        opt_config.deleteEntry("-i");

    str = def_config.readEntry("iLE");
    if (iLE->lineEdit()->text() != str)
        opt_config.writeEntry("iLE", iLE->lineEdit()->text());
    else
        opt_config.deleteEntry("iLE");

    str = def_config.readEntry("ILE1");
    if (ILE->lineEdit()->text() != str)
        opt_config.writeEntry("ILE1", ILE->lineEdit()->text());
    else
        opt_config.deleteEntry("ILE1");

    setFlag = def_config.readEntry("-I", false);
    if (ICB->isChecked() != setFlag)
        opt_config.writeEntry("-I", ICB->isChecked());
    else
        opt_config.deleteEntry("-I");

    setFlag = def_config.readEntry("-k", false);
    if (kCB->isChecked() != setFlag)
        opt_config.writeEntry("-k", kCB->isChecked());
    else
        opt_config.deleteEntry("-k");

    setFlag = def_config.readEntry("-m", false);
    if (mCB->isChecked() != setFlag)
        opt_config.writeEntry("-m", mCB->isChecked());
    else
        opt_config.deleteEntry("-m");

    str = def_config.readEntry("mLE");
    if (mLE->text() != str)
        opt_config.writeEntry("mLE", mLE->text());
    else
        opt_config.deleteEntry("mLE");

    setFlag = def_config.readEntry("-n", false);
    if (nCB->isChecked() != setFlag)
        opt_config.writeEntry("-n", nCB->isChecked());
    else
        opt_config.deleteEntry("-n");

    setFlag = def_config.readEntry("-N", false);
    if (NCB->isChecked() != setFlag)
        opt_config.writeEntry("-N", NCB->isChecked());
    else
        opt_config.deleteEntry("-N");

    setFlag = def_config.readEntry("-q", false);
    if (qCB->isChecked() != setFlag)
        opt_config.writeEntry("-q", qCB->isChecked());
    else
        opt_config.deleteEntry("-q");

    setFlag = def_config.readEntry("-v", false);
    if (vCB->isChecked() != setFlag) {
        opt_config.writeEntry("-v", vCB->isChecked());
        opt_config.writeEntry("VerboseLevel", vSB->value());
    }
    else {
        opt_config.deleteEntry("-v");
        opt_config.deleteEntry("VerboseLevel");
    }

    setFlag = def_config.readEntry("-s", false);
    if (sCB->isChecked() != setFlag)
        opt_config.writeEntry("-s", sCB->isChecked());
    else
        opt_config.deleteEntry("-s");

    setFlag = def_config.readEntry("-T", false);
    if (TCB->isChecked() != setFlag)
        opt_config.writeEntry("-T", TCB->isChecked());
    else
        opt_config.deleteEntry("-T");

    setFlag = def_config.readEntry("-t", false);
    if (tCB->isChecked() != setFlag)
        opt_config.writeEntry("-t", tCB->isChecked());
    else
        opt_config.deleteEntry("-t");

    setFlag = def_config.readEntry("-o", false);
    if (oCB->isChecked() != setFlag)
        opt_config.writeEntry("-o", oCB->isChecked());
    else
        opt_config.deleteEntry("-o");

    str = def_config.readEntry("oLE");
    if (oLE->text() != str)
        opt_config.writeEntry("oLE", oLE->text());
    else
        opt_config.deleteEntry("oLE");

    QString c = def_config.readEntry("Command");
    if (commandLE->text() != c)
        opt_config.writeEntry("Command", commandLE->text());
    else
        opt_config.deleteEntry("Command");
    config.sync();
}


bool KSSH::loadOptions(QString group)
{
    bool setFlag;
    bool ret = false;

    KConfig config;
    KConfigGroup opt_config = config.group(group);

//     if (opt_config.hasKey("Position")) {
//         QPoint mypos = opt_config.readEntry("Position", QPoint(0, 0));
//         if (pos() != mypos)
//             ret = true;
//         move(mypos);
//     }
/*
    if (opt_config.hasKey("Size")) {
        QSize mysize = opt_config.readEntry("Size", size());
        if (size() != mysize)
            ret = true;
        resize(mysize);
    }*/

    if (opt_config.hasKey("VerboseLevel")) {
        int ite = opt_config.readEntry("VerboseLevel", 0);
        if (vSB->value() != ite)
            ret = true;
        vSB->setValue(ite);
    }

    if (opt_config.hasKey("Cipher")) {
        int ite = opt_config.readEntry("Cipher", 0);
        if (ccCB->currentIndex() != ite)
            ret = true;
        ccCB->setCurrentIndex(ite);
    }

    if (opt_config.hasKey("Command")) {
        QString Lstr = opt_config.readEntry("Command");
        if (commandLE->text() != Lstr)
            ret = true;
        commandLE->setText(Lstr);
    }

    if (opt_config.hasKey("eLE")) {
        QString Lstr = opt_config.readEntry("eLE");
        if (eLE->text() != Lstr)
            ret = true;
        eLE->setText(Lstr);
    }

    if (opt_config.hasKey("oLE")) {
        QString Lstr = opt_config.readEntry("oLE");
        if (oLE->text() != Lstr)
            ret = true;
        oLE->setText(Lstr);
    }

    if (opt_config.hasKey("bLE")) {
        QString Lstr = opt_config.readEntry("bLE");
        if (bLE->text() != Lstr)
            ret = true;
        bLE->setText(Lstr);
    }

    if (opt_config.hasKey("mLE")) {
        QString Lstr = opt_config.readEntry("mLE");
        if (mLE->text() != Lstr)
            ret = true;
        mLE->setText(Lstr);
    }

    if (opt_config.hasKey("ILE1")) {
        QString Lstr = opt_config.readEntry("ILE1");
        if (ILE->lineEdit()->text() != Lstr)
            ret = true;
        ILE->lineEdit()->setText(Lstr);
    }

    if (opt_config.hasKey("iLE")) {
        QString Lstr = opt_config.readEntry("iLE");
        if (iLE->lineEdit()->text() != Lstr)
            ret = true;
        iLE->lineEdit()->setText(Lstr);
    }

    if (opt_config.hasKey("cLE")) {
        QString Lstr = opt_config.readEntry("cLE");
        if (cLE->text() != Lstr)
            ret = true;
        cLE->setText(Lstr);
    }

    if (opt_config.hasKey("FLE")) {
        QString Lstr = opt_config.readEntry("FLE");
        if (FLE->lineEdit()->text() != Lstr)
            ret = true;
        FLE->lineEdit()->setText(Lstr);
    }
    if (opt_config.hasKey("RLE")) {
        QString Lstr = opt_config.readEntry("RLE");
        if (RLE->text() != Lstr)ret = true;
        RLE->setText(Lstr);
    }
    if (opt_config.hasKey("-X")) {
        setFlag = opt_config.readEntry("-X", false);
        if (XCB->isChecked() != setFlag)
            ret = true;
        XCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-x")) {
        setFlag = opt_config.readEntry("-x", false);
        if (xCB->isChecked() != setFlag)
            ret = true;
        xCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-p")) {
        setFlag = opt_config.readEntry("-p", false);
        if (pCB->isChecked() != setFlag)
            ret = true;
        pCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("Port")) {
        int v = opt_config.readEntry("Port", 0);
        if (portSB->value() != v)
            ret = true;
        portSB->setValue(v);
    }
    if (opt_config.hasKey("-L")) {
        setFlag = opt_config.readEntry("-L", false);
        if (LCB->isChecked() != setFlag)
            ret = true;
        LCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("LLE")) {
        QString Lstr = opt_config.readEntry("LLE");
        if (LLE->text() != Lstr)
            ret = true;
        LLE->setText(Lstr);
    }
    if (opt_config.hasKey("-R")) {
        setFlag = opt_config.readEntry("-R", false);
        if (RCB->isChecked() != setFlag)
            ret = true;
        RCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-P"))
    {
        setFlag = opt_config.readEntry("-P", false);
        if (PCB->isChecked() != setFlag)
            ret = true;
        PCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-1"))
    {
        setFlag = opt_config.readEntry("-1", false);
        if (ssh1CB->isChecked() != setFlag)
            ret = true;
        ssh1CB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-2")) {
        setFlag = opt_config.readEntry("-2", false);
        if (ssh2CB->isChecked() != setFlag)
            ret = true;
        ssh2CB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-a")) {
        setFlag = opt_config.readEntry("-a", false);
        if (aCB->isChecked() != setFlag)
            ret = true;
        aCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-A")) {
        setFlag = opt_config.readEntry("-A", false);
        if (ACB->isChecked() != setFlag)
            ret = true;
        ACB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-c"))
    {
        setFlag = opt_config.readEntry("-c", false);
        if (cCB->isChecked() != setFlag)
            ret = true;
        cCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-C"))
    {
        setFlag = opt_config.readEntry("-C", false);
        if (CCB->isChecked() != setFlag)
            ret = true;
        CCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-F"))
    {
        setFlag = opt_config.readEntry("-F", false);
        if (FCB->isChecked() != setFlag)
            ret = true;
        FCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-4"))
    {
        setFlag = opt_config.readEntry("-4", false);
        if (IPV4CB->isChecked() != setFlag)
            ret = true;
        IPV4CB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-6"))
    {
        setFlag = opt_config.readEntry("-6", false);
        if (IPV6CB->isChecked() != setFlag)
            ret = true;
        IPV6CB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-b"))
    {
        setFlag = opt_config.readEntry("-b", false);
        if (bCB->isChecked() != setFlag)
            ret = true;
        bCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-c1"))
    {
        setFlag = opt_config.readEntry("-c1", false);
        if (c1CB->isChecked() != setFlag)
            ret = true;
        c1CB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-e"))
    {
        setFlag = opt_config.readEntry("-e", false);
        if (eCB->isChecked() != setFlag)
            ret = true;
        eCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-f"))
    {
        setFlag = opt_config.readEntry("-f", false);
        if (fCB->isChecked() != setFlag)
            ret = true;
        fCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-g"))
    {
        setFlag = opt_config.readEntry("-g", false);
        if (gCB->isChecked() != setFlag)
            ret = true;
        gCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-i"))
    {
        setFlag = opt_config.readEntry("-i", false);
        if (iCB->isChecked() != setFlag)
            ret = true;
        iCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-I"))
    {
        setFlag = opt_config.readEntry("-I", false);
        if (ICB->isChecked() != setFlag)
            ret = true;
        ICB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-k"))
    {
        setFlag = opt_config.readEntry("-k", false);
        if (kCB->isChecked() != setFlag)
            ret = true;
        kCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-m"))
    {
        setFlag = opt_config.readEntry("-m", false);
        if (mCB->isChecked() != setFlag)
            ret = true;
        mCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-n"))
    {
        setFlag = opt_config.readEntry("-n", false);
        if (nCB->isChecked() != setFlag)
            ret = true;
        nCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-N"))
    {
        setFlag = opt_config.readEntry("-N", false);
        if (NCB->isChecked() != setFlag)
            ret = true;
        NCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-q"))
    {
        setFlag = opt_config.readEntry("-q", false);
        if (qCB->isChecked() != setFlag)
            ret = true;
        qCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-v"))
    {
        setFlag = opt_config.readEntry("-v", false);
        if (vCB->isChecked() != setFlag)
            ret = true;
        vCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-s")) {
        setFlag = opt_config.readEntry("-s", false);
        if (sCB->isChecked() != setFlag)
            ret = true;
        sCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-T")) {
        setFlag = opt_config.readEntry("-T", false);
        if (TCB->isChecked() != setFlag)
            ret = true;
        TCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-t")) {
        setFlag = opt_config.readEntry("-t", false);
        if (tCB->isChecked() != setFlag)
            ret = true;
        tCB->setChecked(setFlag);
    }
    if (opt_config.hasKey("-o")) {
        setFlag = opt_config.readEntry("-o", false);
        if (oCB->isChecked() != setFlag)
            ret = true;
        oCB->setChecked(setFlag);
    }
    return ret;
}

void KSSH::userFor(const QString& host)
{
    userCB->clear();
    compUser->clear();

    KConfig config;
    KConfigGroup user_config = config.group(host % QString("-User List"));

    QStringList data = user_config.readEntry("User", QStringList());
    compUser->setItems(data);
    userCB->setHistoryItems(data);

    userCB->setEditText(user_config.readEntry("LastUsed", ""));

    loadOptions("DefaultConfig");

    if (loadOptions(host % QString("-Options"))) {
        QFont tmpFont = optionsPB->font();
        tmpFont.setItalic(true);
        optionsPB->setFont(tmpFont);
    }
    else {
        QFont tmpFont = optionsPB->font();
        tmpFont.setItalic(false);
        optionsPB->setFont(tmpFont);
    }
}

void KSSH::hostEditor()
{
    userHostELB->clear();
    m_user_editor = false;
    m_host_editor = true;
    userHostELB->setTitle(i18n("Hosts:"));
    userHostELB->insertStringList(compHost->items());
    editorF->show();
}


void KSSH::userEditor()
{
    userHostELB->clear();
    m_user_editor = true;
    m_host_editor = false;
    QString host = hostCB->currentText();
    userHostELB->setTitle(i18n("User list for %1:").arg(host));
    userHostELB->insertStringList(compUser->items());
    editorF->show();
}

void KSSH::okEditor()
{
    if (m_user_editor) {
        userCB->clear();
        userCB->setHistoryItems(userHostELB->items());
        compUser->setItems(userHostELB->items());
    }
    if (m_host_editor) {
        hostCB->clear();
        QStringList lista = userHostELB->items();
        hostCB->setHistoryItems(lista);
        compHost->setItems(lista);
    }

    editorF->hide();
    userHostELB->clear();
    m_user_editor = false;
    m_host_editor = false;
}

void KSSH::cancelEditor()
{
    editorF->hide();
    userHostELB->clear();
    m_user_editor = false;
    m_host_editor = false;
}
