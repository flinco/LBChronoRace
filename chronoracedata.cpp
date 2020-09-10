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
    organizationStr(),
    sponsorLogo1(),
    sponsorLogo2(),
    sponsorLogo3(),
    sponsorLogo4()
{
    QPalette palette;

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
        << data.sponsorLogo4;

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

    data.raceTypeIdx = (int) raceTypeIdx32;
    data.resultsIdx = (int) resultsIdx32;

    data.ui->raceType->setCurrentIndex(data.raceTypeIdx);
    data.raceTypeStr = data.ui->raceType->currentText();

    data.ui->results->setCurrentIndex(data.resultsIdx);
    data.resultsStr = data.ui->results->currentText();

    data.ui->organization->setMarkdown(data.organization);
    data.organizationStr = data.ui->organization->toPlainText();

    return in;
}

QTextStream &operator<<(QTextStream &out, const ChronoRaceData &data)
{
    QStringList organizationLines = data.organizationStr.split(QRegExp("\n|\r\n|\r"));
    QString headColumn = QObject::tr("Organization") + ": ";

    for (auto line : organizationLines) {
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
    this->organization = ui->organization->toMarkdown(QTextDocument::MarkdownDialectCommonMark);
    this->organizationStr = ui->organization->toPlainText();
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
    ui->organization->setMarkdown(this->organization);
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

    //QDialog::show();
    QDialog::exec(); // modal
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

void ChronoRaceData::on_loadLeftLogo_clicked()
{
    loadLogo(ui->leftLogo);
}

void ChronoRaceData::on_loadRightLogo_clicked()
{
    loadLogo(ui->rightLogo);
}

void ChronoRaceData::on_loadSponsorLogo1_clicked()
{
    loadLogo(ui->sponsorLogo1);
}

void ChronoRaceData::on_loadSponsorLogo2_clicked()
{
    loadLogo(ui->sponsorLogo2);
}

void ChronoRaceData::on_loadSponsorLogo3_clicked()
{
    loadLogo(ui->sponsorLogo3);
}

void ChronoRaceData::on_loadSponsorLogo4_clicked()
{
    loadLogo(ui->sponsorLogo4);
}

void ChronoRaceData::on_removeLeftLogo_clicked()
{
    deleteLogo(ui->leftLogo);
}

void ChronoRaceData::on_removeRightLogo_clicked()
{
    deleteLogo(ui->rightLogo);
}

void ChronoRaceData::on_removeSponsorLogo1_clicked()
{
    deleteLogo(ui->sponsorLogo1);
}

void ChronoRaceData::on_removeSponsorLogo2_clicked()
{
    deleteLogo(ui->sponsorLogo2);
}

void ChronoRaceData::on_removeSponsorLogo3_clicked()
{
    deleteLogo(ui->sponsorLogo3);
}

void ChronoRaceData::on_removeSponsorLogo4_clicked()
{
    deleteLogo(ui->sponsorLogo4);
}
