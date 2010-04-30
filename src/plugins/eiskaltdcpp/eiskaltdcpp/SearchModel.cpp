/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QPalette>
#include <QColor>
#include <QDir>

#include "SearchModel.h"
#include "SearchFrame.h"
#include "WulforUtil.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/ShareManager.h"

//#define _DEBUG_MODEL_


#include <QtDebug>

using namespace dcpp;

#ifdef _DEBUG_MODEL_
#include <QtDebug>
#endif

void SearchProxyModel::sort(int column, Qt::SortOrder order){
    if (sourceModel())
        sourceModel()->sort(column, order);
}

SearchModel::SearchModel(QObject *parent):
        QAbstractItemModel(parent),
        sortColumn(COLUMN_SF_ESIZE),
        sortOrder(Qt::DescendingOrder),
        filterRole(SearchFrame::None)
{
    QList<QVariant> rootData;
    rootData << tr("Count") << tr("File") << tr("Ext") << tr("Size")
             << tr("Exact size") << tr("TTH")   << tr("Path") << tr("Nick")
             << tr("Free slots") << tr("Total slots")
             << tr("IP") << tr("Hub") << tr("Host");

    rootItem = new SearchItem(rootData);

    sortColumn = -1;
}

SearchModel::~SearchModel()
{
    foreach(SearchItem *i, rootItem->childItems)
        pool.destroy(i);

    rootItem->childItems.clear();

    delete rootItem;
}

int SearchModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<SearchItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant SearchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    SearchItem *item = static_cast<SearchItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
        {
            if (index.column() == COLUMN_SF_FILENAME && !item->isDir)
                return WulforUtil::getInstance()->getPixmapForFile(item->data(COLUMN_SF_FILENAME).toString());
            else if (index.column() == COLUMN_SF_FILENAME && item->isDir)
                return WulforUtil::getInstance()->getPixmap(WulforUtil::eiFOLDER_BLUE);
            break;
        }
        case Qt::DisplayRole:
            return item->data(index.column());
        case Qt::TextAlignmentRole:
        {
            const int i_column = index.column();
            bool align_center = (i_column == COLUMN_SF_ALLSLOTS) || (i_column == COLUMN_SF_EXTENSION) ||
                                (i_column == COLUMN_SF_FREESLOTS);
            bool align_right  = (i_column == COLUMN_SF_ESIZE) || (i_column == COLUMN_SF_SIZE ) || (i_column == COLUMN_SF_COUNT);

            if (align_center)
                return Qt::AlignCenter;
            else if (align_right)
                return Qt::AlignRight;

            break;
        }
        case Qt::ForegroundRole:
        {
            if (filterRole == static_cast<int>(SearchFrame::Highlight)){
                TTHValue t(_tq(item->data(COLUMN_SF_TTH).toString()));

                if (ShareManager::getInstance()->isTTHShared(t))
                    return QColor(0x1F, 0x8F, 0x1F);
            }

            break;
        }
        case Qt::BackgroundColorRole:
            break;
        case Qt::ToolTipRole:
            break;
    }

    return QVariant();
}

void SearchModel::repaint(){
    emit layoutChanged();
}

Qt::ItemFlags SearchModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SearchModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex SearchModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SearchItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SearchItem*>(parent.internalPointer());

    SearchItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex SearchModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    SearchItem *childItem = static_cast<SearchItem*>(index.internalPointer());
    SearchItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int SearchModel::rowCount(const QModelIndex &parent) const
{
    SearchItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SearchItem*>(parent.internalPointer());

    return parentItem->childCount();
}

