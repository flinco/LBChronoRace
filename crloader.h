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
#include "competitor.h"

#include "crtablemodel.h"
#include "startlistmodel.h"
#include "teamslistmodel.h"
#include "timingsmodel.h"
#include "categoriesmodel.h"

namespace loader {
class CRLoader;
}

class CRLoader
{
    Q_DECLARE_TR_FUNCTIONS(CRLoader)

public:
    enum class Encoding
    {
        LATIN1,
        UTF8
    };

    enum class Format
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

    static void loadCSV(QString const &filePath, QAbstractTableModel *model);
    static void saveCSV(QString const &filePath, QAbstractTableModel const *model);

    static void checkString(QAbstractTableModel *model, QString &token, QChar character = '\0');

public:
    static void saveRaceData(QDataStream &out);
    static void loadRaceData(QDataStream &in);
    static QPair<int, int> importStartList(QString const &path);
    static void exportStartList(QString const &path);
    static void exportTeams(QString const &path);
    static QList<Competitor> getStartList();
    static uint getStartListLegs();
    static void setStartListLegs(uint leg);
    static uint getStartListBibMax();
    static uint getStartListNameWidthMax();
    static uint getTeamNameWidthMax();
    static int importTimings(QString const &path);
    static void exportTimings(QString const &path);
    static QVector<Timing> getTimings();
    static int importCategories(QString const &path);
    static void exportCategories(QString const &path);
    static QVector<Category> getCategories();
    static CRTableModel *getStartListModel();
    static CRTableModel *getTeamsListModel();
    static CRTableModel *getTimingsModel();
    static CRTableModel *getCategoriesModel();
    static Encoding getEncoding();
    static void setEncoding(Encoding const &value);
    static Format getFormat();
    static void setFormat(Format const &value);
};

#endif // LBLOADER_H
