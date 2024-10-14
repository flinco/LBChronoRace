/*****************************************************************************
 * Copyright (C) 2021 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include "classentry.hpp"
#include "crloader.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

// Static members
QString ClassEntry::empty("*** ??? ***");

QString ClassEntryElement::formatNameCSV(bool first, QString const &name, QString const &sex, QString const &year) const
{
    return QString("%1%2,%3,%4").arg(first ? "" : ",", name, sex, year);
}

QString ClassEntryElement::formatNameTxt(bool first, QString const &name, QString const &sex, QString const &year) const
{
    return QString("%1%2 (%3,%4)").arg(first ? "" : ", ", name, sex, year);
}

void ClassEntryElement::addNames(bool csvFormat, bool first, QString &entryString, QString const &emptyName) const
{
    Competitor const *c = this->competitor;

    if (c)
        entryString += csvFormat ?
                    formatNameCSV(first, c->getCompetitorName(CRHelper::nameComposition), CRHelper::toSexString(c->getSex()), QString::number(c->getYear())) :
                    formatNameTxt(first, c->getCompetitorName(CRHelper::nameComposition, static_cast<int>(emptyName.size())), CRHelper::toSexString(c->getSex()), QString::number(c->getYear()));
    else
        entryString += csvFormat ?
                    formatNameCSV(first, emptyName, CRHelper::toSexString(Competitor::Sex::UNDEFINED), QString("0")) :
                    formatNameTxt(first, emptyName, CRHelper::toSexString(Competitor::Sex::UNDEFINED), QString("   0"));
}

bool ClassEntryElement::hasCategory(const Category *cat) const
{
    bool found = false;

    if (this->competitor) {
        QListIterator j { this->competitor->getCategories() };
        while (j.hasNext() && !found) {
            found = (cat == j.next());
        }
    }

    return found;
}

uint ClassEntry::getBib() const
{
    return bib;
}

void ClassEntry::setBib(uint newBib)
{
    this->bib = newBib;
}

QString ClassEntry::getName(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return entries[legIdx].competitor ? entries[legIdx].competitor->getCompetitorName(CRHelper::nameComposition) : ClassEntry::empty;
}

QString ClassEntry::getNamesCommon(bool csvFormat) const
{
    QString retString;
    bool skip;
    bool first = true;
    int i;
    Competitor const *c;

    int teamNameWidthMax = csvFormat ? 0 : static_cast<int>(CRLoader::getTeamNameWidthMax());
    int startListNameWidthMax = csvFormat ? 0 : static_cast<int>(CRLoader::getStartListNameWidthMax());

    QString emptyName = ClassEntry::empty;

    if (!csvFormat)
        emptyName.resize(startListNameWidthMax, ' ');

    for (i = 0; i < entries.size(); i++) {
        c = entries[i].competitor;

        // avoid duplicates
        skip = false;
        for (int j = 0; c && (j < i); j++) {
            if ((skip = entries[j].competitor && (entries[j].competitor->getCompetitorName(CRHelper::nameComposition).compare(c->getCompetitorName(CRHelper::nameComposition)) == 0)))
                break;
        }

        if (!skip) {
            entries[i].addNames(csvFormat, first, retString, emptyName);
            first = false;
        }
    }

    for (i = 0; i < entries.size(); i++) {
        if ((c = entries[i].competitor)) {
            QString clubAndTeam = QString("%1 %2").arg(c->getClub(), c->getTeam()).simplified();
            retString += csvFormat ?
                        QString(",%1").arg(clubAndTeam) :
                        QString(" - %1").arg(clubAndTeam, -teamNameWidthMax);
            break;
        }
    }

    if (i == entries.size()) {
        retString += QString(csvFormat ? ",%1" : " - %1").arg(emptyName);
    }

    if (entries.size() > 1) {
        retString += QString(csvFormat ? ",%1" : " (%1)").arg(CRHelper::toSexString(getSex()));
    }

    return retString;
}

QString ClassEntry::getNames(CRLoader::Format format) const
{
    QString retString;

    switch (format) {
    case CRLoader::Format::TEXT:
        retString = getNamesCommon(false);
        break;
    case CRLoader::Format::CSV:
        retString = getNamesCommon(true);
        break;
    case CRLoader::Format::PDF:
        retString = "***Error***";
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    return retString;
}

uint ClassEntry::getYear(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getYear() : 0;
}

Competitor::Sex ClassEntry::getSex() const
{
    uint males = 0;
    uint females = 0;
    Competitor::Sex sex = Competitor::Sex::UNDEFINED;

    for (auto const &e : entries) {
        switch (e.competitor->getSex()) {
        case Competitor::Sex::MALE:
            males++;
            break;
        case Competitor::Sex::FEMALE:
            females++;
            break;
        default:
            throw(ChronoRaceException(tr("Unexpected sex value for bib %1 (%2)").arg(bib).arg(e.competitor->getCompetitorName(CRHelper::nameComposition))));
            break;
        }
    }

    if (males && !females)
        sex = Competitor::Sex::MALE;
    else if (!males && females)
        sex = Competitor::Sex::FEMALE;

    return  sex;
}

Competitor::Sex ClassEntry::getSex(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getSex() : Competitor::Sex::UNDEFINED;
}

QString ClassEntry::getTimes(CRLoader::Format format, int legRankWidth) const
{
    QString retString;

    switch (format) {
    case CRLoader::Format::TEXT:
        for (QVector<ClassEntryElement>::ConstIterator it = entries.constBegin(); it < entries.constEnd(); it++)
            retString.append(QString("%1(%2) %3").arg((it == entries.constBegin()) ? "" : " - ").arg(it->legRanking, legRankWidth).arg(CRHelper::toTimeString(it->time, it->status), 7));
        break;
    case CRLoader::Format::CSV:
        for (QVector<ClassEntryElement>::ConstIterator it = entries.constBegin(); it < entries.constEnd(); it++)
            retString.append(QString("%1%2,%3").arg((it == entries.constBegin()) ? "" : ",").arg(it->legRanking).arg(CRHelper::toTimeString(it->time, Timing::Status::CLASSIFIED)));
        break;
    case CRLoader::Format::PDF:
        retString = "***Error***";
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    return retString;
}

QString ClassEntry::getTime(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return CRHelper::toTimeString(entries[legIdx].time, entries[legIdx].status);
}

uint ClassEntry::getTimeValue(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    switch (entries[legIdx].status) {
    case Timing::Status::DNS:
        return UINT_MAX;
    case Timing::Status::DNF:
        return UINT_MAX - 1;
    //NOSONAR case Timing::Status::DSQ:
    //NOSONAR     return UINT_MAX - 2;
    default:
        return entries[legIdx].time;
    }
}

uint ClassEntry::countEntries() const
{
    return static_cast<uint>(entries.size());
}

void ClassEntry::setTime(Competitor *comp, Timing const &timing, QStringList &messages)
{
    Q_ASSERT(comp);

    uint legHint  = timing.getLeg();
    int  offset   = comp->getOffset();
    auto legIndex = static_cast<int>((legHint > 0) ? (legHint - 1) : entries.size());

    if ((offset < 0) && (legIndex + 1 != qAbs(offset)))
        messages << tr("Leg mismatch for bib %1: detected %2 overriding competitor declared %3").arg(bib).arg(legIndex + 1).arg(qAbs(offset));

    while (entries.size() <= legIndex) {
        // add slots to entry vector
        entries.emplaceBack();
    }

    if (entries[legIndex].competitor && (entries[legIndex].competitor != comp))
        messages << tr("Competitor mismatch for bib %1: found %2 replaced by %3").arg(QString::number(bib), entries[legIndex].competitor->getCompetitorName(CRHelper::nameComposition), comp->getCompetitorName(CRHelper::nameComposition));

    entries[legIndex].competitor = comp;
    entries[legIndex].status = timing.getStatus();
    entries[legIndex].time = timing.getMilliseconds();
    if (isDnf()) {
        totalTime = UINT_MAX - 1;
    } else if (isDns()) {
        totalTime = UINT_MAX;
    } else {
        if (offset < 0) {
            // no offset; use standard timing logic
            // good for both individual and relay races
            uint millis = entries[legIndex].time;
            entries[legIndex].time -= totalTime;
            totalTime = millis;
        } else {
            // competitor with offset; maybe individual
            // race without mass start or relay race with
            // timings sum
            uint millis = entries[legIndex].time - (static_cast<uint>(offset) * 1000);
            entries[legIndex].time = millis;
            totalTime += millis;
        }
    }
}

uint ClassEntry::getLegRanking(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return entries[legIdx].legRanking;
}

void ClassEntry::setLegRanking(uint legIdx, uint ranking)
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    entries[legIdx].legRanking = ranking;
}

QString ClassEntry::getClub() const
{
    QStringList clubs { };

    for (auto const &it : entries) {
        if (it.competitor)
            clubs.append(it.competitor->getClub());
    }

    if (!clubs.isEmpty()) {
        clubs.removeDuplicates();
    }

    return clubs.join(" - ");
}

QString ClassEntry::getTeam() const
{
    QStringList teams { };

    for (auto const &it : entries) {
        if (it.competitor)
            teams.append(it.competitor->getTeam());
    }

    if (!teams.isEmpty()) {
        teams.removeDuplicates();
    }

    return teams.join(" - ");
}

QString ClassEntry::getClubsAndTeam() const
{
    QStringList clubsAndTeam { this->getClub(), this->getTeam() };

    clubsAndTeam.removeAll("");

    return clubsAndTeam.join(" - ");
}

bool ClassEntry::isDnf() const
{
    return std::any_of(entries.constBegin(), entries.constEnd(), [&](ClassEntryElement const &el) {
        return (el.status == Timing::Status::DNF);
    });
}

bool ClassEntry::isDns() const
{
    return std::any_of(entries.constBegin(), entries.constEnd(), [&](ClassEntryElement const &el) {
        return (el.status == Timing::Status::DNS);
    });
}

bool ClassEntry::isDsq() const
{
    return std::any_of(entries.constBegin(), entries.constEnd(), [&](ClassEntryElement const &el) {
        return (el.status == Timing::Status::DSQ);
    });
}

Category const *ClassEntry::getCategory() const
{
    return category;
}

Category const *ClassEntry::getCategory(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getCategory() : Q_NULLPTR;
}

QStringList ClassEntry::setCategory()
{
    QStringList messages;

    switch (entries.size()) {
    case 0:
        messages += tr("No competitors associated to bib %1").arg(bib);
        break;
    case 1:
        ClassEntryHelper::setCategorySingleLeg(this, messages);
        break;
    default:
        ClassEntryHelper::setCategoryMultiLeg(this, messages);
        break;
    }

    return messages;
}

uint ClassEntry::getTotalTime() const
{
    return isDsq() ? (UINT_MAX - 2) : totalTime;
}

QString ClassEntry::getTotalTime(CRLoader::Format format) const
{
    QString retString;

    switch (format) {
    case CRLoader::Format::TEXT:
        [[fallthrough]];
    case CRLoader::Format::CSV:
        [[fallthrough]];
    case CRLoader::Format::PDF:
        if (isDsq())
            retString = CRHelper::toTimeString(totalTime, Timing::Status::DSQ);
        else if (isDnf())
            retString = CRHelper::toTimeString(totalTime, Timing::Status::DNF);
        else if (isDns())
            retString = CRHelper::toTimeString(totalTime, Timing::Status::DNS);
        else
            retString = CRHelper::toTimeString(totalTime, Timing::Status::CLASSIFIED);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    return retString;
}

QString ClassEntry::getDiffTimeTxt(uint referenceTime) const
{
    if (isDsq() || isDns() || isDnf() || (totalTime == referenceTime))
        return QString("");

    if (totalTime > referenceTime)
        return CRHelper::toTimeString(totalTime - referenceTime, Timing::Status::CLASSIFIED, "+");
    else
        return CRHelper::toTimeString(referenceTime - totalTime, Timing::Status::CLASSIFIED, "-");
}

bool ClassEntry::operator< (ClassEntry const &rhs) const { return getTotalTime() <  rhs.getTotalTime(); }
bool ClassEntry::operator> (ClassEntry const &rhs) const { return getTotalTime() >  rhs.getTotalTime(); }
bool ClassEntry::operator<=(ClassEntry const &rhs) const { return getTotalTime() <= rhs.getTotalTime(); }
bool ClassEntry::operator>=(ClassEntry const &rhs) const { return getTotalTime() >= rhs.getTotalTime(); }

bool ClassEntryHelper::allCompetitorsShareTheSameClub(QVector<ClassEntryElement> const &entries, qsizetype fromLeg, qsizetype toLeg, QString const &club)
{
    Competitor const *competitor;
    for ( ; fromLeg < toLeg; fromLeg++) {
        if (!(competitor = entries[fromLeg].competitor))
            continue;

        if (competitor->getClub() != club) {
            return false;
        }
    }

    return true;
}

bool ClassEntryHelper::allCompetitorsAreOfTheSameSex(QVector<ClassEntryElement> const &entries, qsizetype fromLeg, qsizetype toLeg, Competitor::Sex sex)
{
    Competitor const *competitor;
    for ( ; fromLeg < toLeg; fromLeg++) {
        if (!(competitor = entries[fromLeg].competitor))
            continue;

        if (competitor->getSex() != sex) {
            return false;
        }
    }

    return true;
}

void ClassEntryHelper::removeImpossibleCategories(QVector<ClassEntryElement> &entries)
{
    Competitor *comp = entries[0].competitor;
    Category const *cat;
    Competitor::Sex sex = comp->getSex();

    qsizetype count = entries.count();
    QString const &club = comp->getClub();

    QMutableListIterator i { comp->getCategories() };
    while (i.hasNext()) {
        cat = i.next();
        switch (cat->getType()) {
        case Category::Type::MALE:
            [[fallthrough]];
        case Category::Type::FEMALE:
            /* Competitors must all belong to the same Club */
            if (!allCompetitorsShareTheSameClub(entries, 1, count, club))
                i.remove();
            break;
        case Category::Type::RELAY_X:
            [[fallthrough]];
        case Category::Type::RELAY_Y:
            /* Competitors must belong to different Clubs */
            if (allCompetitorsShareTheSameClub(entries, 1, count, club))
                i.remove();
            break;
        case Category::Type::RELAY_MF:
            /* Competitors must have different sex */
            if (allCompetitorsAreOfTheSameSex(entries, 1, count, sex))
                i.remove();
            break;
        default:
            Q_UNREACHABLE();
            break;
        }
    }
}

