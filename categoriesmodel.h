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
    CategoriesModel(QList<Category> const &categoriesList, QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), categories(categoriesList) { };

    friend QDataStream &operator<<(QDataStream &out, CategoriesModel const &data);
    friend QDataStream &operator>>(QDataStream &in, CategoriesModel &data);

    int rowCount(QModelIndex const &parent = QModelIndex()) const override;
    int columnCount(QModelIndex const &parent = QModelIndex()) const override;
    QVariant data(QModelIndex const &index, int role) const override;
    bool setData(QModelIndex const &index, QVariant const &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(QModelIndex const &index) const override;
    bool insertRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    bool removeRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QList<Category> const &getCategories() const;

    void reset();

public slots:
    void refreshCounters(int r) override;

private:
    QList<Category> categories;

signals:
    void error(QString const &message);
};

#endif // LBCATEGORIESMODEL_H
