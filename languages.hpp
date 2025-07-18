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
#include <QScopedPointer>

class Languages : public QObject
{
    Q_OBJECT

public:
    explicit Languages(QMenu *newMenu);

    void loadMenu();

    static void loadStoredLanguage(QTranslator *newQt, QTranslator *newBase, QTranslator *newApp);
    static void loadLanguage(QString const &rLanguage);

private:
    QMenu *menu { Q_NULLPTR };
    QScopedPointer<QActionGroup> group { Q_NULLPTR };

    static QString defaultLangCode;
    static QTranslator *qt;
    static QTranslator *base;
    static QTranslator *app;

    static void switchQtTranslator(QLocale &locale);
    static void switchBaseTranslator(QLocale &locale);
    static void switchAppTranslator(QLocale &locale);

private slots:
    static void changeLanguage(QAction *action);
};

#endif // CRLANGUAGE_HPP
