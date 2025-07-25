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

#ifndef RANKINGSWIZARDSELECTION_HPP
#define RANKINGSWIZARDSELECTION_HPP

#include <QWizardPage>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>

#include "ranking.hpp"
#include "classentry.hpp"
#include "teamclassentry.hpp"

class RankingsWizardSelection : public QWizardPage
{
    Q_OBJECT

public:
    explicit RankingsWizardSelection(QWidget *parent = Q_NULLPTR);

    class RankingsWizardItem
    {
    public:
        Ranking const *categories { Q_NULLPTR };
        bool skip { false };
        QList<ClassEntry const *> ranking { };
        QList<TeamClassEntry const *> teamRanking { };
    };

    void initializePage() override;
    void cleanupPage() override;

private:
    QListWidget rankingsList;
    QVBoxLayout layout;

private slots:
    void toggleSkipRanking(QListWidgetItem const *item) const;

signals:
    void error(QString const &);
};

#endif // RANKINGSWIZARDSELECTION_HPP
