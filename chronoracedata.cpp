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

#include "lbchronorace.hpp"
#include "chronoracedata.hpp"
#include "languages.hpp"

ChronoRaceData::ChronoRaceData(QWidget *parent) : QDialog(parent)
{
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
    QObject::connect(&this->leftLogo, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::removeLogo);

    QObject::connect(ui->loadRightLogo, &QPushButton::clicked, &this->rightLogo, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeRightLogo, &QPushButton::clicked, &this->rightLogo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->rightLogo, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->rightLogo, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::removeLogo);

    QObject::connect(ui->loadSponsorLogo1, &QPushButton::clicked, &this->sponsorLogo1, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeSponsorLogo1, &QPushButton::clicked, &this->sponsorLogo1, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->sponsorLogo1, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->sponsorLogo1, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::removeLogo);

    QObject::connect(ui->loadSponsorLogo2, &QPushButton::clicked, &this->sponsorLogo2, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeSponsorLogo2, &QPushButton::clicked, &this->sponsorLogo2, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->sponsorLogo2, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->sponsorLogo2, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::removeLogo);

    QObject::connect(ui->loadSponsorLogo3, &QPushButton::clicked, &this->sponsorLogo3, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeSponsorLogo3, &QPushButton::clicked, &this->sponsorLogo3, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->sponsorLogo3, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->sponsorLogo3, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::removeLogo);

    QObject::connect(ui->loadSponsorLogo4, &QPushButton::clicked, &this->sponsorLogo4, &ChronoRaceLogo::loadLogo);
    QObject::connect(ui->removeSponsorLogo4, &QPushButton::clicked, &this->sponsorLogo4, &ChronoRaceLogo::removeLogo);
    QObject::connect(&this->sponsorLogo4, &ChronoRaceLogo::logoLoaded, this, &ChronoRaceData::loadLogo);
    QObject::connect(&this->sponsorLogo4, &ChronoRaceLogo::logoRemoved, this, &ChronoRaceData::removeLogo);

    ui->nameComposition->setCurrentIndex(this->nameCompositionIdx);
    this->stringFields[static_cast<int>(StringField::NAME_COMPOSITION)] = ui->nameComposition->currentText();

    /* NB: The following line can be removed or commented out to allow for millisecond-accurate timing;
     *     however, this requires changing the layout of the PDF, as the timing and timing difference
     *     fields are not wide enough to accommodate so many digits. */
    ui->accuracy->removeItem(static_cast<int>(ChronoRaceData::Accuracy::THOUSANDTH));
    ui->accuracy->setCurrentIndex(this->accuracyIdx);
    this->stringFields[static_cast<int>(StringField::ACCURACY)] = ui->accuracy->currentText();

    Languages::loadMenu(ui->language, QStringLiteral("lbrankings"));
    this->translators << Q_NULLPTR; // empty translator for the default combo box entry
    // create all the translators
    for (int index = 1; index < this->ui->language->count(); ++index) {
        auto locale = QLocale(QLocale::codeToLanguage(this->ui->language->itemData(index).toString()));
        while (this->translators.count() <= index)
            this->translators << new QTranslator;
        if (!this->translators[index]->load(locale, QStringLiteral("lbrankings"), QStringLiteral("_"), QStringLiteral(":/i18n")))
            this->ui->language->removeItem(index); // remove the unavailable translation
    }
}

QDataStream &ChronoRaceData::crdSerialize(QDataStream &out)
{
    using enum ChronoRaceData::StringField;

    out << this->leftLogo.pixmap
        << this->rightLogo.pixmap
        << this->stringFields[static_cast<int>(EVENT)]
        << this->stringFields[static_cast<int>(PLACE)]
        << this->raceDate
        << this->startTime
        << qint32(this->raceTypeIdx)
        << qint32(this->resultsIdx)
        << this->stringFields[static_cast<int>(REFEREE)]
        << this->stringFields[static_cast<int>(TIMEKEEPER_1)]
        << this->stringFields[static_cast<int>(TIMEKEEPER_2)]
        << this->stringFields[static_cast<int>(TIMEKEEPER_3)]
        << this->stringFields[static_cast<int>(ORGANIZATION)]
        << this->sponsorLogo1.pixmap
        << this->sponsorLogo2.pixmap
        << this->sponsorLogo3.pixmap
        << this->sponsorLogo4.pixmap
        << this->stringFields[static_cast<int>(LENGTH)]
        << this->stringFields[static_cast<int>(ELEVATION_GAIN)]
        << quint32(this->nameCompositionIdx)
        << quint32(this->accuracyIdx)
        << this->ui->language->itemData(this->languageIdx).toString();
        /* N.B.:  do not save the index for 'language' as it may not correspond to
         * the index that language has in a different instance of the software. */

    this->dirty = false;

    return out;
}

