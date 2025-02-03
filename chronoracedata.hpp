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

#ifndef CHRONORACEDATA_HPP
#define CHRONORACEDATA_HPP

#include <QDialog>
#include <QDate>
#include <QTime>
#include <QLabel>
#include <QDataStream>
#include <QTextStream>
#include <QVector>
#include <QPixmap>

#include "ui_chronoracedata.h"
#include "chronoracelogo.hpp"

class ChronoRaceData : public QDialog
{
    Q_OBJECT

public:
    enum class StringField
    {
        EVENT            = 0,
        PLACE            = 1,
        RACE_TYPE        = 2,
        RESULTS          = 3,
        REFEREE          = 4,
        TIMEKEEPER_1     = 5,
        TIMEKEEPER_2     = 6,
        TIMEKEEPER_3     = 7,
        LENGTH           = 8,
        ELEVATION_GAIN   = 9,
        NAME_COMPOSITION = 10,
        ACCURACY         = 11,
        ORGANIZATION     = 12,
        NUM_OF_FIELDS    = 13
    };

    enum class IndexField
    {
        RACE_TYPE        = 0,
        RESULTS          = 1,
        NAME_COMPOSITION = 2,
        ACCURACY         = 3
    };

    enum class LogoField
    {
        LEFT             = 0,
        RIGHT            = 1,
        SPONSOR_1        = 2,
        SPONSOR_2        = 3,
        SPONSOR_3        = 4,
        SPONSOR_4        = 5
    };

    enum class RaceType
    {
        MASS_START    = 0,
        TIMED_RACE    = 1,
        RELAY_RACE    = 2
    };

    enum class Results
    {
        UNOFFICIAL    = 0,
        OFFICIAL      = 1
    };

    enum class NameComposition
    {
        SURNAME_FIRST = 0,
        NAME_FIRST    = 1,
        SURNAME_ONLY  = 2,
        NAME_ONLY     = 3
    };

    enum class Accuracy
    {
        SECOND     = 0,
        TENTH      = 1,
        HUNDREDTH  = 2,
        THOUSANDTH = 3
    };

    explicit ChronoRaceData(QWidget *parent = Q_NULLPTR);

    QDataStream &crdSerialize(QDataStream &out) const;
    friend QDataStream &operator<<(QDataStream &out, ChronoRaceData const &data)
    {
        return data.crdSerialize(out);
    }

    QDataStream &crdDeserialize(QDataStream &in);
    friend QDataStream &operator>>(QDataStream &in, ChronoRaceData &data)
    {
        return data.crdDeserialize(in);
    }

    QTextStream &crdSerialize(QTextStream &out) const;
    friend QTextStream &operator<<(QTextStream &out, ChronoRaceData const &data)
    {
        return data.crdSerialize(out);
    }

    void saveRaceData();
    void restoreRaceData() const;

    QPixmap getLeftLogo() const;
    QPixmap getRightLogo() const;
    QDate   getDate() const;
    QTime   getStartTime() const;
    QString getField(ChronoRaceData::StringField field) const;
    QVector<QPixmap> getSponsorLogos() const;

    void setField(ChronoRaceData::IndexField field, int newIndex);
    void setField(ChronoRaceData::StringField field, QString const &newValue);
    void setField(ChronoRaceData::LogoField field, QPixmap const &newValue);
    void setRaceDate(QDate const &newDate);
    void setStartTime(QTime const &newStartTime);

    void getGlobalData(ChronoRaceData::NameComposition *gNameComposition, ChronoRaceData::Accuracy *gAccuracy) const;

public slots:
    void loadLogo(QLabel *label = Q_NULLPTR);
    void removeLogo(QLabel *label = Q_NULLPTR) const;

    void accept() override;
    void reject() override;
    void show(); //NOSONAR

private:
    QScopedPointer<Ui::ChronoRaceData> ui { new Ui::ChronoRaceData };

    QDate   date { QDate::currentDate() };
    QTime   startTime { 0, 0 };
    int     raceTypeIdx { 0 };
    int     resultsIdx { 0 };
    int     nameCompositionIdx { 0 };
    int     accuracyIdx { 0 };
    QVector<QString> stringFields;
    ChronoRaceLogo leftLogo;
    ChronoRaceLogo rightLogo;
    ChronoRaceLogo sponsorLogo1;
    ChronoRaceLogo sponsorLogo2;
    ChronoRaceLogo sponsorLogo3;
    ChronoRaceLogo sponsorLogo4;

signals:
    void globalDataChange(ChronoRaceData::NameComposition, ChronoRaceData::Accuracy);
    void error(QString const &);
};

#endif // CHRONORACEDATA_HPP
