/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "remotecontrol.h"
#include "mainwindow.h"
#include "centralwidget.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QThread>
#include <QtCore/QTextStream>
#include <QtCore/QSocketNotifier>

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

#include <QtHelp/QHelpEngine>
#include <QtHelp/QHelpIndexWidget>

#ifdef Q_OS_WIN
#   include "remotecontrol_win.h"
#endif

QT_BEGIN_NAMESPACE

#ifdef Q_OS_WIN

StdInListenerWin::StdInListenerWin(QObject *parent)
    : QThread(parent)
{
}

StdInListenerWin::~StdInListenerWin()
{
    terminate();
    wait();
}

void StdInListenerWin::run()
{
    bool ok = true;
    char chBuf[4096];
    DWORD dwRead;

#ifndef Q_WS_WINCE
    HANDLE hStdin, hStdinDup;

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        return;

    DuplicateHandle(GetCurrentProcess(), hStdin,
        GetCurrentProcess(), &hStdinDup,
        0, false, DUPLICATE_SAME_ACCESS);

    CloseHandle(hStdin);
#else
    HANDLE hStdinDup;
    hStdinDup = stdin;
#endif

    while (ok) {
        ok = ReadFile(hStdinDup, chBuf, sizeof(chBuf), &dwRead, NULL);
        if (ok && dwRead != 0)
            emit receivedCommand(QString::fromLocal8Bit(chBuf, dwRead));
    }
}
#endif

RemoteControl::RemoteControl(MainWindow *mainWindow, QHelpEngine *helpEngine,
                             QFileSystemWatcher *qchWatcher)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_helpEngine(helpEngine)
    , m_debug(false)
    , m_caching(true)
    , m_syncContents(false)
    , m_expandTOC(-2)
    , m_qchWatcher(qchWatcher)

{
    connect(m_mainWindow, SIGNAL(initDone()), this, SLOT(applyCache()));
#ifdef Q_OS_WIN
    StdInListenerWin *l = new StdInListenerWin(this);
    connect(l, SIGNAL(receivedCommand(QString)),
        this, SLOT(handleCommandString(QString)));
    l->start();
#else
    QSocketNotifier *notifier = new QSocketNotifier(fileno(stdin),
        QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(receivedData()));
    notifier->setEnabled(true);
#endif
}

void RemoteControl::receivedData()
{
    QByteArray ba;
    while (true) {
        char c = getc(stdin);
        if (c == EOF || c == '\0')
            break;
        if (c)
            ba.append(c);
         if (c == '\n')
             break;
    }
    handleCommandString(QString::fromLocal8Bit(ba));
}

void RemoteControl::handleCommandString(const QString &cmdString)
{
    QStringList cmds = cmdString.split(QLatin1Char(';'));
    QStringList::const_iterator it = cmds.constBegin();
    while (it != cmds.constEnd()) {
        QString cmd, arg;
        splitInputString(*it, cmd, arg);

        if (m_debug)
            QMessageBox::information(0, tr("Debugging Remote Control"),
                tr("Received Command: %1 %2").arg(cmd).arg(arg));

        if (cmd == QLatin1String("debug"))
            handleDebugCommand(arg);
         else if (cmd == QLatin1String("show"))
            handleShowOrHideCommand(arg, true);
         else if (cmd == QLatin1String("hide"))
            handleShowOrHideCommand(arg, false);
         else if (cmd == QLatin1String("setsource"))
            handleSetSourceCommand(arg);
         else if (cmd == QLatin1String("synccontents"))
            handleSyncContentsCommand();
         else if (cmd == QLatin1String("activatekeyword"))
            handleActivateKeywordCommand(arg);
         else if (cmd == QLatin1String("activateidentifier"))
            handleActivateIdentifierCommand(arg);
         else if (cmd == QLatin1String("expandtoc"))
            handleExpandTocCommand(arg);
         else if (cmd == QLatin1String("setcurrentfilter"))
            handleSetCurrentFilterCommand(arg);
         else if (cmd == QLatin1String("register"))
            handleRegisterCommand(arg);
         else if (cmd == QLatin1String("unregister"))
            handleUnregisterCommand(arg);
         else
            return;

        ++it;
    }
    m_mainWindow->raise();
    m_mainWindow->activateWindow();
}

void RemoteControl::splitInputString(const QString &input, QString &cmd,
                                     QString &arg)
{
    QString cmdLine = input.trimmed();
    int i = cmdLine.indexOf(QLatin1Char(' '));
    cmd = cmdLine.left(i);
    arg = cmdLine.mid(i+1);
    cmd = cmd.toLower();
}

void RemoteControl::handleDebugCommand(const QString &arg)
{
    m_debug = arg == QLatin1String("on");
}

