#include "competitor.h"
#include "lbcrexception.h"

Competitor::Field CompetitorSorter::sortingField = Competitor::CMF_FIRST;
Qt::SortOrder     CompetitorSorter::sortingOrder = Qt::AscendingOrder;

Competitor::Competitor() {
    this->bib  = 0u;
    this->name = "";
    this->sex  = UNDEFINED;
    this->year = 1900;
    this->team = "";
    this->leg  = 1u;
}

Competitor::Competitor(const uint bib) : Competitor() {
    this->bib = bib;
}

const QString& Competitor::getName() const {
    return name;
}

const QString  Competitor::getName(int width) const {
    return QString("%1").arg(this->name, -width);
}

void Competitor::setName(const QString& name) {
    this->name = name;
}

uint Competitor::getBib() const {
    return bib;
}

void Competitor::setBib(uint bib) {
    this->bib = bib;
}


Competitor::Sex Competitor::getSex() const {
    return sex;
}

void Competitor::setSex(const Competitor::Sex sex) {
    this->sex = sex;
}

const QString& Competitor::getTeam() const {
    return team;
}

const QString  Competitor::getTeam(int width) const {
    return QString("%1").arg(this->team, -width);
}

void Competitor::setTeam(const QString& team) {
    this->team = team;
}

uint Competitor::getYear() const {
    return year;
}

void Competitor::setYear(uint year) {
    this->year = year;
}

uint Competitor::getLeg() const {
    return leg;
}

void Competitor::setLeg(uint leg) {
    this->leg = leg;
}

bool Competitor::isValid() {
    return ((bib != 0u) && !name.isEmpty() && (sex != UNDEFINED));
}

Competitor::Sex Competitor::toSex(const QString& sex, const bool strict) {

    if (sex.length() != 1) {
        throw(ChronoRaceException(tr("Illegal sex '%1'").arg(sex)));
    } else {
        if ((sex.compare("M", Qt::CaseInsensitive) == 0))
            return MALE;
        else if ((sex.compare("F", Qt::CaseInsensitive) == 0))
            return FEMALE;
        else if (!strict && (sex.compare("X", Qt::CaseInsensitive) == 0))
            return MISC;
    }

    return UNDEFINED;
}

QString Competitor::toSexString(const Sex sex) {

    switch (sex) {
        case MALE:
            return "M";
        case FEMALE:
            return "F";
        case MISC:
            return "X";
        case UNDEFINED:
            return "U";
        default:
            throw(ChronoRaceException(tr("Unexpected Sex enum value '%1'").arg(sex)));
    };
}

bool Competitor::operator< (const Competitor& rhs) const {
    if (bib < rhs.bib) {
        return true;
    } else if (bib == rhs.bib) {
        return leg < rhs.leg;
    }
    return false;
}

bool Competitor::operator> (const Competitor& rhs) const {
    if (bib > rhs.bib) {
        return true;
    } else if (bib == rhs.bib) {
        return leg > rhs.leg;
    }
    return false;
}

bool Competitor::operator<=(const Competitor& rhs) const {
    return !(*this > rhs);
}

bool Competitor::operator>=(const Competitor& rhs) const {
    return !(*this < rhs);
}

bool CompetitorSorter::operator() (const Competitor& lhs, const Competitor& rhs) {
    switch(sortingField) {
    case Competitor::CMF_NAME:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getName() < rhs.getName()) : (lhs.getName() > rhs.getName());
    case Competitor::CMF_SEX:
        return (sortingOrder == Qt::DescendingOrder) ? (Competitor::toSexString(lhs.getSex()) < Competitor::toSexString(rhs.getSex())) : (Competitor::toSexString(lhs.getSex()) > Competitor::toSexString(rhs.getSex()));
    case Competitor::CMF_YEAR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getYear() < rhs.getYear()) : (lhs.getYear() > rhs.getYear());
    case Competitor::CMF_TEAM:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getTeam() < rhs.getTeam()) : (lhs.getTeam() > rhs.getTeam());
    case Competitor::CMF_BIB:
        // nobreak here
    default:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs < rhs) : (lhs > rhs);
    }

    return false;
}

Qt::SortOrder CompetitorSorter::getSortingOrder() {
    return sortingOrder;
}

void CompetitorSorter::setSortingOrder(const Qt::SortOrder &value) {
    sortingOrder = value;
}

Competitor::Field CompetitorSorter::getSortingField() {
    return sortingField;
}

void CompetitorSorter::setSortingField(const Competitor::Field &value) {
    sortingField = value;
}

Competitor::Field& operator++(Competitor::Field& field) {
    field = static_cast<Competitor::Field>(static_cast<int>(field) + 1);
    //if (field == Competitor::CMF_COUNT)
    //    field = Competitor::CMF_FIRST;
    return field;
}

Competitor::Field operator++(Competitor::Field& field, int) {
    Competitor::Field tmp = field;
    ++field;
    return tmp;
}
