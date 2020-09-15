#include <QSet>
#include <QFile>
#include <QTextStream>
#include <QTextEncoder>
#include <QPushButton>

#include "crloader.h"
#include "lbcrexception.h"

// Members initialization
StartListModel              CRLoader::startListModel;
TeamsListModel              CRLoader::teamsListModel;
TimingsModel                CRLoader::timingsModel;
CategoriesModel             CRLoader::categoriesModel;
QList<QVariant>             CRLoader::standardItemList;
CRLoader::Encoding          CRLoader::encoding              = CRLoader::LATIN1;
CRLoader::Format            CRLoader::format                = CRLoader::PDF;

QAbstractTableModel* CRLoader::getStartListModel()
{
    return &startListModel;
}

QAbstractTableModel* CRLoader::getTeamsListModel()
{
    return &teamsListModel;
}

QAbstractTableModel* CRLoader::getTimingsModel()
{
    return &timingsModel;
}

QAbstractTableModel* CRLoader::getCategoriesModel()
{
    return &categoriesModel;
}

CRLoader::Encoding CRLoader::getEncoding()
{
    return encoding;
}

void CRLoader::setEncoding(const Encoding &value)
{
    encoding = value;
}

CRLoader::Format CRLoader::getFormat()
{
    return format;
}

void CRLoader::setFormat(const Format &value)
{
    format = value;
}

void CRLoader::loadCSV(const QString& filePath, QAbstractTableModel* model)
{
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

void CRLoader::saveCSV(const QString &filePath, const QAbstractTableModel* model)
{
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

void CRLoader::saveRaceData(QDataStream &out)
{
    out << startListModel
        << teamsListModel
        << categoriesModel
        << timingsModel;

}

void CRLoader::loadRaceData(QDataStream &in)
{
    startListModel.reset();
    teamsListModel.reset();
    categoriesModel.reset();
    timingsModel.reset();

    in >> startListModel
       >> teamsListModel
       >> categoriesModel
       >> timingsModel;
}

QPair<int, int> CRLoader::importStartList(const QString& path)
{

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

void CRLoader::exportStartList(const QString& path)
{
    saveCSV(path, &startListModel);
}

void CRLoader::exportTeams(const QString& path)
{

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

QList<Competitor> CRLoader::getStartList()
{
    return QList<Competitor>(startListModel.getStartList());
}

QMultiMap<uint, Competitor> CRLoader::getCompetitors(QStringList& messages)
{
    uint bib, mask, prevMask;
    int offset;
    QMultiMap<uint, Competitor> startList;
    QMap<uint, Competitor>::const_iterator element;
    QMap<uint, uint> legCount;

    for (auto comp : startListModel.getStartList()) {
        bib = comp.getBib();
        element = startList.find(bib);
        if (element != startList.constEnd()) {
            // check if there is a leg set for the competitor
            // otherwise set it automatically
            offset = comp.getOffset();
            comp.setLeg((uint) ((offset < 0) ? qAbs(offset) : (startList.count(bib) + 1)));

            // set a bit in the leg count array
            mask = 0x1 << comp.getLeg();
            if (legCount.contains(bib))
                legCount[bib] |= mask;
            else
                legCount.insert(bib, mask);
        }
        startList.insert(bib, comp);
    }

    prevMask = 0;
    for (auto bib : legCount.keys()) {
        mask = legCount.value(bib);
        if (prevMask && (mask != prevMask)) {
            messages << tr("Missing or extra legs for bib %1").arg(bib);
        }
        prevMask = mask;
    }

    return startList;
}


uint CRLoader::getStartListLegs()
{
    return startListModel.getLegCount();
}

uint CRLoader::getStartListBibMax()
{
    return startListModel.getMaxBib();
}

uint CRLoader::getStartListNameWidthMax()
{
    return startListModel.getCompetitorNameMaxWidth();
}

uint CRLoader::getTeamNameWidthMax()
{
    return teamsListModel.getTeamNameWidthMax();
}

int CRLoader::importTimings(const QString& path)
{

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

void CRLoader::exportTimings(const QString& path)
{
    saveCSV(path, &timingsModel);
}

QVector<Timing> CRLoader::getTimings()
{
    return QVector<Timing>::fromList(timingsModel.getTimings());
}

int CRLoader::importCategories(const QString& path)
{

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

void CRLoader::exportCategories(const QString& path)
{
    saveCSV(path, &categoriesModel);
}

QVector<Category> CRLoader::getCategories()
{
    return QVector<Category>::fromList(categoriesModel.getCategories());
}

void CRLoader::checkString(QAbstractTableModel* model, QString& token, QChar character)
{

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
