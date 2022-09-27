#include "TableModel.h"

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}


QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //返回表头数据，无效的返回None
    if(role==Qt::DisplayRole){
        if(orientation==Qt::Horizontal){
            return m_horHeaderList.value(section,QString::number(section));
        }else if(orientation==Qt::Vertical){
            return QString::number(section);
        }
    }
    return QVariant();
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        if(orientation==Qt::Horizontal&&role==Qt::EditRole){
            m_horHeaderList[section]=value.toString();
            emit headerDataChanged(orientation, section, section);
            return true;
        }
    }
    return false;
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_modelData.count();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_horHeaderList.count();
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return m_modelData.at(index.row()).at(index.column());
    default:
        break;
    }
    return QVariant();
}
