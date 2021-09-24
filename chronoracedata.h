#ifndef CHRONORACEDATA_H
#define CHRONORACEDATA_H

#include <QObject>
#include <QDialog>
#include <QDir>
#include <QDate>
#include <QTime>
#include <QLabel>
#include <QDataStream>
#include <QTextStream>
#include <QVector>

#include "ui_chronoracedata.h"
#include "chronoracelogo.h"

class ChronoRaceData : public QDialog
{
    Q_OBJECT

public:
    enum class StringField
    {
        EVENT          = 0,
        PLACE          = 1,
        RACE_TYPE      = 2,
        RESULTS        = 3,
        REFEREE        = 4,
        TIMEKEEPER_1   = 5,
        TIMEKEEPER_2   = 6,
        TIMEKEEPER_3   = 7,
        LENGTH         = 8,
        ELEVATION_GAIN = 9,
        ORGANIZATION   = 10,
        NUM_OF_FIELDS  = 11
    };

    explicit ChronoRaceData(QWidget *parent = Q_NULLPTR);

    friend QDataStream &operator<<(QDataStream &out, ChronoRaceData const &data);
    friend QDataStream &operator>>(QDataStream &in, ChronoRaceData &data);

    friend QTextStream &operator<<(QTextStream &out, ChronoRaceData const &data);

    void saveRaceData();
    void restoreRaceData() const;

    QPixmap getLeftLogo() const;
    QPixmap getRightLogo() const;
    QString getEvent() const;
    QString getPlace() const;
    QDate   getDate() const;
    QTime   getStartTime() const;
    QString getRaceType() const;
    QString getResults() const;
    QString getReferee() const;
    QString getTimeKeeper(uint idx) const;
    QString getOrganization() const;
    QString getLength() const;
    QString getElevationGain() const;
    QVector<QPixmap> getSponsorLogos() const;

    void setBinFormat(uint format);

public slots:
    void loadLogo(QLabel *label = Q_NULLPTR);
    void deleteLogo(QLabel *label = Q_NULLPTR) const;

    void accept() override;
    void reject() override;
    void show(); //NOSONAR

private:
    QScopedPointer<Ui::ChronoRaceData> ui { new Ui::ChronoRaceData };

    // Binary format for the loader
    uint    binFmt;

    QDate   date { QDate::currentDate() };
    QTime   startTime { 0, 0 };
    int     raceTypeIdx { 0 };
    int     resultsIdx { 0 };
    QVector<QString> stringFields;
    ChronoRaceLogo leftLogo;
    ChronoRaceLogo rightLogo;
    ChronoRaceLogo sponsorLogo1;
    ChronoRaceLogo sponsorLogo2;
    ChronoRaceLogo sponsorLogo3;
    ChronoRaceLogo sponsorLogo4;
};

#endif // CHRONORACEDATA_H
