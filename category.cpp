#include "category.h"
#include "lbcrexception.h"

Category::Field CategorySorter::sortingField = Category::CTF_FIRST;
Qt::SortOrder   CategorySorter::sortingOrder = Qt::AscendingOrder;

Category::Category()
{
    this->team             = false;
    this->sex              = Competitor::UNDEFINED;
    this->toYear           = 0u;
    this->fromYear         = 0u;
    this->fullDescription  = "";
    this->shortDescription = "";
}

Category::Category(const QString& team) : Category()
{
    if (team.length() != 1) {
        throw(ChronoRaceException(tr("Illegal category type - expected 'I' or 'T' - found %1").arg(team)));
    } else {
        this->team = (team.toUpper().compare("T") == 0);
    }
}

QDataStream &operator<<(QDataStream &out, const Category &category)
{
    out << qint32(category.team)
        << Competitor::toSexString(category.sex)
        << quint32(category.toYear)
        << quint32(category.fromYear)
        << category.fullDescription
        << category.shortDescription;

    return out;
}

QDataStream &operator>>(QDataStream &in, Category &category)
{
    quint32 toYear32, fromYear32;
    qint32  team32;
    QString sexStr;

    in >> team32
       >> sexStr
       >> toYear32
       >> fromYear32
       >> category.fullDescription
       >> category.shortDescription;

    category.team     = (bool) team32;
    category.sex      = Competitor::toSex(sexStr);
    category.toYear   = (uint) toYear32;
    category.fromYear = (uint) fromYear32;

    return in;
}

bool Category::isTeam() const
{
    return team;
}

void Category::setTeam(bool team)
{
    this->team = team;
}

uint Category::getFromYear() const
{
    return fromYear;
}

void Category::setFromYear(uint fromYear)
{
    this->fromYear = fromYear;
}

const QString& Category::getFullDescription() const
{
    return fullDescription;
}

void Category::setFullDescription(const QString& fullDescription)
{
    this->fullDescription = fullDescription;
}

Competitor::Sex Category::getSex() const
{
    return sex;
}

void Category::setSex(const Competitor::Sex sex)
{
    this->sex = sex;
}

const QString& Category::getShortDescription() const
{
    return shortDescription;
}

void Category::setShortDescription(const QString& shortDescription)
{
    this->shortDescription = shortDescription;
}

uint Category::getToYear() const
{
    return toYear;
}

void Category::setToYear(uint toYear)
{
    this->toYear = toYear;
}

bool Category::isValid()
{
    return (!fullDescription.isEmpty() && !shortDescription.isEmpty());
}

bool Category::operator< (const Category& rhs) const
{
    return (!this->isTeam() && rhs.isTeam());
}

bool Category::operator> (const Category& rhs) const
{
    return (this->isTeam() && !rhs.isTeam());
}

bool Category::operator<=(const Category& rhs) const
{
    return !(*this > rhs);
}

bool Category::operator>=(const Category& rhs) const
{
    return !(*this < rhs);
}

bool CategorySorter::operator() (const Category& lhs, const Category& rhs)
{
    switch(sortingField) {
    case Category::CTF_SEX:
        return (sortingOrder == Qt::DescendingOrder) ? (Competitor::toSexString(lhs.getSex()) < Competitor::toSexString(rhs.getSex())) : (Competitor::toSexString(lhs.getSex()) > Competitor::toSexString(rhs.getSex()));
    case Category::CTF_TO_YEAR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getToYear() < rhs.getToYear()) : (lhs.getToYear() > rhs.getToYear());
    case Category::CTF_FROM_YEAR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getFromYear() < rhs.getFromYear()) : (lhs.getFromYear() > rhs.getFromYear());
    case Category::CTF_FULL_DESCR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getFullDescription() < rhs.getFullDescription()) : (lhs.getFullDescription() > rhs.getFullDescription());
    case Category::CTF_SHORT_DESCR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getShortDescription() < rhs.getShortDescription()) : (lhs.getShortDescription() > rhs.getShortDescription());
    case Category::CTF_TEAM:
        // nobreak here
    default:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs < rhs) : (lhs > rhs);
    }

    return false;
}

Qt::SortOrder CategorySorter::getSortingOrder()
{
    return sortingOrder;
}

void CategorySorter::setSortingOrder(const Qt::SortOrder &value)
{
    sortingOrder = value;
}

Category::Field CategorySorter::getSortingField()
{
    return sortingField;
}

void CategorySorter::setSortingField(const Category::Field &value)
{
    sortingField = value;
}

Category::Field& operator++(Category::Field& field)
{
    field = static_cast<Category::Field>(static_cast<int>(field) + 1);
    //if (field == Category::CTF_COUNT)
    //    field = Category::CTF_FIRST;
    return field;
}

Category::Field  operator++(Category::Field& field, int)
{
    Category::Field tmp = field;
    ++field;
    return tmp;
}
