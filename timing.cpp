#include "timing.h"
#include "classentry.h"
#include "lbcrexception.h"

Timing::Field TimingSorter::sortingField = Timing::TMF_TIME;
Qt::SortOrder TimingSorter::sortingOrder = Qt::AscendingOrder;

Timing::Timing() {
    this->bib     = 0u;
    this->seconds = 0u;
    this->status  = CLASSIFIED;
}

Timing::Timing(const uint bib) : Timing() {
    this->bib = bib;
}

QDataStream &operator<<(QDataStream &out, const Timing &timing)
{
    out << quint32(timing.bib)
        << quint32(timing.leg)
        << quint32(timing.seconds)
        << qint32(timing.status);

    return out;
}

QDataStream &operator>>(QDataStream &in, Timing &timing)
{
    quint32 bib32, leg32, seconds32;
    qint32  status32;

    in >> bib32
       >> leg32
       >> seconds32
       >> status32;

    timing.bib     = (uint) bib32;
    timing.leg     = (uint) leg32;
    timing.seconds = (uint) seconds32;
    timing.status  = (Timing::Status) status32;

    return in;
}

uint Timing::getBib() const
{
    return bib;
}

void Timing::setBib(uint bib)
{
    this->bib = bib;
}

uint Timing::getLeg() const
{
    return leg;
}

void Timing::setLeg(uint leg)
{
    this->leg = leg;
}

bool Timing::isDnf() const
{
    return (status == DNF);
}

bool Timing::isDns() const
{
    return (status == DNS);
}

uint Timing::getSeconds() const
{
    return seconds;
}

Timing::Status Timing::getStatus() const
{
    return status;
}

QString Timing::getTiming() const
{
    return toTimeStr(this->seconds, this->status);
}

void Timing::setTiming(const QString& timing)
{

    if (timing.length() == 3) {
        if (!timing.toUpper().compare("DNS")) {
            this->status = DNS;
            this->seconds = 0u;
        } else if (!timing.toUpper().compare("DNF")) {
            this->status = DNF;
            this->seconds = 0u;
        } else {
            throw(ChronoRaceException(tr("Illegal timing value '%1'").arg(timing)));
        }
    } else {
        uint val, seconds = 0u;
        bool converted;
        QString sep(":");
        if (timing.contains(".")) {
            sep.clear();
            sep.append(".");
        }
        for (auto token : timing.split(sep, Qt::KeepEmptyParts, Qt::CaseInsensitive)) {
            converted = true;
            val = token.toUInt(&converted);
            if (!converted) {
                seconds = 0u;
                throw(ChronoRaceException(tr("Illegal timing value '%1' for bib '%2'").arg(token).arg(this->bib)));
            } else {
                seconds = (seconds * 60) + val;
            }
        }
        this->seconds = seconds;
    }
}

void Timing::setTiming(const char* timing)
{
    setTiming(QString(timing));
}

bool Timing::isValid()
{
    return ((bib != 0u) && ((status == DNS) || (status == DNF) || ((status == CLASSIFIED) && (seconds != 0u))));
}

const QString Timing::toTimeStr(const uint seconds, const Timing::Status status, const char *prefix)
{

    QString retString((prefix) ? prefix : "");
    switch (status) {
        case Timing::CLASSIFIED:
            retString.append(QString("%1:%2:%3").arg(((seconds / 60) / 60)).arg(((seconds / 60) % 60), 2, 10, QLatin1Char('0')).arg((seconds % 60), 2, 10, QLatin1Char('0')));
            break;
        case Timing::DNF:
            retString.append("DNF");
            break;
        case Timing::DNS:
            retString.append("DNS");
            break;
        default:
            throw(ChronoRaceException(tr("Invalid status value %1").arg(status)));
    }
    return retString;
}

const QString Timing::toTimeStr(const Timing& timing)
{
    return toTimeStr(timing.getSeconds(), timing.getStatus());
}

bool Timing::operator< (const Timing& rhs) const
{
    return (((this->status == DNF) && (rhs.status == DNS)) || ((this->status == CLASSIFIED) && ((rhs.status != CLASSIFIED) || (this->seconds < rhs.seconds))));
}

bool Timing::operator> (const Timing& rhs) const
{
    return (((this->status == DNS) && (rhs.status != DNS)) || ((rhs.status == CLASSIFIED) && ((this->status == DNF) || (this->seconds > rhs.seconds))));
}

bool Timing::operator<=(const Timing& rhs) const
{
    return !(*this > rhs);
}

bool Timing::operator>=(const Timing& rhs) const
{
    return !(*this < rhs);
}

bool TimingSorter::operator() (const Timing& lhs, const Timing& rhs)
{
    switch(sortingField) {
    case Timing::TMF_BIB:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getBib() > rhs.getBib()) : (lhs.getBib() < rhs.getBib());
    case Timing::TMF_TIME:
        // nobreak here
    default:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs > rhs) : (lhs < rhs);
    }

    return false;
}

Qt::SortOrder TimingSorter::getSortingOrder()
{
    return sortingOrder;
}

void TimingSorter::setSortingOrder(const Qt::SortOrder &value)
{
    sortingOrder = value;
}

Timing::Field TimingSorter::getSortingField()
{
    return sortingField;
}

void TimingSorter::setSortingField(const Timing::Field &value)
{
    sortingField = value;
}

Timing::Field& operator++(Timing::Field& field)
{
    field = static_cast<Timing::Field>(static_cast<int>(field) + 1);
    //if (field == Timing::TMF_COUNT)
    //    field = Timing::TMF_FIRST;
    return field;
}

Timing::Field  operator++(Timing::Field& field, int)
{
    Timing::Field tmp = field;
    ++field;
    return tmp;
}
