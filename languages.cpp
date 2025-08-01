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
QPointer<QActionGroup> Languages::group { Q_NULLPTR };
QTranslator *Languages::qt = Q_NULLPTR;
QTranslator *Languages::base = Q_NULLPTR;
QTranslator *Languages::app = Q_NULLPTR;

void Languages::loadMenu(QMenu *menu)
{
    Q_ASSERT(menu != Q_NULLPTR);

    QActionGroup *actionGroup;
    group = QPointer(new QActionGroup(menu));

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
        //NOSONAR QString lang = QLocale::languageToString(locale.language()); // all in english
        QString lang = locale.nativeLanguageName();
        lang.replace(0, 1, lang[0].toUpper()); // italiano --> Italiano
        QIcon ico(QString(":/flags/%1.svg").arg(QLocale::territoryToCode(country).toLower()));

        QAction *action = menu->addAction(ico, lang);
        action->setParent(menu->parent());
        action->setCheckable(true);
        action->setData(langCode);
        actionGroup->addAction(action);

        // set default translators and language checked
        if (defaultLangCode == langCode) {
            action->setChecked(true);
        }
    }
}

void Languages::loadMenu(QComboBox *menu, QStringList const *filter)
{
    Q_ASSERT(menu != Q_NULLPTR);

    QDirIterator it(QStringLiteral(":/i18n"));
    while (it.hasNext()) {
        // get locale extracted by filename
        QString langCode = it.next().section('/', 2, -1); // "lbchronorace_it.qm"
        langCode.truncate(langCode.lastIndexOf('.')); // "lbchronorace_it"
        langCode.remove(0, langCode.lastIndexOf('_') + 1); // "it"

        if ((filter != Q_NULLPTR) && !filter->contains(langCode))
            continue;

        QLocale locale(langCode);
        QLocale::Country country = locale.territory();
        //NOSONAR QString lang = QLocale::languageToString(locale.language()); // all in english
        QString lang = locale.nativeLanguageName();
        lang.replace(0, 1, lang[0].toUpper()); // italiano --> Italiano
        QString iconFile = QString(":/flags/%1.svg").arg(QLocale::territoryToCode(country).toLower());

        menu->addItem(QIcon(iconFile), lang, langCode);
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

    loadLanguage(CRSettings::getLanguage());
}

void Languages::changeLanguage(QAction const *action)
{
    if (action != Q_NULLPTR) {
        QString langCode = action->data().toString();
        // load the language dependant on the action content
        loadLanguage(langCode);
        // store the language in preferences
        CRSettings::setLanguage(langCode);
    }
}

void Languages::switchTranslators(QLocale const &locale)
{
    Q_ASSERT(qt);
    Q_ASSERT(base);
    Q_ASSERT(app);

    // remove the old translator (if any)
    if (!app->isEmpty())
        QApplication::removeTranslator(app);
    if (!base->isEmpty())
        QApplication::removeTranslator(base);
    if (!qt->isEmpty())
        QApplication::removeTranslator(qt);

    QString translationsDir = QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("translations");

    // load new translators
    if (qt->load(locale, QStringLiteral("qt"), QStringLiteral("_"), translationsDir)) {
        QApplication::installTranslator(qt);
    }
    if (base->load(locale, QStringLiteral("qtbase"), QStringLiteral("_"), translationsDir)) {
        QApplication::installTranslator(base);
    }
    if (app->load(locale, QStringLiteral("lbchronorace"), QStringLiteral("_"), QStringLiteral(":/i18n"))) {
        QApplication::installTranslator(app);
    }
}

void Languages::loadLanguage(QString const &rLanguage)
{
    if (defaultLangCode != rLanguage) {
        defaultLangCode = rLanguage;

        QLocale locale(QLocale::codeToLanguage(defaultLangCode));
        QLocale::setDefault(locale);

        switchTranslators(locale);
    }
}

QTranslator const *Languages::getAppTranslator()
{
    return app;
}
