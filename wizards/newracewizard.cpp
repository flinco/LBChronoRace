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

#include <QPointer>
#include <QLabel>
#include <QSizePolicy>
#include <QFileDialog>
#include <QFormLayout>
//NOSONAR #include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QDateEdit>
#include <QTimeEdit>
#include <QComboBox>
#include <QIcon>

#include "lbchronorace.hpp"
#include "lbcrexception.hpp"
#include "wizards/newracewizard.hpp"

NewRaceWizardMainData::NewRaceWizardMainData(QWidget *parent) : QWizardPage(parent)
{
    QPointer<QWidget> widgetPtr(Q_NULLPTR);

    setTitle(tr("General Race Information"));
    setSubTitle(tr("Please enter a name for the event, the place, date, time, and race type."));

    this->setLayout(new QFormLayout(this));
    auto *layout = qobject_cast<QFormLayout *>(this->layout());

    widgetPtr = new QLineEdit;
    layout->addRow(new QLabel(tr("Event")), widgetPtr.data());
    auto *event = qobject_cast<QLineEdit *>(layout->itemAt(0, QFormLayout::ItemRole::FieldRole)->widget());
    event->setPlaceholderText(tr("The name of the race"));
    event->setToolTip(tr("The name of the race"));
    registerField("maindata.event*", event);

    widgetPtr = new QLineEdit;
    layout->addRow(new QLabel(tr("Place")), widgetPtr.data());
    auto *place = qobject_cast<QLineEdit *>(layout->itemAt(1, QFormLayout::ItemRole::FieldRole)->widget());
    place->setPlaceholderText(tr("Where the competition will take place"));
    place->setToolTip(tr("Where the competition will take place"));
    registerField("maindata.place*", place);

    widgetPtr = new QDateEdit;
    layout->addRow(new QLabel(tr("Date")), widgetPtr.data());
    auto *date = qobject_cast<QDateEdit *>(layout->itemAt(2, QFormLayout::ItemRole::FieldRole)->widget());
    date->setDisplayFormat("dd/MM/yyyy");
    date->setDate(QDate::currentDate());
    date->setToolTip(tr("What day will the competition take place"));
    registerField("maindata.date", date);

    widgetPtr = new QTimeEdit;
    layout->addRow(new QLabel(tr("Time")), widgetPtr.data());
    auto *time = qobject_cast<QTimeEdit *>(layout->itemAt(3, QFormLayout::ItemRole::FieldRole)->widget());
    time->setToolTip(tr("What time the race will take place"));
    registerField("maindata.time", time);

    widgetPtr = new QComboBox;
    layout->addRow(new QLabel(tr("Race Type")), widgetPtr.data());
    auto *raceType = qobject_cast<QComboBox *>(layout->itemAt(4, QFormLayout::ItemRole::FieldRole)->widget());
    raceType->addItem(tr("Mass start"));
    raceType->addItem(tr("Timed race"));
    raceType->addItem(tr("Relay race"));
    raceType->setToolTip(tr("What kind of race it will be"));
    registerField("maindata.raceType", raceType);

    widgetPtr = Q_NULLPTR;
}

NewRaceWizardOrganization::NewRaceWizardOrganization(QWidget *parent) : QWizardPage(parent)
{
    QPointer<QWidget> widgetPtr(Q_NULLPTR);

    setTitle(tr("Race Organizing Committee Information"));
    setSubTitle(tr("This information is optional, but it makes the ranking look more professional."));

    this->setLayout(new QFormLayout(this));
    auto *layout = qobject_cast<QFormLayout *>(this->layout());

    widgetPtr = new QPlainTextEdit;
    layout->addRow(new QLabel(tr("Organization")), widgetPtr.data());
    auto *organization = qobject_cast<QPlainTextEdit *>(layout->itemAt(0, QFormLayout::ItemRole::FieldRole)->widget());
    organization->setPlaceholderText(tr("General information about the organizing committee of the race"));
    organization->setToolTip(tr("General information about the organizing committee of the race"));
    registerField("organization.organization", organization, "plainText");

    widgetPtr = Q_NULLPTR;
}

