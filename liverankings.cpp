/*****************************************************************************
 * Copyright (C) 2025 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include <QLocale>
#include <QHeaderView>

#include "liverankings.hpp"
#include "crsettings.hpp"

LiveRankings::LiveRankings(QWidget *parent) : QWidget(parent, Qt::Window)
{
    ui->setupUi(this);

#if defined(Q_OS_WIN)
    this->setWindowState(Qt::WindowFullScreen);
#endif

    // Make the window without panels
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
    this->setFocusPolicy(Qt::NoFocus);

    highProxyModel->setSourceModel(model.data());
    this->ui->highTable->setModel(highProxyModel.data());
    lowProxyModel->setSourceModel(model.data());
    this->ui->lowTable->setModel(lowProxyModel.data());
    model->setSortRole(Qt::UserRole);

    highProxyModel->setMinRow(0);
    highProxyModel->setMaxRow(2);
    lowProxyModel->setMinRow(3);

    this->ui->highTable->setSortingEnabled(false);
    this->ui->highTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    this->ui->highTable->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
    this->ui->highTable->verticalHeader()->setVisible(true);
    this->ui->lowTable->setSortingEnabled(false);
    this->ui->lowTable->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
    this->ui->lowTable->verticalHeader()->setVisible(true);
}

void LiveRankings::setLiveScreen(QScreen const *screen)
{
    this->liveScreen = screen;
}

void LiveRankings::setHeaderData(QString const &title, QString const &place, QDate const &date, QPixmap const &leftLogo, QPixmap const &rightLogo)
{
    if (this->liveScreen == Q_NULLPTR)
        return;

    this->ui->raceTitle->setText(title);

    QString lrRaceDate = QLocale::system().toString(date, (this->liveScreen->availableSize().width() < 1280) ? QLocale::FormatType::ShortFormat : QLocale::FormatType::LongFormat);
    QString lrSubTitle = place % (place.isEmpty() ? "" : " - ") % lrRaceDate;
    this->ui->raceInfo->setText(lrSubTitle);

    int height = this->ui->titleLayout->geometry().height();
    int width;
    int lWidth;
    int rWidth;

    QPixmap lrLeftPixmap;
    QPixmap lrRightPixmap;

    /* Prepare left logo */
    if (leftLogo.isNull()) {
        lrLeftPixmap = QPixmap(height, height);
        lrLeftPixmap.fill(Qt::transparent);
    } else {
        lrLeftPixmap = leftLogo.scaledToHeight(height, Qt::SmoothTransformation);
        this->ui->leftLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    lWidth = lrLeftPixmap.width();

    /* Prepare right logo */
    if (rightLogo.isNull()) {
        lrRightPixmap = QPixmap(height, height);
        lrRightPixmap.fill(Qt::transparent);
    } else {
        lrRightPixmap = rightLogo.scaledToHeight(height, Qt::SmoothTransformation);
        ui->rightLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    rWidth = lrRightPixmap.width();

    /* Set labels width */
    width = (lWidth > rWidth) ? lWidth : rWidth;
    this->ui->leftLogo->setFixedWidth(width);
    this->ui->rightLogo->setFixedWidth(width);

    /* Set the pixmaps */
    this->ui->leftLogo->setPixmap(lrLeftPixmap);
    this->ui->rightLogo->setPixmap(lrRightPixmap);
}

void LiveRankings::scrollToLastItem(QStandardItem const *item)
{
    if (item != Q_NULLPTR) {
        auto const &index = lowProxyModel->mapFromSource(model->indexFromItem(item));
        auto row = index.isValid() ? index.row() : -1;

        if (row < 3)
           this->ui->lowTable->scrollToTop();
        else if (row + 1 == lowProxyModel->rowCount())
           this->ui->lowTable->scrollToBottom();
        else
           this->ui->lowTable->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void LiveRankings::resizeColumns()
{
    if (this->liveScreen == Q_NULLPTR)
        return;

    auto columns = model->columnCount();
    auto margins = this->ui->highTable->contentsMargins();
    auto fullWidth = this->liveScreen->availableSize().width();

    int count = ((columns + 1) / 2) - 1;

    if (count < 1)
        return;

    auto availableWidth = fullWidth - ((columns + 1) * (margins.left() + margins.right()));
    if (count == 1) {
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(0, Qt::Horizontal, tr("Bib", "long"), Qt::DisplayRole);
        this->ui->highTable->verticalHeader()->setMinimumWidth(availableWidth / 30);
        this->ui->highTable->setColumnWidth(0, availableWidth / 10);
        this->ui->lowTable->verticalHeader()->setMinimumWidth(availableWidth / 30);
        this->ui->lowTable->setColumnWidth(0, availableWidth / 10);
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Fixed);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(2, Qt::Horizontal, tr("Timing", "long"), Qt::DisplayRole);
        this->ui->highTable->setColumnWidth(2, 3 * availableWidth / 20);
        this->ui->lowTable->setColumnWidth(2, 3 * availableWidth / 20);
        model->setHeaderData(1, Qt::Horizontal, tr("Competitor", "long"), Qt::DisplayRole);
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
    } else if (fullWidth < 1680) {
        auto bibWidth = availableWidth / 20;
        auto timeWidth = (availableWidth * 56) / 1000;
        auto sNameWidth = (availableWidth - ((3 * bibWidth / 2) + (timeWidth * (count + 1)))) / count;
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(0, Qt::Horizontal, tr("Bib", "short"), Qt::DisplayRole);
        this->ui->highTable->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->highTable->setColumnWidth(0, bibWidth);
        this->ui->lowTable->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->lowTable->setColumnWidth(0, bibWidth);
        do {
            this->ui->highTable->horizontalHeader()->setSectionResizeMode((2 * count) - 1, QHeaderView::ResizeMode::Fixed);
            this->ui->lowTable->horizontalHeader()->setSectionResizeMode((2 * count) - 1, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((2 * count) - 1, Qt::Horizontal, tr("Competitor %1", "short").arg(count), Qt::DisplayRole);
            this->ui->highTable->setColumnWidth((2 * count) - 1, sNameWidth);
            this->ui->lowTable->setColumnWidth((2 * count) - 1, sNameWidth);
            this->ui->highTable->horizontalHeader()->setSectionResizeMode(2 * count, QHeaderView::ResizeMode::Fixed);
            this->ui->lowTable->horizontalHeader()->setSectionResizeMode(2 * count, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData(2 * count, Qt::Horizontal, tr("Timing %1", "short").arg(count), Qt::DisplayRole);
            this->ui->highTable->setColumnWidth(2 * count, timeWidth);
            this->ui->lowTable->setColumnWidth(2 * count, timeWidth);
        } while (--count);
        model->setHeaderData(columns - 1, Qt::Horizontal, tr("Timing", "short"), Qt::DisplayRole);
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(columns - 1, QHeaderView::ResizeMode::Stretch);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(columns - 1, QHeaderView::ResizeMode::Stretch);
    } else {
        auto bibWidth = availableWidth / 20;
        auto timeWidth = (availableWidth * 56) / 1000;
        auto lNameWidth = (availableWidth - ((3 * bibWidth / 2) + (timeWidth * (count + 1)))) / count;
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(0, Qt::Horizontal, (fullWidth < 1920) ? tr("Bib", "short") : tr("Bib", "long"), Qt::DisplayRole);
        this->ui->highTable->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->highTable->setColumnWidth(0, bibWidth);
        this->ui->lowTable->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->lowTable->setColumnWidth(0, bibWidth);
        do {
            this->ui->highTable->horizontalHeader()->setSectionResizeMode((2 * count) - 1, QHeaderView::ResizeMode::Fixed);
            this->ui->lowTable->horizontalHeader()->setSectionResizeMode((2 * count) - 1, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((2 * count) - 1, Qt::Horizontal, tr("Competitor %1", "long").arg(count), Qt::DisplayRole);
            this->ui->highTable->setColumnWidth((2 * count) - 1, lNameWidth);
            this->ui->lowTable->setColumnWidth((2 * count) - 1, lNameWidth);
            this->ui->highTable->horizontalHeader()->setSectionResizeMode(2 * count, QHeaderView::ResizeMode::Fixed);
            this->ui->lowTable->horizontalHeader()->setSectionResizeMode(2 * count, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData(2 * count, Qt::Horizontal, tr("Timing %1", "long").arg(count), Qt::DisplayRole);
            this->ui->highTable->setColumnWidth(2 * count, timeWidth);
            this->ui->lowTable->setColumnWidth(2 * count, timeWidth);
        } while (--count);
        model->setHeaderData(columns - 1, Qt::Horizontal, tr("Timing", "long"), Qt::DisplayRole);
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(columns - 1, QHeaderView::ResizeMode::Stretch);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(columns - 1, QHeaderView::ResizeMode::Stretch);
    }
}

void LiveRankings::activate()
{
    using enum CRSettings::Color;

    QString style = QStringLiteral("background-color: ") + CRSettings::getColor(LiveRankingsBackgroundColor).name();
    this->setStyleSheet(style);
    style = QStringLiteral("font: bold; color: ") + CRSettings::getColor(LiveRankingsTitleColor).name();
    this->ui->raceTitle->setStyleSheet(style);
    style = QStringLiteral("color: ") + CRSettings::getColor(LiveRankingsTextColor).name();
    this->ui->raceInfo->setStyleSheet(style);
    this->ui->highTable->setStyleSheet(style);
    this->ui->lowTable->setStyleSheet(style);

    this->ui->retranslateUi(this);
    QWidget::show();
    resizeColumns();
}

void LiveRankings::demoStep()
{
    if (this->liveScreen == Q_NULLPTR) {
        if (this->demoIndex.isValid())
            this->demoIndex = QModelIndex();
    } else {
        auto const &viewPortRect = this->ui->lowTable->viewport()->rect();
        while (this->demoIndex.isValid()) {
            this->demoIndex = lowProxyModel->index(demoIndex.row() + 1, 0);

            // element is not the bottom one but it is not visible
            if (this->demoIndex.isValid() && !viewPortRect.contains(this->ui->lowTable->visualRect(this->demoIndex)))
                break;
        }

        if (!this->demoIndex.isValid()) {
            this->demoIndex = lowProxyModel->index(0, 0);
            this->ui->lowTable->scrollToTop();
        } else {
            this->ui->lowTable->scrollTo(this->demoIndex, QAbstractItemView::ScrollHint::PositionAtTop);
        }
    }
}

QStandardItemModel *LiveRankings::getModel()
{
    return model.data();
}
