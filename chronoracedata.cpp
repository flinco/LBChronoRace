/*****************************************************************************
 * Copyright (C) 2021 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#include <QFileDialog>
#include <QFileInfo>

#include "lbchronorace.h"
#include "chronoracedata.h"

ChronoRaceData::ChronoRaceData(QWidget *parent) : QDialog(parent) {

    this->stringFields.resize(static_cast<int>(StringField::NUM_OF_FIELDS));

    ui->setupUi(this);

    ui->leftLogo->setStyleSheet("QLabel { background-color : white; }");
    ui->rightLogo->setStyleSheet("QLabel { background-color : white; }");
    ui->sponsorLogo1->setStyleSheet("QLabel { background-color : white; }");
    ui->sponsorLogo2->setStyleSheet("QLabel { background-color : white; }");
    ui->sponsorLogo3->setStyleSheet("QLabel { background-color : white; }");
    ui->sponsorLogo4->setStyleSheet("QLabel { background-color : white; }");

    ui->raceType->setCurrentIndex(this->raceTypeIdx);
    this->stringFields[static_cast<int>(StringField::RACE_TYPE)] = ui->raceType->currentText();

    ui->results->setCurrentIndex(this->resultsIdx);
    this->stringFields[static_cast<int>(StringField::RESULTS)] = ui->results->currentText();

    this->leftLogo.uiElement = ui->leftLogo;
    this->rightLogo.uiElement = ui->rightLogo;
    this->sponsorLogo1.uiElement = ui->sponsorLogo1;
    this->sponsorLogo2.uiElement = ui->sponsorLogo2;
    this->sponsorLogo3.uiElement = ui->sponsorLogo3;
    this->sponsorLogo4.uiElement = ui->sponsorLogo4;

    QObject::connect(ui->loadLeftLogo, &QPushButton::clicked, &this->leftLogo, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeLeftLogo, &QPushButton::clicked, &this->leftLogo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->leftLogo, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->leftLogo, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::deleteLogo);

    QObject::connect(ui->loadRightLogo, &QPushButton::clicked, &this->rightLogo, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeRightLogo, &QPushButton::clicked, &this->rightLogo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->rightLogo, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->rightLogo, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::deleteLogo);

    QObject::connect(ui->loadSponsorLogo1, &QPushButton::clicked, &this->sponsorLogo1, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeSponsorLogo1, &QPushButton::clicked, &this->sponsorLogo1, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->sponsorLogo1, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->sponsorLogo1, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::deleteLogo);

    QObject::connect(ui->loadSponsorLogo2, &QPushButton::clicked, &this->sponsorLogo2, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeSponsorLogo2, &QPushButton::clicked, &this->sponsorLogo2, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->sponsorLogo2, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->sponsorLogo2, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::deleteLogo);

    QObject::connect(ui->loadSponsorLogo3, &QPushButton::clicked, &this->sponsorLogo3, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeSponsorLogo3, &QPushButton::clicked, &this->sponsorLogo3, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->sponsorLogo3, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->sponsorLogo3, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::deleteLogo);

    QObject::connect(ui->loadSponsorLogo4, &QPushButton::clicked, &this->sponsorLogo4, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeSponsorLogo4, &QPushButton::clicked, &this->sponsorLogo4, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->sponsorLogo4, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->sponsorLogo4, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::deleteLogo);
}

QDataStream &operator<<(QDataStream &out, ChronoRaceData const &data)
{
    out << data.leftLogo.pixmap
        << data.rightLogo.pixmap
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::EVENT)]
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::PLACE)]
        << data.date
        << data.startTime
        << qint32(data.raceTypeIdx)
        << qint32(data.resultsIdx)
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::REFEREE)]
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_1)]
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_2)]
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_3)]
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::ORGANIZATION)]
        << data.sponsorLogo1.pixmap
        << data.sponsorLogo2.pixmap
        << data.sponsorLogo3.pixmap
        << data.sponsorLogo4.pixmap
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::LENGTH)]
        << data.stringFields[static_cast<int>(ChronoRaceData::StringField::ELEVATION_GAIN)];

    return out;
}

QDataStream &operator>>(QDataStream &in, ChronoRaceData &data)
{
    qint32 raceTypeIdx32;
    qint32 resultsIdx32;

    in >> data.leftLogo.pixmap
       >> data.rightLogo.pixmap
       >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::EVENT)]
       >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::PLACE)]
       >> data.date
       >> data.startTime
       >> raceTypeIdx32
       >> resultsIdx32
       >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::REFEREE)]
       >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_1)]
       >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_2)]
       >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_3)]
       >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::ORGANIZATION)]
       >> data.sponsorLogo1.pixmap
       >> data.sponsorLogo2.pixmap
       >> data.sponsorLogo3.pixmap
       >> data.sponsorLogo4.pixmap;

    if (data.binFmt == LBCHRONORACE_BIN_FMT_v2)
        in >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::LENGTH)]
           >> data.stringFields[static_cast<int>(ChronoRaceData::StringField::ELEVATION_GAIN)];

    data.raceTypeIdx = raceTypeIdx32;
    data.resultsIdx = resultsIdx32;

    data.ui->raceType->setCurrentIndex(data.raceTypeIdx);
    data.stringFields[static_cast<int>(ChronoRaceData::StringField::RACE_TYPE)] = data.ui->raceType->currentText();

    data.ui->results->setCurrentIndex(data.resultsIdx);
    data.stringFields[static_cast<int>(ChronoRaceData::StringField::RESULTS)] = data.ui->results->currentText();

    return in;
}

QTextStream &operator<<(QTextStream &out, ChronoRaceData const &data)
{
    static QRegularExpression re("\n|\r\n|\r");

    QStringList organizationLines = data.stringFields[static_cast<int>(ChronoRaceData::StringField::ORGANIZATION)].split(re);
    QString headColumn = QObject::tr("Organization") + ": ";

    for (auto const &line : organizationLines) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << headColumn;
        out.setFieldWidth(0);
        out << line << Qt::endl;
        headColumn.fill(' ');
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::EVENT)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Event") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::EVENT)] << Qt::endl;
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::PLACE)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Place") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::PLACE)] << Qt::endl;
    }

    if (data.date.isValid()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Date") + ": ";
        out.setFieldWidth(0);
        out << data.date.toString("dd/MM/yyyy") << Qt::endl;
    }

    if (data.startTime.isValid()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Start Time") + ": ";
        out.setFieldWidth(0);
        out << data.startTime.toString("H:mm") << Qt::endl;
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::RACE_TYPE)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Race Type") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::RACE_TYPE)] << Qt::endl;
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::LENGTH)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Length") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::LENGTH)] << Qt::endl;
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::ELEVATION_GAIN)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Elevation Gain") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::ELEVATION_GAIN)] << Qt::endl;
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::REFEREE)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Referee") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::REFEREE)] << Qt::endl;
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_1)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Timekeeper 1") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_1)] << Qt::endl;
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_2)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Timekeeper 2") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_2)] << Qt::endl;
    }

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_3)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Timekeeper 3") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::TIMEKEEPER_3)] << Qt::endl;
    }

    out << Qt::endl;

    if (!data.stringFields[static_cast<int>(ChronoRaceData::StringField::RESULTS)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Results") + ": ";
        out.setFieldWidth(0);
        out << data.stringFields[static_cast<int>(ChronoRaceData::StringField::RESULTS)] << Qt::endl;
    }

    return out;
}

void ChronoRaceData::saveRaceData()
{
    this->leftLogo.pixmap = ui->leftLogo->pixmap();
    this->rightLogo.pixmap = ui->rightLogo->pixmap();
    this->stringFields[static_cast<int>(StringField::EVENT)] = ui->event->text();
    this->stringFields[static_cast<int>(StringField::PLACE)] = ui->place->text();
    this->date = ui->date->date();
    this->startTime = ui->startTime->time();
    this->raceTypeIdx = ui->raceType->currentIndex();
    this->stringFields[static_cast<int>(StringField::RACE_TYPE)] = ui->raceType->currentText();
    this->resultsIdx = ui->results->currentIndex();
    this->stringFields[static_cast<int>(StringField::RESULTS)] = ui->results->currentText();
    this->stringFields[static_cast<int>(StringField::REFEREE)] = ui->referee->text();
    this->stringFields[static_cast<int>(StringField::TIMEKEEPER_1)] = ui->timeKeeper1->text();
    this->stringFields[static_cast<int>(StringField::TIMEKEEPER_2)] = ui->timeKeeper2->text();
    this->stringFields[static_cast<int>(StringField::TIMEKEEPER_3)] = ui->timeKeeper3->text();
    this->stringFields[static_cast<int>(StringField::LENGTH)] = ui->length->text();
    this->stringFields[static_cast<int>(StringField::ELEVATION_GAIN)] = ui->elevationGain->text();
    this->stringFields[static_cast<int>(StringField::ORGANIZATION)] = ui->organization->toPlainText();
    this->sponsorLogo1.pixmap = ui->sponsorLogo1->pixmap();
    this->sponsorLogo2.pixmap = ui->sponsorLogo2->pixmap();
    this->sponsorLogo3.pixmap = ui->sponsorLogo3->pixmap();
    this->sponsorLogo4.pixmap = ui->sponsorLogo4->pixmap();
}

void ChronoRaceData::restoreRaceData() const
{
    ui->leftLogo->setPixmap(this->leftLogo.pixmap);
    ui->rightLogo->setPixmap(this->rightLogo.pixmap);
    ui->event->setText(this->stringFields[static_cast<int>(StringField::EVENT)]);
    ui->place->setText(this->stringFields[static_cast<int>(StringField::PLACE)]);
    ui->date->setDate(this->date);
    ui->startTime->setTime(this->startTime);
    ui->raceType->setCurrentIndex(this->raceTypeIdx);
    ui->results->setCurrentIndex(this->resultsIdx);
    ui->referee->setText(this->stringFields[static_cast<int>(StringField::REFEREE)]);
    ui->timeKeeper1->setText(this->stringFields[static_cast<int>(StringField::TIMEKEEPER_1)]);
    ui->timeKeeper2->setText(this->stringFields[static_cast<int>(StringField::TIMEKEEPER_2)]);
    ui->timeKeeper3->setText(this->stringFields[static_cast<int>(StringField::TIMEKEEPER_3)]);
    ui->length->setText(this->stringFields[static_cast<int>(StringField::LENGTH)]);
    ui->elevationGain->setText(this->stringFields[static_cast<int>(StringField::ELEVATION_GAIN)]);
    ui->organization->setPlainText(this->stringFields[static_cast<int>(StringField::ORGANIZATION)]);
    ui->sponsorLogo1->setPixmap(this->sponsorLogo1.pixmap);
    ui->sponsorLogo2->setPixmap(this->sponsorLogo2.pixmap);
    ui->sponsorLogo3->setPixmap(this->sponsorLogo3.pixmap);
    ui->sponsorLogo4->setPixmap(this->sponsorLogo4.pixmap);
}

void ChronoRaceData::accept()
{
    saveRaceData();
    QDialog::accept();
}

void ChronoRaceData::reject()
{
    restoreRaceData();
    QDialog::reject();
}

void ChronoRaceData::show()
{
    ui->retranslateUi(this);
    restoreRaceData();
    this->setWindowModality(Qt::ApplicationModal);
    QDialog::show();
}

void ChronoRaceData::setBinFormat(uint format)
{
    this->binFmt = format;
}

void ChronoRaceData::loadLogo(QLabel *label)
{
    if (label) {
        QString logoFileName = QFileDialog::getOpenFileName(this, tr("Select Logo"),
            LBChronoRace::lastSelectedPath.absolutePath(), tr("Images (*.png *.xpm *.jpg *.gif);;All Files (*)"));
        if (!logoFileName.isEmpty()) {
            auto newLogo = QPixmap(logoFileName);

            if (!newLogo.isNull()) {
                LBChronoRace::lastSelectedPath = QFileInfo(logoFileName).absoluteDir();
                /* get label dimensions */
                //NOSONAR int w = label->width();
                //NOSONAR int h = label->height();

                label->setPixmap(newLogo);
                /* set a scaled pixmap to a w x h window keeping its aspect ratio */
                //NOSONAR label->setPixmap(newLogo.scaled(w, h, Qt::KeepAspectRatio));
            }
        }
    }
}

