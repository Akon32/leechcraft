/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef PUBLICHUBS_H
#define PUBLICHUBS_H

#include <QWidget>
#include <QCloseEvent>
#include <QMenu>
#include <QPixmap>
#include <QSortFilterProxyModel>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"

#include "ArenaWidget.h"
#include "WulforUtil.h"
#include "PublicHubModel.h"
#include "Func.h"

#include "ui_UIPublicHubs.h"

class PublicHubsCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1207);

    PublicHubsCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~PublicHubsCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class PublicHubs :
        public  QWidget,
        public  dcpp::Singleton<PublicHubs>,
        public  ArenaWidget,
        public  dcpp::FavoriteManagerListener,
        private Ui::UIPublicHubs
{
Q_OBJECT
Q_INTERFACES(ArenaWidget)
friend class dcpp::Singleton<PublicHubs>;

public:
    QString  getArenaTitle(){ return tr("Public Hubs"); }
    QString  getArenaShortTitle(){ return getArenaTitle(); }
    QWidget *getWidget(){ return this; }
    QMenu   *getMenu(){ return NULL; }
    const QPixmap &getPixmap(){ return WulforUtil::getInstance()->getPixmap(WulforUtil::eiSERVER); }
    void CTRL_F_pressed() { slotFilter(); }
    ArenaWidget::Role role() const { return ArenaWidget::PublicHubs; }

    bool isFindFrameActivated();

public slots:
    void slotFilter();

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void customEvent(QEvent *);

    virtual void on(DownloadStarting, const std::string& l) throw();
    virtual void on(DownloadFailed, const std::string& l) throw();
    virtual void on(DownloadFinished, const std::string& l) throw();
    virtual void on(LoadedFromCache, const std::string& l) throw();

private slots:
    void slotContextMenu();
    void slotHeaderMenu();
    void slotHubChanged(int);
    void slotFilterColumnChanged();
    void slotDoubleClicked(const QModelIndex&);

private:
    PublicHubs(QWidget *parent = NULL);
    ~PublicHubs();

    void setStatus(QString);

    void onFinished(QString);
    void updateList();

    dcpp::HubEntryList entries;
    PublicHubModel *model;
    PublicHubProxyModel *proxy;
};

#endif // PUBLICHUBS_H