NewRaceWizardAuxData::NewRaceWizardAuxData(QWidget *parent) : QWizardPage(parent)
{
    QPointer<QWidget> widgetPtr(Q_NULLPTR);

    setTitle(tr("Additional Race Information"));
    setSubTitle(tr("This information is optional, but it makes the ranking look more professional."));

    this->setLayout(new QFormLayout(this));
    auto *layout = qobject_cast<QFormLayout *>(this->layout());

    widgetPtr = new QLineEdit;
    layout->addRow(new QLabel(tr("Referee")), widgetPtr.data());
    auto *referee = qobject_cast<QLineEdit *>(layout->itemAt(0, QFormLayout::ItemRole::FieldRole)->widget());
    referee->setPlaceholderText(tr("The race referee"));
    referee->setToolTip(tr("The race referee"));
    registerField("auxdata.referee", referee);

    widgetPtr = new QLineEdit;
    layout->addRow(new QLabel(tr("Timekeeper 1")), widgetPtr.data());
    auto *timeKeeper1 = qobject_cast<QLineEdit *>(layout->itemAt(1, QFormLayout::ItemRole::FieldRole)->widget());
    timeKeeper1->setPlaceholderText(tr("The first timekeeper of the race"));
    timeKeeper1->setToolTip(tr("The name of the first timekeeper of the race"));
    registerField("auxdata.timeKeeper1", timeKeeper1);

    widgetPtr = new QLineEdit;
    layout->addRow(new QLabel(tr("Timekeeper 2")), widgetPtr.data());
    auto *timeKeeper2 = qobject_cast<QLineEdit *>(layout->itemAt(2, QFormLayout::ItemRole::FieldRole)->widget());
    timeKeeper2->setPlaceholderText(tr("The second timekeeper of the race"));
    timeKeeper2->setToolTip(tr("The name of the second timekeeper of the race"));
    registerField("auxdata.timeKeeper2", timeKeeper2);

    widgetPtr = new QLineEdit;
    layout->addRow(new QLabel(tr("Timekeeper 3")), widgetPtr.data());
    auto *timeKeeper3 = qobject_cast<QLineEdit *>(layout->itemAt(3, QFormLayout::ItemRole::FieldRole)->widget());
    timeKeeper3->setPlaceholderText(tr("The third timekeeper of the race"));
    timeKeeper3->setToolTip(tr("The name of the third timekeeper of the race"));
    registerField("auxdata.timeKeeper3", timeKeeper3);

    widgetPtr = new QLineEdit;
    layout->addRow(new QLabel(tr("Length")), widgetPtr.data());
    auto *length = qobject_cast<QLineEdit *>(layout->itemAt(4, QFormLayout::ItemRole::FieldRole)->widget());
    length->setPlaceholderText(tr("The length of the race"));
    length->setToolTip(tr("The length of the race (usually in m or km)"));
    registerField("auxdata.length", length);

    widgetPtr = new QLineEdit;
    layout->addRow(new QLabel(tr("Elevation Gain")), widgetPtr.data());
    auto *elevationGain = qobject_cast<QLineEdit *>(layout->itemAt(5, QFormLayout::ItemRole::FieldRole)->widget());
    elevationGain->setPlaceholderText(tr("The elevation gain of the race"));
    elevationGain->setToolTip(tr("The elevation gain of the race (usually in m)"));
    registerField("auxdata.elevation", elevationGain);

    widgetPtr = Q_NULLPTR;
}

