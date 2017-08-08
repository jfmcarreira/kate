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

#ifndef KATETIKZVIEWERCONFIGPAGE_H
#define KATETIKZVIEWERCONFIGPAGE_H

#include <ktexteditor/configpage.h>

#include <QCheckBox>
#include <QIcon>
#include <QString>

class KateTikzViewerPlugin;

class KateTikzViewerConfigPage : public KTextEditor::ConfigPage
{
  Q_OBJECT

public:
  explicit KateTikzViewerConfigPage( QWidget* parent = 0, KateTikzViewerPlugin *plugin = 0 );
  virtual ~KateTikzViewerConfigPage()
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
  KateTikzViewerPlugin *mPlugin;
};

#endif // KATETIKZVIEWERCONFIGPAGE_H

// kate: space-indent on; indent-width 2; replace-tabs on;
