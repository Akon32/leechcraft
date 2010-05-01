#include "TransferViewModel.h"

#include "WulforUtil.h"

#include <QtGui>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QPalette>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QFontMetrics>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QSize>
#include <QStyleOptionProgressBar>
#include <QHash>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ShareManager.h"
#include "dcpp/Util.h"

#define _DEBUG_ 1

#if _DEBUG_
#include <QtDebug>
#endif

#include <set>

TransferViewModel::TransferViewModel(QObject *parent)
    : QAbstractItemModel(parent), iconsScaled(false)
{
    QList<QVariant> rootData;
    rootData << tr("Users") << tr("Speed") << tr("Status") << tr("Size")
             << tr("Time left") << tr("File name") << tr("Host") << tr("IP");

    rootItem = new TransferViewItem(rootData, NULL);

    column_map.insert("USER", COLUMN_TRANSFER_USERS);
    column_map.insert("SPEED", COLUMN_TRANSFER_SPEED);
    column_map.insert("STAT", COLUMN_TRANSFER_STATS);
    column_map.insert("ESIZE", COLUMN_TRANSFER_SIZE);
    column_map.insert("TLEFT", COLUMN_TRANSFER_TLEFT);
    column_map.insert("FNAME", COLUMN_TRANSFER_FNAME);
    column_map.insert("HOST", COLUMN_TRANSFER_HOST);
    column_map.insert("IP", COLUMN_TRANSFER_IP);

    sortColumn = COLUMN_TRANSFER_SIZE;
    sortOrder = Qt::DescendingOrder;
}

TransferViewModel::~TransferViewModel()
{
    if (rootItem)
        delete rootItem;
}

int TransferViewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TransferViewItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant TransferViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TransferViewItem *item = reinterpret_cast<TransferViewItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole:
        {
            if (index.column() != 0)
                break;

            if (item->download)
                return WulforUtil::getInstance()->getPixmap(WulforUtil::eiDOWN).scaled(18, 18);
            else
                return WulforUtil::getInstance()->getPixmap(WulforUtil::eiUP).scaled(18, 18);
        }
        case Qt::DisplayRole:
        {
            if (index.column() == COLUMN_TRANSFER_SPEED)
                return WulforUtil::formatBytes(item->data(COLUMN_TRANSFER_SPEED).toDouble()) + tr("/s");
            else if (index.column() == COLUMN_TRANSFER_SIZE)
                return WulforUtil::formatBytes(item->data(COLUMN_TRANSFER_SIZE).toLongLong());
            else if (index.column() == COLUMN_TRANSFER_TLEFT){
                int time = item->data(COLUMN_TRANSFER_TLEFT).toInt();

                if (time < 0)
                    return QTime(0, 0, 0).toString("hh:mm:ss");
                else
                    return QTime(0, 0, 0).addSecs(time).toString("hh:mm:ss");
            }

            return item->data(index.column());
        }
        case Qt::TextAlignmentRole:
        {
            if (index.column() == COLUMN_TRANSFER_SPEED || index.column() == COLUMN_TRANSFER_SIZE)
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            else
                return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        case Qt::ForegroundRole:
        {
            break;
        }
        case Qt::BackgroundColorRole:
            break;
        case Qt::ToolTipRole:
        {
            if (index.column() == COLUMN_TRANSFER_FNAME)
                return item->target;

            break;
        }
    }

    return QVariant();
}

namespace {

template <Qt::SortOrder order>
struct Compare {
    void static sort(int col, QList<TransferViewItem*>& items) {
        qStableSort(items.begin(), items.end(), getAttrComp(col));
    }

    void static insertSorted(int col, QList<TransferViewItem*>& items, TransferViewItem* item) {
        QList<TransferViewItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
        items.insert(it, item);
    }

