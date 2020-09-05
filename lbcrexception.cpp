#include "lbcrexception.h"

ChronoRaceException::ChronoRaceException()
{
    this->message = "No message";
}

ChronoRaceException::ChronoRaceException(const QString& message)
{
    this->message = message;
}

ChronoRaceException::ChronoRaceException(const char* message)
{
    this->message = message;
}

void ChronoRaceException::setMessage(const QString& message)
{
    this->message = message;
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
    return new ChronoRaceException(*this);
}
