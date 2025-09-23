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

#include "livestartlist.hpp"
#include "crsettings.hpp"

LiveStartList::LiveStartList(QWidget *parent) : QWidget(parent, Qt::Window)
{
    ui->setupUi(this);

#if defined(Q_OS_WIN)
    this->setWindowState(Qt::WindowFullScreen);
#endif

    // Make our window without panels
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
    this->setFocusPolicy(Qt::NoFocus);

    this->ui->table->setModel(model.data());
    model->setSortRole(Qt::UserRole);

    this->ui->table->setSortingEnabled(false);
    this->ui->table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    this->ui->table->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
    this->ui->table->verticalHeader()->setVisible(true);
}

void LiveStartList::setLiveScreen(QScreen const *screen)
{
    this->liveScreen = screen;
}

void LiveStartList::setHeaderData(QString const &title, QString const &place, QDate const &date, QPixmap const &leftLogo, QPixmap const &rightLogo)
{
    if (this->liveScreen == Q_NULLPTR)
        return;

    this->ui->raceTitle->setText(title);

    QString lsRaceDate = QLocale::system().toString(date, (this->liveScreen->availableSize().width() < 1280) ? QLocale::FormatType::ShortFormat : QLocale::FormatType::LongFormat);
    QString lsSubTitle = place % (place.isEmpty() ? "" : " - ") % lsRaceDate;
    this->ui->raceInfo->setText(lsSubTitle);

    int height = this->ui->titleLayout->geometry().height();
    int width;
    int lWidth;
    int rWidth;

    QPixmap lsLeftPixmap;
    QPixmap lsRightPixmap;

    /* Prepare left logo */
    if (leftLogo.isNull()) {
        lsLeftPixmap = QPixmap(height, height);
        lsLeftPixmap.fill(Qt::transparent);
    } else {
        lsLeftPixmap = leftLogo.scaledToHeight(height, Qt::SmoothTransformation);
        this->ui->leftLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    lWidth = lsLeftPixmap.width();

    /* Prepare right logo */
    if (rightLogo.isNull()) {
        lsRightPixmap = QPixmap(height, height);
        lsRightPixmap.fill(Qt::transparent);
    } else {
        lsRightPixmap = rightLogo.scaledToHeight(height, Qt::SmoothTransformation);
        ui->rightLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    rWidth = lsRightPixmap.width();

    /* Set labels width */
    width = (lWidth > rWidth) ? lWidth : rWidth;
    this->ui->leftLogo->setFixedWidth(width);
    this->ui->rightLogo->setFixedWidth(width);

    /* Set the pixmaps */
    this->ui->leftLogo->setPixmap(lsLeftPixmap);
    this->ui->rightLogo->setPixmap(lsRightPixmap);
}

void LiveStartList::resizeColumns()
{
    if (this->liveScreen != Q_NULLPTR) {
        auto columns = model->columnCount();
        auto margins = this->ui->table->contentsMargins();
        auto fullWidth = this->liveScreen->availableSize().width();
        auto availableWidth = fullWidth - ((columns + 1) * (margins.left() + margins.right()));
        if (this->timeColumns)
            this->resizeColumns2(columns, fullWidth, availableWidth);
        else
            this->resizeColumns1(columns, fullWidth, availableWidth);
    }
}

void LiveStartList::resizeColumns1(int columns, int fullWidth, int availableWidth)
{
    int count = (columns - 1) / 3;

    if (count < 1)
        return;

    if (count == 1) {
        this->ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(0, Qt::Horizontal, tr("Bib", "long"), Qt::DisplayRole);
        this->ui->table->verticalHeader()->setMinimumWidth(availableWidth / 30);
        this->ui->table->setColumnWidth(0, availableWidth / 10);
        this->ui->table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(2, Qt::Horizontal, tr("Sex", "long"), Qt::DisplayRole);
        this->ui->table->setColumnWidth(2, availableWidth / 20);
        this->ui->table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(3, Qt::Horizontal, tr("Year", "long"), Qt::DisplayRole);
        this->ui->table->setColumnWidth(3, availableWidth / 10);
        model->setHeaderData(1, Qt::Horizontal, tr("Competitor", "long"), Qt::DisplayRole);
        this->ui->table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
    } else if (fullWidth < 1680) {
        auto bibWidth = availableWidth / 20;
        auto sNameWidth = (availableWidth - bibWidth - (count * (3 * bibWidth / 2))) / count;
        model->setHeaderData(0, Qt::Horizontal, tr("Bib", "short"), Qt::DisplayRole);
        this->ui->table->verticalHeader()->setMinimumWidth(bibWidth / 2);
        do {
            this->ui->table->horizontalHeader()->setSectionResizeMode((3 * count) - 2, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((3 * count) - 2, Qt::Horizontal, tr("Competitor %1", "short").arg(count), Qt::DisplayRole);
            this->ui->table->setColumnWidth((3 * count) - 2, sNameWidth);
            this->ui->table->horizontalHeader()->setSectionResizeMode((3 * count) - 1, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((3 * count) - 1, Qt::Horizontal, tr("Sex", "short"), Qt::DisplayRole);
            this->ui->table->setColumnWidth((3 * count) - 1, bibWidth / 2);
            this->ui->table->horizontalHeader()->setSectionResizeMode(3 * count, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData(3 * count, Qt::Horizontal, tr("Year", "short"), Qt::DisplayRole);
            this->ui->table->setColumnWidth(3 * count, bibWidth);
        } while (--count);
        this->ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    } else {
        auto bibWidth = availableWidth / 20;
        auto lNameWidth = (availableWidth - bibWidth - (count * (3 * bibWidth / 2))) / count;
        model->setHeaderData(0, Qt::Horizontal, (fullWidth < 1920) ? tr("Bib", "short") : tr("Bib", "long"), Qt::DisplayRole);
        this->ui->table->verticalHeader()->setMinimumWidth(bibWidth / 2);
        do {
            this->ui->table->horizontalHeader()->setSectionResizeMode((3 * count) - 2, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((3 * count) - 2, Qt::Horizontal, tr("Competitor %1", "long").arg(count), Qt::DisplayRole);
            this->ui->table->setColumnWidth((3 * count) - 2, lNameWidth);
            this->ui->table->horizontalHeader()->setSectionResizeMode((3 * count) - 1, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((3 * count) - 1, Qt::Horizontal, (fullWidth < 1920) ? tr("Sex", "short") : tr("Sex", "long"), Qt::DisplayRole);
            this->ui->table->setColumnWidth((3 * count) - 1, bibWidth / 2);
            this->ui->table->horizontalHeader()->setSectionResizeMode(3 * count, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData(3 * count, Qt::Horizontal, (fullWidth < 1920) ? tr("Year", "short") : tr("Year", "long"), Qt::DisplayRole);
            this->ui->table->setColumnWidth(3 * count, bibWidth);
        } while (--count);
        this->ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    }
}

void LiveStartList::resizeColumns2(int columns, int fullWidth, int availableWidth)
{
    int count = (columns - 1) / 4;

    if (count < 1)
        return;

    if (count == 1) {
        this->ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(0, Qt::Horizontal, tr("Bib", "long"), Qt::DisplayRole);
        this->ui->table->verticalHeader()->setMinimumWidth(availableWidth / 30);
        this->ui->table->setColumnWidth(0, availableWidth / 10);
        this->ui->table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(2, Qt::Horizontal, tr("Sex", "long"), Qt::DisplayRole);
        this->ui->table->setColumnWidth(2, availableWidth / 10);
        this->ui->table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(3, Qt::Horizontal, tr("Year", "long"), Qt::DisplayRole);
        this->ui->table->setColumnWidth(3, availableWidth / 10);
        this->ui->table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeMode::Fixed);
        model->setHeaderData(4, Qt::Horizontal, tr("Start Time", "long"), Qt::DisplayRole);
        this->ui->table->setColumnWidth(4, 3 * availableWidth / 20);
        model->setHeaderData(1, Qt::Horizontal, tr("Competitor", "long"), Qt::DisplayRole);
        this->ui->table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
    } else if (fullWidth < 1680) {
        auto bibWidth = availableWidth / 20;
        auto startWidth = (availableWidth * 64) / 1000;
        auto sNameWidth = (availableWidth - bibWidth - (((3 * bibWidth / 2) + startWidth) * count)) / count;
        model->setHeaderData(0, Qt::Horizontal, tr("Bib", "short"), Qt::DisplayRole);
        this->ui->table->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->table->setColumnWidth(0, bibWidth);
        do {
            this->ui->table->horizontalHeader()->setSectionResizeMode((4 * count) - 3, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((4 * count) - 3, Qt::Horizontal, tr("Competitor %1", "short").arg(count), Qt::DisplayRole);
            this->ui->table->setColumnWidth((4 * count) - 3, sNameWidth);
            this->ui->table->horizontalHeader()->setSectionResizeMode((4 * count) - 2, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((4 * count) - 2, Qt::Horizontal, tr("Sex", "short"), Qt::DisplayRole);
            this->ui->table->setColumnWidth((4 * count) - 2, bibWidth / 2);
            this->ui->table->horizontalHeader()->setSectionResizeMode((4 * count) - 1, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((4 * count) - 1, Qt::Horizontal, tr("Year", "short"), Qt::DisplayRole);
            this->ui->table->setColumnWidth((4 * count) - 1, bibWidth);
            this->ui->table->horizontalHeader()->setSectionResizeMode(4 * count, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData(4 * count, Qt::Horizontal, tr("Start Time", "short"), Qt::DisplayRole);
            this->ui->table->setColumnWidth(4 * count, startWidth);
        } while (--count);
        this->ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    } else {
        auto bibWidth = availableWidth / 20;
        auto startWidth = (availableWidth * 64) / 1000;
        auto lNameWidth = (availableWidth - bibWidth - (((3 * bibWidth / 2) + startWidth) * count)) / count;
        model->setHeaderData(0, Qt::Horizontal, (fullWidth < 1920) ? tr("Bib", "short") : tr("Bib", "long"), Qt::DisplayRole);
        this->ui->table->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->table->setColumnWidth(0, bibWidth);
        do {
            this->ui->table->horizontalHeader()->setSectionResizeMode((4 * count) - 3, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((4 * count) - 3, Qt::Horizontal, tr("Competitor %1", "long").arg(count), Qt::DisplayRole);
            this->ui->table->setColumnWidth((4 * count) - 3, lNameWidth);
            this->ui->table->horizontalHeader()->setSectionResizeMode((4 * count) - 2, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((4 * count) - 2, Qt::Horizontal, (fullWidth < 1920) ? tr("Sex", "short") : tr("Sex", "long"), Qt::DisplayRole);
            this->ui->table->setColumnWidth((4 * count) - 2, bibWidth / 2);
            this->ui->table->horizontalHeader()->setSectionResizeMode((4 * count) - 1, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData((4 * count) - 1, Qt::Horizontal, (fullWidth < 1920) ? tr("Year", "short") : tr("Year", "long"), Qt::DisplayRole);
            this->ui->table->setColumnWidth((4 * count) - 1, bibWidth);
            this->ui->table->horizontalHeader()->setSectionResizeMode(4 * count, QHeaderView::ResizeMode::Fixed);
            model->setHeaderData(4 * count, Qt::Horizontal, tr("Start Time", "long"), Qt::DisplayRole);
            this->ui->table->setColumnWidth(4 * count, startWidth);
        } while (--count);
        this->ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    }
}

void LiveStartList::activate()
{
    using enum CRSettings::Color;

    QString style = QStringLiteral("background-color: ") + CRSettings::getColor(LiveStartListBackgroundColor).name();
    this->setStyleSheet(style);
    style = QStringLiteral("font: bold; color: ") + CRSettings::getColor(LiveStartListTitleColor).name();
    this->ui->raceTitle->setStyleSheet(style);
    style = QStringLiteral("color: ") + CRSettings::getColor(LiveStartListTextColor).name();
    this->ui->raceInfo->setStyleSheet(style);
    this->ui->table->setStyleSheet(style);

    this->ui->retranslateUi(this);
    QWidget::show();
    resizeColumns();
}

void LiveStartList::demoStep()
{
    if (this->liveScreen == Q_NULLPTR) {
        if (this->demoIndex.isValid())
            this->demoIndex = QModelIndex();
    } else {
        auto const &viewPortRect = this->ui->table->viewport()->rect();
        while (this->demoIndex.isValid()) {
            this->demoIndex = model->index(demoIndex.row() + 1, 0);

            // element is not the bottom one but it is not visible
            if (this->demoIndex.isValid() && !viewPortRect.contains(this->ui->table->visualRect(this->demoIndex)))
                break;
        }

        if (!this->demoIndex.isValid()) {
            this->demoIndex = model->index(0, 0);
            this->ui->table->scrollToTop();
        } else {
            this->ui->table->scrollTo(this->demoIndex, QAbstractItemView::ScrollHint::PositionAtTop);
        }
    }
}

void LiveStartList::setTimeColumns(bool newTimeColumns)
{
    timeColumns = newTimeColumns;
}

QStandardItemModel *LiveStartList::getModel()
{
    return model.data();
}