    private:
        typedef bool (*AttrComp)(const TransferViewItem * l, const TransferViewItem * r);
        AttrComp static getAttrComp(int column) {
            switch (column){
                 case COLUMN_TRANSFER_SIZE:
                     return NumCmp<COLUMN_TRANSFER_SIZE>;
                 case COLUMN_TRANSFER_SPEED:
                     return NumCmp<COLUMN_TRANSFER_SPEED>;
                 case COLUMN_TRANSFER_FNAME:
                     return AttrCmp<COLUMN_TRANSFER_FNAME>;
                 case COLUMN_TRANSFER_HOST:
                     return AttrCmp<COLUMN_TRANSFER_HOST>;
                 case COLUMN_TRANSFER_IP:
                     return AttrCmp<COLUMN_TRANSFER_IP>;
                 case COLUMN_TRANSFER_TLEFT:
                     return NumCmp<COLUMN_TRANSFER_TLEFT>;
                 case COLUMN_TRANSFER_STATS:
                     return AttrCmp<COLUMN_TRANSFER_STATS>;
                 case COLUMN_TRANSFER_USERS:
                     return AttrCmp<COLUMN_TRANSFER_USERS>;
                 default:
                     return AttrCmp<COLUMN_TRANSFER_USERS>;
            }

            return 0;
        }
        template <int i>
        bool static AttrCmp(const TransferViewItem * l, const TransferViewItem * r) {
            return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
        }
        template <int column>
        bool static NumCmp(const TransferViewItem * l, const TransferViewItem * r) {
            return Cmp(l->data(column).toULongLong(), r->data(column).toULongLong());
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
}

QVariant TransferViewModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex TransferViewModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TransferViewItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TransferViewItem*>(parent.internalPointer());

    TransferViewItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TransferViewModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TransferViewItem *childItem = static_cast<TransferViewItem*>(index.internalPointer());
    TransferViewItem *parentItem = childItem->parent();

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    if (parentItem != rootItem && !rootItem->childItems.contains(parentItem))
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TransferViewModel::rowCount(const QModelIndex &parent) const
{
    TransferViewItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TransferViewItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void TransferViewModel::sort(int column, Qt::SortOrder order) {
    sortColumn = column;
    sortOrder = order;

    if (!rootItem || rootItem->childItems.empty())
        return;

    if (column == -1)
        return;

    QHash<TransferViewItem*, QModelIndex> hash;

    for (int i = 0; i < rootItem->childCount(); i++){
        hash.insert(rootItem->child(i), createIndex(i, 0, rootItem->child(i)));
    }

    //emit layoutAboutToBeChanged();

    if (order == Qt::AscendingOrder)
        Compare<Qt::AscendingOrder>().sort(column, rootItem->childItems);
    else if (order == Qt::DescendingOrder)
        Compare<Qt::DescendingOrder>().sort(column, rootItem->childItems);

    //emit layoutChanged();

    for (int i = 0; i < rootItem->childCount(); i++){
        changePersistentIndex(hash.value(rootItem->child(i)), createIndex(i, 0, rootItem->child(i)));
    }
}

void TransferViewModel::initTransfer(VarMap params){
    if (params.empty())
        return;

    TransferViewItem *item, *to;

    if (!findTransfer(vstr(params["CID"]), vbol(params["DOWN"]), &item))
        return;

    bool needParent = (vstr(params["FNAME"]) != tr("File list"));

    if (needParent){
        to = getParent(vstr(params["TARGET"]), params);
        TransferViewItem *p = item->parent();

        if (p == to)
            return;

        moveTransfer(item, p, to);

        if (p != rootItem && p->childCount() == 0 && rootItem->childItems.contains(p)){
            beginRemoveRows(QModelIndex(), p->row(), p->row());
            {
                rootItem->childItems.removeAt(p->row());

                delete p;
            }
            endRemoveRows();
        }

        sort(sortColumn, sortOrder);
    }

    updateTransfer(params);
}

void TransferViewModel::addConnection(VarMap params){
    if (params.empty())
        return;

    bool bGroup = false;
    TransferViewItem *i;
    TransferViewItem *to = NULL;
    bool bDownload = vbol(params["DOWN"]);

    if (findTransfer(vstr(params["CID"]), vbol(params["DOWN"]), &i)) {
        return;
    } else if (bDownload) {
        bGroup = vbol(params["BGROUP"]);
        if (bGroup)
            to = getParent(vstr(params["TARGET"]), params);
    }

    QList<QVariant> data;

    data << params["USER"] << "" << params["STAT"] << "" << "" << params["FNAME"] << params["HOST"] << "";
    TransferViewItem *item = new TransferViewItem(data, (to && bGroup) ? to : rootItem);

    item->download = vbol(params["DOWN"]);
    item->cid = vstr(params["CID"]);

    if (item->download && bGroup)
        item->target = vstr(params["TARGET"]);

    transfer_hash.insertMulti(item->cid, item);

    if (!to)
        rootItem->appendChild(item);
    else
        to->appendChild(item);

    emit layoutChanged();
}

void TransferViewModel::updateTransfer(VarMap params){
    if (params.empty())
        return;

    TransferViewItem *item;

    if (!findTransfer(vstr(params["CID"]), vbol(params["DOWN"]), &item))
        return;

    QMap<QString, int>::const_iterator i = column_map.constBegin();

    for (; i != column_map.constEnd(); ++i){
        if (params.contains(i.key()))
            item->updateColumn(i.value(), params[i.key()]);
    }

    if (params.contains("DPOS"))
        item->dpos = vdbl(params["DPOS"]);
    if (params.contains("PERC"))
        item->percent = vdbl(params["PERC"]);
    if (params.contains("TARGET"))
        item->target = vstr(params["TARGET"]);
    if (params.contains("FAIL"))
        item->fail = vbol(params["FAIL"]);

    if (!vbol(params["DOWN"])){
        if (!rootItem->childItems.contains(item))
            rootItem->appendChild(item);
    }

    if (item->parent() != rootItem && rootItem->childItems.contains(item->parent())){
        if (params.contains("FPOS"))
            item->parent()->dpos = vlng(params["FPOS"]);

        updateParent(item->parent());
    }

    emit layoutChanged();
}

void TransferViewModel::removeTransfer(VarMap params){
    if (params.empty() || vstr(params["CID"]).isEmpty())
        return;

    QMultiHash<QString, TransferViewItem* >::iterator i = transfer_hash.find(vstr(params["CID"]));

    while (i != transfer_hash.end() && i.key() == vstr(params["CID"])){
        if (i.value()->download == vbol(params["DOWN"])){
            TransferViewItem *item = i.value();
            TransferViewItem *p = item->parent();

            beginRemoveRows(createIndexForItem(p), item->row(), item->row());
            {
                p->childItems.removeAt(item->row());
                delete item;
            }
            endRemoveRows();

            transfer_hash.erase(i);

            if (p != rootItem && p->childCount() == 0){
                beginRemoveRows(QModelIndex(), p->row(), p->row());
                {
                    rootItem->childItems.removeAt(p->row());

                    delete p;
                }
                endRemoveRows();
            }

            return;
        }

        ++i;
    }
}

bool TransferViewModel::findTransfer(const QString &cid, bool download, TransferViewItem **item){
    if (!item)
        return false;

    QMultiHash<QString, TransferViewItem* >::const_iterator i = transfer_hash.find(cid);

    while (i != transfer_hash.end() && i.key() == cid && !cid.isEmpty()){
        if (i.value()->download == download){
           *item = i.value();

           return true;
        }

        ++i;
    }

    return false;
}

bool TransferViewModel::findParent(const QString &target, TransferViewItem **item, bool download){
    if (!item)
        return false;

    foreach (TransferViewItem *i, rootItem->childItems){
        if ((i->download == download) && i->target == target && i->cid.isEmpty()){
            *item = i;

            return true;
        }
    }

    return false;
}

TransferViewItem *TransferViewModel::getParent(const QString &target, const VarMap &params){
    TransferViewItem *p;

    if (findParent(target, &p))
        return p;

    QList<QVariant> data;

    data << params["USER"] << 0 << "" << params["ESIZE"]
         << params["TLEFT"] << params["FNAME"] << params["HOST"] << "";

    p = new TransferViewItem(data, rootItem);
    p->download = true;
    p->target = target;
    p->dpos = vlng(params["FPOS"]);

    rootItem->appendChild(p);

    return p;
}

void TransferViewModel::moveTransfer(TransferViewItem *item, TransferViewItem *from, TransferViewItem *to){
    if (!(item && from && to) || !from->childItems.contains(item))
        return;

    beginRemoveRows(createIndexForItem(from), item->row(), item->row());
    {
        from->childItems.removeAt(item->row());
    }
    endRemoveRows();

    beginInsertColumns(createIndexForItem(to), to->childCount(), to->childCount());
    {
        to->appendChild(item);
    }
    endInsertColumns();
}

void TransferViewModel::updateParent(TransferViewItem *p){
    if (!p || p->childCount() < 1 || p == rootItem)
        return;

    QString users;
    QList<QString> hubs;
    int active = 0;
    double speed = 0.0;
    qint64 totalSize = 0;
    qint64 timeLeft = 0;
    double progress = 0.0;

    totalSize = vlng(p->data(COLUMN_TRANSFER_SIZE));

    foreach (TransferViewItem *i, p->childItems){
        if (!i->fail){
            active++;
            speed += vdbl(i->data(COLUMN_TRANSFER_SPEED));
        }

        if (!users.isEmpty())
            users += ", ";

        users += vstr(i->data(COLUMN_TRANSFER_USERS));

        if (!hubs.contains(vstr(i->data(COLUMN_TRANSFER_HOST))))
            hubs.append(vstr(i->data(COLUMN_TRANSFER_HOST)));
    }

    if (totalSize > 0)
        progress = (double)(p->dpos * 100.0) / totalSize;
    if (speed > 0)
        timeLeft = (totalSize - p->dpos) / speed;

    if (active)
        p->updateColumn(COLUMN_TRANSFER_STATS, tr("Downloaded "));
    else
        p->updateColumn(COLUMN_TRANSFER_STATS, tr("Waiting for slot "));

    QString stat = vstr(p->data(COLUMN_TRANSFER_STATS)) + WulforUtil::formatBytes(p->dpos)
                   + QString(" (%1%)").arg(progress, 0, 'f', 1)  + QString(tr(" from %1/%2 user(s)")).arg(active).arg(p->childCount());

    QString hubs_str = "";

    foreach(QString s, hubs)
        hubs += s + " ";

    if (vstr(p->data(COLUMN_TRANSFER_FNAME)).startsWith(tr("TTH: "))){
        QString name = vstr(p->data(COLUMN_TRANSFER_FNAME));
        name.remove(0, tr("TTH: ").length());

        p->updateColumn(COLUMN_TRANSFER_FNAME, name);
    }

    p->updateColumn(COLUMN_TRANSFER_USERS, users);
    p->updateColumn(COLUMN_TRANSFER_TLEFT, timeLeft);
    p->updateColumn(COLUMN_TRANSFER_HOST, hubs_str);
    p->updateColumn(COLUMN_TRANSFER_SPEED, speed);
    p->updateColumn(COLUMN_TRANSFER_STATS, stat);
    p->percent = p->percent == 100.0? 100.0 : progress;
}

void TransferViewModel::updateTransferPos(VarMap params, qint64 pos){
    if (params.empty() || !params.contains("CID"))
        return;

    TransferViewItem *item;

    if (!findTransfer(vstr(params["CID"]), vbol(params["DOWN"]), &item))
        return;

    item->dpos = pos;

    emit layoutChanged();
}

void TransferViewModel::finishParent(VarMap params){
    if (params.empty() || !params.contains("TARGET"))
        return;

    QString target = vstr(params["TARGET"]);
    TransferViewItem *p;

    if (!findParent(target, &p))
        return;

    p->updateColumn(COLUMN_TRANSFER_STATS, tr("Finished"));
    p->percent = 100.0;

    foreach (TransferViewItem *i, p->childItems){
        i->updateColumn(COLUMN_TRANSFER_STATS, tr("Finished"));
        i->percent = 100.0;
    }

    emit layoutChanged();
}

int TransferViewModel::getSortColumn() const {
    return sortColumn;
}

void TransferViewModel::setSortColumn(int c) {
    sortColumn = c;
}

Qt::SortOrder TransferViewModel::getSortOrder() const {
    return sortOrder;
}

void TransferViewModel::setSortOrder(Qt::SortOrder o) {
    sortOrder = o;
}

QModelIndex TransferViewModel::createIndexForItem(TransferViewItem *item){
    if (!rootItem || !item)
        return QModelIndex();

    QStack<TransferViewItem*> stack;
    TransferViewItem *root = item->parent();

    while (root && (root != rootItem)){
        stack.push(root);

        root = root->parent();
    }

    QModelIndex parent = QModelIndex();
    QModelIndex child;

    while (!stack.empty()){
        TransferViewItem *el = stack.pop();

        parent = index(el->row(), COLUMN_TRANSFER_FNAME, parent);
    }

    return index(item->row(), COLUMN_TRANSFER_FNAME, parent);
}

void TransferViewModel::clear(){
    blockSignals(true);

    qDeleteAll(rootItem->childItems);
    rootItem->childItems.clear();

    blockSignals(false);

    emit layoutChanged();
}

void TransferViewModel::repaint(){
    emit layoutChanged();
}

TransferViewItem::TransferViewItem(const QList<QVariant> &data, TransferViewItem *parent) :
    itemData(data),
    parentItem(parent),
    download(false),
    percent(0.0),
    dpos(0L),
    fail(false)
{
}

TransferViewItem::TransferViewItem(const TransferViewItem &item){
    itemData = item.itemData;
    download = item.download;
    cid = item.cid;
    target = item.target;
    percent = item.percent;
    dpos = item.dpos;
    fail = item.fail;
}
void TransferViewItem::operator=(const TransferViewItem &item){
    itemData = item.itemData;
    download = item.download;
    cid = item.cid;
    target = item.target;
    percent = item.percent;
    dpos = item.dpos;
    fail = item.fail;
}

TransferViewItem::~TransferViewItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);