QDataStream &ChronoRaceData::crdDeserialize(QDataStream &in)
{
    qint32 raceTypeIdx32;
    qint32 resultsIdx32;
    qint32 nameCompositionIdx32 = 0;
    qint32 accuracyIdx32 = 0;

    int langIndex;
    QString langCode;

    using enum ChronoRaceData::StringField;

    in >> this->leftLogo.pixmap
       >> this->rightLogo.pixmap
       >> this->stringFields[static_cast<int>(EVENT)]
       >> this->stringFields[static_cast<int>(PLACE)]
       >> this->raceDate
       >> this->startTime
       >> raceTypeIdx32
       >> resultsIdx32
       >> this->stringFields[static_cast<int>(REFEREE)]
       >> this->stringFields[static_cast<int>(TIMEKEEPER_1)]
       >> this->stringFields[static_cast<int>(TIMEKEEPER_2)]
       >> this->stringFields[static_cast<int>(TIMEKEEPER_3)]
       >> this->stringFields[static_cast<int>(ORGANIZATION)]
       >> this->sponsorLogo1.pixmap
       >> this->sponsorLogo2.pixmap
       >> this->sponsorLogo3.pixmap
       >> this->sponsorLogo4.pixmap;

    if (LBChronoRace::binFormat > LBCHRONORACE_BIN_FMT_v1)
        in >> this->stringFields[static_cast<int>(LENGTH)]
           >> this->stringFields[static_cast<int>(ELEVATION_GAIN)];

    if (LBChronoRace::binFormat > LBCHRONORACE_BIN_FMT_v4)
        in >> nameCompositionIdx32
           >> accuracyIdx32;

    if (LBChronoRace::binFormat > LBCHRONORACE_BIN_FMT_v5)
        in >> langCode;

    langIndex = langCode.isEmpty() ? 0 : this->ui->language->findData(langCode);
    if (langIndex < 0) {
        langIndex = 0;
        emit error(tr("Error: language code %1 not supported").arg(langCode));
    }

    this->raceTypeIdx = raceTypeIdx32;
    this->resultsIdx = resultsIdx32;

    this->nameCompositionIdx = nameCompositionIdx32;
    this->accuracyIdx = accuracyIdx32;

    this->ui->raceType->setCurrentIndex(this->raceTypeIdx);
    this->stringFields[static_cast<int>(RACE_TYPE)] = this->ui->raceType->currentText();

    this->ui->results->setCurrentIndex(this->resultsIdx);
    this->stringFields[static_cast<int>(RESULTS)] = this->ui->results->currentText();

    this->ui->nameComposition->setCurrentIndex(this->nameCompositionIdx);
    this->stringFields[static_cast<int>(NAME_COMPOSITION)] = this->ui->nameComposition->currentText();

    this->ui->accuracy->setCurrentIndex(this->accuracyIdx);
    this->stringFields[static_cast<int>(ACCURACY)] = this->ui->accuracy->currentText();

    this->ui->language->setCurrentIndex(langIndex);
    this->languageIdx = langIndex;

    return in;
}

