#ifndef LBCATEGORIESMODEL_H
#define LBCATEGORIESMODEL_H

#include <QObject>
#include <QDataStream>

#include "crtablemodel.h"
#include "category.h"

class CategoriesModel : public CRTableModel
{
    Q_OBJECT

public:
    CategoriesModel(QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), categories() { }
    CategoriesModel(const QList<Category>& categoriesList, QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), categories(categoriesList) { }

    friend QDataStream &operator<<(QDataStream &out, const CategoriesModel &data);
    friend QDataStream &operator>>(QDataStream &in, CategoriesModel &data);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    void reset();

    const QList<Category>& getCategories() const;

public slots:
    void refreshCounters(int r);

private:
    QList<Category> categories;
};

#endif // LBCATEGORIESMODEL_H
