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

#include "katetikzviewerconfigpage.h"

#include <KLocalizedString>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <QAction>

#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QShowEvent>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QFileInfo>

#include <KPluginLoader>
#include <KPluginFactory>
#include <kaboutdata.h>
#include <kpluginfactory.h>
#include <KAuthorized>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KXMLGUIFactory>

KateTikzViewerConfigPage::KateTikzViewerConfigPage( QWidget* parent, KateTikzViewerPlugin *plugin )
	: KTextEditor::ConfigPage( parent )
	, mPlugin( plugin )
{
}

QString KateTikzViewerConfigPage::name() const
{
	return i18n("TikZ Preview");
}

QString KateTikzViewerConfigPage::fullName() const
{
	return i18n("TikZ Preview Settings");
}

QIcon KateTikzViewerConfigPage::icon() const
{
	return QIcon::fromTheme(QStringLiteral("ktikz"));
}

void KateTikzViewerConfigPage::apply()
{
	KConfigGroup config(KSharedConfig::openConfig(), "TikzView");
	config.sync();
}

void KateTikzViewerConfigPage::reset()
{
	KConfigGroup config(KSharedConfig::openConfig(), "TikzView");
}

// kate: space-indent on; indent-width 2; replace-tabs on;