QTextStream &ChronoRaceData::crdSerialize(QTextStream &out) const
{
    static QRegularExpression re("\n|\r\n|\r");

    using enum ChronoRaceData::StringField;

    QTranslator const *translator = this->getTranslator();

    QStringList organizationLines = this->stringFields[static_cast<int>(ORGANIZATION)].split(re);
    QString headColumn = translator->translate("QObject", "Organization") + ": ";

    for (auto const &line : std::as_const(organizationLines)) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << headColumn;
        out.setFieldWidth(0);
        out << line << Qt::endl;
        headColumn.fill(' ');
    }

    if (!this->stringFields[static_cast<int>(EVENT)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Event") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(EVENT)] << Qt::endl;
    }

    if (!this->stringFields[static_cast<int>(PLACE)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Place") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(PLACE)] << Qt::endl;
    }

    if (this->raceDate.isValid()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Date") + ": ";
        out.setFieldWidth(0);
        out << this->raceDate.toString("dd/MM/yyyy") << Qt::endl;
    }

    if (this->startTime.isValid()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Start Time") + ": ";
        out.setFieldWidth(0);
        out << this->startTime.toString("H:mm") << Qt::endl;
    }

    if (!this->stringFields[static_cast<int>(RACE_TYPE)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Race Type") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(RACE_TYPE)] << Qt::endl;
    }

    if (!this->stringFields[static_cast<int>(LENGTH)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Length") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(LENGTH)] << Qt::endl;
    }

    if (!this->stringFields[static_cast<int>(ELEVATION_GAIN)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Elevation Gain") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(ELEVATION_GAIN)] << Qt::endl;
    }

    if (!this->stringFields[static_cast<int>(REFEREE)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Referee") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(REFEREE)] << Qt::endl;
    }

    if (!this->stringFields[static_cast<int>(TIMEKEEPER_1)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Timekeeper 1") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(TIMEKEEPER_1)] << Qt::endl;
    }

    if (!this->stringFields[static_cast<int>(TIMEKEEPER_2)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Timekeeper 2") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(TIMEKEEPER_2)] << Qt::endl;
    }

    if (!this->stringFields[static_cast<int>(TIMEKEEPER_3)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Timekeeper 3") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(TIMEKEEPER_3)] << Qt::endl;
    }

    out << Qt::endl;

    if (!this->stringFields[static_cast<int>(RESULTS)].isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << translator->translate("QObject", "Results") + ": ";
        out.setFieldWidth(0);
        out << this->stringFields[static_cast<int>(RESULTS)] << Qt::endl;
    }

    return out;
}

void ChronoRaceData::saveRaceData()
{
    using enum ChronoRaceData::StringField;

    if (auto logo = ui->leftLogo->pixmap(); this->leftLogo.pixmap.toImage() != logo.toImage()) {
        this->leftLogo.pixmap = logo;
        this->dirty = true;
    }
    if (auto logo = ui->rightLogo->pixmap(); this->rightLogo.pixmap.toImage() != logo.toImage()) {
        this->rightLogo.pixmap = logo;
        this->dirty = true;
    }
    if (auto text = ui->event->text(); this->stringFields[static_cast<int>(EVENT)] != text) {
        this->stringFields[static_cast<int>(EVENT)] = text;
        this->dirty = true;
    }
    if (auto text = ui->place->text(); this->stringFields[static_cast<int>(PLACE)] != text) {
        this->stringFields[static_cast<int>(PLACE)] = text;
        this->dirty = true;
    }
    if (auto date = ui->date->date(); this->raceDate != date) {
        this->raceDate = date;
        this->dirty = true;
    }
    if (auto time = ui->startTime->time(); this->startTime != time) {
        this->startTime = time;
        this->dirty = true;
    }
    if (auto index = ui->raceType->currentIndex(); this->raceTypeIdx != index) {
        this->raceTypeIdx = index;
        this->stringFields[static_cast<int>(RACE_TYPE)] = ui->raceType->currentText();
        this->dirty = true;
    }
    if (auto index = ui->results->currentIndex(); this->resultsIdx != index) {
        this->resultsIdx = index;
        this->stringFields[static_cast<int>(RESULTS)] = ui->results->currentText();
        this->dirty = true;
    }
    if (auto text = ui->referee->text(); this->stringFields[static_cast<int>(REFEREE)] != text) {
        this->stringFields[static_cast<int>(REFEREE)] = text;
        this->dirty = true;
    }
    if (auto text = ui->timeKeeper1->text(); this->stringFields[static_cast<int>(TIMEKEEPER_1)] != text) {
        this->stringFields[static_cast<int>(TIMEKEEPER_1)] = text;
        this->dirty = true;
    }
    if (auto text = ui->timeKeeper2->text(); this->stringFields[static_cast<int>(TIMEKEEPER_2)] != text) {
        this->stringFields[static_cast<int>(TIMEKEEPER_2)] = text;
        this->dirty = true;
    }
    if (auto text = ui->timeKeeper3->text(); this->stringFields[static_cast<int>(TIMEKEEPER_3)] != text) {
        this->stringFields[static_cast<int>(TIMEKEEPER_3)] = text;
        this->dirty = true;
    }
    if (auto text = ui->length->text(); this->stringFields[static_cast<int>(LENGTH)] != text) {
        this->stringFields[static_cast<int>(LENGTH)] = text;
        this->dirty = true;
    }
    if (auto text = ui->elevationGain->text(); this->stringFields[static_cast<int>(ELEVATION_GAIN)] != text) {
        this->stringFields[static_cast<int>(ELEVATION_GAIN)] = text;
        this->dirty = true;
    }
    if (auto text = ui->organization->toPlainText(); this->stringFields[static_cast<int>(ORGANIZATION)] != text) {
        this->stringFields[static_cast<int>(ORGANIZATION)] = text;
        this->dirty = true;
    }
    if (auto logo = ui->sponsorLogo1->pixmap(); this->sponsorLogo1.pixmap.toImage() != logo.toImage()) {
        this->sponsorLogo1.pixmap = logo;
        this->dirty = true;
    }
    if (auto logo = ui->sponsorLogo2->pixmap(); this->sponsorLogo2.pixmap.toImage() != logo.toImage()) {
        this->sponsorLogo2.pixmap = logo;
        this->dirty = true;
    }
    if (auto logo = ui->sponsorLogo3->pixmap(); this->sponsorLogo3.pixmap.toImage() != logo.toImage()) {
        this->sponsorLogo3.pixmap = logo;
        this->dirty = true;
    }
    if (auto logo = ui->sponsorLogo4->pixmap(); this->sponsorLogo4.pixmap.toImage() != logo.toImage()) {
        this->sponsorLogo4.pixmap = logo;
        this->dirty = true;
    }
    if (auto index = ui->nameComposition->currentIndex(); this->nameCompositionIdx != index) {
        this->nameCompositionIdx = index;
        this->stringFields[static_cast<int>(NAME_COMPOSITION)] = ui->nameComposition->currentText();
        this->dirty = true;
    }
    if (auto index = ui->accuracy->currentIndex(); this->accuracyIdx != index) {
        this->accuracyIdx = index;
        this->stringFields[static_cast<int>(ACCURACY)] = ui->accuracy->currentText();
        this->dirty = true;
    }
    if (auto index = ui->language->currentIndex(); this->languageIdx != index) {
        this->languageIdx = index;
        this->dirty = true;
    }

    emit globalDataChange(static_cast<NameComposition>(this->nameCompositionIdx), static_cast<Accuracy>(this->accuracyIdx));
}

void ChronoRaceData::restoreRaceData()
{
    using enum ChronoRaceData::StringField;

    ui->leftLogo->setPixmap(this->leftLogo.pixmap);
    ui->rightLogo->setPixmap(this->rightLogo.pixmap);
    ui->event->setText(this->stringFields[static_cast<int>(EVENT)]);
    ui->place->setText(this->stringFields[static_cast<int>(PLACE)]);
    ui->date->setDate(this->raceDate);
    ui->startTime->setTime(this->startTime);
    ui->raceType->setCurrentIndex(this->raceTypeIdx);
    ui->results->setCurrentIndex(this->resultsIdx);
    ui->referee->setText(this->stringFields[static_cast<int>(REFEREE)]);
    ui->timeKeeper1->setText(this->stringFields[static_cast<int>(TIMEKEEPER_1)]);
    ui->timeKeeper2->setText(this->stringFields[static_cast<int>(TIMEKEEPER_2)]);
    ui->timeKeeper3->setText(this->stringFields[static_cast<int>(TIMEKEEPER_3)]);
    ui->length->setText(this->stringFields[static_cast<int>(LENGTH)]);
    ui->elevationGain->setText(this->stringFields[static_cast<int>(ELEVATION_GAIN)]);
    ui->organization->setPlainText(this->stringFields[static_cast<int>(ORGANIZATION)]);
    ui->sponsorLogo1->setPixmap(this->sponsorLogo1.pixmap);
    ui->sponsorLogo2->setPixmap(this->sponsorLogo2.pixmap);
    ui->sponsorLogo3->setPixmap(this->sponsorLogo3.pixmap);
    ui->sponsorLogo4->setPixmap(this->sponsorLogo4.pixmap);
    ui->nameComposition->setCurrentIndex(this->nameCompositionIdx);
    ui->accuracy->setCurrentIndex(this->accuracyIdx);
    ui->language->setCurrentIndex(this->languageIdx);
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

QTranslator const *ChronoRaceData::getTranslator() const
{
    QTranslator const *translator = this->translators[this->languageIdx];
    return ((translator == Q_NULLPTR) || translator->isEmpty()) ? Languages::getAppTranslator() : translator;
}

bool ChronoRaceData::isDirty() const
{
    return dirty;
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

void ChronoRaceData::removeLogo(QLabel *label) const
{
    if (label) {
        label->setPixmap(QPixmap());
    }
}

void ChronoRaceData::getGlobalData(ChronoRaceData::NameComposition *gNameComposition, ChronoRaceData::Accuracy *gAccuracy) const
{
    if (gNameComposition != Q_NULLPTR)
        *gNameComposition = static_cast<ChronoRaceData::NameComposition>(this->nameCompositionIdx);

    if (gAccuracy != Q_NULLPTR)
        *gAccuracy = static_cast<ChronoRaceData::Accuracy>(this->accuracyIdx);
}

QTime ChronoRaceData::getStartTime() const
{
    return startTime;
}

QDate ChronoRaceData::getDate() const
{
    return raceDate;
}

QPixmap ChronoRaceData::getField(ChronoRaceData::LogoField field) const
{
    switch (field) {
        using enum ChronoRaceData::LogoField;

        case LEFT:
            return this->leftLogo.pixmap;
        case RIGHT:
            return this->rightLogo.pixmap;
        case SPONSOR_1:
            return this->sponsorLogo1.pixmap;
        case SPONSOR_2:
            return this->sponsorLogo2.pixmap;
        case SPONSOR_3:
            return this->sponsorLogo3.pixmap;
        case SPONSOR_4:
            return this->sponsorLogo4.pixmap;
        default:
            return QPixmap();
            break;
    }
}


QString ChronoRaceData::getField(ChronoRaceData::StringField field) const
{
    switch (field) {
        using enum ChronoRaceData::StringField;

        case EVENT:
            [[fallthrough]];
        case PLACE:
            [[fallthrough]];
        case REFEREE:
            [[fallthrough]];
        case TIMEKEEPER_1:
            [[fallthrough]];
        case TIMEKEEPER_2:
            [[fallthrough]];
        case TIMEKEEPER_3:
            [[fallthrough]];
        case LENGTH:
            [[fallthrough]];
        case ELEVATION_GAIN:
            [[fallthrough]];
        case ORGANIZATION:
            [[fallthrough]];
        case RACE_TYPE:
            [[fallthrough]];
        case RESULTS:
            return this->stringFields[static_cast<qsizetype>(field)];
            break;
        default:
            return QString();
            break;
    }
}

QStringList ChronoRaceData::getFieldValues(ChronoRaceData::IndexField field)
{
    QStringList values;

    switch (field) {
        using enum ChronoRaceData::IndexField;

        case RACE_TYPE:
            for (int index = 0; index < ui->raceType->count(); ++index) {
                values << ui->raceType->itemText(index);
            }
            break;
        case RESULTS:
            for (int index = 0; index < ui->results->count(); ++index) {
                values << ui->results->itemText(index);
            }
            break;
        case NAME_COMPOSITION:
            for (int index = 0; index < ui->nameComposition->count(); ++index) {
                values << ui->nameComposition->itemText(index);
            }
            break;
        case ACCURACY:
            for (int index = 0; index < ui->accuracy->count(); ++index) {
                values << ui->accuracy->itemText(index);
            }
            break;
        case LANGUAGE:
            for (int index = 0; index < ui->language->count(); ++index) {
                values << ui->language->itemData(index).toString();
            }
            break;
        default:
            emit error(tr("Error: unknown index field"));
            break;
    }

    return values;
}

int ChronoRaceData::getFieldIndex(ChronoRaceData::IndexField field)
{
    int index = -1;

    switch (field) {
        using enum ChronoRaceData::IndexField;

        case RACE_TYPE:
            index = ui->raceType->currentIndex();
            break;
        case RESULTS:
            index = ui->results->currentIndex();
            break;
        case NAME_COMPOSITION:
            index = ui->nameComposition->currentIndex();
            break;
        case ACCURACY:
            index = ui->accuracy->currentIndex();
            break;
        case LANGUAGE:
            index = ui->language->currentIndex();
            break;
        default:
            emit error(tr("Error: unknown index field"));
            break;
    }

    return index;
}

void ChronoRaceData::setField(ChronoRaceData::IndexField field, int newIndex)
{
    switch (field) {
        using enum ChronoRaceData::IndexField;

        case RACE_TYPE:
            if ((newIndex < ui->raceType->count()) && (this->raceTypeIdx != newIndex)) {
                this->raceTypeIdx = newIndex;
                this->stringFields[static_cast<qsizetype>(ChronoRaceData::StringField::RACE_TYPE)] = ui->raceType->itemText(this->raceTypeIdx);
                this->dirty = true;
            }
            break;
        case RESULTS:
            if ((newIndex < ui->results->count()) && (this->resultsIdx != newIndex)) {
                this->resultsIdx = newIndex;
                this->stringFields[static_cast<qsizetype>(ChronoRaceData::StringField::RESULTS)] = ui->results->itemText(this->resultsIdx);
                this->dirty = true;
            }
            break;
        case NAME_COMPOSITION:
            if ((newIndex < ui->nameComposition->count()) && (this->nameCompositionIdx != newIndex)) {
                this->nameCompositionIdx = newIndex;
                this->stringFields[static_cast<qsizetype>(ChronoRaceData::StringField::NAME_COMPOSITION)] = ui->nameComposition->itemText(this->nameCompositionIdx);
                this->dirty = true;
            }
            break;
        case ACCURACY:
            if ((newIndex < ui->accuracy->count()) && (this->accuracyIdx != newIndex)) {
                this->accuracyIdx = newIndex;
                this->stringFields[static_cast<qsizetype>(ChronoRaceData::StringField::ACCURACY)] = ui->accuracy->itemText(this->accuracyIdx);
                this->dirty = true;
            }
            break;
        case LANGUAGE:
            if ((newIndex < ui->language->count()) && (this->languageIdx != newIndex)) {
                this->languageIdx = newIndex;
                this->dirty = true;
            }
            break;
        default:
            emit error(tr("Error: index field to be set unknown"));
            break;
    }
}

void ChronoRaceData::setField(ChronoRaceData::StringField field, QString const &newValue)
{
    switch (field) {
        using enum ChronoRaceData::StringField;

        case EVENT:
            [[fallthrough]];
        case PLACE:
            [[fallthrough]];
        case REFEREE:
            [[fallthrough]];
        case TIMEKEEPER_1:
            [[fallthrough]];
        case TIMEKEEPER_2:
            [[fallthrough]];
        case TIMEKEEPER_3:
            [[fallthrough]];
        case LENGTH:
            [[fallthrough]];
        case ELEVATION_GAIN:
            [[fallthrough]];
        case ORGANIZATION:
            if (this->stringFields[static_cast<qsizetype>(field)] != newValue) {
                this->stringFields[static_cast<qsizetype>(field)] = newValue;
                this->dirty = true;
            }
            break;
        default:
            emit error(tr("Error: string field to be set unknown"));
            break;
    }
}

void ChronoRaceData::setField(ChronoRaceData::LogoField field, QPixmap const &newValue)
{
    QPixmap *logo = Q_NULLPTR;

    switch (field) {
        using enum ChronoRaceData::LogoField;

        case LEFT:
            logo = &this->leftLogo.pixmap;
            break;
        case RIGHT:
            logo = &this->rightLogo.pixmap;
            break;
        case SPONSOR_1:
            logo = &this->sponsorLogo1.pixmap;
            break;
        case SPONSOR_2:
            logo = &this->sponsorLogo2.pixmap;
            break;
        case SPONSOR_3:
            logo = &this->sponsorLogo3.pixmap;
            break;
        case SPONSOR_4:
            logo = &this->sponsorLogo4.pixmap;
            break;
        default:
            // do nothing
            break;
    }

    if (logo == Q_NULLPTR) {
        emit error(tr("Error: logo field to be set unknown"));
    } else if (logo->toImage() != newValue.toImage()) {
        *logo = newValue;
        this->dirty = true;
    }

}

void ChronoRaceData::setRaceDate(QDate const &newDate)
{
    if (this->raceDate != newDate) {
        this->raceDate = newDate;
        this->dirty = true;
    }
}

void ChronoRaceData::setStartTime(QTime const &newStartTime)
{
    if (this->startTime != newStartTime) {
        this->startTime = newStartTime;
        this->dirty = true;
    }
}
