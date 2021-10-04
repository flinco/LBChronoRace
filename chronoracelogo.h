#ifndef CHRONORACELOGO_H
#define CHRONORACELOGO_H

#include <QCoreApplication>
#include <QPixmap>
#include <QLabel>

class ChronoRaceLogo : public QObject
{
    Q_OBJECT

public:
    QPixmap pixmap;
    QLabel *uiElement;

public slots:
    void loadLogo();
    void removeLogo();

signals:
    void logoLoaded(QLabel *label = Q_NULLPTR);
    void logoRemoved(QLabel *label = Q_NULLPTR);
};

#endif // CHRONORACELOGO_H
