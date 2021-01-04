#include <QFileDialog>
#include <QFileInfo>

#include "chronoracedata.h"
#include "ui_chronoracedata.h"

#include "lbchronorace.h"

ChronoRaceData::ChronoRaceData(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChronoRaceData),
    leftLogo(),
    rightLogo(),
    event(),
    place(),
    date(QDate::currentDate()),
    startTime(0, 0),
    raceTypeStr(),
    resultsStr(),
    referee(),
    timeKeeper1(),
    timeKeeper2(),
    timeKeeper3(),
    organization(),
    sponsorLogo1(),
    sponsorLogo2(),
    sponsorLogo3(),
    sponsorLogo4()
{
    this->raceTypeIdx = 0;
    this->resultsIdx = 0;

    ui->setupUi(this);

    ui->leftLogo->setStyleSheet("QLabel { background-color : white; }");
    ui->rightLogo->setStyleSheet("QLabel { background-color : white; }");
    ui->sponsorLogo1->setStyleSheet("QLabel { background-color : white; }");
    ui->sponsorLogo2->setStyleSheet("QLabel { background-color : white; }");
    ui->sponsorLogo3->setStyleSheet("QLabel { background-color : white; }");
    ui->sponsorLogo4->setStyleSheet("QLabel { background-color : white; }");

    ui->raceType->setCurrentIndex(this->raceTypeIdx);
    this->raceTypeStr = ui->raceType->currentText();

    ui->results->setCurrentIndex(this->resultsIdx);
    this->resultsStr = ui->results->currentText();

    this->connect(ui->loadLeftLogo, &QPushButton::clicked, this, &ChronoRaceData::loadLeftLogo);
    this->connect(ui->loadRightLogo, &QPushButton::clicked, this, &ChronoRaceData::loadRightLogo);
    this->connect(ui->loadSponsorLogo1, &QPushButton::clicked, this, &ChronoRaceData::loadSponsorLogo1);
    this->connect(ui->loadSponsorLogo2, &QPushButton::clicked, this, &ChronoRaceData::loadSponsorLogo2);
    this->connect(ui->loadSponsorLogo3, &QPushButton::clicked, this, &ChronoRaceData::loadSponsorLogo3);
    this->connect(ui->loadSponsorLogo4, &QPushButton::clicked, this, &ChronoRaceData::loadSponsorLogo4);
    this->connect(ui->removeLeftLogo, &QPushButton::clicked, this, &ChronoRaceData::removeLeftLogo);
    this->connect(ui->removeRightLogo, &QPushButton::clicked, this, &ChronoRaceData::removeRightLogo);
    this->connect(ui->removeSponsorLogo1, &QPushButton::clicked, this, &ChronoRaceData::removeSponsorLogo1);
    this->connect(ui->removeSponsorLogo2, &QPushButton::clicked, this, &ChronoRaceData::removeSponsorLogo2);
    this->connect(ui->removeSponsorLogo3, &QPushButton::clicked, this, &ChronoRaceData::removeSponsorLogo3);
    this->connect(ui->removeSponsorLogo4, &QPushButton::clicked, this, &ChronoRaceData::removeSponsorLogo4);
}

ChronoRaceData::~ChronoRaceData()
{
    delete ui;
}

QDataStream &operator<<(QDataStream &out, const ChronoRaceData &data)
{
    out << data.leftLogo
        << data.rightLogo
        << data.event
        << data.place
        << data.date
        << data.startTime
        << qint32(data.raceTypeIdx)
        << qint32(data.resultsIdx)
        << data.referee
        << data.timeKeeper1
        << data.timeKeeper2
        << data.timeKeeper3
        << data.organization
        << data.sponsorLogo1
        << data.sponsorLogo2
        << data.sponsorLogo3
        << data.sponsorLogo4
        << data.length
        << data.elevationGain;

    return out;
}

QDataStream &operator>>(QDataStream &in, ChronoRaceData &data)
{
    qint32 raceTypeIdx32;
    qint32 resultsIdx32;

    in >> data.leftLogo
       >> data.rightLogo
       >> data.event
       >> data.place
       >> data.date
       >> data.startTime
       >> raceTypeIdx32
       >> resultsIdx32
       >> data.referee
       >> data.timeKeeper1
       >> data.timeKeeper2
       >> data.timeKeeper3
       >> data.organization
       >> data.sponsorLogo1
       >> data.sponsorLogo2
       >> data.sponsorLogo3
       >> data.sponsorLogo4;

    if (data.binFmt == LBCHRONORACE_BIN_FMT_v2)
        in >> data.length
           >> data.elevationGain;

    data.raceTypeIdx = (int) raceTypeIdx32;
    data.resultsIdx = (int) resultsIdx32;

    data.ui->raceType->setCurrentIndex(data.raceTypeIdx);
    data.raceTypeStr = data.ui->raceType->currentText();

    data.ui->results->setCurrentIndex(data.resultsIdx);
    data.resultsStr = data.ui->results->currentText();

    return in;
}

