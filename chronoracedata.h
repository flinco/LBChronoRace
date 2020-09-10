#ifndef CHRONORACEDATA_H
#define CHRONORACEDATA_H

#include <QDialog>
#include <QDir>
#include <QDate>
#include <QTime>
#include <QLabel>
#include <QDataStream>
#include <QTextStream>

namespace Ui {
class ChronoRaceData;
}

class ChronoRaceData : public QDialog
{
    Q_OBJECT

public:
    explicit ChronoRaceData(QWidget *parent = Q_NULLPTR);
    ~ChronoRaceData();

    friend QDataStream &operator<<(QDataStream &out, const ChronoRaceData &data);
    friend QDataStream &operator>>(QDataStream &in, ChronoRaceData &data);

    friend QTextStream &operator<<(QTextStream &out, const ChronoRaceData &data);

    void saveRaceData();
    void restoreRaceData();

    QPixmap getLeftLogo() const;
    QPixmap getRightLogo() const;
    QString getEvent() const;
    QString getPlace() const;
    QDate   getDate() const;
    QTime   getStartTime() const;
    QString getRaceType() const;
    QString getResults() const;
    QString getReferee() const;
    QString getTimeKeeper1() const;
    QString getTimeKeeper2() const;
    QString getTimeKeeper3() const;
    QString getOrganization() const;
    QString getLength() const;
    QString getElevationGain() const;
    QVector<QPixmap> getSponsorLogos() const;

    void setBinFormat(uint binFmt);

public slots:
    void accept();
    void reject();
    void show();

private:
    Ui::ChronoRaceData *ui;

    // Binary format for the loader
    uint    binFmt;

    QPixmap leftLogo;
    QPixmap rightLogo;
    QString event;
    QString place;
    QDate   date;
    QTime   startTime;
    int     raceTypeIdx;
    QString raceTypeStr;
    int     resultsIdx;
    QString resultsStr;
    QString referee;
    QString timeKeeper1;
    QString timeKeeper2;
    QString timeKeeper3;
    QString length;
    QString elevationGain;
    QString organization;
    QPixmap sponsorLogo1;
    QPixmap sponsorLogo2;
    QPixmap sponsorLogo3;
    QPixmap sponsorLogo4;

    bool loadLogo(QLabel *label);
    bool deleteLogo(QLabel *label);

private slots:
    void on_loadLeftLogo_clicked();
    void on_loadRightLogo_clicked();
    void on_loadSponsorLogo1_clicked();
    void on_loadSponsorLogo2_clicked();
    void on_loadSponsorLogo3_clicked();
    void on_loadSponsorLogo4_clicked();
    void on_removeLeftLogo_clicked();
    void on_removeRightLogo_clicked();
    void on_removeSponsorLogo1_clicked();
    void on_removeSponsorLogo2_clicked();
    void on_removeSponsorLogo3_clicked();
    void on_removeSponsorLogo4_clicked();
};

#endif // CHRONORACEDATA_H