    parentItem = NULL;
}

void TransferViewItem::appendChild(TransferViewItem *item) {
    item->parentItem = this;
    childItems.append(item);
}

TransferViewItem *TransferViewItem::child(int row) {
    return childItems.value(row);
}

int TransferViewItem::childCount() const {
    return childItems.count();
}

int TransferViewItem::columnCount() const {
    return itemData.count();
}

QVariant TransferViewItem::data(int column) const {
    return itemData.value(column);
}

TransferViewItem *TransferViewItem::parent() {
    return parentItem;
}

int TransferViewItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TransferViewItem*>(this));

    return -1;
}

void TransferViewItem::updateColumn(int column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}

TransferViewDelegate::TransferViewDelegate(QObject *parent):
        QStyledItemDelegate(parent)
{
}

TransferViewDelegate::~TransferViewDelegate(){
}

void TransferViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if (index.column() != COLUMN_TRANSFER_STATS){
        QStyledItemDelegate::paint(painter, option, index);

        return;
    }

    QStyleOptionProgressBarV2 progressBarOption;
    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.rect = option.rect;
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;

    TransferViewItem *item = reinterpret_cast<TransferViewItem*>(index.internalPointer());

    if (!item){
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    double percent = item->percent;

    QString status = item->data(COLUMN_TRANSFER_STATS).toString();

    progressBarOption.text = status;
    progressBarOption.progress = static_cast<int>(percent);

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
