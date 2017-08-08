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

#ifndef KATETIKZVIEWERPLUGIN_H
#define KATETIKZVIEWERPLUGIN_H

#include "katetikzviewerpluginview.h"

#include <ktexteditor/document.h>
#include <ktexteditor/plugin.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/configpage.h>
#include <KTextEditor/SessionConfigInterface>

class KateTikzViewerPlugin: public KTextEditor::Plugin
{
	Q_OBJECT

friend class KateTikzViewerPluginView;

public:
	explicit KateTikzViewerPlugin( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
	virtual ~KateTikzViewerPlugin();

	QObject *createView (KTextEditor::MainWindow *mainWindow);

	int configPages() const { return 1; }
	KTextEditor::ConfigPage *configPage (int number = 0, QWidget *parent = 0);

	void readConfig();

	QByteArray previousEditorEnv() {return m_previousEditorEnv;}

private:
	QList<KateTikzViewerPluginView*> mViews;
	QByteArray m_previousEditorEnv;
};

#endif // KATETIKZVIEWERPLUGIN_H

// kate: space-indent on; indent-width 2; replace-tabs on;
