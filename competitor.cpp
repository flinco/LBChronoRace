#include "competitor.h"
#include "lbcrexception.h"

Competitor::Field CompetitorSorter::sortingField = Competitor::CMF_FIRST;
Qt::SortOrder     CompetitorSorter::sortingOrder = Qt::AscendingOrder;

Competitor::Competitor()
{
    this->bib      = 0u;
    this->name     = "";
    this->sex      = UNDEFINED;
    this->year     = 1900;
    this->team     = "";
    this->leg      = 1u;
    this->offset   = -1;
    this->category = "";
}

Competitor::Competitor(const uint bib) : Competitor()
{
    this->bib = bib;
}

QDataStream &operator<<(QDataStream &out, const Competitor &comp)
{
    out << quint32(comp.bib)
        << comp.name
        << Competitor::toSexString(comp.sex)
        << quint32(comp.year)
        << comp.team
        << quint32(comp.leg)
        << qint32(comp.offset);

    return out;
}

QDataStream &operator>>(QDataStream &in, Competitor &comp)
{
    quint32 bib32, year32, leg32;
    qint32  offset32;
    QString sexStr;

    in >> bib32
       >> comp.name
       >> sexStr
       >> year32
       >> comp.team
       >> leg32
       >> offset32;

    comp.bib    = (uint) bib32;
    comp.sex    = Competitor::toSex(sexStr);
    comp.year   = (uint) year32;
    comp.leg    = (uint) leg32;
    comp.offset = (int) offset32;

    return in;
}

const QString& Competitor::getName() const
{
    return name;
}

const QString  Competitor::getName(int width) const
{
    return QString("%1").arg(this->name, -width);
}

void Competitor::setName(const QString& name)
{
    this->name = name;
}

uint Competitor::getBib() const
{
    return bib;
}

void Competitor::setBib(uint bib)
{
    this->bib = bib;
}


Competitor::Sex Competitor::getSex() const
{
    return sex;
}

void Competitor::setSex(const Competitor::Sex sex)
{
    this->sex = sex;
}

const QString& Competitor::getTeam() const
{
    return team;
}

const QString  Competitor::getTeam(int width) const
{
    return QString("%1").arg(this->team, -width);
}

void Competitor::setTeam(const QString& team)
{
    this->team = team;
}

uint Competitor::getYear() const
{
    return year;
}

void Competitor::setYear(uint year)
{
    this->year = year;
}

uint Competitor::getLeg() const
{
    return leg;
}

void Competitor::setLeg(uint leg)
{
    this->leg = leg;
}

int Competitor::getOffset() const
{
    return offset;
}
void Competitor::setOffset(int offset)
{
    this->offset = offset;
}

bool Competitor::isValid()
{
    return ((bib != 0u) && !name.isEmpty() && (sex != UNDEFINED));
}

const QString& Competitor::getCategory() const
{
    return this->category;
}

void Competitor::setCategory(const QString& category)
{
    this->category = category;
}

Competitor::Sex Competitor::toSex(const QString& sex, const bool strict)
{

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

QString Competitor::toSexString(const Sex sex)
{

    switch (sex) {
        case MALE:
            return "M";
        case FEMALE:
            return "F";
        case MISC:
            return "X";
        case UNDEFINED:
            return "U";
    }

    throw(ChronoRaceException(tr("Unexpected Sex enum value '%1'").arg(sex)));
}

int Competitor::toOffset(const QString& offset)
{
    QStringList list = offset.split(":");

    int retval = -1;
    int h, m, s, l = 0;
    bool check_h, check_m, check_s, check_l;
    switch (list.count()) {
        case 3:
            h = list[0].toInt(&check_h, 10);
            m = list[1].toInt(&check_m, 10);
            s = list[2].toInt(&check_s, 10);
            if (check_h && check_m && check_s)
                retval = (h * 3600) + (m * 60) + s;
        break;
        case 2:
            m = list[0].toInt(&check_m, 10);
            s = list[1].toInt(&check_s, 10);
            if (check_m && check_s)
                retval = (m * 60) + s;
        break;
        case 1:
            if (list[0].contains(tr("Leg"))) {
                l = list[0].remove(tr("Leg")).trimmed().toInt(&check_l, 10);
                if (check_l)
                    retval = -qAbs(l);
            } else {
                l = list[0].toInt(&check_l, 10);
                if (check_l && (l < 0))
                    retval = l;
            }
        break;
        default:
            // do nothing
        break;
    }

    return retval;
}

QString Competitor::toOffsetString(int offset)
{
    if (offset < 0)
        return tr("Leg %n", "", qAbs(offset));
    else
        return QString("%1:%2:%3").arg(((offset / 60) / 60)).arg(((offset / 60) % 60), 2, 10, QChar('0')).arg((offset % 60), 2, 10, QChar('0'));
}

bool Competitor::operator< (const Competitor& rhs) const
{
    if (bib < rhs.bib) {
        return true;
    } else if (bib == rhs.bib) {
        return leg < rhs.leg;
    }
    return false;
}

bool Competitor::operator> (const Competitor& rhs) const
{
    if (bib > rhs.bib) {
        return true;
    } else if (bib == rhs.bib) {
        return leg > rhs.leg;
    }
    return false;
}

bool Competitor::operator<=(const Competitor& rhs) const
{
    return !(*this > rhs);
}

bool Competitor::operator>=(const Competitor& rhs) const
{
    return !(*this < rhs);
}

bool CompetitorSorter::operator() (const Competitor& lhs, const Competitor& rhs)
{
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
}

bool CompetitorSorter::operator() (const Competitor* lhs, const Competitor* rhs)
{
    if (!lhs)
        return false;
    else if (!rhs)
        return true;
    else if (lhs->getOffset() != rhs->getOffset())
        return qAbs(lhs->getOffset()) < qAbs(rhs->getOffset());
    else  if (lhs->getBib() != rhs->getBib())
        return lhs->getBib() < rhs->getBib();

    return false;
}

Qt::SortOrder CompetitorSorter::getSortingOrder()
{
    return sortingOrder;
}

void CompetitorSorter::setSortingOrder(const Qt::SortOrder &value)
{
    sortingOrder = value;
}

Competitor::Field CompetitorSorter::getSortingField()
{
    return sortingField;
}

void CompetitorSorter::setSortingField(const Competitor::Field &value)
{
    sortingField = value;
}

Competitor::Field& operator++(Competitor::Field& field)
{
    field = static_cast<Competitor::Field>(static_cast<int>(field) + 1);
    //if (field == Competitor::CMF_COUNT)
    //    field = Competitor::CMF_FIRST;
    return field;
}

Competitor::Field operator++(Competitor::Field& field, int)
{
    Competitor::Field tmp = field;
    ++field;
    return tmp;
}
