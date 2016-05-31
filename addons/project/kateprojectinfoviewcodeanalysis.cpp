/*  This file is part of the Kate project.
 *
 *  Copyright (C) 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kateprojectinfoviewcodeanalysis.h"
#include "kateprojectpluginview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>

#include <klocalizedstring.h>
#include <kmessagewidget.h>

KateProjectInfoViewCodeAnalysis::KateProjectInfoViewCodeAnalysis(KateProjectPluginView *pluginView, KateProject *project)
    : QWidget()
    , m_pluginView(pluginView)
    , m_project(project)
    , m_messageWidget(0)
    , m_startStopAnalysis(new QPushButton(i18n("Start Analysis...")))
    , m_treeView(new QTreeView())
    , m_model(new QStandardItemModel(m_treeView))
    , m_analyzer(0)
{
    /**
     * default style
     */
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setRootIsDecorated(false);
    m_model->setHorizontalHeaderLabels(QStringList() << i18n("File") << i18n("Line") << i18n("Severity") << i18n("Message"));

    /**
     * attach model
     * kill selection model
     */
    QItemSelectionModel *m = m_treeView->selectionModel();
    m_treeView->setModel(m_model);
    delete m;

    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(1, Qt::AscendingOrder);
    m_treeView->sortByColumn(2, Qt::AscendingOrder);

    /**
     * layout widget
     */
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->addWidget(m_treeView);
    QHBoxLayout *hlayout = new QHBoxLayout;
    layout->addLayout(hlayout);
    hlayout->setSpacing(0);
    hlayout->addStretch();
    hlayout->addWidget(m_startStopAnalysis);
    setLayout(layout);

    /**
     * connect needed signals
     */
    connect(m_startStopAnalysis, SIGNAL(clicked(bool)), this, SLOT(slotStartStopClicked()));
    connect(m_treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotClicked(const QModelIndex &)));
}

KateProjectInfoViewCodeAnalysis::~KateProjectInfoViewCodeAnalysis()
{
}

void KateProjectInfoViewCodeAnalysis::slotStartStopClicked()
{
    /**
     * get files for cppcheck
     */
    QStringList files = m_project->files().filter(QRegExp(QStringLiteral("\\.(cpp|cxx|cc|c\\+\\+|c|tpp|txx)$")));

    /**
     * clear existing entries
     */
    m_model->removeRows(0, m_model->rowCount(), QModelIndex());

    /**
     * launch cppcheck
     */
    m_analyzer = new QProcess(this);
    m_analyzer->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_analyzer, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(m_analyzer, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(finished(int, QProcess::ExitStatus)));

    QStringList args;
    args << QStringLiteral("-q") << QStringLiteral("--inline-suppr") << QStringLiteral("--enable=all") << QStringLiteral("--template={file}////{line}////{severity}////{message}") << QStringLiteral("--file-list=-");
    m_analyzer->start(QStringLiteral("cppcheck"), args);

    if (m_messageWidget) {
        delete m_messageWidget;
        m_messageWidget = 0;
    }

    if (!m_analyzer->waitForStarted()) {
        m_messageWidget = new KMessageWidget();
        m_messageWidget->setCloseButtonVisible(true);
        m_messageWidget->setMessageType(KMessageWidget::Warning);
        m_messageWidget->setWordWrap(false);
        m_messageWidget->setText(i18n("Please install 'cppcheck'."));
        static_cast<QVBoxLayout *>(layout())->insertWidget(0, m_messageWidget);
        m_messageWidget->animatedShow();
        return;
    }
    /**
     * write files list and close write channel
     */
    m_analyzer->write(files.join(QStringLiteral("\n")).toLocal8Bit());
    m_analyzer->closeWriteChannel();
}

void KateProjectInfoViewCodeAnalysis::slotReadyRead()
{
    /**
     * get results of analysis
     */
    while (m_analyzer->canReadLine()) {
        /**
         * get one line, split it, skip it, if too few elements
         */
        QString line = QString::fromLocal8Bit(m_analyzer->readLine());
        QStringList elements = line.split(QRegExp(QStringLiteral("////")), QString::SkipEmptyParts);
        if (elements.size() < 4) {
            continue;
        }

        /**
         * feed into model
         */
        QList<QStandardItem *> items;
        QStandardItem *fileNameItem = new QStandardItem(QFileInfo(elements[0]).fileName());
        fileNameItem->setToolTip(elements[0]);
        items << fileNameItem;
        items << new QStandardItem(elements[1]);
        items << new QStandardItem(elements[2]);
        const auto message = elements[3].simplified();
        auto messageItem = new QStandardItem(message);
        messageItem->setToolTip(message);
        items << messageItem;
        m_model->appendRow(items);
    }

    /**
     * tree view polish ;)
     */
    m_treeView->resizeColumnToContents(2);
    m_treeView->resizeColumnToContents(1);
    m_treeView->resizeColumnToContents(0);
}

void KateProjectInfoViewCodeAnalysis::slotClicked(const QModelIndex &index)
{
    /**
     * get path
     */
    QString filePath = m_model->item(index.row(), 0)->toolTip();
    if (filePath.isEmpty()) {
        return;
    }

    /**
     * create view
     */
    KTextEditor::View *view = m_pluginView->mainWindow()->openUrl(QUrl::fromLocalFile(filePath));
    if (!view) {
        return;
    }

    /**
     * set cursor, if possible
     */
    int line = m_model->item(index.row(), 1)->text().toInt();
    if (line >= 1) {
        view->setCursorPosition(KTextEditor::Cursor(line - 1, 0));
    }
}

void KateProjectInfoViewCodeAnalysis::finished(int exitCode, QProcess::ExitStatus)
{
  m_messageWidget = new KMessageWidget();
  m_messageWidget->setCloseButtonVisible(true);
  m_messageWidget->setWordWrap(false);

  if (exitCode == 0) {
    m_messageWidget->setMessageType(KMessageWidget::Information);
    m_messageWidget->setText(i18n("Analysis finished."));
  } else {
    // unfortunately, output was eaten by slotReadyRead()
    // TODO: get stderr output, show it here
    m_messageWidget->setMessageType(KMessageWidget::Warning);
    m_messageWidget->setText(i18n("Analysis failed!"));
  }
  static_cast<QVBoxLayout*>(layout ())->insertWidget(0, m_messageWidget);
  m_messageWidget->animatedShow ();
}