QTextStream &operator<<(QTextStream &out, const ChronoRaceData &data)
{
    QRegularExpression re("\n|\r\n|\r");
    QStringList organizationLines = data.organization.split(re);
    QString headColumn = QObject::tr("Organization") + ": ";

    for (const auto &line : organizationLines) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << headColumn;
        out.setFieldWidth(0);
        out << line << Qt::endl;
        headColumn.fill(' ');
    }

    if (!data.event.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Event") + ": ";
        out.setFieldWidth(0);
        out << data.event << Qt::endl;
    }

    if (!data.place.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Place") + ": ";
        out.setFieldWidth(0);
        out << data.place << Qt::endl;
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

    if (!data.raceTypeStr.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Race Type") + ": ";
        out.setFieldWidth(0);
        out << data.raceTypeStr << Qt::endl;
    }

    if (!data.length.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Length") + ": ";
        out.setFieldWidth(0);
        out << data.length << Qt::endl;
    }

    if (!data.elevationGain.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Elevation Gain") + ": ";
        out.setFieldWidth(0);
        out << data.elevationGain << Qt::endl;
    }

    if (!data.referee.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Referee") + ": ";
        out.setFieldWidth(0);
        out << data.referee << Qt::endl;
    }

    if (!data.timeKeeper1.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Timekeeper 1") + ": ";
        out.setFieldWidth(0);
        out << data.timeKeeper1 << Qt::endl;
    }

    if (!data.timeKeeper2.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Timekeeper 2") + ": ";
        out.setFieldWidth(0);
        out << data.timeKeeper2 << Qt::endl;
    }

    if (!data.timeKeeper3.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Timekeeper 3") + ": ";
        out.setFieldWidth(0);
        out << data.timeKeeper3 << Qt::endl;
    }

    out << Qt::endl;

    if (!data.resultsStr.isEmpty()) {
        out.setFieldWidth(20);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << QObject::tr("Results") + ": ";
        out.setFieldWidth(0);
        out << data.resultsStr << Qt::endl;
    }

    return out;
}

void ChronoRaceData::saveRaceData()
{
    this->leftLogo = ui->leftLogo->pixmap(Qt::ReturnByValue);
    this->rightLogo = ui->rightLogo->pixmap(Qt::ReturnByValue);
    this->event = ui->event->text();
    this->place = ui->place->text();
    this->date = ui->date->date();
    this->startTime = ui->startTime->time();
    this->raceTypeIdx = ui->raceType->currentIndex();
    this->raceTypeStr = ui->raceType->currentText();
    this->resultsIdx = ui->results->currentIndex();
    this->resultsStr = ui->results->currentText();
    this->referee = ui->referee->text();
    this->timeKeeper1 = ui->timeKeeper1->text();
    this->timeKeeper2 = ui->timeKeeper2->text();
    this->timeKeeper3 = ui->timeKeeper3->text();
    this->length = ui->length->text();
    this->elevationGain = ui->elevationGain->text();
    this->organization = ui->organization->toPlainText();
    this->sponsorLogo1 = ui->sponsorLogo1->pixmap(Qt::ReturnByValue);
    this->sponsorLogo2 = ui->sponsorLogo2->pixmap(Qt::ReturnByValue);
    this->sponsorLogo3 = ui->sponsorLogo3->pixmap(Qt::ReturnByValue);
    this->sponsorLogo4 = ui->sponsorLogo4->pixmap(Qt::ReturnByValue);
}

