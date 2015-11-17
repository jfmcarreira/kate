/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

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

#ifndef __KATE_CONSOLE_H__
#define __KATE_CONSOLE_H__

#include <KTextEditor/Plugin>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/configpage.h>

#include <QList>
#include <QKeyEvent>

#include <KXMLGUIClient>

class QShowEvent;

namespace KParts
{
  class ReadOnlyPart;
}

class KateConsole;
class KateKonsolePluginView;

class KateKonsolePlugin: public KTextEditor::Plugin
{
    Q_OBJECT
    
  friend class KateKonsolePluginView;
  
  public:
    explicit KateKonsolePlugin( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
    virtual ~KateKonsolePlugin();

    QObject *createView (KTextEditor::MainWindow *mainWindow);

    int configPages() const { return 1; }
    KTextEditor::ConfigPage *configPage (int number = 0, QWidget *parent = 0);

    void readConfig();

    QByteArray previousEditorEnv() {return m_previousEditorEnv;}
    
  private:
    QList<KateKonsolePluginView*> mViews;
    QByteArray m_previousEditorEnv;
};

class KateKonsolePluginView : public QObject
{
    Q_OBJECT

  public:
    /**
      * Constructor.
      */
    KateKonsolePluginView (KateKonsolePlugin* plugin, KTextEditor::MainWindow *mainWindow);

    /**
     * Virtual destructor.
     */
    ~KateKonsolePluginView ();

    void readConfig();

  private:
    KateKonsolePlugin *m_plugin;
    KateConsole *m_console;
};

/**
 * KateConsole
 * This class is used for the internal terminal emulator
 * It uses internally the konsole part, thx to konsole devs :)
 */
class KateConsole : public QWidget, public KXMLGUIClient
{
    Q_OBJECT

  public:
    /**
     * construct us
     * @param mw main window
     * @param parent toolview
     */
    KateConsole (KateKonsolePlugin* plugin, KTextEditor::MainWindow *mw, QWidget* parent);

    /**
     * destruct us
     */
    ~KateConsole ();

    void readConfig();

    /**
     * cd to dir
     * @param path given local directory
     */
    void cd (const QString & path);

    /**
     * send given text to console
     * @param text commands for console
     */
    void sendInput( const QString& text );

    KTextEditor::MainWindow *mainWindow()
    {
      return m_mw;
    }

  public Q_SLOTS:
    /**
     * pipe current document to console
     */
    void slotPipeToConsole ();

    /**
     * synchronize the konsole with the current document (cd to the directory)
     */
    void slotSync(KTextEditor::View *view = 0);
    /**
     * When syncing is done by the user, also show the terminal if it is hidden
     */
    void slotManualSync();

  private Q_SLOTS:
    /**
     * the konsole exited ;)
     * handle that, hide the dock
     */
    void slotDestroyed ();

    /**
     * construct console if needed
     */
    void loadConsoleIfNeeded();

    /**
     * set or clear focus as appropriate.
     */
    void slotToggleFocus();
    
    /**
     * Handle that shortcuts are not eaten by console
     */
    void overrideShortcut (QKeyEvent *event, bool &override);

  protected:
    /**
     * the konsole get shown
     * @param ev show event
     */
    void showEvent(QShowEvent *ev);

  private:
    /**
     * console part
     */
    KParts::ReadOnlyPart *m_part;

    /**
     * main window of this console
     */
    KTextEditor::MainWindow *m_mw;

    /**
     * toolview for this console
     */
    QWidget *m_toolView;
    
    KateKonsolePlugin *m_plugin;
    QString m_currentPath;
};

class KateKonsoleConfigPage : public KTextEditor::ConfigPage {
    Q_OBJECT
  public:
    explicit KateKonsoleConfigPage( QWidget* parent = 0, KateKonsolePlugin *plugin = 0 );
    virtual ~KateKonsoleConfigPage()
    {}

    virtual QString name() const;
    virtual QString fullName() const;
    virtual QIcon icon() const;

    virtual void apply();
    virtual void reset();
    virtual void defaults()
    {}
  private:
    class QCheckBox *cbAutoSyncronize;
    class QCheckBox *cbSetEditor;
    KateKonsolePlugin *mPlugin;
};
#endif
// kate: space-indent on; indent-width 2; replace-tabs on;

