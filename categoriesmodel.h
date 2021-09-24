#ifndef LBCATEGORIESMODEL_H
#define LBCATEGORIESMODEL_H

#include <QObject>
#include <QDataStream>

#include "crtablemodel.h"
#include "category.h"

class CategoriesModel : public CRTableModel
{
    Q_OBJECT
    using CRTableModel::CRTableModel;

public:
    CategoriesModel(const QList<Category>& categoriesList, QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), categories(categoriesList) { };

    friend QDataStream &operator<<(QDataStream &out, const CategoriesModel &data);
    friend QDataStream &operator>>(QDataStream &in, CategoriesModel &data);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void reset();

    const QList<Category>& getCategories() const;

public slots:
    void refreshCounters(int r) override;

private:
    QList<Category> categories;

signals:
    void error(QString const &message);
};

#endif // LBCATEGORIESMODEL_H
