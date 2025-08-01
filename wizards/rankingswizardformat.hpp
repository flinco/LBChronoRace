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

#ifndef RANKINGSWIZARDFORMAT_HPP
#define RANKINGSWIZARDFORMAT_HPP

#include <QWizardPage>
#include <QFormLayout>

class RankingsWizardFormat : public QWizardPage
{
    Q_OBJECT

public:
    explicit RankingsWizardFormat(QWidget *parent = Q_NULLPTR);

    void initializePage() override;
    int nextId() const override;

private:
    QFormLayout layout;

private slots:
    void formatChange(int index) const;

signals:
    void error(QString const &);
};

#endif // RANKINGSWIZARDFORMAT_HPP
