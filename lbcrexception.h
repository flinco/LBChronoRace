#ifndef LBEXCEPTION_H
#define LBEXCEPTION_H

#include <QException>
#include <QString>

class ChronoRaceException : public QException
{
private:
    QString message;
public:
    ChronoRaceException();
    ChronoRaceException(const QString& message);
    ChronoRaceException(const char* message);

    void raise() const;
    ChronoRaceException *clone() const;

    void setMessage(const QString& message);
    QString& getMessage();
};

#endif // LBEXCEPTION_H
