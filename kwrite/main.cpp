/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kwrite.h"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageBox>
#include <KDBusService>

#include <QCommandLineParser>
#include <QApplication>
#include <QTextCodec>
#include <QFileInfo>
#include <QDir>

extern "C" Q_DECL_EXPORT int kdemain(int argc, char **argv)
{
    /**
     * first init the app
     */
    QApplication app(argc, argv);

    /**
     * enable high dpi support
     */
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    /**
     * Connect application with translation catalogs
     */
    KLocalizedString::setApplicationDomain("kwrite");

    /**
     * then use i18n and co
     */
    KAboutData aboutData(QStringLiteral("kwrite"),
                         i18n("KWrite"),
                         QStringLiteral(KATE_VERSION),
                         i18n("KWrite - Text Editor"), KAboutLicense::LGPL_V2,
                         i18n("(c) 2000-2014 The Kate Authors"), QString(), QStringLiteral("http://kate-editor.org"));

    /**
     * right dbus prefix == org.kde.
     */
    aboutData.setOrganizationDomain(QByteArray("kde.org"));

    aboutData.addAuthor(i18n("Christoph Cullmann"), i18n("Maintainer"), QStringLiteral("cullmann@kde.org"), QStringLiteral("http://www.cullmann.io"));
    aboutData.addAuthor(i18n("Anders Lund"), i18n("Core Developer"), QStringLiteral("anders@alweb.dk"), QStringLiteral("http://www.alweb.dk"));
    aboutData.addAuthor(i18n("Joseph Wenninger"), i18n("Core Developer"), QStringLiteral("jowenn@kde.org"), QStringLiteral("http://stud3.tuwien.ac.at/~e9925371"));
    aboutData.addAuthor(i18n("Hamish Rodda"), i18n("Core Developer"), QStringLiteral("rodda@kde.org"));
    aboutData.addAuthor(i18n("Dominik Haumann"), i18n("Developer & Highlight wizard"), QStringLiteral("dhdev@gmx.de"));
    aboutData.addAuthor(i18n("Waldo Bastian"), i18n("The cool buffersystem"), QStringLiteral("bastian@kde.org"));
    aboutData.addAuthor(i18n("Charles Samuels"), i18n("The Editing Commands"), QStringLiteral("charles@kde.org"));
    aboutData.addAuthor(i18n("Matt Newell"), i18nc("Credit text for someone that did testing and some other similar things", "Testing, ..."), QStringLiteral("newellm@proaxis.com"));
    aboutData.addAuthor(i18n("Michael Bartl"), i18n("Former Core Developer"), QStringLiteral("michael.bartl1@chello.at"));
    aboutData.addAuthor(i18n("Michael McCallum"), i18n("Core Developer"), QStringLiteral("gholam@xtra.co.nz"));
    aboutData.addAuthor(i18n("Jochen Wilhemly"), i18n("KWrite Author"), QStringLiteral("digisnap@cs.tu-berlin.de"));
    aboutData.addAuthor(i18n("Michael Koch"), i18n("KWrite port to KParts"), QStringLiteral("koch@kde.org"));
    aboutData.addAuthor(i18n("Christian Gebauer"), QString(), QStringLiteral("gebauer@kde.org"));
    aboutData.addAuthor(i18n("Simon Hausmann"), QString(), QStringLiteral("hausmann@kde.org"));
    aboutData.addAuthor(i18n("Glen Parker"), i18n("KWrite Undo History, Kspell integration"), QStringLiteral("glenebob@nwlink.com"));
    aboutData.addAuthor(i18n("Scott Manson"), i18n("KWrite XML Syntax highlighting support"), QStringLiteral("sdmanson@alltel.net"));
    aboutData.addAuthor(i18n("John Firebaugh"), i18n("Patches and more"), QStringLiteral("jfirebaugh@kde.org"));
    aboutData.addAuthor(i18n("Gerald Senarclens de Grancy"), i18n("QA and Scripting"), QStringLiteral("oss@senarclens.eu"), QStringLiteral("http://find-santa.eu/"));

    aboutData.addCredit(i18n("Matteo Merli"), i18n("Highlighting for RPM Spec-Files, Perl, Diff and more"), QStringLiteral("merlim@libero.it"));
    aboutData.addCredit(i18n("Rocky Scaletta"), i18n("Highlighting for VHDL"), QStringLiteral("rocky@purdue.edu"));
    aboutData.addCredit(i18n("Yury Lebedev"), i18n("Highlighting for SQL"));
    aboutData.addCredit(i18n("Chris Ross"), i18n("Highlighting for Ferite"));
    aboutData.addCredit(i18n("Nick Roux"), i18n("Highlighting for ILERPG"));
    aboutData.addCredit(i18n("Carsten Niehaus"), i18n("Highlighting for LaTeX"));
    aboutData.addCredit(i18n("Per Wigren"), i18n("Highlighting for Makefiles, Python"));
    aboutData.addCredit(i18n("Jan Fritz"), i18n("Highlighting for Python"));
    aboutData.addCredit(i18n("Daniel Naber"));
    aboutData.addCredit(i18n("Roland Pabel"), i18n("Highlighting for Scheme"));
    aboutData.addCredit(i18n("Cristi Dumitrescu"), i18n("PHP Keyword/Datatype list"));
    aboutData.addCredit(i18n("Carsten Pfeiffer"), i18nc("Credit text for someone that helped a lot", "Very nice help"));
    aboutData.addCredit(i18n("All people who have contributed and I have forgotten to mention"));

    /**
     * bugzilla
     */
    aboutData.setProductName(QByteArray("kate/kwrite"));

    /**
     * register about data
     */
    KAboutData::setApplicationData(aboutData);

    /**
     * set app stuff from about data component name and org. name from KAboutData
     */
    app.setApplicationName(aboutData.componentName());
    app.setApplicationDisplayName(aboutData.displayName());
    app.setOrganizationDomain(aboutData.organizationDomain());
    app.setApplicationVersion(aboutData.version());

    /**
     * set the program icon
     */
    QApplication::setWindowIcon(QIcon::fromTheme(QLatin1String("accessories-text-editor")));

    /**
     * Create command line parser and feed it with known options
     */
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.setApplicationDescription(aboutData.shortDescription());
    parser.addHelpOption();
    parser.addVersionOption();

    // -e/--encoding option
    const QCommandLineOption useEncoding(QStringList() << QStringLiteral("e") << QStringLiteral("encoding"), i18n("Set encoding for the file to open."), QStringLiteral("encoding"));
    parser.addOption(useEncoding);

    // -l/--line option
    const QCommandLineOption gotoLine(QStringList() << QStringLiteral("l") << QStringLiteral("line"), i18n("Navigate to this line."), QStringLiteral("line"));
    parser.addOption(gotoLine);

    // -c/--column option
    const QCommandLineOption gotoColumn(QStringList() << QStringLiteral("c") << QStringLiteral("column"), i18n("Navigate to this column."), QStringLiteral("column"));
    parser.addOption(gotoColumn);

    // -i/--stdin option
    const QCommandLineOption readStdIn(QStringList() << QStringLiteral("i") << QStringLiteral("stdin"), i18n("Read the contents of stdin."));
    parser.addOption(readStdIn);

    // --tempfile option
    const QCommandLineOption tempfile(QStringList() << QStringLiteral("tempfile"), i18n("The files/URLs opened by the application will be deleted after use"));
    parser.addOption(tempfile);

    // urls to open
    parser.addPositionalArgument(QStringLiteral("urls"), i18n("Documents to open."), QStringLiteral("[urls...]"));

    /**
     * do the command line parsing
     */
    parser.process(app);

    /**
     * handle standard options
     */
    aboutData.processCommandLine(&parser);

    if (app.isSessionRestored()) {
        KWrite::restore();
    } else {
        bool nav = false;
        int line = 0, column = 0;

        QTextCodec *codec = parser.isSet(QStringLiteral("encoding")) ? QTextCodec::codecForName(parser.value(QStringLiteral("encoding")).toLocal8Bit()) : 0;

        if (parser.isSet(QStringLiteral("line"))) {
            line = parser.value(QStringLiteral("line")).toInt() - 1;
            nav = true;
        }

        if (parser.isSet(QStringLiteral("column"))) {
            column = parser.value(QStringLiteral("column")).toInt() - 1;
            nav = true;
        }

        if (parser.positionalArguments().count() == 0) {
            KWrite *t = new KWrite;

            if (parser.isSet(QStringLiteral("stdin"))) {
                QTextStream input(stdin, QIODevice::ReadOnly);

                // set chosen codec
                if (codec) {
                    input.setCodec(codec);
                }

                QString line;
                QString text;

                do {
                    line = input.readLine();
                    text.append(line + QLatin1Char('\n'));
                } while (!line.isNull());

                KTextEditor::Document *doc = t->view()->document();
                if (doc) {
                    doc->setText(text);
                }
            }

            if (nav && t->view()) {
                t->view()->setCursorPosition(KTextEditor::Cursor(line, column));
            }
        } else {
            int docs_opened = 0;
            Q_FOREACH(const QString positionalArgument, parser.positionalArguments()) {
                QUrl url;

                // convert to an url
                QRegExp withProtocol(QStringLiteral("^[a-zA-Z]+:")); // TODO: remove after Qt supports this on its own
                if (withProtocol.indexIn(positionalArgument) == 0) {
                    url = QUrl::fromUserInput(positionalArgument);
                } else {
                    url = QUrl::fromLocalFile(positionalArgument);
                }

                // this file is no local dir, open it, else warn
                bool noDir = !url.isLocalFile() || !QFileInfo(url.toLocalFile()).isDir();

                if (noDir) {
                    ++docs_opened;
                    KWrite *t = new KWrite();

                    if (codec) {
                        t->view()->document()->setEncoding(QString::fromLatin1(codec->name()));
                    }

                    t->loadURL(url);

                    if (nav) {
                        t->view()->setCursorPosition(KTextEditor::Cursor(line, column));
                    }
                } else {
                    KMessageBox::sorry(0, i18n("The file '%1' could not be opened: it is not a normal file, it is a folder.", url.toString()));
                }
            }
            if (!docs_opened) {
                ::exit(1);    // see http://bugs.kde.org/show_bug.cgi?id=124708
            }
        }
    }

    // no window there, uh, ohh, for example borked session config !!!
    // create at least one !!
    if (KWrite::noWindows()) {
        new KWrite();
    }

    /**
     * finally register this kwrite instance for dbus
     */
    const KDBusService dbusService(KDBusService::Multiple);

    /**
     * Run the event loop
     */
    return app.exec();
}
