#include "lbchronorace.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QFontDatabase>
#include <QWindow>
#include <QIcon>
#include <QTimer>

#ifdef Q_OS_WIN
#include <QStyleFactory>
#endif

int main(int argc, char *argv[])
{
    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // Deprecated and always enabled in Qt6
    QApplication app(argc, argv);

    QTranslator qtTranslator;
    QTranslator lbcrTranslator;

    auto appDir = QDir(QCoreApplication::applicationDirPath());

    if (auto translDir = QDir(appDir.path() + QDir::separator() + "translations");
        qtTranslator.load(QLocale(), QStringLiteral("qt"), QStringLiteral("_"), translDir.path()))
        QApplication::installTranslator(&qtTranslator);

    if (lbcrTranslator.load(QString(":/%1").arg(QStringLiteral(LBCHRONORACE_NAME))))
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
       win->setIcon(QIcon(":/LBChronoRace.ico"));
    }

    return QApplication::exec();
}