void ClassEntryHelper::removeLowerWeigthCategories(QList<Category const *> &categories, QString const &name, uint bib)
{
    Category::Type type;
    Category const *cat1 = Q_NULLPTR;
    Category const *cat2 = Q_NULLPTR;

    uint w1;
    uint w2;

    QList<Category const *> fullList;
    categories.swap(fullList);

    QMutableListIterator i { fullList };
    QMutableListIterator j { categories };

    while (i.hasNext()) {
        cat1 = i.next();
        type = cat1->getType();
        w1 = cat1->getWeight();
        i.remove();

        j = categories;
        while (j.hasNext()) {
            cat2 = j.next();

            if (cat2->getType() != type) {
                continue;
            }

            w2 = cat2->getWeight();
            if (w1 < w2) {
                qDebug() << tr("Removing candidate category '%1' associated to competitor %2 - bib %3").arg(cat2->getFullDescription(), name, QString::number(bib));
                cat1 = Q_NULLPTR;
                break;
            }

            if (w1 > w2) {
                qDebug() << tr("Removing candidate category '%1' associated to competitor %2 - bib %3").arg(j.peekPrevious()->getFullDescription(), name, QString::number(bib));
                j.remove();
            }
        }

        if (cat1)
            j.insert(cat1);
    }
}

