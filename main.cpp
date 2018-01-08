#include "lbchronorace.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTranslator qtTranslator;

    qtTranslator.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator lbcrTranslator;
    lbcrTranslator.load("lbchronorace_" + QLocale::system().name());
    app.installTranslator(&lbcrTranslator);

    LBChronoRace w;
    w.show();

    return app.exec();
}





