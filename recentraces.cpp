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

#include "recentraces.hpp"
#include "crsettings.hpp"

constexpr int LBCHRONORACE_MAX_RECENT = 9;

QActionGroup *RecentRaces::actionGroup = Q_NULLPTR;
QPointer<QActionGroup> RecentRaces::group { Q_NULLPTR };
QStringList RecentRaces::recentRaces { };
QAction *RecentRaces::separator = Q_NULLPTR;

void RecentRaces::loadMenu(QMenu *menu)
{
    Q_ASSERT(menu != Q_NULLPTR);

    QAction const *clearAction;
    group = QPointer(new QActionGroup(menu));


    actionGroup = group.data();

    separator = menu->addSeparator();
    clearAction = menu->addAction(tr("Clear"));
    QObject::connect(clearAction, &QAction::triggered, &RecentRaces::clear);

    CRSettings::readRecent(recentRaces);
    rebuildMenu();
}

void RecentRaces::store()
{
    CRSettings::writeRecent(recentRaces);
}

void RecentRaces::update(const QString &path)
{
    recentRaces.removeAll(path);
    recentRaces.prepend(path);

    while (recentRaces.size() > LBCHRONORACE_MAX_RECENT)
        recentRaces.removeLast();

    rebuildMenu();
}

void RecentRaces::rebuildMenu()
{
    Q_ASSERT(actionGroup != Q_NULLPTR);

    QMenu *menu = qobject_cast<QMenu *>(actionGroup->parent());
    // Clear the action group
    foreach (auto *action, actionGroup->actions()) {
        menu->removeAction(action);
        actionGroup->removeAction(action);
    }

    // Rebuild and connect
    QAction *action;
    qsizetype size = recentRaces.size();
    for (qsizetype i = 0; i < size; ++i) {
        action = actionGroup->addAction(new QAction(QString::number(i + 1) + " | " + recentRaces.at(i), menu->parent()));
        action->setCheckable(false);
        action->setData(recentRaces.at(i));
    }
    if (auto actions = actionGroup->actions(); !actions.isEmpty()) {
        menu->insertActions(separator, actions);
    }
}

void RecentRaces::clear()
{
    recentRaces.clear();
    rebuildMenu();
}
