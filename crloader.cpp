#include <QSet>
#include <QFile>
#include <QTextStream>
#include <QTextEncoder>
#include <QPushButton>

#include "crloader.h"
#include "lbcrexception.h"

// Members initialization
QMultiMap<uint, Competitor> CRLoader::startList;
StartListModel              CRLoader::startListModel;
int                         CRLoader::startListLegCount     = 0u;
uint                        CRLoader::startListBibMax       = 0u;
int                         CRLoader::startListNameWidthMax = 0;
TeamsListModel              CRLoader::teamsListModel;
int                         CRLoader::teamNameWidthMax      = 0;
QVector<Timing>             CRLoader::timings               = {};
TimingsModel                CRLoader::timingsModel;
QVector<Category>           CRLoader::categories            = {};
CategoriesModel             CRLoader::categoriesModel;
QList<QVariant>             CRLoader::standardItemList;
CRLoader::Encoding          CRLoader::encoding              = CRLoader::LATIN1;
CRLoader::Format            CRLoader::format                = CRLoader::CSV;

QAbstractTableModel* CRLoader::getStartListModel() {
    return &startListModel;
}

QAbstractTableModel* CRLoader::getTeamsListModel() {
    return &teamsListModel;
}

QAbstractTableModel* CRLoader::getTimingsModel() {
    return &timingsModel;
}

QAbstractTableModel* CRLoader::getCategoriesModel() {
    return &categoriesModel;
}

CRLoader::Encoding CRLoader::getEncoding() {
    return encoding;
}

void CRLoader::setEncoding(const Encoding &value) {
    encoding = value;
}

CRLoader::Format CRLoader::getFormat() {
    return format;
}

void CRLoader::setFormat(const Format &value) {
    format = value;
}

void CRLoader::clearStartList() {
    startList.clear();
    startListLegCount = 0u;
    startListBibMax = 0u;
    startListNameWidthMax = 0;
    teamNameWidthMax = 0;
}

void CRLoader::clearTimings() {
    timings.clear();
}

void CRLoader::clearCategories() {
    categories.clear();
}

void CRLoader::loadCSV(const QString& filePath, QAbstractTableModel* model) {

    QFile file (filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QString data;
        switch (encoding) {
            case UTF8:
                data = QString::fromUtf8(file.readAll());
                break;
            case LATIN1:
                // no break here
            default:
                data = QString::fromLatin1(file.readAll());
                break;
        }
        standardItemList.clear();
        data.remove( QRegExp("\r") ); //remove all ocurrences of CR (Carriage Return)
        QString temp;
        QChar character;
        QTextStream textStream(&data);
        while (!textStream.atEnd()) {
            textStream >> character;
            if ((character == ',') || (character == ';')) {
                checkString(model, temp, character);
            } else if (character == '\n') {
                checkString(model, temp, character);
            } else if (textStream.atEnd()) {
                temp.append(character);
                checkString(model, temp);
            } else {
                temp.append(character);
            }
        }
    }
}

void CRLoader::saveCSV(const QString &filePath, const QAbstractTableModel* model) {

    Q_ASSERT(model);

    QFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(filePath)));
    }
    QTextStream outStream(&outFile);

    switch (CRLoader::getEncoding()) {
        case CRLoader::UTF8:
            outStream.setCodec("UTF-8");
            break;
        case CRLoader::LATIN1:
            // no break here
        default:
            outStream.setCodec("ISO-8859-1");
            break;
    }

    int rowCount = model->rowCount();
    int columnCount = model->columnCount();
    for (int r = 0; r < rowCount; r++) {
        for (int c = 0; c < columnCount; c++) {
            if (c != 0) outStream << ',';
            model->index(r, c, QModelIndex());
            outStream << model->data(model->index(r, c, QModelIndex()), Qt::DisplayRole).toString();
        }
        outStream << Qt::endl;
    }

    outStream.flush();
    outFile.close();
}

QPair<int, int> CRLoader::loadStartList(const QString& path) {

    startListModel.reset();
    teamsListModel.reset();

    loadCSV(path, &startListModel);
    int rowCount = startListModel.rowCount();
    int columnCount = startListModel.columnCount();

    if ((columnCount) != Competitor::CMF_COUNT) {
        startListModel.reset();
        throw(ChronoRaceException(tr("Wrong number of columns; expected %1 - found %2").arg(Competitor::CMF_COUNT).arg(columnCount)));
    }

    return QPair<int, int>(rowCount, teamsListModel.rowCount());
}

