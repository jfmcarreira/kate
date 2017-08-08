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

#include "katetikzviewer.h"
#include "katetikzviewerplugin.h"

#include <QIcon>
#include <KLocalizedString>
#include <QVBoxLayout>
#include <KService>
#include <QString>
#include <KXMLGUIFactory>
#include <KPluginLoader>
#include <KParts/ReadOnlyPart>

KateTikzViewer::KateTikzViewer (KateTikzViewerPlugin* plugin, KTextEditor::MainWindow *mw, QWidget* parent)
  : QWidget(parent)
  , m_mw (mw)
  , m_plugin(plugin)
  , m_part (0)
  , m_view(0)
{

  KXMLGUIClient::setComponentName (QStringLiteral("katetizkviewer"), i18n ("Kate TikZ Viewer"));

  m_mw->guiFactory()->addClient (this);

  new QVBoxLayout(this);
  layout()->setContentsMargins(0, 0, 0, 0);



  connect(m_mw, SIGNAL(viewChanged(KTextEditor::View *)), this, SLOT(slotViewChanged(KTextEditor::View *)));
}

KateTikzViewer::~KateTikzViewer ()
{
  m_mw->guiFactory()->removeClient (this);
}

void KateTikzViewer::readConfig()
{
}

void KateTikzViewer::showEvent(QShowEvent *)
{
  loadIfNeeded();

  if( isVisible() ) reload();
}

void KateTikzViewer::loadIfNeeded()
{
  if (m_part) return;

  if (!window() || !parentWidget()) return;
  if (!window() || !isVisibleTo(window())) return;

  KPluginFactory *factory = KPluginLoader(QStringLiteral("ktikzpart")).factory();
  if (!factory)
    return;

  m_part = factory->create<KParts::ReadOnlyPart>(m_toolView, this);

  if (!m_part) return;
  layout()->addWidget(m_part->widget());
  m_part->widget()->show();

  //m_mw->guiFactory()->addClient(m_part);
}

void KateTikzViewer::reload()
{
  if (!m_view) return;
  m_part->closeUrl();
  m_part->openUrl(m_view->document()->url());
}

void KateTikzViewer::slotViewChanged(KTextEditor::View *view)
{
  m_view = view;

  if(!isVisible()) return;
  if (!m_part) return;

  reload();

}

// kate: space-indent on; indent-width 2; replace-tabs on;