NewRaceWizardLogos::NewRaceWizardLogos(QWidget *parent) : QWizardPage(parent)
{
//NOSONAR     QLabel *pixmap;
//NOSONAR     QPushButton *loadButton;
//NOSONAR     QPushButton *removeButton;

//NOSONAR     setTitle(tr("Race and Sponsor Logos"));
//NOSONAR     setSubTitle(tr("More sponsor logos can be added in the <b>Edit Race Information</b> window."));

//NOSONAR     //this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

//NOSONAR     this->setLayout(new QGridLayout(this));
//NOSONAR     auto *layout = qobject_cast<QGridLayout *>(this->layout());
//NOSONAR     //layout->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);

//NOSONAR     QSizePolicy sizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Ignored);
//NOSONAR     sizePolicy.setHorizontalStretch(0);
//NOSONAR     sizePolicy.setVerticalStretch(0);
//NOSONAR     sizePolicy.setHeightForWidth(false);

//NOSONAR     layout->addWidget(new QLabel(tr("Left Logo")), 0, 0, Qt::AlignRight | Qt::AlignVCenter);
//NOSONAR     layout->addWidget(new QLabel(tr("Logo 1 not loaded")), 0, 1, 4, 1, Qt::AlignCenter); // testo inutile?
//NOSONAR     pixmap = qobject_cast<QLabel *>(layout->itemAtPosition(0, 1)->widget());
//NOSONAR     pixmap->setStyleSheet("QLabel { background-color : white; }");
//NOSONAR     pixmap->setToolTip(tr("The logo in the upper left corner of the PDF"));
//NOSONAR     pixmap->setSizePolicy(sizePolicy);
//NOSONAR     pixmap->setFrameShape(QFrame::Shape::Box);
//NOSONAR     pixmap->setScaledContents(true);
//NOSONAR     pixmap->setPixmap(leftLogo.pixmap);
//NOSONAR     layout->addWidget(new QPushButton(tr("Load")), 1, 0, Qt::AlignCenter);
//NOSONAR     loadButton = qobject_cast<QPushButton *>(layout->itemAtPosition(1, 0)->widget());
//NOSONAR     loadButton->setToolTip(tr("Add a logo in the upper left corner of the PDF"));
//NOSONAR     layout->addWidget(new QPushButton(tr("Remove")), 2, 0, Qt::AlignCenter);
//NOSONAR     removeButton = qobject_cast<QPushButton *>(layout->itemAtPosition(2, 0)->widget());
//NOSONAR     removeButton->setToolTip(tr("Removes the logo from the upper left corner of the PDF"));
//NOSONAR     leftLogo.uiElement = pixmap;
//NOSONAR     QObject::connect(loadButton, &QPushButton::clicked, &leftLogo, &ChronoRaceLogo::loadLogo);
//NOSONAR     QObject::connect(removeButton, &QPushButton::clicked, &leftLogo, &ChronoRaceLogo::removeLogo);
//NOSONAR     QObject::connect(&leftLogo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
//NOSONAR     QObject::connect(&leftLogo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
//NOSONAR     registerField("logos.left", pixmap);

//NOSONAR     layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding), 3, 0, 1, 1);

//NOSONAR     layout->addWidget(new QLabel(tr("Right Logo")), 0, 2, Qt::AlignRight | Qt::AlignVCenter);
//NOSONAR     layout->addWidget(new QLabel(tr("Logo 2 not loaded")), 0, 3, 4, 1, Qt::AlignCenter); // testo inutile?
//NOSONAR     pixmap = qobject_cast<QLabel *>(layout->itemAtPosition(0, 3)->widget());
//NOSONAR     pixmap->setStyleSheet("QLabel { background-color : white; }");
//NOSONAR     pixmap->setToolTip(tr("The logo in the upper right corner of the PDF"));
//NOSONAR     pixmap->setSizePolicy(sizePolicy);
//NOSONAR     pixmap->setFrameShape(QFrame::Shape::Box);
//NOSONAR     pixmap->setScaledContents(true);
//NOSONAR     pixmap->setPixmap(rightLogo.pixmap);
//NOSONAR     layout->addWidget(new QPushButton(tr("Load")), 1, 2, Qt::AlignCenter);
//NOSONAR     loadButton = qobject_cast<QPushButton *>(layout->itemAtPosition(1, 2)->widget());
//NOSONAR     loadButton->setToolTip(tr("Add a logo in the upper right corner of the PDF"));
//NOSONAR     layout->addWidget(new QPushButton(tr("Remove")), 2, 2, Qt::AlignCenter);
//NOSONAR     removeButton = qobject_cast<QPushButton *>(layout->itemAtPosition(2, 2)->widget());
//NOSONAR     removeButton->setToolTip(tr("Removes the logo from the upper right corner of the PDF"));
//NOSONAR     rightLogo.uiElement = pixmap;
//NOSONAR     QObject::connect(loadButton, &QPushButton::clicked, &rightLogo, &ChronoRaceLogo::loadLogo);
//NOSONAR     QObject::connect(removeButton, &QPushButton::clicked, &rightLogo, &ChronoRaceLogo::removeLogo);
//NOSONAR     QObject::connect(&rightLogo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
//NOSONAR     QObject::connect(&rightLogo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
//NOSONAR     registerField("logos.right", pixmap);

//NOSONAR     layout->addWidget(new QLabel(tr("1st sponsor logo")), 4, 0, Qt::AlignRight | Qt::AlignVCenter);
//NOSONAR     layout->addWidget(new QLabel(tr("Sponsor 1 not loaded")), 4, 1, 4, 1, Qt::AlignCenter); // testo inutile?
//NOSONAR     pixmap = qobject_cast<QLabel *>(layout->itemAtPosition(4, 1)->widget());
//NOSONAR     pixmap->setStyleSheet("QLabel { background-color : white; }");
//NOSONAR     pixmap->setToolTip(tr("First sponsor logo (at the bottom of the PDF)"));
//NOSONAR     pixmap->setSizePolicy(sizePolicy);
//NOSONAR     pixmap->setFrameShape(QFrame::Shape::Box);
//NOSONAR     pixmap->setScaledContents(true);
//NOSONAR     pixmap->setPixmap(sponsor1Logo.pixmap);
//NOSONAR     layout->addWidget(new QPushButton(tr("Load")), 5, 0, Qt::AlignCenter);
//NOSONAR     loadButton = qobject_cast<QPushButton *>(layout->itemAtPosition(5, 0)->widget());
//NOSONAR     loadButton->setToolTip(tr("Add the first sponsor logo (at the bottom of the PDF)"));
//NOSONAR     layout->addWidget(new QPushButton(tr("Remove")), 6, 0, Qt::AlignCenter);
//NOSONAR     removeButton = qobject_cast<QPushButton *>(layout->itemAtPosition(6, 0)->widget());
//NOSONAR     removeButton->setToolTip(tr("Remove the first sponsor logo (from the bottom of the PDF)"));
//NOSONAR     sponsor1Logo.uiElement = pixmap;
//NOSONAR     QObject::connect(loadButton, &QPushButton::clicked, &sponsor1Logo, &ChronoRaceLogo::loadLogo);
//NOSONAR     QObject::connect(removeButton, &QPushButton::clicked, &sponsor1Logo, &ChronoRaceLogo::removeLogo);
//NOSONAR     QObject::connect(&sponsor1Logo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
//NOSONAR     QObject::connect(&sponsor1Logo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
//NOSONAR     registerField("logos.sponsor1", pixmap);

//NOSONAR     layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding), 7, 0, 1, 1);

//NOSONAR     layout->addWidget(new QLabel(tr("2nd sponsor logo")), 4, 2, Qt::AlignRight | Qt::AlignVCenter);
//NOSONAR     layout->addWidget(new QLabel(tr("Sponsor 2 not loaded")), 4, 3, 4, 1, Qt::AlignCenter); // testo inutile?
//NOSONAR     pixmap = qobject_cast<QLabel *>(layout->itemAtPosition(4, 3)->widget());
//NOSONAR     pixmap->setStyleSheet("QLabel { background-color : white; }");
//NOSONAR     pixmap->setToolTip(tr("Second sponsor logo (at the bottom of the PDF)"));
//NOSONAR     pixmap->setSizePolicy(sizePolicy);
//NOSONAR     pixmap->setFrameShape(QFrame::Shape::Box);
//NOSONAR     pixmap->setScaledContents(true);
//NOSONAR     pixmap->setPixmap(sponsor2Logo.pixmap);
//NOSONAR     layout->addWidget(new QPushButton(tr("Load")), 5, 2, Qt::AlignCenter);
//NOSONAR     loadButton = qobject_cast<QPushButton *>(layout->itemAtPosition(5, 2)->widget());
//NOSONAR     loadButton->setToolTip(tr("Add the second sponsor logo (at the bottom of the PDF)"));
//NOSONAR     layout->addWidget(new QPushButton(tr("Remove")), 6, 2, Qt::AlignCenter);
//NOSONAR     removeButton = qobject_cast<QPushButton *>(layout->itemAtPosition(6, 2)->widget());
//NOSONAR     removeButton->setToolTip(tr("Remove the second sponsor logo (from the bottom of the PDF)"));
//NOSONAR     sponsor2Logo.uiElement = pixmap;
//NOSONAR     QObject::connect(loadButton, &QPushButton::clicked, &sponsor2Logo, &ChronoRaceLogo::loadLogo);
//NOSONAR     QObject::connect(removeButton, &QPushButton::clicked, &sponsor2Logo, &ChronoRaceLogo::removeLogo);
//NOSONAR     QObject::connect(&sponsor2Logo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
//NOSONAR     QObject::connect(&sponsor2Logo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
//NOSONAR     registerField("logos.sponsor2", pixmap);

//NOSONAR     layout->setColumnStretch(0, 1);
//NOSONAR     layout->setColumnStretch(1, 1);
//NOSONAR     layout->setColumnStretch(2, 1);
//NOSONAR     layout->setColumnStretch(3, 1);
//NOSONAR     // layout->setRowStretch(0, 1);
//NOSONAR     // layout->setRowStretch(1, 1);
//NOSONAR     // layout->setRowStretch(2, 1);
//NOSONAR     // layout->setRowStretch(3, 1);
//NOSONAR     // layout->setRowStretch(4, 1);
//NOSONAR     // layout->setRowStretch(5, 1);
//NOSONAR     // layout->setRowStretch(6, 1);
//NOSONAR     // layout->setRowStretch(7, 1);
//NOSONAR     // layout->setRowStretch(8, 1);
//NOSONAR     // layout->setRowStretch(9, 1);
}

