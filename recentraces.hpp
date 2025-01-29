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

#ifndef RECENTRACES_HPP
#define RECENTRACES_HPP

#include <QObject>
#include <QScopedPointer>
#include <QSettings>
#include <QList>
#include <QMenu>
#include <QAction>

class RecentRaces : public QObject
{
    Q_OBJECT
public:
    explicit RecentRaces(QMenu *parent);

    QMenu *getMenu();
    void readSettings();

    void update(QString const &path);

signals:
    void triggered(QString const &);

private:
    QSettings settings { QSettings::Format::NativeFormat, QSettings::Scope::UserScope, QStringLiteral(LBCHRONORACE_ORGANIZATION), QStringLiteral(LBCHRONORACE_NAME) };

    QScopedPointer<QMenu> menu { Q_NULLPTR };
    QStringList recentRaces;

    QAction const *clearAction { Q_NULLPTR };
    QList<QAction const *> menuActions { };

    void rebuildMenu();

public slots:
    void writeSettings();

private slots:
    void load();
    void clear();
};

#endif // RECENTRACES_HPP