QModelIndex SearchModel::createIndexForItem(SearchItem *item){
    if (!rootItem || !item)
        return QModelIndex();

    QStack<SearchItem*> stack;
    SearchItem *root = item->parent();

    while (root && (root != rootItem)){
        stack.push(root);

        root = root->parent();
    }

    QModelIndex parent = QModelIndex();
    QModelIndex child;

    while (!stack.empty()){
        SearchItem *el = stack.pop();

        parent = index(el->row(), COLUMN_SF_FILENAME, parent);
    }

    return index(item->row(), COLUMN_SF_FILENAME, parent);
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<SearchItem*>& items) {
#ifdef _DEBUG_MODEL_
        qDebug() << "Sorting by " << col << " column and " << WulforUtil::getInstance()->sortOrderToInt(order) << " order.";
#endif
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<SearchItem*>& items, SearchItem* item) {
        QList<SearchItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
#ifdef _DEBUG_MODEL_
        qDebug() << "Item inserted at " << items.indexOf(*it) << " position";
#endif
    }

    private:
        typedef bool (*AttrComp)(const SearchItem * l, const SearchItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                case COLUMN_SF_FILENAME:
                    return AttrCmp<COLUMN_SF_FILENAME>;
                case COLUMN_SF_EXTENSION:
                    return AttrCmp<COLUMN_SF_EXTENSION>;
                case COLUMN_SF_TTH:
                    return AttrCmp<COLUMN_SF_TTH>;
                case COLUMN_SF_PATH:
                    return AttrCmp<COLUMN_SF_PATH>;
                case COLUMN_SF_NICK:
                    return AttrCmp<COLUMN_SF_NICK>;
                case COLUMN_SF_IP:
                    return AttrCmp<COLUMN_SF_IP>;
                case COLUMN_SF_HUB:
                    return AttrCmp<COLUMN_SF_HUB>;
                case COLUMN_SF_HOST:
                    return AttrCmp<COLUMN_SF_HOST>;
                case COLUMN_SF_SIZE:
                    return NumCmp<COLUMN_SF_ESIZE>;
                case COLUMN_SF_ESIZE:
                    return NumCmp<COLUMN_SF_ESIZE>;
                case COLUMN_SF_COUNT:
                    return NumCmp<COLUMN_SF_COUNT>;
                case COLUMN_SF_FREESLOTS:
                    return NumCmp<COLUMN_SF_FREESLOTS>;
                case COLUMN_SF_ALLSLOTS:
                    return NumCmp<COLUMN_SF_ALLSLOTS>;
            }

            throw SearchListException(QString("%1:%2 invalid sort column: %3").arg(__func__).arg(__LINE__).arg(column), SearchListException::Sort);
        }
        template <int i>
        bool static AttrCmp(const SearchItem * l, const SearchItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <typename T, T (SearchItem::*attr)>
        bool static AttrCmp(const SearchItem * l, const SearchItem * r) {
            return Cmp(l->*attr, r->*attr);
        }
        template <int i>
        bool static NumCmp(const SearchItem * l, const SearchItem * r) {
            return Cmp(l->data(i).toULongLong(), r->data(i).toULongLong());
        }
        template <typename T>
        bool static Cmp(const T& l, const T& r);
};

template <> template <typename T>
bool inline Compare<Qt::AscendingOrder>::Cmp(const T& l, const T& r) {
    return l < r;
}

template <> template <typename T>
bool inline Compare<Qt::DescendingOrder>::Cmp(const T& l, const T& r) {
    return l > r;
}

} //namespace

void SearchModel::sort(int column, Qt::SortOrder order) {
    sortColumn = column;
    sortOrder = order;
#ifdef _DEBUG_MODEL_
    if (sortColumn != column)
        qDebug() << "Sorting by " << column << " column and " << WulforUtil::getInstance()->sortOrderToInt(order) << " order.";
#endif

    emit layoutAboutToBeChanged();

    try {
        if (order == Qt::AscendingOrder)
            Compare<Qt::AscendingOrder>().sort(column, rootItem->childItems);
        else if (order == Qt::DescendingOrder)
            Compare<Qt::DescendingOrder>().sort(column, rootItem->childItems);
    }
    catch (SearchListException &e){
        sort(COLUMN_SF_FILENAME, order);
    }

    emit layoutChanged();
}

bool SearchModel::hasChildren(const QModelIndex &parent) const{
    if (!parent.isValid())
        return true;

    SearchItem *item = reinterpret_cast<SearchItem*>(parent.internalPointer());

    return (item->childCount() > 0);
}

bool SearchModel::canFetchMore(const QModelIndex &) const{
    return true;
}

bool SearchModel::addResultPtr(const QMap<QString, QVariant> &map){
    try {
        return addResult(map["FILE"].toString(),
                  map["SIZE"].toULongLong(),
                  map["TTH"].toString(),
                  map["PATH"].toString(),
                  map["NICK"].toString(),
                  map["FSLS"].toULongLong(),
                  map["ASLS"].toULongLong(),
                  map["IP"].toString(),
                  map["HUB"].toString(),
                  map["HOST"].toString(),
                  map["CID"].toString(),
                  map["ISDIR"].toBool());
    }
    catch (SearchListException){
        return false;
    }
}

