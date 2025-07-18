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

#ifndef CRLANGUAGE_HPP
#define CRLANGUAGE_HPP

#include <QCoreApplication>
#include <QTranslator>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QPointer>

class Languages : public QObject
{
    Q_OBJECT

public:
    static void loadMenu(QMenu *menu);

    static void loadStoredLanguage(QTranslator *newQt, QTranslator *newBase, QTranslator *newApp);
    static void loadLanguage(QString const &rLanguage);

private:
    static QPointer<QActionGroup> group;

    static QString defaultLangCode;
    static QTranslator *qt;
    static QTranslator *base;
    static QTranslator *app;

    static void switchTranslators(QLocale const &locale);

private slots:
    static void changeLanguage(QAction const *action);
};

#endif // CRLANGUAGE_HPP
