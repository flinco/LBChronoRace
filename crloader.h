#ifndef LBLOADER_H
#define LBLOADER_H

#include <QCoreApplication>
#include <QString>
#include <QVector>
#include <QMap>
#include <QMultiMap>
#include <QStandardItem>

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
            CSV,
            TEXT
        };

    static QMultiMap<uint, Competitor> startList;
    static StartListModel              startListModel;
    static int                         startListLegCount;
    static uint                        startListBibMax;
    static int                         startListNameWidthMax;
    static TeamsListModel              teamsListModel;
    static int                         teamNameWidthMax;
    static QVector<Timing>             timings;
    static TimingsModel                timingsModel;
    static QVector<Category>           categories;
    static CategoriesModel             categoriesModel;
    static QList<QVariant>             standardItemList;
    static Encoding                    encoding;
    static Format                      format;

private:
    static void clearStartList();
    static void clearTimings();
    static void clearCategories();

public:
    static void loadCSV(const QString& filePath, QAbstractTableModel* model);
    static void saveCSV(const QString& filePath, const QAbstractTableModel* model);
    static QPair<int, int> loadStartList(const QString& path);
    static void saveStartList(const QString& path);
    static void saveTeams(const QString& path);
    static QMultiMap<uint, Competitor>& getStartList(QStringList& messages);
    static uint getStartListLegs();
    static uint getStartListBibMax();
    static int getStartListNameWidthMax();
    static int getTeamNameWidthMax();
    static int loadTimings(const QString& path);
    static void saveTimings(const QString& path);
    static QVector<Timing>& getTimings(QStringList& messages);
    static int loadCategories(const QString& path);
    static void saveCategories(const QString& path);
    static QVector<Category>& getCategories(QStringList& messages);
    static void checkString(QAbstractTableModel* model, QString& token, QChar character = 0);
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
