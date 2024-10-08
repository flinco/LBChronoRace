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

#ifndef RANKINGSWIZARDMODE_H
#define RANKINGSWIZARDMODE_H

#include <QWizardPage>
#include <QVBoxLayout>
#include <QRadioButton>

class RankingsWizardMode : public QWizardPage
{
    Q_OBJECT

public:
    explicit RankingsWizardMode(QWidget *parent = Q_NULLPTR);

private:
    QVBoxLayout layout;

    QRadioButton singleFile;
    QRadioButton multiFile;

private slots:
    void toggleSingleMode(bool checked) const;

signals:
    void error(QString const &);
};

#endif // RANKINGSWIZARDMODE_H