void ChronoRaceData::restoreRaceData()
{
    ui->leftLogo->setPixmap(this->leftLogo);
    ui->rightLogo->setPixmap(this->rightLogo);
    ui->event->setText(this->event);
    ui->place->setText(this->place);
    ui->date->setDate(this->date);
    ui->startTime->setTime(this->startTime);
    ui->raceType->setCurrentIndex(this->raceTypeIdx);
    ui->results->setCurrentIndex(this->resultsIdx);
    ui->referee->setText(this->referee);
    ui->timeKeeper1->setText(this->timeKeeper1);
    ui->timeKeeper2->setText(this->timeKeeper2);
    ui->timeKeeper3->setText(this->timeKeeper3);
    ui->length->setText(this->length);
    ui->elevationGain->setText(this->elevationGain);
    ui->organization->setPlainText(this->organization);
    ui->sponsorLogo1->setPixmap(this->sponsorLogo1);
    ui->sponsorLogo2->setPixmap(this->sponsorLogo2);
    ui->sponsorLogo3->setPixmap(this->sponsorLogo3);
    ui->sponsorLogo4->setPixmap(this->sponsorLogo4);
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

void ChronoRaceData::setBinFormat(uint binFmt)
{
    this->binFmt = binFmt;
}

QVector<QPixmap> ChronoRaceData::getSponsorLogos() const
{
    QVector<QPixmap> sponsorLogos;

    if (!sponsorLogo1.isNull())
        sponsorLogos.push_back(sponsorLogo1);

    if (!sponsorLogo2.isNull())
        sponsorLogos.push_back(sponsorLogo2);

    if (!sponsorLogo3.isNull())
        sponsorLogos.push_back(sponsorLogo3);

    if (!sponsorLogo4.isNull())
        sponsorLogos.push_back(sponsorLogo4);

    return sponsorLogos;
}

QString ChronoRaceData::getElevationGain() const
{
    return elevationGain;
}

QString ChronoRaceData::getLength() const
{
    return length;
}

QString ChronoRaceData::getOrganization() const
{
    return organization;
}

QString ChronoRaceData::getTimeKeeper3() const
{
    return timeKeeper3;
}

QString ChronoRaceData::getTimeKeeper2() const
{
    return timeKeeper2;
}

QString ChronoRaceData::getTimeKeeper1() const
{
    return timeKeeper1;
}

QString ChronoRaceData::getReferee() const
{
    return referee;
}

QString ChronoRaceData::getResults() const
{
    return resultsStr;
}

QString ChronoRaceData::getRaceType() const
{
    return raceTypeStr;
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
    return place;
}

QPixmap ChronoRaceData::getLeftLogo() const
{
    return leftLogo;
}

QPixmap ChronoRaceData::getRightLogo() const
{
    return rightLogo;
}

QString ChronoRaceData::getEvent() const
{
    return event;
}

bool ChronoRaceData::loadLogo(QLabel *label)
{
    bool retval = false;

    if (label) {
        QString logoFileName = QFileDialog::getOpenFileName(this, tr("Select Logo"),
            LBChronoRace::lastSelectedPath.absolutePath(), tr("Images (*.png *.xpm *.jpg *.gif);;All Files (*)"));
        if (!logoFileName.isEmpty()) {
            QPixmap newLogo = QPixmap(logoFileName);

            if ((retval = !newLogo.isNull())) {
                LBChronoRace::lastSelectedPath = QFileInfo(logoFileName).absoluteDir();
                // get label dimensions
                //int w = label->width();
                //int h = label->height();

                label->setPixmap(newLogo);
                // set a scaled pixmap to a w x h window keeping its aspect ratio
                //label->setPixmap(newLogo.scaled(w, h, Qt::KeepAspectRatio));
            }
        }
    }

    return retval;
}

bool ChronoRaceData::deleteLogo(QLabel *label)
{
    bool retval = false;

    if (label) {
        label->setPixmap(QPixmap());
        retval = true;
    }

    return retval;
}

void ChronoRaceData::loadLeftLogo()
{
    loadLogo(ui->leftLogo);
}

void ChronoRaceData::loadRightLogo()
{
    loadLogo(ui->rightLogo);
}

void ChronoRaceData::loadSponsorLogo1()
{
    loadLogo(ui->sponsorLogo1);
}

void ChronoRaceData::loadSponsorLogo2()
{
    loadLogo(ui->sponsorLogo2);
}

void ChronoRaceData::loadSponsorLogo3()
{
    loadLogo(ui->sponsorLogo3);
}

void ChronoRaceData::loadSponsorLogo4()
{
    loadLogo(ui->sponsorLogo4);
}

void ChronoRaceData::removeLeftLogo()
{
    deleteLogo(ui->leftLogo);
}

void ChronoRaceData::removeRightLogo()
{
    deleteLogo(ui->rightLogo);
}

void ChronoRaceData::removeSponsorLogo1()
{
    deleteLogo(ui->sponsorLogo1);
}

void ChronoRaceData::removeSponsorLogo2()
{
    deleteLogo(ui->sponsorLogo2);
}

void ChronoRaceData::removeSponsorLogo3()
{
    deleteLogo(ui->sponsorLogo3);
}

void ChronoRaceData::removeSponsorLogo4()
{
    deleteLogo(ui->sponsorLogo4);
}
