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

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include "crloader.hpp"
#include "lbcrexception.hpp"

// Members initialization
StartListModel              CRLoader::startListModel;
TeamsListModel              CRLoader::teamsListModel;
TimingsModel                CRLoader::timingsModel;
CategoriesModel             CRLoader::categoriesModel;
QList<QVariant>             CRLoader::standardItemList;
CRLoader::Encoding          CRLoader::encoding              = CRLoader::Encoding::UTF8;
CRLoader::Format            CRLoader::format                = CRLoader::Format::PDF;

CRTableModel *CRLoader::getStartListModel()
{
    return &startListModel;
}

CRTableModel *CRLoader::getTeamsListModel()
{
    return &teamsListModel;
}

CRTableModel *CRLoader::getTimingsModel()
{
    return &timingsModel;
}

CRTableModel *CRLoader::getCategoriesModel()
{
    return &categoriesModel;
}

QStringList CRLoader::getClubs()
{
    return QStringList(teamsListModel.getTeamsList());
}

CRLoader::Encoding CRLoader::getEncoding()
{
    return encoding;
}

void CRLoader::setEncoding(Encoding const &value)
{
    encoding = value;
}

QString CRLoader::encodingToLabel(Encoding const &value)
{
    switch (value) {
    case CRLoader::Encoding::UTF8:
        return tr("UTF-8");
    case CRLoader::Encoding::LATIN1:
        return tr("ISO-8859-1 (Latin-1)");
    default:
        return tr("Unknown encoding %1").arg(static_cast<int>(value));
    }
}

CRLoader::Format CRLoader::getFormat()
{
    return format;
}

void CRLoader::setFormat(Format const &value)
{
    format = value;
}

QString CRLoader::formatToLabel(Format const &value)
{
    switch (value) {
    case CRLoader::Format::PDF:
        return tr("PDF");
    case CRLoader::Format::TEXT:
        return tr("Text");
    case CRLoader::Format::CSV:
        return tr("CSV");
    default:
        return tr("Unknown format %1").arg(static_cast<int>(value));
    }
}

void CRLoader::loadCSV(QString const &filePath, QAbstractTableModel *model)
{
    static QRegularExpression re("\r");

    QFile file (filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QString data;
        if (encoding == Encoding::UTF8)
            data = QString::fromUtf8(file.readAll());
        else /* if (encoding == Encoding::LATIN1) */ //NOSONAR
            data = QString::fromLatin1(file.readAll());
        standardItemList.clear();
        data.remove(re); //remove all ocurrences of CR (Carriage Return)
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

void CRLoader::saveCSV(QString const &filePath, QAbstractTableModel const *model)
{
    Q_ASSERT(model);

    QFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(filePath)));
    }
    QTextStream outStream(&outFile);

    if (CRLoader::getEncoding() == Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else /* if (encoding == Encoding::LATIN1) */ //NOSONAR
        outStream.setEncoding(QStringConverter::Latin1);

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
    standardItemList.clear();
    startListModel.reset();
    teamsListModel.reset();
    categoriesModel.reset();
    timingsModel.reset();

    in >> startListModel
       >> teamsListModel
       >> categoriesModel
       >> timingsModel;

    startListModel.refreshCounters(0);
    teamsListModel.refreshCounters(0);
    categoriesModel.refreshCounters(0);
    timingsModel.refreshCounters(0);
}

QPair<int, int> CRLoader::importStartList(QString const &path)
{

    startListModel.reset();
    teamsListModel.reset();

    loadCSV(path, &startListModel);
    int rowCount = startListModel.rowCount();

    if (int columnCount = startListModel.columnCount(); columnCount != static_cast<int>(Competitor::Field::CMF_COUNT)) {
        startListModel.reset();
        throw(ChronoRaceException(tr("Wrong number of columns; expected %1 - found %2").arg(static_cast<int>(Competitor::Field::CMF_COUNT)).arg(columnCount)));
    }

    return QPair<int, int>(rowCount, teamsListModel.rowCount());
}

void CRLoader::exportStartList(QString const &path)
{
    saveCSV(path, &startListModel);
}

