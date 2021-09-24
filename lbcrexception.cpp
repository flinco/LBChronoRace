#include "lbcrexception.h"

void ChronoRaceException::setMessage(QString const &newMessage)
{
    this->message = newMessage;
}

QString& ChronoRaceException::getMessage() {
    return this->message;
}
void ChronoRaceException::raise() const
{
    throw *this;
}

ChronoRaceException* ChronoRaceException::clone() const
{
    return new ChronoRaceException(*this); //NOSONAR
}
