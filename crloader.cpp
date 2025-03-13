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
#include "lbchronorace.hpp"
#include "lbcrexception.hpp"

// Members initialization
StartListModel              CRLoader::startListModel;
TeamsListModel              CRLoader::teamsListModel;
TimingsModel                CRLoader::timingsModel;
RankingsModel               CRLoader::rankingsModel;
CategoriesModel             CRLoader::categoriesModel;
QList<QVariant>             CRLoader::standardItemList;
QStringConverter::Encoding  CRLoader::encoding              = QStringConverter::Encoding::Latin1;
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

CRTableModel *CRLoader::getRankingsModel()
{
    return &rankingsModel;
}

CRTableModel *CRLoader::getCategoriesModel()
{
    return &categoriesModel;
}

QStringList CRLoader::getClubs()
{
    return QStringList(teamsListModel.getTeamsList());
}

QStringConverter::Encoding CRLoader::getEncoding()
{
    return encoding;
}

void CRLoader::setEncoding(QStringConverter::Encoding const &value)
{
    encoding = value;
}

CRLoader::Format CRLoader::getFormat()
{
    return format;
}

void CRLoader::setFormat(Format const &value)
{
    format = value;
}

void CRLoader::loadCSV(QString const &filePath, QAbstractTableModel *model)
{
    static QRegularExpression re("\r");

    QFile file (filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QString data;
        if (encoding == QStringConverter::Encoding::Utf8)
            data = QString::fromUtf8(file.readAll());
        else /* if (encoding == QStringConverter::Encoding::Latin1) */ //NOSONAR
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
        throw(ChronoRaceException(tr("Error: unable to open %1").arg(filePath)));
    }
    QTextStream outStream(&outFile);

    outStream.setEncoding(CRLoader::getEncoding());

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
        << rankingsModel
        << categoriesModel
        << timingsModel;

}

void CRLoader::loadRaceData(QDataStream &in)
{
    standardItemList.clear();
    startListModel.reset();
    teamsListModel.reset();
    rankingsModel.reset();
    categoriesModel.reset();
    timingsModel.reset();

    in >> startListModel
       >> teamsListModel;
    if (LBChronoRace::binFormat > LBCHRONORACE_BIN_FMT_v3) {
        in >> rankingsModel;
    }
    in >> categoriesModel
       >> timingsModel;

    startListModel.refreshCounters(0);
    teamsListModel.refreshCounters(0);
    rankingsModel.refreshCounters(0);
    categoriesModel.refreshCounters(0);
    timingsModel.refreshCounters(0);
}

