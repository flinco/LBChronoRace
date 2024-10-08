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

#include "lbchronorace.hpp"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QFontDatabase>
#include <QWindow>
#include <QIcon>
#include <QTimer>

#ifdef Q_OS_WIN
#include <QStyleFactory>
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLocale lbcrLocale;
    QTranslator qtTranslator;
    QTranslator qtbaseTranslator;
    QTranslator lbcrTranslator;

    QString translationsDir = QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("translations");

    if (qtTranslator.load(lbcrLocale, QStringLiteral("qt"), QStringLiteral("_"), translationsDir))
        QApplication::installTranslator(&qtTranslator);

    if (qtbaseTranslator.load(lbcrLocale, QStringLiteral("qtbase"), QStringLiteral("_"), translationsDir))
        QApplication::installTranslator(&qtbaseTranslator);

    if (lbcrTranslator.load(lbcrLocale, QStringLiteral(LBCHRONORACE_NAME), QStringLiteral("_"), QStringLiteral(":/i18n")))
        QApplication::installTranslator(&lbcrTranslator);

#ifdef Q_OS_WIN
    app.setStyle(QStyleFactory::create("Fusion"));
    //app.setStyle(QStyleFactory::create("WindowsVista"));
#endif

    QFontDatabase::addApplicationFont(":/fonts/LiberationMono-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationMono-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationMono-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationMono-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationSans-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationSans-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationSans-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationSerif-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationSerif-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationSerif-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/LiberationSerif-Regular.ttf");

    LBChronoRace w(Q_NULLPTR, &app);
    QTimer::singleShot(0, &w, &LBChronoRace::initialize);
    w.show();

    foreach (auto win, app.allWindows()) {
       win->setIcon(QIcon(":/icons/LBChronoRace.ico"));
    }

    return QApplication::exec();
}





