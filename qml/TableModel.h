#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QList>

class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableModel(QObject *parent = nullptr);


    Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::DisplayRole) override;

    // 数据，这三个必须实现
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
     QVector<QVector<QVariant>> m_modelData;
    // 水平表头
    QList<QString> m_horHeaderList;
};

#endif // TABLEMODEL_H