void ClassEntryHelper::setCategorySingleLeg(ClassEntry *entry, QStringList &messages)
{
    Competitor *comp = entry->entries[0].competitor;
    QString const &name = comp->getCompetitorName(CRHelper::nameComposition);

    if (QList<Category const *> &categories = comp->getCategories(); categories.isEmpty()) {
        messages += tr("No category fits the competitor with bib %1 - %2").arg(entry->bib).arg(name);
    } else {
        /* Slim down the list removing impossible category types */
        removeImpossibleCategories(entry->entries);

        /* Slim down the list removing categories having same type but lower weight */
        removeLowerWeigthCategories(categories, name, entry->bib);

        qsizetype i = categories.count();
        while (i-- > 1) {
            qDebug() << tr("Dropping category '%1' associated to competitor %2 - bib %3").arg(categories[i]->getFullDescription(), name, QString::number(entry->bib));
            categories.removeAt(i);
        }

        entry->category = comp->getCategory();
    }
}

void ClassEntryHelper::setCategoryMultiLeg(ClassEntry *entry, QStringList &messages)
{
    QVector<ClassEntryElement> const *entries = &entry->entries;
    qsizetype count = entries->count();
    Competitor *comp = entries->at(0).competitor;
    qsizetype leg;

    /* Slim down the list removing impossible category types */
    removeImpossibleCategories(entry->entries);

    /* Slim down the list of the first leg scanning the lists of all other legs */
    Category const *cat;
    QMutableListIterator i { comp->getCategories() };
    while (i.hasNext()) {
        cat = i.next();

        /* Category must be present in all the legs */
        for (leg = 1; leg < count; leg++) {
            if (!entries->at(leg).hasCategory(cat)) {
                qDebug() <<  tr("Dropping category '%1' associated to competitor %2 - bib %3 - leg 1").arg(i.peekPrevious()->getFullDescription(), comp->getCompetitorName(CRHelper::nameComposition), QString::number(entry->bib));
                i.remove();
                break;
            }
        }
    }

    /* Slim down the list removing categories having same type but lower weight */
    removeLowerWeigthCategories(comp->getCategories(), comp->getCompetitorName(CRHelper::nameComposition), entry->bib);

    /* Slim down the list of all other legs using the first one as reference */
    for (leg = 1; leg < count; leg++) {
        if (!(comp = entries->at(leg).competitor))
            continue;

        i = comp->getCategories();
        while (i.hasNext()) {
            if (!entry->entries[0].hasCategory(i.next())) {
                qDebug() <<  tr("Dropping category '%1' associated to competitor %2 - bib %3 - leg 1").arg(i.peekPrevious()->getFullDescription(), comp->getCompetitorName(CRHelper::nameComposition), QString::number(entry->bib));
                i.remove();
            }
        }
    }

    entry->category = entries->at(0).competitor->getCategory();

    if (entry->category == Q_NULLPTR)
        messages += tr("No category fits the team with bib %1").arg(entry->bib);
}