void CRLoader::saveStartList(const QString& path) {
    saveCSV(path, &startListModel);
}

void CRLoader::saveTeams(const QString& path) {

    QFile outFile(path);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(path)));
    }
    QTextStream outStream(&outFile);

    switch (CRLoader::getEncoding()) {
        case CRLoader::UTF8:
            outStream.setCodec("UTF-8");
            break;
        case CRLoader::LATIN1:
            // no break here
        default:
            outStream.setCodec("ISO-8859-1");
            break;
    }

    int rowCount = teamsListModel.rowCount();
    for (int r = 0; r < rowCount; ++r)
        outStream << teamsListModel.data(teamsListModel.index(r, 0, QModelIndex()), Qt::DisplayRole).toString() << Qt::endl;

    outStream.flush();
    outFile.close();
}

QMultiMap<uint, Competitor>& CRLoader::getStartList(QStringList& messages) {

    bool legCountOk = false;
    uint lastBib = 0u;
    QMultiMap<uint, Competitor>::iterator element;

    clearStartList();
    element = startList.end();

    int rowCount = startListModel.rowCount();
    //int columnCount = model->columnCount();
    for (int r = 0; r < rowCount; r++) {
        for (Competitor::Field f = Competitor::CMF_FIRST; f < Competitor::CMF_COUNT; f++) {
            QVariant value = startListModel.data(startListModel.index(r, f, QModelIndex()), Qt::EditRole);

            if ((f != Competitor::CMF_BIB) && (element == startList.end())) {
                throw(ChronoRaceException(tr("Competitor not found")));
            }

            switch(f) {
                case Competitor::CMF_BIB:
                    {
                        uint startListBib = value.toUInt();
                        if (!lastBib) {
                            lastBib = startListBib;
                        }
                        if (legCountOk && (lastBib != startListBib)) {
                            // new bib, check
                            if (startList.count(lastBib) != startListLegCount) {
                                throw(ChronoRaceException(tr("Wrong leg components for bib %1 - expected %2 - found %3").arg(lastBib).arg(startListLegCount).arg(startList.count(startListBib))));
                            }
                        }

                        element = startList.insert(startListBib, Competitor(startListBib));
                        element->setLeg(startList.count(startListBib));

                        if (!legCountOk) {
                            if (lastBib == startListBib) {
                                // same bib, count
                                startListLegCount++;
                            } else {
                                // new bib, freeze
                                legCountOk = true;
                            }
                        }
                        lastBib = startListBib;

                        if (lastBib > startListBibMax) {
                            startListBibMax = lastBib;
                        }
                    }
                    break;
                case Competitor::CMF_NAME:
                    {
                        element->setName(value.toString());
                        if (startListNameWidthMax < element->getName().length())
                            startListNameWidthMax = element->getName().length();
                    }
                    break;
                case Competitor::CMF_SEX:
                    {
                        element->setSex(Competitor::toSex(value.toString()));
                    }
                    break;
                case Competitor::CMF_YEAR:
                    {
                        element->setYear(value.toUInt());
                    }
                    break;
                case Competitor::CMF_TEAM:
                    {
                        element->setTeam(value.toString());

                        if (teamNameWidthMax < element->getTeam().length())
                            teamNameWidthMax = element->getTeam().length();
                    }
                    break;
            case Competitor::CMF_OFFSET:
                {
                    element->setOffset(Competitor::toOffset(value.toString()));
                }
                break;
                default:
                    {
                        throw(ChronoRaceException(tr("Field enumeration value not managed!")));
                    }
            }
        }

        if ((element != startList.end()) && !element->isValid()) {
            messages << tr("Invalid competitor at row %1").arg(r + 1);
            element = startList.erase(element);
        }
    }

    return startList;
}


uint CRLoader::getStartListLegs() {
    return startListLegCount;
}

uint CRLoader::getStartListBibMax() {
    return startListBibMax;
}

int CRLoader::getStartListNameWidthMax() {
    return startListNameWidthMax;
}

int CRLoader::getTeamNameWidthMax() {
    return teamNameWidthMax;
}

int CRLoader::loadTimings(const QString& path) {

    timingsModel.reset();

    loadCSV(path, &timingsModel);

    int rowCount = timingsModel.rowCount();
    int columnCount = timingsModel.columnCount();

    if (columnCount != Timing::TMF_COUNT) {
        timingsModel.reset();
        throw(ChronoRaceException(tr("Wrong number of columns; expected %1 - found %2").arg(Timing::TMF_COUNT).arg(columnCount)));
    }

    return rowCount;
}

