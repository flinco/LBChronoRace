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

#include <QFile>

#include "recentraces.hpp"

constexpr int LBCHRONORACE_MAX_RECENT = 9;

RecentRaces::RecentRaces(QMenu *parent)
{
    menu.reset(new QMenu(parent));
}

QMenu *RecentRaces::getMenu()
{
    return menu.data();
}

void RecentRaces::readSettings()
{
    QFile recentFile;
    QString recentPath;

    recentRaces.clear();
    int size = settings.beginReadArray("RecentRaces");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        recentPath = settings.value("path").toString();
        recentFile.setFileName(recentPath);

        if (!recentFile.exists())
            continue;

        recentRaces.append(recentPath);
    }
    settings.endArray();

    rebuildMenu();
}

void RecentRaces::writeSettings()
{
    int i = 0;

    // Save recent races list
    settings.beginWriteArray("RecentRaces");
    for (auto const &recentRacePath : std::as_const(recentRaces)) {
        settings.setArrayIndex(i);
        settings.setValue("path", recentRacePath);
        if (++i == LBCHRONORACE_MAX_RECENT)
            break;
    }
    settings.endArray();
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
    // Disconnect all first
    for (auto const *action : std::as_const(menuActions)) {
        disconnect(action, &QAction::triggered, this, &RecentRaces::load);
    }
    if (clearAction != Q_NULLPTR)
        disconnect(clearAction, &QAction::triggered, this, &RecentRaces::clear);

    // Clear the menu and the actions
    clearAction = Q_NULLPTR;
    menuActions.clear();
    menu->clear();

    // Rebuild and connect
    QAction const *menuAction;
    qsizetype size = recentRaces.size();
    for (qsizetype i = 0; i < size; ++i) {
        menuAction = menu->addAction(QString::number(i + 1) + " | " + recentRaces.at(i));
        connect(menuAction, &QAction::triggered, this, &RecentRaces::load);
        menuActions.append(menuAction);
    }
    menu->addSeparator();
    clearAction = menu->addAction(tr("Clear"));
    connect(clearAction, &QAction::triggered, this, &RecentRaces::clear);

}

void RecentRaces::load()
{
    QAction const *action;

    if ((action = qobject_cast<QAction *>(sender()))) {
        emit triggered(action->text().section('|', 1).trimmed());
    }
}

void RecentRaces::clear()
{
    recentRaces.clear();
    rebuildMenu();
}
