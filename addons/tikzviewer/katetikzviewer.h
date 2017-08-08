/* This file is part of the KDE project
   Copyright (C) 2017 Joao Carreira <jfmcarreira gmail com>

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

#ifndef KATETIKZVIEWER_H
#define KATETIKZVIEWER_H

#include <KTextEditor/ConfigPage>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>

#include <KXMLGUIClient>

class KateKonsolePlugin;
class KateTikzViewerPlugin;
class KateTikzViewer;


namespace KParts
{
class ReadOnlyPart;
}


class KateTikzViewer: public QWidget, public KXMLGUIClient
{
  Q_OBJECT

public:
  /**
   * Constructor.
   */
  KateTikzViewer (KateTikzViewerPlugin* plugin, KTextEditor::MainWindow *mw, QWidget* parent);

  /**
   * Virtual destructor.
   */
  ~KateTikzViewer ();

  void readConfig();

public Q_SLOTS:
  void slotViewChanged(KTextEditor::View *);

protected:
  /**
   * the tikz viewer get shown
   * @param ev show event
   */
  void showEvent(QShowEvent *ev);

private:

  /**
   * construct tikz viewer if needed
   */
  void loadIfNeeded();

  void reload();

  /**
   * main window of this console
   */
  KTextEditor::MainWindow *m_mw;

  KateTikzViewerPlugin *m_plugin;

  /**
   * KTikz part
   */
  KParts::ReadOnlyPart *m_part;

  /**
   * Toolview for this console
   */
  QWidget *m_toolView;

  KTextEditor::View *m_view;



};


#endif // KATETIKZVIEWER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