void ChronoRaceData::deleteLogo(QLabel *label) const
{
    if (label) {
        label->setPixmap(QPixmap());
    }
}

QVector<QPixmap> ChronoRaceData::getSponsorLogos() const
{
    QVector<QPixmap> sponsorLogos;

    if (!this->sponsorLogo1.pixmap.isNull())
        sponsorLogos.push_back(this->sponsorLogo1.pixmap);

    if (!this->sponsorLogo2.pixmap.isNull())
        sponsorLogos.push_back(this->sponsorLogo2.pixmap);

    if (!this->sponsorLogo3.pixmap.isNull())
        sponsorLogos.push_back(this->sponsorLogo3.pixmap);

    if (!this->sponsorLogo4.pixmap.isNull())
        sponsorLogos.push_back(this->sponsorLogo4.pixmap);

    return sponsorLogos;
}

QString ChronoRaceData::getElevationGain() const
{
    return this->stringFields[static_cast<int>(StringField::ELEVATION_GAIN)];
}

QString ChronoRaceData::getLength() const
{
    return this->stringFields[static_cast<int>(StringField::LENGTH)];
}

QString ChronoRaceData::getOrganization() const
{
    return this->stringFields[static_cast<int>(StringField::ORGANIZATION)];
}

QString ChronoRaceData::getTimeKeeper(uint idx) const
{
    switch (idx) {
        case 0:
            return this->stringFields[static_cast<int>(StringField::TIMEKEEPER_1)];
        case 1:
            return this->stringFields[static_cast<int>(StringField::TIMEKEEPER_2)];
        case 2:
            return this->stringFields[static_cast<int>(StringField::TIMEKEEPER_3)];
        default:
            return QString();
    }
}

QString ChronoRaceData::getReferee() const
{
    return this->stringFields[static_cast<int>(StringField::REFEREE)];
}

QString ChronoRaceData::getResults() const
{
    return this->stringFields[static_cast<int>(StringField::RESULTS)];
}

QString ChronoRaceData::getRaceType() const
{
    return this->stringFields[static_cast<int>(StringField::RACE_TYPE)];
}

QTime ChronoRaceData::getStartTime() const
{
    return startTime;
}

QDate ChronoRaceData::getDate() const
{
    return date;
}

QString ChronoRaceData::getPlace() const
{
    return this->stringFields[static_cast<int>(StringField::PLACE)];
}

QPixmap ChronoRaceData::getLeftLogo() const
{
    return this->leftLogo.pixmap;
}

QPixmap ChronoRaceData::getRightLogo() const
{
    return this->rightLogo.pixmap;
}

QString ChronoRaceData::getEvent() const
{
    return this->stringFields[static_cast<int>(StringField::EVENT)];
}