void CRLoader::saveTimings(const QString& path) {
    saveCSV(path, &timingsModel);
}

QVector<Timing>& CRLoader::getTimings(QStringList& messages) {

    clearTimings();

    int rowCount = timingsModel.rowCount();
    //int columnCount = model->columnCount();
    for (int r = 0; r < rowCount; r++) {
        for (Timing::Field f = Timing::TMF_FIRST; f < Timing::TMF_COUNT; f++) {
            QVariant value = timingsModel.data(timingsModel.index(r, f, QModelIndex()), Qt::DisplayRole);

            switch(f) {
                case Timing::TMF_BIB:
                    {
                        timings.push_back(Timing(value.toUInt()));
                    }
                    break;
                case Timing::TMF_TIME:
                    {
                        timings.back().setTiming(value.toString());
                    }
                    break;
                default:
                    {
                        throw(ChronoRaceException(tr("Field enumeration value not managed!")));
                    }
            }
        }

        if (!timings.back().isValid()) {
            messages << tr("Invalid timing at row %1").arg(r + 1);
            timings.removeLast();
        }
    }

    return timings;
}

int CRLoader::loadCategories(const QString& path) {

    categoriesModel.reset();

    loadCSV(path, &categoriesModel);

    int rowCount = categoriesModel.rowCount();
    int columnCount = categoriesModel.columnCount();

    if (columnCount != Category::CTF_COUNT) {
        categoriesModel.reset();
        throw(ChronoRaceException(tr("Wrong number of columns; expected %1 - found %2").arg(Category::CTF_COUNT).arg(columnCount)));
    }

    return rowCount;
}

void CRLoader::saveCategories(const QString& path) {
    saveCSV(path, &categoriesModel);
}

QVector<Category>& CRLoader::getCategories(QStringList& messages) {

    clearCategories();

    int rowCount = categoriesModel.rowCount();
    //int columnCount = model->columnCount();
    for (int r = 0; r < rowCount; r++) {
        for (Category::Field f = Category::CTF_FIRST; f < Category::CTF_COUNT; f++) {
            QVariant value = categoriesModel.data(categoriesModel.index(r, f, QModelIndex()), Qt::DisplayRole);

            switch(f) {
                case Category::CTF_TEAM:
                    {
                        categories.push_back(Category(value.toString()));
                    }
                    break;
                case Category::CTF_SEX:
                    {
                        categories.back().setSex(Competitor::toSex(value.toString()));
                    }
                    break;
                case Category::CTF_TO_YEAR:
                    {
                        categories.back().setToYear(value.toUInt());
                    }
                    break;
                case Category::CTF_FROM_YEAR:
                    {
                        categories.back().setFromYear(value.toUInt());
                    }
                    break;
                case Category::CTF_FULL_DESCR:
                    {
                        categories.back().setFullDescription(value.toString());
                    }
                    break;
                case Category::CTF_SHORT_DESCR:
                    {
                        categories.back().setShortDescription(value.toString());
                    }
                    break;
                default:
                    {
                        throw(ChronoRaceException(tr("Field enumeration value not managed!")));
                    }
            }
        }

        if (!categories.back().isValid()) {
            messages << tr("Invalid category at row %1").arg(r + 1);
            categories.removeLast();
        }
    }
    return categories;
}

void CRLoader::checkString(QAbstractTableModel* model, QString& token, QChar character) {

    Q_ASSERT(model);

    if (token.count("\"") % 2 == 0) {
        //if (temp.size() == 0 && character != ',') //problem with line endings
        //    return;
        if (token.startsWith( QChar('\"')) && token.endsWith( QChar('\"') ) ) {
             token.remove( QRegExp("^\"") );
             token.remove( QRegExp("\"$") );
        }
        //FIXME: will possibly fail if there are 4 or more reapeating double quotes
        token.replace("\"\"", "\"");
        standardItemList.append(QVariant(token));
        if ((character != QChar(',')) && (character != QChar(';'))) {
            int rowCount = model->rowCount();
            int c, columnCount = model->columnCount();
            if (standardItemList.size() != columnCount) {
                throw(ChronoRaceException(tr("Wrong number of elements in CSV row; expected %1 - found %2").arg(columnCount).arg(standardItemList.size())));
            }
            model->insertRow(rowCount, QModelIndex());
            c = 0;
            for (auto item : standardItemList) {
                model->setData(model->index(rowCount, c++, QModelIndex()), item, Qt::EditRole);
            }
            standardItemList.clear();
        }
        token.clear();
    } else {
        token.append(character);
    }
}
