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

#include "katetikzviewerpluginview.h"
#include "katetikzviewer.h"
#include "katetikzviewerplugin.h"

#include <QIcon>
#include <KLocalizedString>
#include <KService>
#include <QString>

KateTikzViewerPluginView::KateTikzViewerPluginView (KateTikzViewerPlugin* plugin, KTextEditor::MainWindow *mainWindow)
	: QObject(mainWindow)
	, m_plugin(plugin)
{
	// init console
	QWidget *toolView = mainWindow->createToolView (plugin, QStringLiteral("kate_private_plugin_katetikzviewerplugin"),
																		 KTextEditor::MainWindow::Right,
																		 QIcon::fromTheme(QStringLiteral("ktikz")), i18n("TikZ Viewer"));

	m_viewer = new KateTikzViewer(m_plugin, mainWindow, toolView);

	// register this view
	m_plugin->mViews.append ( this );
}

KateTikzViewerPluginView::~KateTikzViewerPluginView ()
{
	// unregister this view
	m_plugin->mViews.removeAll (this);

	// cleanup, kill toolview + console
	QWidget *toolview = m_viewer->parentWidget();
	delete m_viewer;
	delete toolview;
}

void KateTikzViewerPluginView::readConfig()
{
	//m_viewer->readConfig();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