QPair<int, int> CRLoader::importStartList(QString const &path, bool append)
{
    int startListCount = 0;
    int teamsListCount = 0;

    if (append) {
        startListCount = startListModel.rowCount();
        teamsListCount = teamsListModel.rowCount();
    } else {
        startListModel.reset();
        teamsListModel.reset();
    }

    loadCSV(path, &startListModel);

    if (int columnCount = startListModel.columnCount(); columnCount != static_cast<int>(Competitor::Field::CMF_COUNT)) {
        if (!append)
            startListModel.reset();
        throw(ChronoRaceException(tr("Incorrect number of columns; expected %1 - found %2").arg(static_cast<int>(Competitor::Field::CMF_COUNT)).arg(columnCount)));
    }

    if (append) {
        startListCount = startListModel.rowCount() - startListCount;
        teamsListCount = teamsListModel.rowCount() - teamsListCount;
    } else {
        startListCount = startListModel.rowCount();
        teamsListCount = teamsListModel.rowCount();
    }

    return QPair<int, int>(startListCount, teamsListCount);
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

void CRLoader::addTiming(Action action, QString const &bib, QString const &timing)
{
    using enum CRLoader::Action;

    if (action == ADD) {
        QString temp;

        temp = bib;
        checkString(&timingsModel, temp, ',');
        temp = "0";
        checkString(&timingsModel, temp, ',');
        temp = timing;
        checkString(&timingsModel, temp, ',');
        temp = "CLS";
        checkString(&timingsModel, temp);
    } else if (action == END) {
        QString temp;

        QList<Competitor> startList { startListModel.getStartList() };

        // Remove all items for which a time has been recorded from the starting list
        for (auto const &time : std::as_const(timingsModel.getTimings())) {
            if (auto result = std::ranges::find_if(startList, [time](Competitor const &comp) { return comp.getBib() == time.getBib(); });
                result != startList.end()) {
                startList.erase(result);
            }
        }

        // Add any remaining competitor as DNF
        for (auto const &comp : std::as_const(startList)) {
            temp = QString::number(comp.getBib());
            checkString(&timingsModel, temp, ',');
            temp = "0";
            checkString(&timingsModel, temp, ',');
            temp = "0:00:00.000";
            checkString(&timingsModel, temp, ',');
            temp = "DNF";
            checkString(&timingsModel, temp);
        }
    } else if (action == BEGIN) {
        timingsModel.reset();
    } else {
        throw(ChronoRaceException(tr("Unexpected action value: %1 (add timing)").arg(static_cast<int>(action))));
    }
}

QList<Timing> const &CRLoader::getTimings()
{
    return timingsModel.getTimings();
}

int CRLoader::importTeams(QString const &path, bool append)
{
    int rowCount = 0;

    if (append)
        rowCount = teamsListModel.rowCount();
    else
        teamsListModel.reset();

    loadCSV(path, &teamsListModel);

    if (int columnCount = teamsListModel.columnCount(); columnCount != 1) {
        if (!append)
            teamsListModel.reset();
        throw(ChronoRaceException(tr("Incorrect number of columns; expected %1 - found %2").arg(1).arg(columnCount)));
    }

    rowCount = teamsListModel.rowCount() - (append ? rowCount : 0);

    return rowCount;
}

int CRLoader::importCategories(QString const &path, bool append)
{
    int rowCount = 0;

    if (append)
        rowCount = categoriesModel.rowCount();
    else
        categoriesModel.reset();

    loadCSV(path, &categoriesModel);

    if (int columnCount = categoriesModel.columnCount(); columnCount != static_cast<int>(Category::Field::CTF_COUNT)) {
        if (!append)
            categoriesModel.reset();
        throw(ChronoRaceException(tr("Incorrect number of columns; expected %1 - found %2").arg(static_cast<int>(Category::Field::CTF_COUNT)).arg(columnCount)));
    }

    rowCount = categoriesModel.rowCount() - (append ? rowCount : 0);

    return rowCount;
}

int CRLoader::importRankings(QString const &path, bool append)
{
    int rowCount = 0;

    if (append)
        rowCount = rankingsModel.rowCount();
    else
        rankingsModel.reset();

    loadCSV(path, &rankingsModel);

    if (int columnCount = rankingsModel.columnCount(); columnCount != static_cast<int>(Ranking::Field::RTF_COUNT)) {
        if (!append)
            rankingsModel.reset();
        throw(ChronoRaceException(tr("Incorrect number of columns; expected %1 - found %2").arg(static_cast<int>(Ranking::Field::RTF_COUNT)).arg(columnCount)));
    }

    rowCount = rankingsModel.rowCount() - (append ? rowCount : 0);

    rankingsModel.parseCategories();

    return rowCount;
}

int CRLoader::importTimings(QString const &path, bool append)
{
    int rowCount = 0;

    if (append)
        rowCount = timingsModel.rowCount();
    else
        timingsModel.reset();

    loadCSV(path, &timingsModel);

    if (int columnCount = timingsModel.columnCount(); columnCount != static_cast<int>(Timing::Field::TMF_COUNT)) {
        if (!append)
            timingsModel.reset();
        throw(ChronoRaceException(tr("Incorrect number of columns; expected %1 - found %2").arg(static_cast<int>(Timing::Field::TMF_COUNT)).arg(columnCount)));
    }

    rowCount = timingsModel.rowCount() - (append ? rowCount : 0);

    return rowCount;
}

void CRLoader::exportModel(Model model, QString const &path)
{
    switch (model) {
        using enum CRLoader::Model;

        case STARTLIST:
            saveCSV(path, &startListModel);
            break;
        case TEAMSLIST:
            {
                QFile outFile(path);
                if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    throw(ChronoRaceException(tr("Error: unable to open %1").arg(path)));
                }
                QTextStream outStream(&outFile);

                outStream.setEncoding(CRLoader::getEncoding());

                int rowCount = teamsListModel.rowCount();
                for (int r = 0; r < rowCount; ++r)
                    outStream << teamsListModel.data(teamsListModel.index(r, 0, QModelIndex()), Qt::DisplayRole).toString() << Qt::endl;

                outStream.flush();
                outFile.close();
            }
            break;
        case RANKINGS:
            saveCSV(path, &rankingsModel);
            break;
        case CATEGORIES:
            saveCSV(path, &categoriesModel);
            break;
        case TIMINGS:
            saveCSV(path, &timingsModel);
            break;
        default:
            throw(ChronoRaceException(tr("Unexpected model value: %1 (export)").arg(static_cast<int>(model))));
            break;
    }
}

QList<Ranking> const &CRLoader::getRankings()
{
    return rankingsModel.getRankings();
}

QList<Category> const &CRLoader::getCategories()
{
    return categoriesModel.getCategories();
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
                throw(ChronoRaceException(tr("Incorrect number of elements in CSV row; expected %1 - found %2").arg(columnCount).arg(standardItemList.size())));
            }
            model->insertRow(rowCount, QModelIndex());
            for (auto const &item : std::as_const(standardItemList)) {
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