void CRLoader::exportTeams(QString const &path)
{

    QFile outFile(path);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(path)));
    }
    QTextStream outStream(&outFile);

    if (CRLoader::getEncoding() == Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else /* if (encoding == Encoding::LATIN1) */ //NOSONAR
        outStream.setEncoding(QStringConverter::Latin1);

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

uint CRLoader::getStartListLegs()
{
    return startListModel.getLegCount();
}

void CRLoader::setStartListLegs(uint leg)
{
    startListModel.setLegCount(leg);
}

uint CRLoader::getStartListBibMax()
{
    return startListModel.getMaxBib();
}

uint CRLoader::getStartListNameWidthMax()
{
    return startListModel.getCompetitorNameWidthMax();
}

uint CRLoader::getTeamNameWidthMax()
{
    return startListModel.getTeamNameWidthMax();
}

int CRLoader::importTimings(QString const &path)
{

    timingsModel.reset();

    loadCSV(path, &timingsModel);

    int rowCount = timingsModel.rowCount();

    if (int columnCount = timingsModel.columnCount(); columnCount != static_cast<int>(Timing::Field::TMF_COUNT)) {
        timingsModel.reset();
        throw(ChronoRaceException(tr("Wrong number of columns; expected %1 - found %2").arg(static_cast<int>(Timing::Field::TMF_COUNT)).arg(columnCount)));
    }

    return rowCount;
}

void CRLoader::exportTimings(QString const &path)
{
    saveCSV(path, &timingsModel);
}

void CRLoader::clearTimings()
{
    timingsModel.reset();
}

void CRLoader::addTiming(QString const &bib, QString const &timing)
{
    QString temp;

    temp = bib;
    checkString(&timingsModel, temp, ',');
    temp = "0";
    checkString(&timingsModel, temp, ',');
    temp = timing;
    checkString(&timingsModel, temp);
}

QVector<Timing> CRLoader::getTimings()
{
    return QVector<Timing>::fromList(timingsModel.getTimings());
}

int CRLoader::importCategories(QString const &path)
{

    categoriesModel.reset();

    loadCSV(path, &categoriesModel);

    int rowCount = categoriesModel.rowCount();

    if (int columnCount = categoriesModel.columnCount(); columnCount != static_cast<int>(Category::Field::CTF_COUNT)) {
        categoriesModel.reset();
        throw(ChronoRaceException(tr("Wrong number of columns; expected %1 - found %2").arg(static_cast<int>(Category::Field::CTF_COUNT)).arg(columnCount)));
    }

    return rowCount;
}

void CRLoader::exportCategories(QString const &path)
{
    saveCSV(path, &categoriesModel);
}

QVector<Category> CRLoader::getCategories()
{
    return QVector<Category>::fromList(categoriesModel.getCategories());
}

void CRLoader::checkString(QAbstractTableModel *model, QString &token, QChar character)
{
    static QRegularExpression re1("^\"");
    static QRegularExpression re2("\"$");

    Q_ASSERT(model);

    if (token.count("\"") % 2 == 0) {
        //NOSONAR if (temp.size() == 0 && character != ',') //problem with line endings
        //NOSONAR     return;
        if (token.startsWith( QChar('\"')) && token.endsWith( QChar('\"') ) ) {
             token.remove(re1);
             token.remove(re2);
        }
        /* FIXME: will possibly fail if there are 4 or more reapeating double quotes */
        token.replace("\"\"", "\"");
        standardItemList.append(QVariant(token));
        if ((character != QChar(',')) && (character != QChar(';'))) {
            int columnIndex = 0;
            int rowCount = model->rowCount();
            if (int columnCount = model->columnCount(); columnCount != standardItemList.size()) {
                throw(ChronoRaceException(tr("Wrong number of elements in CSV row; expected %1 - found %2").arg(columnCount).arg(standardItemList.size())));
            }
            model->insertRow(rowCount, QModelIndex());
            for (auto const &item : standardItemList) {
                model->setData(model->index(rowCount, columnIndex, QModelIndex()), item, Qt::EditRole);
                columnIndex++;
            }
            standardItemList.clear();
        }
        token.clear();
    } else {
        token.append(character);
    }
}
