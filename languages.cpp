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

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QDirIterator>

#include "languages.hpp"
#include "crsettings.hpp"

QString Languages::defaultLangCode = { };
QTranslator *Languages::qt = Q_NULLPTR;
QTranslator *Languages::base = Q_NULLPTR;
QTranslator *Languages::app = Q_NULLPTR;

Languages::Languages(QMenu *newMenu)
{
    this->menu = newMenu;
}

void Languages::loadMenu()
{
    QActionGroup *actionGroup;
    this->group.reset(new QActionGroup(this->menu));

    actionGroup = group.data();
    actionGroup->setExclusive(true);

    QObject::connect(actionGroup, &QActionGroup::triggered, &Languages::changeLanguage);

    QDirIterator it(QStringLiteral(":/i18n"));
    while (it.hasNext()) {
        // get locale extracted by filename
        QString langCode = it.next().section('/', 2, -1); // "lbchronorace_it.qm"
        langCode.truncate(langCode.lastIndexOf('.')); // "lbchronorace_it"
        langCode.remove(0, langCode.lastIndexOf('_') + 1); // "it"

        QLocale locale(langCode);
        QLocale::Country country = locale.territory();
        QString lang = QLocale::languageToString(locale.language());
        QIcon ico(QString(":/flags/%1.svg").arg(QLocale::territoryToCode(country).toLower()));

        QAction *action = menu->addAction(ico, lang);
        action->setParent(this);
        action->setCheckable(true);
        action->setData(langCode);
        actionGroup->addAction(action);

        // set default translators and language checked
        if (defaultLangCode == langCode) {
            action->setChecked(true);
        }

    }
}

void Languages::loadStoredLanguage(QTranslator *newQt, QTranslator *newBase, QTranslator *newApp)
{
    using enum QLocale::Language;

    Q_ASSERT(newQt);
    Q_ASSERT(newBase);
    Q_ASSERT(newApp);

    qt = newQt;
    base = newBase;
    app = newApp;

    defaultLangCode = CRSettings::getLanguage();
    QLocale::Language language = QLocale::codeToLanguage(defaultLangCode);
    QLocale::setDefault(QLocale(language));

    QLocale lbcrLocale;

    switchQtTranslator(lbcrLocale);
    switchBaseTranslator(lbcrLocale);
    switchAppTranslator(lbcrLocale);
}

void Languages::changeLanguage(QAction *action)
{
    if (action != Q_NULLPTR) {
        QString langCode = action->data().toString();
        // load the language dependant on the action content
        loadLanguage(langCode);
        // store the language in preferences
        CRSettings::setLanguage(langCode);
    }
}

void Languages::switchQtTranslator(QLocale &locale)
{
    Q_ASSERT(qt);

    // remove the old translator (if any)
    if (!qt->isEmpty())
        QApplication::removeTranslator(qt);

    QString translationsDir = QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("translations");

    // load the new translator
    if (qt->load(locale, QStringLiteral("qt"), QStringLiteral("_"), translationsDir)) {
        QApplication::installTranslator(qt);
    }
}

// TODO questo è da rivedere perché non sembra sia installato
void Languages::switchBaseTranslator(QLocale &locale)
{
    Q_ASSERT(base);

    // remove the old translator (if any)
    if (!base->isEmpty())
        QApplication::removeTranslator(base);

    QString translationsDir = QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("translations");

    // load the new translator
    if (base->load(locale, QStringLiteral("qtbase"), QStringLiteral("_"), translationsDir)) {
        QApplication::installTranslator(base);
    }
}

void Languages::switchAppTranslator(QLocale &locale)
{
    Q_ASSERT(app);

    // remove the old translator (if any)
    if (!app->isEmpty())
        QApplication::removeTranslator(app);

    // load the new translator
    if (app->load(locale, QStringLiteral("lbchronorace"), QStringLiteral("_"), QStringLiteral(":/i18n"))) {
        QApplication::installTranslator(app);
    }
}

void Languages::loadLanguage(QString const &rLanguage)
{
    if (defaultLangCode != rLanguage) {
        defaultLangCode = rLanguage;

        QLocale::Language language = QLocale::codeToLanguage(defaultLangCode);
        QLocale::setDefault(QLocale(language));

        QLocale lbcrLocale;

        switchQtTranslator(lbcrLocale);
        switchBaseTranslator(lbcrLocale);
        switchAppTranslator(lbcrLocale);
    }
}
