#ifndef LBEXCEPTION_H
#define LBEXCEPTION_H

#include <QException>
#include <QString>

class ChronoRaceException : public QException
{
private:
    QString message { "No message" };
public:
    explicit ChronoRaceException(QString const &message) : message(message) { };
    explicit ChronoRaceException(char const *message) : message(message) { };

    void raise() const override;
    ChronoRaceException *clone() const override;

    void setMessage(QString const &newMessage);
    QString& getMessage();
};

#endif // LBEXCEPTION_H
