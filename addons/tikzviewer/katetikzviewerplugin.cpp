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

#include "katetikzviewerplugin.h"
#include "katetikzviewerconfigpage.h"

#include <KLocalizedString>

//#include <ktexteditor/document.h>
//#include <ktexteditor/view.h>

//#include <QAction>

//#include <QApplication>
//#include <QStyle>
//#include <QIcon>
//#include <QShowEvent>
//#include <QLabel>
//#include <QCheckBox>
//#include <QVBoxLayout>
//#include <QFileInfo>

//#include <KPluginLoader>
#include <KPluginFactory>
//#include <kaboutdata.h>
//#include <kpluginfactory.h>
#include <KAuthorized>
//#include <KConfigGroup>
//#include <KSharedConfig>
//#include <KXMLGUIFactory>
#include <KMessageBox>

K_PLUGIN_FACTORY_WITH_JSON (KateTikzViewerPluginFactory, "katetikzviewerplugin.json", registerPlugin<KateTikzViewerPlugin>();)

KateTikzViewerPlugin::KateTikzViewerPlugin( QObject* parent, const QList<QVariant>& ):
		KTextEditor::Plugin ( parent )
{
}

KateTikzViewerPlugin::~KateTikzViewerPlugin()
{
}

QObject *KateTikzViewerPlugin::createView (KTextEditor::MainWindow *mainWindow)
{
	KateTikzViewerPluginView *view = new KateTikzViewerPluginView (this, mainWindow);
	return view;
}

KTextEditor::ConfigPage *KateTikzViewerPlugin::configPage (int number, QWidget *parent)
{
	if (number != 0)
		return 0;
	return new KateTikzViewerConfigPage(parent, this);
}

void KateTikzViewerPlugin::readConfig()
{
	foreach ( KateTikzViewerPluginView *view, mViews )
		view->readConfig();
}

#include "katetikzviewerplugin.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
