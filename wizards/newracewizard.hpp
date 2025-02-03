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

#ifndef NEWRACEWIZARD_HPP
#define NEWRACEWIZARD_HPP

#include <QWizard>
#include <QWizardPage>

#include "chronoracedata.hpp"
#include "chronoracelogo.hpp"

class NewRaceWizardMainData : public QWizardPage
{
    Q_OBJECT

public:
    explicit NewRaceWizardMainData(QWidget *parent = Q_NULLPTR);
};

class NewRaceWizardOrganization : public QWizardPage
{
    Q_OBJECT

public:
    explicit NewRaceWizardOrganization(QWidget *parent = Q_NULLPTR);
};

class NewRaceWizardAuxData : public QWizardPage
{
    Q_OBJECT

public:
    explicit NewRaceWizardAuxData(QWidget *parent = Q_NULLPTR);
};

class NewRaceWizardLogos : public QWizardPage
{
    Q_OBJECT

public:
    explicit NewRaceWizardLogos(QWidget *parent = Q_NULLPTR);

public slots:
    void loadLogo(QLabel *label = Q_NULLPTR);
    void deleteLogo(QLabel *label = Q_NULLPTR) const;

protected:
    void setupLogo(QHBoxLayout *hBoxLayout, QPushButton **loadButton, QPushButton **removeButton, QIcon const &iconLoad, QIcon const &iconRemove) const;
};

class NewRaceWizardRaceLogos : public NewRaceWizardLogos
{
    Q_OBJECT

public:
    explicit NewRaceWizardRaceLogos(QWidget *parent = Q_NULLPTR);

private:
    ChronoRaceLogo leftLogo { };
    ChronoRaceLogo rightLogo { };
};

class NewRaceWizardSponsorLogos : public NewRaceWizardLogos
{
    Q_OBJECT

public:
    explicit NewRaceWizardSponsorLogos(QWidget *parent = Q_NULLPTR);

private:
    ChronoRaceLogo sponsor1Logo { };
    ChronoRaceLogo sponsor2Logo { };
    ChronoRaceLogo sponsor3Logo { };
    ChronoRaceLogo sponsor4Logo { };
};

class NewRaceWizard : public QWizard
{
    Q_OBJECT

public:
    explicit NewRaceWizard(ChronoRaceData *race, QWidget *parent = Q_NULLPTR);

private:
    ChronoRaceData *raceData { Q_NULLPTR };

    NewRaceWizardMainData     mainDataPage;
    NewRaceWizardOrganization organizationPage;
    NewRaceWizardAuxData      auxDataPage;
    NewRaceWizardRaceLogos    raceLogosPage;
    NewRaceWizardSponsorLogos sponsorLogosPage;

private slots:
    void updateRaceData(bool checked);
};

#endif // NEWRACEWIZARD_HPP