void RemoteControl::handleShowOrHideCommand(const QString &arg, bool show)
{
    if (arg.toLower() == QLatin1String("contents"))
        m_mainWindow->setContentsVisible(show);
    else if (arg.toLower() == QLatin1String("index"))
        m_mainWindow->setIndexVisible(show);
    else if (arg.toLower() == QLatin1String("bookmarks"))
        m_mainWindow->setBookmarksVisible(show);
    else if (arg.toLower() == QLatin1String("search"))
        m_mainWindow->setSearchVisible(show);
}

void RemoteControl::handleSetSourceCommand(const QString &arg)
{
    QUrl url(arg);
    if (url.isValid()) {
        if (url.isRelative())
            url = CentralWidget::instance()->currentSource().resolved(url);
        if (m_caching) {
            clearCache();
            m_setSource = url;
        } else {
            CentralWidget::instance()->setSource(url);
        }
    }
}

void RemoteControl::handleSyncContentsCommand()
{
    if (m_caching)
        m_syncContents = true;
    else
        m_mainWindow->syncContents();
}

void RemoteControl::handleActivateKeywordCommand(const QString &arg)
{
    if (m_caching) {
        clearCache();
        m_activateKeyword = arg;
    } else {
        m_mainWindow->setIndexString(arg);
        if (!arg.isEmpty())
            m_helpEngine->indexWidget()->activateCurrentItem();
    }
}

void RemoteControl::handleActivateIdentifierCommand(const QString &arg)
{
    if (m_caching) {
        clearCache();
        m_activateIdentifier = arg;
    } else {
        const QMap<QString, QUrl> &links =
            m_helpEngine->linksForIdentifier(arg);
        if (!links.isEmpty())
            CentralWidget::instance()->setSource(links.constBegin().value());
    }
}

void RemoteControl::handleExpandTocCommand(const QString &arg)
{
    bool ok = false;
    int depth = -2;
    if (!arg.isEmpty())
        depth = arg.toInt(&ok);
    if (!ok || depth < -2)
        depth = -2;

    if (m_caching)
        m_expandTOC = depth;
    else if (depth != -2)
        m_mainWindow->expandTOC(depth);
}

void RemoteControl::handleSetCurrentFilterCommand(const QString &arg)
{
    if (m_helpEngine->customFilters().contains(arg)) {
        if (m_caching) {
            clearCache();
            m_currentFilter = arg;
        } else {
            m_helpEngine->setCurrentFilter(arg);
        }
    }
}

void RemoteControl::handleRegisterCommand(const QString &arg)
{
    const QString &absFileName = QFileInfo(arg).absoluteFilePath();
    if (m_helpEngine->registeredDocumentations().
        contains(QHelpEngineCore::namespaceName(absFileName)))
        return;
    if (m_helpEngine->registerDocumentation(absFileName)) {
        m_qchWatcher->addPath(absFileName);
        m_helpEngine->setupData();
        Q_ASSERT(m_qchWatcher->files().count()
                 == m_helpEngine->registeredDocumentations().count());
    }
}

void RemoteControl::handleUnregisterCommand(const QString &arg)
{
    const QString &absFileName = QFileInfo(arg).absoluteFilePath();
    const QString &ns = QHelpEngineCore::namespaceName(absFileName);
    if (m_helpEngine->registeredDocumentations().contains(ns)) {
        CentralWidget* widget = CentralWidget::instance();
        widget->closeTabs(widget->currentSourceFileList().keys(ns));
        const QString docFile = m_helpEngine->documentationFileName(ns);
        if (m_helpEngine->unregisterDocumentation(ns)) {
            m_qchWatcher->removePath(docFile);
            m_helpEngine->setupData();
        }
    }
}

void RemoteControl::applyCache()
{
    if (m_setSource.isValid()) {
        CentralWidget::instance()->setSource(m_setSource);
    } else if (!m_activateKeyword.isEmpty()) {
        m_mainWindow->setIndexString(m_activateKeyword);
        m_helpEngine->indexWidget()->activateCurrentItem();
    } else if (!m_activateIdentifier.isEmpty()) {
        QMap<QString, QUrl> links =
            m_helpEngine->linksForIdentifier(m_activateIdentifier);
        if (links.count())
            CentralWidget::instance()->setSource(links.constBegin().value());
    } else if (!m_currentFilter.isEmpty()) {
        m_helpEngine->setCurrentFilter(m_currentFilter);
    }

    if (m_syncContents)
        m_mainWindow->syncContents();

    Q_ASSERT(m_expandTOC >= -2);
    if (m_expandTOC != -2)
        m_mainWindow->expandTOC(m_expandTOC);

    m_caching = false;
}

void RemoteControl::clearCache()
{
    m_currentFilter.clear();
    m_setSource.clear();
    m_syncContents = false;
    m_activateKeyword.clear();
    m_activateIdentifier.clear();
}

QT_END_NAMESPACE