void NewRaceWizardLogos::setupLogo(QHBoxLayout *hBoxLayout, QPushButton **loadButton, QPushButton **removeButton, QIcon const &iconLoad, QIcon const &iconRemove) const
{
    QVBoxLayout *vBoxLayout;

    hBoxLayout->addLayout(new QVBoxLayout, 1);
    vBoxLayout = qobject_cast<QVBoxLayout *>(hBoxLayout->itemAt(1)->layout());
    vBoxLayout->addWidget(new QPushButton(tr("Load")));
    *loadButton = qobject_cast<QPushButton *>(vBoxLayout->itemAt(0)->widget());
    (*loadButton)->setIcon(iconLoad);
    vBoxLayout->addWidget(new QPushButton(tr("Remove")));
    *removeButton = qobject_cast<QPushButton *>(vBoxLayout->itemAt(1)->widget());
    (*removeButton)->setIcon(iconRemove);
    vBoxLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding));
}

NewRaceWizardRaceLogos::NewRaceWizardRaceLogos(QWidget *parent) : NewRaceWizardLogos(parent)
{
    QLabel *pixmap;
    QPushButton *loadButtonRL = Q_NULLPTR;
    QPushButton *removeButtonRL = Q_NULLPTR;
    QHBoxLayout *hBoxLayoutRL;
    QPointer<QLayout> layoutPtr(Q_NULLPTR);
    QIcon iconLoad;
    QIcon iconRemove;

    setTitle(tr("Race Logos"));
    setSubTitle(tr("Race logos are optional, but they make the ranking look more professional."));

    iconLoad.addFile(QString::fromUtf8(":/material/icons/add_photo_alternate.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    iconRemove.addFile(QString::fromUtf8(":/material/icons/hide_image.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);

    this->setLayout(new QFormLayout(this));
    auto *layout = qobject_cast<QFormLayout *>(this->layout());
    //NOSONAR layout->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);
    layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::ExpandingFieldsGrow);
    //NOSONAR layout->setFormAlignment(Qt::AlignmentFlag::AlignLeading | Qt::AlignmentFlag::AlignVCenter);

    QSizePolicy sizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Ignored);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(false);

    layoutPtr = new QHBoxLayout;
    layout->addRow(new QLabel(tr("Left Logo")), layoutPtr.data());
    hBoxLayoutRL = qobject_cast<QHBoxLayout *>(layout->itemAt(0, QFormLayout::ItemRole::FieldRole)->layout());
    hBoxLayoutRL->addWidget(new QLabel(tr("Logo 1 not loaded")), 1);
    pixmap = qobject_cast<QLabel *>(hBoxLayoutRL->itemAt(0)->widget());
    pixmap->setStyleSheet("QLabel { background-color : white; }");
    pixmap->setToolTip(tr("The logo in the upper left corner of the PDF"));
    pixmap->setSizePolicy(sizePolicy);
    pixmap->setFrameShape(QFrame::Shape::Box);
    pixmap->setScaledContents(true);
    pixmap->setPixmap(leftLogo.pixmap);
    setupLogo(hBoxLayoutRL, &loadButtonRL, &removeButtonRL, iconLoad, iconRemove);
    loadButtonRL->setToolTip(tr("Add a logo in the upper left corner of the PDF"));
    removeButtonRL->setToolTip(tr("Removes the logo from the upper left corner of the PDF"));

    leftLogo.uiElement = pixmap;
    QObject::connect(loadButtonRL, &QPushButton::clicked, &leftLogo, &ChronoRaceLogo::loadLogo);
    QObject::connect(removeButtonRL, &QPushButton::clicked, &leftLogo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&leftLogo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
    QObject::connect(&leftLogo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
    registerField("logos.left", pixmap, "pixmap");

    layoutPtr = new QHBoxLayout;
    layout->addRow(new QLabel(tr("Right Logo")), layoutPtr.data());
    hBoxLayoutRL = qobject_cast<QHBoxLayout *>(layout->itemAt(1, QFormLayout::ItemRole::FieldRole)->layout());
    hBoxLayoutRL->addWidget(new QLabel(tr("Logo 2 not loaded")), 1);
    pixmap = qobject_cast<QLabel *>(hBoxLayoutRL->itemAt(0)->widget());
    pixmap->setStyleSheet("QLabel { background-color : white; }");
    pixmap->setToolTip(tr("The logo in the upper right corner of the PDF"));
    pixmap->setSizePolicy(sizePolicy);
    pixmap->setFrameShape(QFrame::Shape::Box);
    pixmap->setScaledContents(true);
    pixmap->setPixmap(rightLogo.pixmap);
    setupLogo(hBoxLayoutRL, &loadButtonRL, &removeButtonRL, iconLoad, iconRemove);
    loadButtonRL->setToolTip(tr("Add a logo in the upper right corner of the PDF"));
    removeButtonRL->setToolTip(tr("Removes the logo from the upper right corner of the PDF"));
    rightLogo.uiElement = pixmap;
    QObject::connect(loadButtonRL, &QPushButton::clicked, &rightLogo, &ChronoRaceLogo::loadLogo);
    QObject::connect(removeButtonRL, &QPushButton::clicked, &rightLogo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&rightLogo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
    QObject::connect(&rightLogo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
    registerField("logos.right", pixmap, "pixmap");

    layoutPtr = Q_NULLPTR;
}

NewRaceWizardSponsorLogos::NewRaceWizardSponsorLogos(QWidget *parent) : NewRaceWizardLogos(parent)
{
    QLabel *pixmap;
    QPushButton *loadButtonSL = Q_NULLPTR;
    QPushButton *removeButtonSL = Q_NULLPTR;
    QHBoxLayout *hBoxLayoutSL;
    QPointer<QLayout> layoutPtr(Q_NULLPTR);
    QIcon iconLoad;
    QIcon iconRemove;

    setTitle(tr("Sponsor Logos"));
    setSubTitle(tr("Sponsor logos are optional, but they make the ranking look more professional."));

    iconLoad.addFile(QString::fromUtf8(":/material/icons/add_photo_alternate.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    iconRemove.addFile(QString::fromUtf8(":/material/icons/hide_image.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);

    this->setLayout(new QFormLayout(this));
    auto *layout = qobject_cast<QFormLayout *>(this->layout());
    //NOSONAR layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::AllNonFixedFieldsGrow);

    QSizePolicy sizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Ignored);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(false);

    layoutPtr = new QHBoxLayout;
    layout->addRow(new QLabel(tr("1st sponsor logo")), layoutPtr.data());
    hBoxLayoutSL = qobject_cast<QHBoxLayout *>(layout->itemAt(0, QFormLayout::ItemRole::FieldRole)->layout());
    hBoxLayoutSL->addWidget(new QLabel(tr("Sponsor 1 not loaded")), 2);
    pixmap = qobject_cast<QLabel *>(hBoxLayoutSL->itemAt(0)->widget());
    pixmap->setStyleSheet("QLabel { background-color : white; }");
    pixmap->setToolTip(tr("First sponsor logo (at the bottom of the PDF)"));
    pixmap->setSizePolicy(sizePolicy);
    pixmap->setFrameShape(QFrame::Shape::Box);
    pixmap->setScaledContents(true);
    pixmap->setPixmap(sponsor1Logo.pixmap);
    setupLogo(hBoxLayoutSL, &loadButtonSL, &removeButtonSL, iconLoad, iconRemove);
    loadButtonSL->setToolTip(tr("Add the first sponsor logo (at the bottom of the PDF)"));
    removeButtonSL->setToolTip(tr("Remove the first sponsor logo (from the bottom of the PDF)"));
    sponsor1Logo.uiElement = pixmap;
    QObject::connect(loadButtonSL, &QPushButton::clicked, &sponsor1Logo, &ChronoRaceLogo::loadLogo);
    QObject::connect(removeButtonSL, &QPushButton::clicked, &sponsor1Logo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&sponsor1Logo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
    QObject::connect(&sponsor1Logo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
    registerField("logos.sponsor1", pixmap, "pixmap");

    layoutPtr = new QHBoxLayout;
    layout->addRow(new QLabel(tr("2nd sponsor logo")), layoutPtr.data());
    hBoxLayoutSL = qobject_cast<QHBoxLayout *>(layout->itemAt(1, QFormLayout::ItemRole::FieldRole)->layout());
    hBoxLayoutSL->addWidget(new QLabel(tr("Sponsor 2 not loaded")), 2);
    pixmap = qobject_cast<QLabel *>(hBoxLayoutSL->itemAt(0)->widget());
    pixmap->setStyleSheet("QLabel { background-color : white; }");
    pixmap->setToolTip(tr("Second sponsor logo (at the bottom of the PDF)"));
    pixmap->setSizePolicy(sizePolicy);
    pixmap->setFrameShape(QFrame::Shape::Box);
    pixmap->setScaledContents(true);
    pixmap->setPixmap(sponsor2Logo.pixmap);
    setupLogo(hBoxLayoutSL, &loadButtonSL, &removeButtonSL, iconLoad, iconRemove);
    loadButtonSL->setToolTip(tr("Add the second sponsor logo (at the bottom of the PDF)"));
    removeButtonSL->setToolTip(tr("Remove the second sponsor logo (from the bottom of the PDF)"));
    sponsor2Logo.uiElement = pixmap;
    QObject::connect(loadButtonSL, &QPushButton::clicked, &sponsor2Logo, &ChronoRaceLogo::loadLogo);
    QObject::connect(removeButtonSL, &QPushButton::clicked, &sponsor2Logo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&sponsor2Logo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
    QObject::connect(&sponsor2Logo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
    registerField("logos.sponsor2", pixmap, "pixmap");

    layoutPtr = new QHBoxLayout;
    layout->addRow(new QLabel(tr("3rd sponsor logo")), layoutPtr.data());
    hBoxLayoutSL = qobject_cast<QHBoxLayout *>(layout->itemAt(2, QFormLayout::ItemRole::FieldRole)->layout());
    hBoxLayoutSL->addWidget(new QLabel(tr("Sponsor 3 not loaded")), 2);
    pixmap = qobject_cast<QLabel *>(hBoxLayoutSL->itemAt(0)->widget());
    pixmap->setStyleSheet("QLabel { background-color : white; }");
    pixmap->setToolTip(tr("Third sponsor logo (at the bottom of the PDF)"));
    pixmap->setSizePolicy(sizePolicy);
    pixmap->setFrameShape(QFrame::Shape::Box);
    pixmap->setScaledContents(true);
    pixmap->setPixmap(sponsor3Logo.pixmap);
    setupLogo(hBoxLayoutSL, &loadButtonSL, &removeButtonSL, iconLoad, iconRemove);
    loadButtonSL->setToolTip(tr("Add the third sponsor logo (at the bottom of the PDF)"));
    removeButtonSL->setToolTip(tr("Remove the third sponsor logo (from the bottom of the PDF)"));
    sponsor3Logo.uiElement = pixmap;
    QObject::connect(loadButtonSL, &QPushButton::clicked, &sponsor3Logo, &ChronoRaceLogo::loadLogo);
    QObject::connect(removeButtonSL, &QPushButton::clicked, &sponsor3Logo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&sponsor3Logo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
    QObject::connect(&sponsor3Logo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
    registerField("logos.sponsor3", pixmap, "pixmap");

    layoutPtr = new QHBoxLayout;
    layout->addRow(new QLabel(tr("4th sponsor logo")), layoutPtr.data());
    hBoxLayoutSL = qobject_cast<QHBoxLayout *>(layout->itemAt(3, QFormLayout::ItemRole::FieldRole)->layout());
    hBoxLayoutSL->addWidget(new QLabel(tr("Sponsor 4 not loaded")), 2);
    pixmap = qobject_cast<QLabel *>(hBoxLayoutSL->itemAt(0)->widget());
    pixmap->setStyleSheet("QLabel { background-color : white; }");
    pixmap->setToolTip(tr("Fourth sponsor logo (at the bottom of the PDF)"));
    pixmap->setSizePolicy(sizePolicy);
    pixmap->setFrameShape(QFrame::Shape::Box);
    pixmap->setScaledContents(true);
    pixmap->setPixmap(sponsor4Logo.pixmap);
    setupLogo(hBoxLayoutSL, &loadButtonSL, &removeButtonSL, iconLoad, iconRemove);
    loadButtonSL->setToolTip(tr("Add the fourth sponsor logo (at the bottom of the PDF)"));
    removeButtonSL->setToolTip(tr("Remove the fourth sponsor logo (from the bottom of the PDF)"));
    sponsor4Logo.uiElement = pixmap;
    QObject::connect(loadButtonSL, &QPushButton::clicked, &sponsor4Logo, &ChronoRaceLogo::loadLogo);
    QObject::connect(removeButtonSL, &QPushButton::clicked, &sponsor4Logo, &ChronoRaceLogo::removeLogo);
    QObject::connect(&sponsor4Logo, &ChronoRaceLogo::logoLoaded, this, &NewRaceWizardLogos::loadLogo);
    QObject::connect(&sponsor4Logo, &ChronoRaceLogo::logoRemoved, this, &NewRaceWizardLogos::deleteLogo);
    registerField("logos.sponsor4", pixmap, "pixmap");

    layoutPtr = Q_NULLPTR;
}

void NewRaceWizardLogos::loadLogo(QLabel *label)
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

void NewRaceWizardLogos::deleteLogo(QLabel *label) const
{
    if (label) {
        label->setPixmap(QPixmap());
    }
}

NewRaceWizard::NewRaceWizard(ChronoRaceData *race, QWidget *parent) :
    QWizard(parent),
    raceData(race),
    mainDataPage(this),
    organizationPage(this),
    auxDataPage(this),
    raceLogosPage(this),
    sponsorLogosPage(this)
{
    //NOSONAR setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
#ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
#endif

    addPage(&mainDataPage);
    addPage(&organizationPage);
    addPage(&auxDataPage);
    addPage(&raceLogosPage);
    addPage(&sponsorLogosPage);

    setWindowTitle(tr("New Race Wizard"));
    //NOSONAR setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo.png"));

    QObject::connect(this->button(QWizard::FinishButton), &QAbstractButton::clicked, this, &NewRaceWizard::updateRaceData);
}

void NewRaceWizard::updateRaceData(bool checked)
{
    Q_UNUSED(checked)

    if (raceData) {
        using enum ChronoRaceData::StringField;
        using enum ChronoRaceData::LogoField;

        raceData->setField(EVENT, field("maindata.event").toString());
        raceData->setField(PLACE, field("maindata.place").toString());
        raceData->setRaceDate(field("maindata.date").toDate());
        raceData->setStartTime(field("maindata.time").toTime());
        raceData->setField(ChronoRaceData::IndexField::RACE_TYPE, field("maindata.raceType").toInt());
        raceData->setField(ORGANIZATION, field("organization.organization").toString());
        raceData->setField(REFEREE, field("auxdata.referee").toString());
        raceData->setField(TIMEKEEPER_1, field("auxdata.timeKeeper1").toString());
        raceData->setField(TIMEKEEPER_2, field("auxdata.timeKeeper2").toString());
        raceData->setField(TIMEKEEPER_3, field("auxdata.timeKeeper3").toString());
        raceData->setField(LENGTH, field("auxdata.length").toString());
        raceData->setField(ELEVATION_GAIN, field("auxdata.elevation").toString());
        raceData->setField(LEFT, field("logos.left").value<QPixmap>());
        raceData->setField(RIGHT, field("logos.right").value<QPixmap>());
        raceData->setField(SPONSOR_1, field("logos.sponsor1").value<QPixmap>());
        raceData->setField(SPONSOR_2, field("logos.sponsor2").value<QPixmap>());
        raceData->setField(SPONSOR_3, field("logos.sponsor3").value<QPixmap>());
        raceData->setField(SPONSOR_4, field("logos.sponsor4").value<QPixmap>());
        raceData->setField(ChronoRaceData::IndexField::RESULTS, 0);
        raceData->setField(ChronoRaceData::IndexField::NAME_COMPOSITION, 0);
        raceData->setField(ChronoRaceData::IndexField::ACCURACY, 0);
    } else {
        throw(ChronoRaceException(tr("Error: race information not initialized")));
    }
}