bool SearchModel::addResult
        (
        const QString &file,
        qulonglong size,
        const QString &tth,
        const QString &path,
        const QString &nick,
        const int free_slots,
        const int all_slots,
        const QString &ip,
        const QString &hub,
        const QString &host,
        const QString &cid,
        const bool isDir
        )
{
    if (file.isEmpty() || file.isNull())
        return false;

    SearchItem *item;

    QFileInfo file_info(QDir::toNativeSeparators(file));
    QString ext = "";

    if (size > 0)
        ext = file_info.suffix().toUpper();

    SearchItem * parent = NULL;

    if (!isDir && tths.contains(tth)) {
        parent = tths[tth];
        if (parent->exists(cid))
            return false;
    } else {
        parent = rootItem;
    }

    QList<QVariant> item_data;

    item_data << QVariant() << file << ext << WulforUtil::formatBytes(size)
              << size << tth << path << nick << free_slots
              << all_slots << ip << hub << host,

    item = pool.construct(item_data, parent);

    if (!item)
        throw SearchListException();

    item->isDir = isDir;
    item->cid = cid;

    if (parent == rootItem && !isDir)
        tths.insert(tth, item);
    else {
        parent->appendChild(item);

        if (sortColumn != COLUMN_SF_COUNT){
            emit layoutChanged();
        }
        else
            sort(sortColumn, sortOrder);

        return true;
    }

    if (sortOrder == Qt::AscendingOrder)
        Compare<Qt::AscendingOrder>().insertSorted(sortColumn, rootItem->childItems, item);
    else if (sortOrder == Qt::DescendingOrder)
        Compare<Qt::DescendingOrder>().insertSorted(sortColumn, rootItem->childItems, item);

    emit layoutChanged();

    return true;
}

int SearchModel::getSortColumn() const {
    return sortColumn;
}

void SearchModel::setSortColumn(int c) {
    sortColumn = c;
}

Qt::SortOrder SearchModel::getSortOrder() const {
    return sortOrder;
}

void SearchModel::setSortOrder(Qt::SortOrder o) {
    sortOrder = o;
}

void SearchModel::clearModel(){

#ifdef _DEBUG_MODEL_
    qDebug() << "Cleaning the model.";
#endif
    QList<SearchItem*> &childs = rootItem->childItems;

    foreach(SearchItem *i, childs)
        pool.destroy(i);

    rootItem->childItems.clear();

    tths.clear();

    reset();

    emit layoutChanged();

#ifdef _DEBUG_MODEL_
    qDebug() << "Cleaning done";
#endif
}

void SearchModel::removeItem(const SearchItem *item){
    if (!okToFind(item))
        return;

    QModelIndex i = createIndexForItem(const_cast<SearchItem*>(item));

    beginRemoveRows(i, item->row(), item->row());

    SearchItem *p = const_cast<SearchItem*>(item->parent());
    p->childItems.removeAt(item->row());

    if (tths[item->data(COLUMN_SF_TTH).toString()] == item)
        tths.remove(item->data(COLUMN_SF_TTH).toString());

    endRemoveRows();

    reset();

    delete item;
}

void SearchModel::setFilterRole(int role){
    filterRole = role;
}

bool SearchModel::okToFind(const SearchItem *item){
    if (!item)
        return false;

    if (!rootItem->childItems.contains(const_cast<SearchItem*>(item))){
        QString tth = item->data(COLUMN_SF_TTH).toString();

        SearchItem *tth_root = tths.value(tth);//try to find item by tth

        foreach (SearchItem *i, tth_root->childItems){
            if (item == i)
                return true;
        }
    }
    else
        return true;

    return false;
}

SearchItem::SearchItem(const QList<QVariant> &data, SearchItem *parent) :
    isDir(false),
    itemData(data),
    parentItem(parent)
{
}

SearchItem::~SearchItem()
{
    //All items allocated in pool that have auto delete
}

void SearchItem::appendChild(SearchItem *item) {
    childItems.append(item);
    count = childItems.size();
}

SearchItem *SearchItem::child(int row) {
    return childItems.value(row);
}

int SearchItem::childCount() const {
    return childItems.count();
}

int SearchItem::columnCount() const {
    return itemData.count();
}

QVariant SearchItem::data(int column) const {
    if (column == COLUMN_SF_COUNT && childItems.size() > 0 && parentItem != 0)
        return childItems.size()+1;

    return itemData.value(column);
}

SearchItem *SearchItem::parent() const{
    return parentItem;
}

int SearchItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<SearchItem*>(this));

    return 0;
}

bool SearchItem::exists(const QString &user_cid) const {
    if (childItems.isEmpty())
        return cid == user_cid;

    foreach(SearchItem *child, childItems) {
        if (child->cid == user_cid)
            return true;
    }
    return false;
}

SearchListException::SearchListException() :
    message("Unknown"), type(Unkn)
{}

SearchListException::SearchListException(const SearchListException &ex) :
    message(ex.message), type(ex.type)
{}

SearchListException::SearchListException(const QString& message, Type type) :
    message(message), type(type)
{}

SearchListException::~SearchListException(){
}

SearchListException &SearchListException::operator =(const SearchListException &ex2) {
    type = ex2.type;
    message = ex2.message;

    return (*this);
}
