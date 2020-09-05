#include "lbchronorace.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#ifdef Q_OS_WIN
#include <QStyleFactory>
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTranslator qtTranslator;
    QTranslator lbcrTranslator;

    QDir appDir = QDir(QCoreApplication::applicationDirPath());
    QDir translDir = QDir(appDir.path() + QDir::separator() + "translations");

    if (qtTranslator.load(QLocale(), QStringLiteral("qtbase"), QStringLiteral("_"), translDir.path()))
        app.installTranslator(&qtTranslator);

    if (lbcrTranslator.load(QLocale(), QStringLiteral("LBChronoRace"), QStringLiteral("_"), appDir.path()))
        app.installTranslator(&lbcrTranslator);

#ifdef Q_OS_WIN
    app.setStyle(QStyleFactory::create("Fusion"));
    //app.setStyle(QStyleFactory::create("WindowsVista"));
#endif

    if (!app.testAttribute(Qt::AA_EnableHighDpiScaling))
        app.setAttribute(Qt::AA_EnableHighDpiScaling);

    LBChronoRace w(nullptr, &app);
    w.show();
    return app.exec();
}





