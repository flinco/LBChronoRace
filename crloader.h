#ifndef LBLOADER_H
#define LBLOADER_H

#include <QCoreApplication>
#include <QString>
#include <QVector>
#include <QMap>
#include <QMultiMap>
#include <QStandardItem>
#include <QDataStream>

#include "timing.h"
#include "category.h"

#include "startlistmodel.h"
#include "teamslistmodel.h"
#include "timingsmodel.h"
#include "categoriesmodel.h"

namespace ChronoRace {
class CRLoader;
}

class CRLoader
{
    Q_DECLARE_TR_FUNCTIONS(CRLoader)

public:
    enum Encoding
        {
            LATIN1,
            UTF8
        };

    enum Format
        {
            PDF,
            TEXT,
            CSV
        };

private:
    static StartListModel              startListModel;
    static TeamsListModel              teamsListModel;
    static TimingsModel                timingsModel;
    static CategoriesModel             categoriesModel;
    static QList<QVariant>             standardItemList;
    static Encoding                    encoding;
    static Format                      format;

    static void loadCSV(const QString& filePath, QAbstractTableModel* model);
    static void saveCSV(const QString& filePath, const QAbstractTableModel* model);

    static void checkString(QAbstractTableModel* model, QString& token, QChar character = 0);

public:
    static void saveRaceData(QDataStream &out);
    static void loadRaceData(QDataStream &in);
    static QPair<int, int> importStartList(const QString& path);
    static void exportStartList(const QString& path);
    static void exportTeams(const QString& path);
    static QList<Competitor> getStartList();
    static QMultiMap<uint, Competitor> getCompetitors(QStringList& messages);
    static uint getStartListLegs();
    static void setStartListLegs(uint leg);
    static uint getStartListBibMax();
    static uint getStartListNameWidthMax();
    static uint getTeamNameWidthMax();
    static int importTimings(const QString& path);
    static void exportTimings(const QString& path);
    static QVector<Timing> getTimings();
    static int importCategories(const QString& path);
    static void exportCategories(const QString& path);
    static QVector<Category> getCategories();
    static QAbstractTableModel* getStartListModel();
    static QAbstractTableModel* getTeamsListModel();
    static QAbstractTableModel* getTimingsModel();
    static QAbstractTableModel* getCategoriesModel();
    static Encoding getEncoding();
    static void setEncoding(const Encoding &value);
    static Format getFormat();
    static void setFormat(const Format &value);
};

#endif // LBLOADER_H
