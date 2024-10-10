/*****************************************************************************
 * Copyright (C) 2024 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#include <QHeaderView>

#include "livetable.hpp"
#include "lbcrexception.hpp"

LiveTable::LiveTable(QWidget *parent) : QWidget(parent)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    this->setWindowState(Qt::WindowFullScreen);
#endif

    // Make our window without panels
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
    this->setFocusPolicy(Qt::NoFocus);

    // Stretch the second column and double the width of the timings column
    auto *header = this->ui->tableWidget->horizontalHeader();
    this->ui->tableWidget->setColumnWidth(2, 3 * this->ui->tableWidget->columnWidth(0) / 2);
    header->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);

    //this->ui->leftLogo->setGeometry();
    //this->ui->rightLogo->setSizePolicy(QSizePolicy::Fixed);
}

void LiveTable::setRaceInfo(const ChronoRaceData *raceData)
{
    int height;
    int width;
    int lWidth;
    int rWidth;

    QString title = "";
    QString racePlace = "";
    QString raceDate = "";

    QPixmap leftLogo;
    QPixmap rightLogo;

    if (raceData) {
        title = raceData->getEvent();
        racePlace = raceData->getPlace();
        raceDate = QLocale::system().toString(raceData->getDate(), "dddd dd/MM/yyyy");

        leftLogo = raceData->getLeftLogo();
        rightLogo = raceData->getRightLogo();
    }

    QString subTitle = racePlace % (racePlace.isEmpty() ? "" : " - ") % raceDate;

    this->ui->raceTitle->setText(title.isEmpty() ? tr("The Race") : title);
    this->ui->raceInfo->setText(subTitle.isEmpty() ? tr("LBChronoRace") : subTitle);

    height = this->ui->titleLayout->geometry().height();

    /* Prepare left logo */
    if (leftLogo.isNull()) {
        leftLogo = QPixmap(height, height);
        leftLogo.fill(Qt::transparent);
    } else {
        leftLogo = leftLogo.scaledToHeight(height, Qt::SmoothTransformation);
        this->ui->leftLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    lWidth = leftLogo.width();

    /* Prepare right logo */
    if (rightLogo.isNull()) {
        rightLogo = QPixmap(height, height);
        rightLogo.fill(Qt::transparent);
    } else {
        rightLogo = rightLogo.scaledToHeight(height, Qt::SmoothTransformation);
        this->ui->rightLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    rWidth = rightLogo.width();

    /* Set labels width */
    width = (lWidth > rWidth) ? lWidth : rWidth;
    this->ui->leftLogo->setFixedWidth(width);
    this->ui->rightLogo->setFixedWidth(width);

    /* Set the pixmaps */
    this->ui->leftLogo->setPixmap(leftLogo);
    this->ui->rightLogo->setPixmap(rightLogo);
}

void LiveTable::addEntry(uint bib, uint timing) const
{
    removeEntry(bib, timing);

    int rowCount = ui->tableWidget->rowCount();

    ui->tableWidget->sortByColumn(-1, Qt::SortOrder::AscendingOrder);
    ui->tableWidget->setRowCount(rowCount + 1);
    ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(bib)));
    //ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(timing));
    //ui->tableWidget->setItem(rowCount, 2, new QTableWidgetItem(competitor));
    ui->tableWidget->sortByColumn(2, Qt::SortOrder::AscendingOrder);
}

void LiveTable::removeEntry(uint bib, uint timing) const
{
    Q_UNUSED(bib)
    Q_UNUSED(timing)

    // QTableWidgetItem const *item;
    // for (int r = ui->tableWidget->rowCount() - 1; r >= 0; r--) {
    //     if (item = ui->tableWidget->item(r, 0);
    //         !item || item->data(Qt::ItemDataRole::DisplayRole).toString().compare(bib) == 0)
    //         ui->tableWidget->removeRow(r);
    // }
}

void LiveTable::setStartList(QList<Competitor> const &newStartList)
{
    uint bib = 0;

    ui->tableWidget->clearContents();
    this->startList.clear();

    for (auto &competitor : newStartList) {
        bib = competitor.getBib();
        this->startList.insert(bib, competitor);
    }

    auto values = this->startList.values(bib);
    auto count = values.count();
    if (count == 1) {
        this->ui->tableWidget->setColumnCount(3);
        this->ui->tableWidget->setColumnWidth(2, 3 * this->ui->tableWidget->columnWidth(0) / 2);
        this->mode = (values.at(0).getOffset() < 0) ? LiveMode::INDIVIDUAL : LiveMode::CHRONO;
        this->ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Competitor")));
        this->ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Timing")));
    } else if (count > 1) {
        this->ui->tableWidget->setColumnCount(3 + static_cast<int>(count));
        this->ui->tableWidget->setColumnWidth(2 + static_cast<int>(count), 3 * this->ui->tableWidget->columnWidth(0) / 2);
        this->mode = (values.at(0).getOffset() < 0) ? LiveMode::RELAY : LiveMode::CHRONO_RELAY;
        this->ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Team")));
        this->ui->tableWidget->setHorizontalHeaderItem(2 + static_cast<int>(count), new QTableWidgetItem(tr("Timing")));
        do {
            this->ui->tableWidget->setColumnWidth(1 + static_cast<int>(count), 3 * this->ui->tableWidget->columnWidth(0) / 2);
            this->ui->tableWidget->setHorizontalHeaderItem(1 + static_cast<int>(count), new QTableWidgetItem(tr("Leg %1").arg(count)));
        } while (--count);
    } else {
        throw(ChronoRaceException(tr("Enter competitors to use the Live View")));
    }
}
