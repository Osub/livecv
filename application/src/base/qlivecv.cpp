/****************************************************************************
**
** Copyright (C) 2014-2016 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Live CV Application.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#include "qlivecv.h"
#include "qlivecvlog.h"
#include "qcodedocument.h"
#include "qlivecvarguments.h"

#include "qdocumentcodeinterface.h"
#include "qproject.h"
#include "qprojectentry.h"
#include "qprojectfile.h"
#include "qprojectfilemodel.h"
#include "qprojectnavigationmodel.h"
#include "qprojectdocumentmodel.h"
#include "qprojectdocument.h"

#include "qdocumentqmlhandler.h"
#include "qdocumentqmlinfo.h"

#include <QUrl>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QGuiApplication>

QLiveCV::QLiveCV(int argc, const char* const argv[])
    : m_engine(new QQmlApplicationEngine)
    , m_document(new QCodeDocument)
    , m_codeInterface(0)
    , m_dir(QGuiApplication::applicationDirPath())
    , m_project(new lcv::QProject)
{
    solveImportPaths();

    lcv::QDocumentQmlHandler* qmlHandler = new lcv::QDocumentQmlHandler(m_engine);
    m_codeInterface = new lcv::QDocumentCodeInterface(qmlHandler);

    m_arguments = new QLiveCVArguments(
        " Live CV v" + versionString() + "\n"
        " --------------------------------------------------- ",
        argc,
        argv
    );

    QObject::connect(
        m_project, SIGNAL(inFocusChanged(QProjectDocument*)),
        m_codeInterface, SLOT(setDocument(QProjectDocument*))
    );
    QObject::connect(
        m_project, SIGNAL(pathChanged(QString)),
        qmlHandler, SLOT(newProject(QString))
    );
    QObject::connect(
        m_project, SIGNAL(directoryChanged(QString)),
        qmlHandler, SLOT(directoryChanged(QString))
    );
    QObject::connect(
        m_project, SIGNAL(fileChanged(QString)),
        qmlHandler, SLOT(fileChanged(QString))
    );

    if ( !m_arguments->consoleFlag() )
        qInstallMessageHandler(&QLiveCVLog::logFunction);
    if ( m_arguments->fileLogFlag() )
        QLiveCVLog::instance().enableFileLog();
    if ( m_arguments->script() != "" )
        m_project->openProject(m_arguments->script());
}

QLiveCV::~QLiveCV(){
    delete m_engine;
    delete m_document;
}

void QLiveCV::solveImportPaths(){
    QStringList importPaths = m_engine->importPathList();
    m_engine->setImportPathList(QStringList());
    for ( QStringList::iterator it = importPaths.begin(); it != importPaths.end(); ++it ){
        if ( *it != dir() )
            m_engine->addImportPath(*it);
    }
    m_engine->addImportPath(dir() + "/plugins");
}

void QLiveCV::loadLibrary(const QString &library){
    m_lcvlib.setFileName(library);
    m_lcvlib.load();
}

void QLiveCV::loadQml(const QUrl &url){
    m_engine->rootContext()->setContextProperty("project", m_project);
    m_engine->rootContext()->setContextProperty("codeDocument", m_document);
    m_engine->rootContext()->setContextProperty("codeHandler", m_codeInterface);
    m_engine->rootContext()->setContextProperty("lcvlog", &QLiveCVLog::instance());
    m_engine->rootContext()->setContextProperty("arguments", m_arguments);
#ifdef Q_OS_LINUX
    m_engine->rootContext()->setContextProperty("isLinux", true);
#else
    m_engine->rootContext()->setContextProperty("isLinux", false);
#endif

    m_engine->load(url);
}

void QLiveCV::registerTypes(){
    qmlRegisterUncreatableType<QCodeDocument>(
        "Cv", 1, 0, "Document", "Only access to the document object is allowed.");
    qmlRegisterUncreatableType<QLiveCVLog>(
        "Cv", 1, 0, "MessageLog", "Type is singleton.");
    qmlRegisterUncreatableType<lcv::QDocumentCodeInterface>(
        "Cv", 1, 0, "DocumentCodeInterface", "DocumentCodeInterface is singleton.");

    qmlRegisterUncreatableType<lcv::QProjectFileModel>(
        "Cv", 1, 0, "ProjectFileModel", "Cannot create a ProjectFileModel instance.");
    qmlRegisterUncreatableType<lcv::QProjectDocumentModel>(
        "Cv", 1, 0, "ProjectDocumentModel", "Cannot create a ProjectDocumentModel instance.");
    qmlRegisterUncreatableType<lcv::QProjectNavigationModel>(
        "Cv", 1, 0, "ProjectNavigationModel", "Cannot create a ProjectNavigationModel instance.");

    qmlRegisterUncreatableType<lcv::QProjectEntry>(
        "Cv", 1, 0, "ProjectEntry", "ProjectEntry objects are managed by the ProjectFileModel.");
    qmlRegisterUncreatableType<lcv::QProjectFile>(
        "Cv", 1, 0, "ProjectFile", "ProjectFile objects are managed by the ProjectFileModel.");
    qmlRegisterUncreatableType<lcv::QProjectDocument>(
        "Cv", 1, 0, "ProjectDocument", "ProjectDocument objects are managed by the Project class.");
}
