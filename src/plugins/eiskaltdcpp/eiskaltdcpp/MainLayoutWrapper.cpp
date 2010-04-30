#include "MainLayoutWrapper.h"

#include <stdlib.h>
#include <string>
#include <iostream>

#include <QPushButton>
#include <QSize>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QtDebug>
#include <QTextCodec>
#include <QMessageBox>
#include <QKeyEvent>
#include <QFileDialog>
#include <QProgressBar>
#include <QFileDialog>
#include <QRegExp>
#ifdef FREE_SPACE_BAR
    #include <boost/filesystem.hpp>
#endif //FREE_SPACE_BAR
#include "HubFrame.h"
#include "HubManager.h"
#include "HashProgress.h"
#include "PMWindow.h"
#include "TransferView.h"
#include "ShareBrowser.h"
#include "QuickConnect.h"
#include "SearchFrame.h"
#include "Settings.h"
#include "FavoriteHubs.h"
#include "PublicHubs.h"
#include "FavoriteUsers.h"
#include "DownloadQueue.h"
#include "FinishedTransfers.h"
#include "AntiSpamFrame.h"
#include "IPFilterFrame.h"
#include "ToolBar.h"
#include "Magnet.h"
#include "SpyFrame.h"
#include "Notification.h"

#ifdef USE_ASPELL
#include "SpellCheck.h"
#endif

#include "core.h"

#include "UPnPMapper.h"
#include "WulforSettings.h"
#include "WulforUtil.h"

#include "Version.h"

#include "dcpp/DCPlusPlus.h"

#include "dcpp/HashManager.h"
#include "dcpp/forward.h"
#include "dcpp/QueueManager.h"
#include "dcpp/Thread.h"

using namespace std;

#ifndef WIN32
#include <unistd.h>
#include <signal.h>

void installHandlers(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1){
        std::cout << "Cannot handle SIGPIPE" << std::endl;
    }

    std::cout << "Signal handlers installed." << std::endl;
}
#endif

void callBack(void* x, const std::string& a)
{
	qDebug () << Q_FUNC_INFO << "Loading:" << a.c_str ();
}

MainLayoutWrapper* MainLayoutWrapper::S_StaticThis = 0;

void MainLayoutWrapper::Init (ICoreProxy_ptr proxy)
{
	S_StaticThis = this;
    dcpp::startup(callBack, NULL);
	dcpp::TimerManager::getInstance()->start();

    installHandlers();

    HashManager::getInstance()->setPriority(Thread::IDLE);

    WulforSettings::newInstance();
    WulforSettings::getInstance()->load();
    WulforSettings::getInstance()->loadTranslation();
    WulforSettings::getInstance()->loadTheme();

    WulforUtil::newInstance();

    if (WulforUtil::getInstance()->loadUserIcons())
        std::cout << "UserList icons has been loaded" << std::endl;

    if (WulforUtil::getInstance()->loadIcons())
        std::cout << "Application icons has been loaded" << std::endl;

    UPnP::newInstance();
    UPnP::getInstance()->start();
    UPnPMapper::newInstance();

    HubManager::newInstance();

    WulforSettings::getInstance()->loadTheme();

    if (WBGET(WB_APP_ENABLE_EMOTICON)){
        EmoticonFactory::newInstance();
        EmoticonFactory::getInstance()->load();
    }

#ifdef USE_ASPELL
    if (WBGET(WB_APP_ENABLE_ASPELL))
        SpellCheck::newInstance();
#endif

    Notification::newInstance();
    Notification::getInstance()->enableTray(WBGET(WB_TRAY_ENABLED));

    core_ptr = proxy;

    if (WBGET(WB_MAINWINDOW_HIDE))
        getInstance()->hide();
}

void MainLayoutWrapper::SecondInit ()
{
	ConstructAsConstructor ();

        autoconnect();
        parseCmdLine();
}

void MainLayoutWrapper::Release ()
{
	ReleaseAsClosed ();
	ReleaseAsDestructor ();
	ReleaseAsAfterExec ();
}

QString MainLayoutWrapper::GetName () const
{
	return "EiskaltDC++";
}

QString MainLayoutWrapper::GetInfo () const
{
        return tr ("EiskaltDC++ is a cool ADC/NMDC client.");
}

QStringList MainLayoutWrapper::Provides () const
{
	return QStringList ("directconnect");
}

QStringList MainLayoutWrapper::Needs () const
{
	return QStringList ();
}

QStringList MainLayoutWrapper::Uses () const
{
	return QStringList ();
}

void MainLayoutWrapper::SetProvider (QObject*, const QString&)
{
}

QIcon MainLayoutWrapper::GetIcon () const
{
	return QIcon ();
}

QList<QAction*> MainLayoutWrapper::GetActions() const {
    return toolBarActions;
}

void MainLayoutWrapper::ReleaseAsClosed ()
{
    saveSettings();

    blockSignals(true);

    if (TransferView::getInstance()){
        TransferView::getInstance()->close();
        TransferView::deleteInstance();
    }

    if (FavoriteHubs::getInstance()){
        FavoriteHubs::getInstance()->setUnload(true);
        FavoriteHubs::getInstance()->close();

        FavoriteHubs::deleteInstance();
    }

    if (PublicHubs::getInstance()){
        PublicHubs::getInstance()->setUnload(true);
        PublicHubs::getInstance()->close();

        PublicHubs::deleteInstance();
    }

    if (FinishedDownloads::getInstance()){
        FinishedDownloads::getInstance()->setUnload(true);
        FinishedDownloads::getInstance()->close();

        FinishedDownloads::deleteInstance();
    }

    if (FinishedUploads::getInstance()){
        FinishedUploads::getInstance()->setUnload(true);
        FinishedUploads::getInstance()->close();

        FinishedUploads::deleteInstance();
    }

    if (FavoriteUsers::getInstance()){
        FavoriteUsers::getInstance()->setUnload(true);
        FavoriteUsers::getInstance()->close();

        FavoriteUsers::deleteInstance();
    }

    if (DownloadQueue::getInstance()){
        DownloadQueue::getInstance()->setUnload(true);
        DownloadQueue::getInstance()->close();

        DownloadQueue::deleteInstance();
    }

    if (SpyFrame::getInstance()){
        SpyFrame::getInstance()->setUnload(true);
        SpyFrame::getInstance()->close();

        SpyFrame::deleteInstance();
    }

    QMap< ArenaWidget*, QWidget* > map = arenaMap;
    QMap< ArenaWidget*, QWidget* >::iterator it = map.begin();

    for(; it != map.end(); ++it){
        if (arenaMap.contains(it.key()))//some widgets can autodelete itself from arena widgets
            it.value()->close();
    }
}

void MainLayoutWrapper::ReleaseAsAfterExec ()
{
	qDebug () << Q_FUNC_INFO << "Shutting down...";

    WulforSettings::getInstance()->save();

    EmoticonFactory::deleteInstance();

#ifdef USE_ASPELL
    if (SpellCheck::getInstance())
        SpellCheck::deleteInstance();
#endif

    UPnPMapper::deleteInstance();
    UPnP::getInstance()->stop();
    UPnP::deleteInstance();

    Notification::deleteInstance();

    HubManager::deleteInstance();

    WulforUtil::deleteInstance();
    WulforSettings::deleteInstance();

    dcpp::shutdown();
}

void MainLayoutWrapper::ReleaseAsDestructor ()
{
    LogManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
    QueueManager::getInstance()->removeListener(this);

    if (AntiSpam::getInstance()){
        AntiSpam::getInstance()->saveLists();
        AntiSpam::getInstance()->saveSettings();
        AntiSpam::deleteInstance();
    }

    if (IPFilter::getInstance()){
        IPFilter::getInstance()->saveList();
        IPFilter::deleteInstance();
    }

    //delete arena;

    delete fBar;
    delete tBar;
}

MainLayoutWrapper* MainLayoutWrapper::getInstance ()
{
	return S_StaticThis;
}

void MainLayoutWrapper::ConstructAsConstructor ()
{
    arenaMap.clear();
    arenaWidgets.clear();

    init();

    retranslateUi();

    LogManager::getInstance()->addListener(this);
    TimerManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);

    startSocket();

    //setStatusMessage(tr("Ready"));

    TransferView::newInstance();

    transfer_dock->setWidget(TransferView::getInstance());

    blockSignals(true);
    toolsTransfers->setChecked(transfer_dock->isVisible());
    blockSignals(false);

    if (WBGET(WB_ANTISPAM_ENABLED)){
        AntiSpam::newInstance();

        AntiSpam::getInstance()->loadLists();
        AntiSpam::getInstance()->loadSettings();
    }

    if (WBGET(WB_IPFILTER_ENABLED)){
        IPFilter::newInstance();

        IPFilter::getInstance()->loadList();
    }

    QFont f;
}

void MainLayoutWrapper::closeEvent(QCloseEvent *c_e){
}

void MainLayoutWrapper::showEvent(QShowEvent *e){
    if (e->spontaneous())
        redrawToolPanel();

    //QWidget *wg = arena->widget();

    /*bool pmw = false;

    if (wg != 0)
        pmw = (typeid(*wg) == typeid(PMWindow));

    HubFrame *fr = HubManager::getInstance()->activeHub();

    bool enable = (fr && (fr == arena->widget()));

    chatClear->setEnabled(enable || pmw);
    findInChat->setEnabled(enable);
    chatDisable->setEnabled(enable); */

    e->accept();
}

void MainLayoutWrapper::customEvent(QEvent *e){
    if (e->type() == MainLayoutWrapperCustomEvent::Event){
        MainLayoutWrapperCustomEvent *c_e = reinterpret_cast<MainLayoutWrapperCustomEvent*>(e);

        c_e->func()->call();
    }

    e->accept();
}

bool MainLayoutWrapper::eventFilter(QObject *obj, QEvent *e){

}

void MainLayoutWrapper::init(){
    /*arena = new QDockWidget();
    arena->setFloating(false);
    arena->setAllowedAreas(Qt::RightDockWidgetArea);
    arena->setFeatures(QDockWidget::NoDockWidgetFeatures);
    arena->setContextMenuPolicy(Qt::CustomContextMenu);
    arena->setTitleBarWidget(new QWidget(arena));*/

    transfer_dock = new QDockWidget(this);
    transfer_dock->setObjectName("transfer_dock");
    transfer_dock->setFloating(false);
    transfer_dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    transfer_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    transfer_dock->setContextMenuPolicy(Qt::CustomContextMenu);
    transfer_dock->setTitleBarWidget(new QWidget(transfer_dock));

    QMainWindow *mw = core_ptr->GetMainWindow();

    //setCentralWidget(arena);
    //addDockWidget(Qt::RightDockWidgetArea, arena);
    mw->addDockWidget(Qt::BottomDockWidgetArea, transfer_dock);

    transfer_dock->hide();

    history.setSize(30);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    setWindowIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiICON_APPL));

    //setWindowTitle(QString("%1").arg(EISKALTDCPP_WND_TITLE));

    initActions();

    initMenuBar();

    initStatusBar();

    initToolbar();

    initHotkeys();

    loadSettings();
}

void MainLayoutWrapper::loadSettings(){
    /*WulforSettings *WS = WulforSettings::getInstance();

    bool showMax = WS->getBool(WB_MAINWINDOW_MAXIMIZED);
    int w = WS->getInt(WI_MAINWINDOW_WIDTH);
    int h = WS->getInt(WI_MAINWINDOW_HEIGHT);
    int xPos = WS->getInt(WI_MAINWINDOW_X);
    int yPos = WS->getInt(WI_MAINWINDOW_Y);

    QPoint p(xPos, yPos);
    QSize  sz(w, h);

    if (p.x() >= 0 || p.y() >= 0)
        this->move(p);

    if (sz.width() > 0 || sz.height() > 0)
        this->resize(sz);

    if (showMax)
        this->showMaximized();

    QString wstate = WSGET(WS_MAINWINDOW_STATE);

    if (!wstate.isEmpty())
        this->restoreState(QByteArray::fromBase64(wstate.toAscii()));*/
}

void MainLayoutWrapper::saveSettings(){
    /*static bool stateIsSaved = false;

    if (stateIsSaved)
        return;

    WISET(WI_MAINWINDOW_HEIGHT, height());
    WISET(WI_MAINWINDOW_WIDTH, width());
    WISET(WI_MAINWINDOW_X, x());
    WISET(WI_MAINWINDOW_Y, y());

    WBSET(WB_MAINWINDOW_MAXIMIZED, isMaximized());
    WBSET(WB_MAINWINDOW_HIDE, !isVisible());

    WSSET(WS_MAINWINDOW_STATE, saveState().toBase64());

    stateIsSaved = true;*/
}

void MainLayoutWrapper::initActions(){

    WulforUtil *WU = WulforUtil::getInstance();

    {
        fileFileListBrowserLocal = new QAction("", this);
        fileFileListBrowserLocal->setShortcut(tr("Ctrl+L"));
        fileFileListBrowserLocal->setIcon(WU->getPixmap(WulforUtil::eiOWN_FILELIST));
        connect(fileFileListBrowserLocal, SIGNAL(triggered()), this, SLOT(slotFileBrowseOwnFilelist()));

        fileFileListBrowser = new QAction("", this);
        fileFileListBrowser->setShortcut(tr("Shift+L"));
        fileFileListBrowser->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(fileFileListBrowser, SIGNAL(triggered()), this, SLOT(slotFileBrowseFilelist()));

        fileOpenLogFile = new QAction("", this);
        fileOpenLogFile->setIcon(WU->getPixmap(WulforUtil::eiOPEN_LOG_FILE));
        connect(fileOpenLogFile, SIGNAL(triggered()), this, SLOT(slotFileOpenLogFile()));

        fileFileListRefresh = new QAction("", this);
        fileFileListRefresh->setShortcut(tr("Ctrl+R"));
        fileFileListRefresh->setIcon(WU->getPixmap(WulforUtil::eiREFRLIST));
        connect(fileFileListRefresh, SIGNAL(triggered()), this, SLOT(slotFileRefreshShare()));

        fileHashProgress = new QAction("", this);
        fileHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        connect(fileHashProgress, SIGNAL(triggered()), this, SLOT(slotFileHashProgress()));

        fileHideWindow = new QAction(tr("Hide window"), this);
        fileHideWindow->setShortcut(tr("Esc"));
        fileHideWindow->setIcon(WU->getPixmap(WulforUtil::eiHIDEWINDOW));
        connect(fileHideWindow, SIGNAL(triggered()), this, SLOT(slotHideWindow()));

        if (!WBGET(WB_TRAY_ENABLED))
            fileHideWindow->setText(tr("Show/hide find frame"));

        hubsHubReconnect = new QAction("", this);
        hubsHubReconnect->setIcon(WU->getPixmap(WulforUtil::eiRECONNECT));
        connect(hubsHubReconnect, SIGNAL(triggered()), this, SLOT(slotHubsReconnect()));

        hubsQuickConnect = new QAction("", this);
        hubsQuickConnect->setShortcut(tr("Ctrl+H"));
        hubsQuickConnect->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));
        connect(hubsQuickConnect, SIGNAL(triggered()), this, SLOT(slotQC()));

        hubsFavoriteHubs = new QAction("", this);
        hubsFavoriteHubs->setIcon(WU->getPixmap(WulforUtil::eiFAVSERVER));
        connect(hubsFavoriteHubs, SIGNAL(triggered()), this, SLOT(slotHubsFavoriteHubs()));

        hubsPublicHubs = new QAction("", this);
        hubsPublicHubs->setIcon(WU->getPixmap(WulforUtil::eiSERVER));
        connect(hubsPublicHubs, SIGNAL(triggered()), this, SLOT(slotHubsPublicHubs()));

        hubsFavoriteUsers = new QAction("", this);
        hubsFavoriteUsers->setIcon(WU->getPixmap(WulforUtil::eiFAVUSERS));
        connect(hubsFavoriteUsers, SIGNAL(triggered()), this, SLOT(slotHubsFavoriteUsers()));

        toolsOptions = new QAction("", this);
        toolsOptions->setShortcut(tr("Ctrl+O"));
        toolsOptions->setIcon(WU->getPixmap(WulforUtil::eiCONFIGURE));
        connect(toolsOptions, SIGNAL(triggered()), this, SLOT(slotToolsSettings()));

        toolsTransfers = new QAction("", this);
        toolsTransfers->setShortcut(tr("Ctrl+T"));
        toolsTransfers->setIcon(WU->getPixmap(WulforUtil::eiTRANSFER));
        toolsTransfers->setCheckable(true);
        connect(toolsTransfers, SIGNAL(toggled(bool)), this, SLOT(slotToolsTransfer(bool)));
        //transfer_dock->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

        toolsDownloadQueue = new QAction("", this);
        toolsDownloadQueue->setShortcut(tr("Ctrl+D"));
        toolsDownloadQueue->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(toolsDownloadQueue, SIGNAL(triggered()), this, SLOT(slotToolsDownloadQueue()));

        toolsFinishedDownloads = new QAction("", this);
        toolsFinishedDownloads->setIcon(WU->getPixmap(WulforUtil::eiDOWNLIST));
        connect(toolsFinishedDownloads, SIGNAL(triggered()), this, SLOT(slotToolsFinishedDownloads()));

        toolsFinishedUploads = new QAction("", this);
        toolsFinishedUploads->setIcon(WU->getPixmap(WulforUtil::eiUPLIST));
        connect(toolsFinishedUploads, SIGNAL(triggered()), this, SLOT(slotToolsFinishedUploads()));

        toolsSpy = new QAction("", this);
        toolsSpy->setIcon(WU->getPixmap(WulforUtil::eiSPY));
        connect(toolsSpy, SIGNAL(triggered()), this, SLOT(slotToolsSpy()));

        toolsAntiSpam = new QAction("", this);
        toolsAntiSpam->setIcon(WU->getPixmap(WulforUtil::eiSPAM));
        connect(toolsAntiSpam, SIGNAL(triggered()), this, SLOT(slotToolsAntiSpam()));

        toolsIPFilter = new QAction("", this);
        toolsIPFilter->setIcon(WU->getPixmap(WulforUtil::eiFILTER));
        connect(toolsIPFilter, SIGNAL(triggered()), this, SLOT(slotToolsIPFilter()));

        toolsSearch = new QAction("", this);
        toolsSearch->setShortcut(tr("Ctrl+S"));
        toolsSearch->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
        connect(toolsSearch, SIGNAL(triggered()), this, SLOT(slotToolsSearch()));

        toolsHideProgressSpace = new QAction(tr("Hide free space bar"), this);
        if (!WBGET(WB_SHOW_FREE_SPACE))
            toolsHideProgressSpace->setText(tr("Show free space bar"));
#if (!defined FREE_SPACE_BAR)
        toolsHideProgressSpace->setVisible(false);
#endif
        toolsHideProgressSpace->setIcon(WU->getPixmap(WulforUtil::eiFREESPACE));
        connect(toolsHideProgressSpace, SIGNAL(triggered()), this, SLOT(slotHideProgressSpace()));

        chatClear = new QAction("", this);
        chatClear->setIcon(WU->getPixmap(WulforUtil::eiCLEAR));
        connect(chatClear, SIGNAL(triggered()), this, SLOT(slotChatClear()));

        findInChat = new QAction("", this);
        findInChat->setShortcut(tr("Ctrl+F"));
        findInChat->setIcon(WU->getPixmap(WulforUtil::eiFIND));
        connect(findInChat, SIGNAL(triggered()), this, SLOT(slotFindInChat()));

        chatDisable = new QAction("", this);
        chatDisable->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));
        connect(chatDisable, SIGNAL(triggered()), this, SLOT(slotChatDisable()));

        QAction *separator0 = new QAction("", this);
        separator0->setSeparator(true);
        QAction *separator1 = new QAction("", this);
        separator1->setSeparator(true);
        QAction *separator2 = new QAction("", this);
        separator2->setSeparator(true);
        QAction *separator3 = new QAction("", this);
        separator3->setSeparator(true);
        QAction *separator4 = new QAction("", this);
        separator4->setSeparator(true);
        QAction *separator5 = new QAction("", this);
        separator5->setSeparator(true);
        QAction *separator6 = new QAction("", this);
        separator6->setSeparator(true);

        fileMenuActions << fileFileListBrowser
                << fileFileListBrowserLocal
                << fileFileListRefresh
                << fileHashProgress
                << separator0
                << fileOpenLogFile
                << separator1
                << fileHideWindow;

        hubsMenuActions << hubsHubReconnect
                << hubsQuickConnect
                << hubsFavoriteHubs
                << hubsPublicHubs
                << separator0
                << hubsFavoriteUsers;

        toolsMenuActions << toolsSearch
                << separator0
                << toolsTransfers
                << toolsDownloadQueue
                << toolsFinishedDownloads
                << toolsFinishedUploads
                << separator1
                << toolsSpy
                << toolsAntiSpam
                << toolsIPFilter
                << separator2
                << toolsHideProgressSpace
                << separator3
                << toolsOptions;

        toolBarActions << toolsOptions
                << separator0
                << fileFileListBrowserLocal
                << fileFileListRefresh
                << fileHashProgress
                << separator1
                << hubsHubReconnect
                << hubsQuickConnect
                << separator2
                << hubsFavoriteHubs
                << hubsFavoriteUsers
                << toolsSearch
                << hubsPublicHubs
                << separator3
                << toolsTransfers
                << toolsDownloadQueue
                << toolsFinishedDownloads
                << toolsFinishedUploads
                << separator4
                << chatClear
                << findInChat
                << chatDisable
                << separator5
                << toolsSpy
                << toolsAntiSpam
                << toolsIPFilter;
    }
    {
        menuWidgets = new QMenu("", this);
    }
    {
        aboutClient = new QAction("", this);
        aboutClient->setIcon(WU->getPixmap(WulforUtil::eiICON_APPL));
        connect(aboutClient, SIGNAL(triggered()), this, SLOT(slotAboutClient()));

        aboutQt = new QAction("", this);
        aboutQt->setIcon(WU->getPixmap(WulforUtil::eiQT_LOGO));
        connect(aboutQt, SIGNAL(triggered()), this, SLOT(slotAboutQt()));
    }
}

void MainLayoutWrapper::initHotkeys(){
    ctrl_pgdown = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageDown), this);
    ctrl_pgup   = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp), this);
    ctrl_w      = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);

    ctrl_pgdown->setContext(Qt::WindowShortcut);
    ctrl_pgup->setContext(Qt::WindowShortcut);
    ctrl_w->setContext(Qt::WindowShortcut);

    connect(ctrl_pgdown, SIGNAL(activated()), tBar, SLOT(nextTab()));
    connect(ctrl_pgup,   SIGNAL(activated()), tBar, SLOT(prevTab()));
    connect(ctrl_w,      SIGNAL(activated()), this, SLOT(slotCloseCurrentWidget()));
}

void MainLayoutWrapper::initMenuBar(){
    {
        menuFile = new QMenu("", this);

        menuFile->addActions(fileMenuActions);
    }
    {
        menuHubs = new QMenu("", this);

        menuHubs->addActions(hubsMenuActions);
    }
    {
        menuTools = new QMenu("", this);

        menuTools->addActions(toolsMenuActions);
    }
    {
        menuAbout = new QMenu("", this);

        menuAbout->addAction(aboutClient);
        menuAbout->addAction(aboutQt);
    }

    /*menuBar()->addMenu(menuFile);
    menuBar()->addMenu(menuHubs);
    menuBar()->addMenu(menuTools);
    menuBar()->addMenu(menuWidgets);
    menuBar()->addMenu(menuAbout);
    menuBar()->setContextMenuPolicy(Qt::CustomContextMenu);*/
}

void MainLayoutWrapper::initStatusBar(){
/*    statusLabel = new QLabel(statusBar());
    statusLabel->setFrameShadow(QFrame::Plain);
    statusLabel->setFrameShape(QFrame::NoFrame);
    statusLabel->setAlignment(Qt::AlignRight);
    statusLabel->setToolTip(tr("Counts"));

    statusDSPLabel = new QLabel(statusBar());
    statusDSPLabel->setFrameShadow(QFrame::Plain);
    statusDSPLabel->setFrameShape(QFrame::NoFrame);
    statusDSPLabel->setAlignment(Qt::AlignRight);
    statusDSPLabel->setToolTip(tr("Download speed (per sec.)"));

    statusUSPLabel = new QLabel(statusBar());
    statusUSPLabel->setFrameShadow(QFrame::Plain);
    statusUSPLabel->setFrameShape(QFrame::NoFrame);
    statusUSPLabel->setAlignment(Qt::AlignRight);
    statusUSPLabel->setToolTip(tr("Upload speed (per sec.)"));

    statusDLabel = new QLabel(statusBar());
    statusDLabel->setFrameShadow(QFrame::Plain);
    statusDLabel->setFrameShape(QFrame::NoFrame);
    statusDLabel->setAlignment(Qt::AlignRight);
    statusDLabel->setToolTip(tr("Downloaded"));

    statusULabel = new QLabel(statusBar());
    statusULabel->setFrameShadow(QFrame::Plain);
    statusULabel->setFrameShape(QFrame::NoFrame);
    statusULabel->setAlignment(Qt::AlignRight);
    statusULabel->setToolTip(tr("Uploaded"));

    msgLabel = new QLabel(statusBar());
    msgLabel->setFrameShadow(QFrame::Plain);
    msgLabel->setFrameShape(QFrame::NoFrame);
    msgLabel->setAlignment(Qt::AlignLeft);
#if (defined FREE_SPACE_BAR)
    progressSpace = new QProgressBar(this);
    progressSpace->setMaximum(100);
    progressSpace->setMinimum(0);
    progressSpace->setMinimumWidth(100);
    progressSpace->setMaximumWidth(250);
    progressSpace->setFixedHeight(18);
    progressSpace->setToolTip(tr("Space free"));

    if (!WBGET(WB_SHOW_FREE_SPACE))
        progressSpace->hide();
#else //FREE_SPACE_BAR
    WBSET(WB_SHOW_FREE_SPACE, false);
#endif //FREE_SPACE_BAR

    statusBar()->addWidget(msgLabel);
    statusBar()->addPermanentWidget(statusDLabel);
    statusBar()->addPermanentWidget(statusULabel);
    statusBar()->addPermanentWidget(statusDSPLabel);
    statusBar()->addPermanentWidget(statusUSPLabel);
    statusBar()->addPermanentWidget(statusLabel);
#if (defined FREE_SPACE_BAR)
    statusBar()->addPermanentWidget(progressSpace);
#endif //FREE_SPACE_BAR*/
}

void MainLayoutWrapper::retranslateUi(){
    //Retranslate menu actions
    {
        menuFile->setTitle(tr("&File"));

        fileOpenLogFile->setText(tr("Open log file"));

        fileFileListBrowser->setText(tr("Open filelist..."));

        fileFileListBrowserLocal->setText(tr("Open own filelist"));

        fileFileListRefresh->setText(tr("Refresh share"));

        fileHashProgress->setText(tr("Hash progress"));

        menuHubs->setTitle(tr("&Hubs"));

        hubsHubReconnect->setText(tr("Reconnect to hub"));

        hubsFavoriteHubs->setText(tr("Favourite hubs"));

        hubsPublicHubs->setText(tr("Public hubs"));

        hubsFavoriteUsers->setText(tr("Favourite users"));

        hubsQuickConnect->setText(tr("Quick connect"));

        menuTools->setTitle(tr("&Tools"));

        toolsTransfers->setText(tr("Transfers"));

        toolsDownloadQueue->setText(tr("Download queue"));

        toolsFinishedDownloads->setText(tr("Finished downloads"));

        toolsFinishedUploads->setText(tr("Finished uploads"));

        toolsSpy->setText(tr("Search Spy"));

        toolsAntiSpam->setText(tr("AntiSpam module"));

        toolsIPFilter->setText(tr("IPFilter module"));

        toolsOptions->setText(tr("Options"));

        toolsSearch->setText(tr("Search"));

        chatClear->setText(tr("Clear chat"));

        findInChat->setText(tr("Find in chat"));

        chatDisable->setText(tr("Disable/enable chat"));

        menuWidgets->setTitle(tr("&Widgets"));

        menuAbout->setTitle(tr("&Help"));

        aboutClient->setText(tr("About EiskaltDC++"));

        aboutQt->setText(tr("About Qt"));
    }
    {
        //arena->setWindowTitle(tr("Main layout"));
    }
}

void MainLayoutWrapper::initToolbar(){
    fBar = new ToolBar(this);
    fBar->setObjectName("fBar");
    fBar->addActions(toolBarActions);
    fBar->setContextMenuPolicy(Qt::CustomContextMenu);
    fBar->setMovable(true);
    fBar->setFloatable(true);
    fBar->setAllowedAreas(Qt::AllToolBarAreas);

    tBar = new ToolBar(this);
    tBar->setObjectName("tBar");
    tBar->initTabs();
    tBar->setContextMenuPolicy(Qt::CustomContextMenu);
    tBar->setMovable(true);
    tBar->setFloatable(true);
    tBar->setAllowedAreas(Qt::AllToolBarAreas);

    /*addToolBar(fBar);
    addToolBar(tBar);*/
}

void MainLayoutWrapper::newHubFrame(QString address, QString enc){
    if (address.isEmpty())
        return;

    HubFrame *fr = NULL;

    if (fr = HubManager::getInstance()->getHub(address)){
        mapWidgetOnArena(fr);

        return;
    }

    fr = new HubFrame(NULL, address, enc);
    fr->setAttribute(Qt::WA_DeleteOnClose);

    addArenaWidget(fr);
    mapWidgetOnArena(fr);

    addArenaWidgetOnToolbar(fr);
}

void MainLayoutWrapper::updateStatus(QMap<QString, QString> map){
    /*if (!statusLabel)
        return;

    statusLabel->setText(map["STATS"]);
    statusUSPLabel->setText(map["USPEED"]);
    statusDSPLabel->setText(map["DSPEED"]);
    statusDLabel->setText(map["DOWN"]);
    statusULabel->setText(map["UP"]);

    QFontMetrics metrics(font());

    statusUSPLabel->setFixedWidth(metrics.width(statusUSPLabel->text()) > statusUSPLabel->width()? metrics.width(statusUSPLabel->text()) + 10 : statusUSPLabel->width());
    statusDSPLabel->setFixedWidth(metrics.width(statusDSPLabel->text()) > statusDSPLabel->width()? metrics.width(statusDSPLabel->text()) + 10 : statusDSPLabel->width());
    statusDLabel->setFixedWidth(metrics.width(statusDLabel->text()) > statusDLabel->width()? metrics.width(statusDLabel->text()) + 10 : statusDLabel->width());
    statusULabel->setFixedWidth(metrics.width(statusULabel->text()) > statusULabel->width()? metrics.width(statusULabel->text()) + 10 : statusULabel->width());

    if (WBGET(WB_SHOW_FREE_SPACE)) {
#ifdef FREE_SPACE_BAR
        boost::filesystem::space_info info;
        if (boost::filesystem::exists(SETTING(DOWNLOAD_DIRECTORY)))
            info = boost::filesystem::space(boost::filesystem::path(SETTING(DOWNLOAD_DIRECTORY)));
        else if (boost::filesystem::exists(Util::getPath(Util::PATH_USER_CONFIG)))
            info = boost::filesystem::space(boost::filesystem::path(Util::getPath(Util::PATH_USER_CONFIG)));

        if (info.capacity) {
            float total = info.capacity;
            float percent = 100.0f*(total-info.available)/total;
            QString format = tr("Free %1")
                             .arg(_q(dcpp::Util::formatBytes(info.available)));

            QString tooltip = tr("Free %1 of %2")
                              .arg(_q(dcpp::Util::formatBytes(info.available)))
                              .arg(_q(dcpp::Util::formatBytes(total)));

            progressSpace->setFormat(format);
            progressSpace->setToolTip(tooltip);
            progressSpace->setValue(static_cast<unsigned>(percent));
        }
#endif //FREE_SPACE_BAR
    }*/
}

void MainLayoutWrapper::setStatusMessage(QString msg){
    //msgLabel->setText(msg);
}

void MainLayoutWrapper::autoconnect(){
    const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();

    for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
        FavoriteHubEntry* entry = *i;

        if (entry->getConnect()) {
            if (entry->getNick().empty() && SETTING(NICK).empty())
                continue;

            QString encoding = WulforUtil::getInstance()->dcEnc2QtEnc(QString::fromStdString(entry->getEncoding()));

            newHubFrame(QString::fromStdString(entry->getServer()), encoding);
        }
    }
}

void MainLayoutWrapper::parseCmdLine(){
    QStringList args = qApp->arguments();

    foreach (QString arg, args){
        if (arg.startsWith("magnet:?xt=urn:tree:tiger:")){
            Magnet m(this);
            m.setLink(arg);

            m.exec();
        }
        else if (arg.startsWith("dchub://")){
            newHubFrame(arg, "");
        }
        else if (arg.startsWith("adc://") || arg.startsWith("adcs://")){
            newHubFrame(arg, "UTF-8");
        }
    }
}

void MainLayoutWrapper::parseInstanceLine(QString data){
    QStringList args = data.split("\n", QString::SkipEmptyParts);

    foreach (QString arg, args){
        if (arg.startsWith("magnet:?xt=urn:tree:tiger:")){
            Magnet m(this);
            m.setLink(arg);

            m.exec();
        }
        else if (arg.startsWith("dchub://")){
            newHubFrame(arg, "");
        }
        else if (arg.startsWith("adc://") || arg.startsWith("adcs://")){
            newHubFrame(arg, "UTF-8");
        }
    }
}

void MainLayoutWrapper::browseOwnFiles(){
    slotFileBrowseOwnFilelist();
}

void MainLayoutWrapper::slotFileBrowseFilelist(){
    static ShareBrowser *local_share = NULL;
    QString file = QFileDialog::getOpenFileName(this, tr("Choose file to open"), QString::fromStdString(Util::getPath(Util::PATH_FILE_LISTS)),
            tr("Modern XML Filelists") + " (*.xml.bz2);;" +
            tr("Modern XML Filelists uncompressed") + " (*.xml);;" +
            tr("All files") + " (*)");
    UserPtr user = DirectoryListing::getUserFromFilename(_tq(file));
    if (user) {
        local_share = new ShareBrowser(user, file, "");
    } else {
        setStatusMessage(tr("Unable to load file list: Invalid file list name"));
    }
}

void MainLayoutWrapper::redrawToolPanel(){
    tBar->redraw();

    QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.begin();
    QHash<QAction*, ArenaWidget*>::iterator end = menuWidgetsHash.end();

    for(; it != end; ++it){//also redraw all widget menu items
        it.key()->setText(it.value()->getArenaShortTitle());
        it.key()->setIcon(it.value()->getPixmap());
    }
}

void MainLayoutWrapper::addArenaWidget(ArenaWidget *wgt){
    if (!arenaWidgets.contains(wgt) && wgt && wgt->getWidget()){
        arenaWidgets.push_back(wgt);
        arenaMap[wgt] = wgt->getWidget();
    }
}

void MainLayoutWrapper::remArenaWidget(ArenaWidget *awgt){
    if (arenaWidgets.contains(awgt)){
        arenaWidgets.removeAt(arenaWidgets.indexOf(awgt));
        arenaMap.erase(arenaMap.find(awgt));

        chatClear->setEnabled(false);
        findInChat->setEnabled(false);
        chatDisable->setEnabled(false);
    }
}

void MainLayoutWrapper::mapWidgetOnArena(ArenaWidget *awgt){
    /**
      Void function (only for plugin)
    */
}

void MainLayoutWrapper::remWidgetFromArena(ArenaWidget *awgt){
    /**
      Void function (only for plugin)
    */
}

void MainLayoutWrapper::addArenaWidgetOnToolbar(ArenaWidget *awgt, bool keepFocus){
    if (!arenaWidgets.contains(awgt) || shown.contains(awgt))
        return;

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(true);

    shown.insert(awgt);

    emit addNewTab(awgt->getArenaShortTitle(), awgt->getWidget());
    emit changeTabIcon(awgt->getWidget(), awgt->getPixmap());
    emit raiseTab(awgt->getWidget());
}

void MainLayoutWrapper::remArenaWidgetFromToolbar(ArenaWidget *awgt){
    if (awgt->toolButton())
        awgt->toolButton()->setChecked(false);

    if (shown.contains(awgt))
        shown.remove(awgt);

    emit removeTab(awgt->getWidget());
}

void MainLayoutWrapper::toggleSingletonWidget(ArenaWidget *a){
    if (!a)
        return;

    if (sender() && typeid(*sender()) == typeid(QAction) && a->getWidget()){
        QAction *act = reinterpret_cast<QAction*>(sender());;

        act->setCheckable(true);

        a->setToolButton(act);
    }

    if (shown.contains(a))
        remArenaWidgetFromToolbar(a);
    else
        addArenaWidgetOnToolbar(a);
}

void MainLayoutWrapper::startSocket(){
    SearchManager::getInstance()->disconnect();
    ConnectionManager::getInstance()->disconnect();

    if (ClientManager::getInstance()->isActive()) {
        QString msg = "";
        try {
            ConnectionManager::getInstance()->listen();
        } catch(const Exception &e) {
            msg = tr("Cannot listen socket because: \n") + QString::fromStdString(e.getError()) + tr("\n\nPlease check your connection settings");

            QMessageBox::warning(this, tr("Connection Manager: Warning"), msg, QMessageBox::Ok);
        }
        try {
            SearchManager::getInstance()->listen();
        } catch(const Exception &e) {
            msg = tr("Cannot listen socket because: \n") + QString::fromStdString(e.getError()) + tr("\n\nPlease check your connection settings");

            QMessageBox::warning(this, tr("Search Manager: Warning"), msg, QMessageBox::Ok);
        }
    }

    UPnPMapper::getInstance()->forward();
}

void MainLayoutWrapper::showShareBrowser(dcpp::UserPtr usr, QString file, QString jump_to){
    ShareBrowser *sb = new ShareBrowser(usr, file, jump_to);
}

void MainLayoutWrapper::slotFileOpenLogFile(){
    QString f = QFileDialog::getOpenFileName(this, tr("Open log file"),_q(SETTING(LOG_DIRECTORY)), tr("Log files (*.log);;All files (*.*)"));

    if (!f.isEmpty()){
        if (f.startsWith("/"))
            f = "file://" + f;
        else
            f = "file:///" + f;

        QDesktopServices::openUrl(f);
    }
}

void MainLayoutWrapper::slotFileBrowseOwnFilelist(){
    static ShareBrowser *local_share = NULL;

    if (arenaWidgets.contains(local_share)){
        mapWidgetOnArena(local_share);

        return;
    }

    UserPtr user = ClientManager::getInstance()->getMe();
    QString file = QString::fromStdString(ShareManager::getInstance()->getOwnListFile());

    local_share = new ShareBrowser(user, file, "");
}

void MainLayoutWrapper::slotFileRefreshShare(){
    ShareManager *SM = ShareManager::getInstance();

    SM->setDirty();
    SM->refresh(true);

    HashProgress progress(this);
    progress.slotAutoClose(true);

    progress.exec();
}

void MainLayoutWrapper::slotFileHashProgress(){
    HashProgress progress(this);

    progress.exec();
}

void MainLayoutWrapper::slotHubsReconnect(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->reconnect();
}

void MainLayoutWrapper::slotToolsSearch(){
    SearchFrame *sf = new SearchFrame();

    sf->setAttribute(Qt::WA_DeleteOnClose);
}

void MainLayoutWrapper::slotToolsDownloadQueue(){
    if (!DownloadQueue::getInstance())
        DownloadQueue::newInstance();

    toggleSingletonWidget(DownloadQueue::getInstance());
}

void MainLayoutWrapper::slotToolsFinishedDownloads(){
    if (!FinishedDownloads::getInstance())
        FinishedDownloads::newInstance();

    toggleSingletonWidget(FinishedDownloads::getInstance());
}

void MainLayoutWrapper::slotToolsFinishedUploads(){
    if (!FinishedUploads::getInstance())
        FinishedUploads::newInstance();

    toggleSingletonWidget(FinishedUploads::getInstance());
}

void MainLayoutWrapper::slotToolsSpy(){
    if (!SpyFrame::getInstance())
        SpyFrame::newInstance();

    toggleSingletonWidget(SpyFrame::getInstance());
}

void MainLayoutWrapper::slotToolsAntiSpam(){
    AntiSpamFrame fr(this);

    fr.exec();
}

void MainLayoutWrapper::slotToolsIPFilter(){
    IPFilterFrame fr(this);

    fr.exec();
}

void MainLayoutWrapper::slotHubsFavoriteHubs(){
    if (!FavoriteHubs::getInstance())
        FavoriteHubs::newInstance();

    toggleSingletonWidget(FavoriteHubs::getInstance());
}

void MainLayoutWrapper::slotHubsPublicHubs(){
    if (!PublicHubs::getInstance())
        PublicHubs::newInstance();

    toggleSingletonWidget(PublicHubs::getInstance());
}

void MainLayoutWrapper::slotHubsFavoriteUsers(){
    if (!FavoriteUsers::getInstance())
        FavoriteUsers::newInstance();

    toggleSingletonWidget(FavoriteUsers::getInstance());
}

void MainLayoutWrapper::slotToolsSettings(){
    Settings s;

    s.exec();

    //reload some settings
    if (!WBGET(WB_TRAY_ENABLED))
        fileHideWindow->setText(tr("Show/hide find frame"));
    else
        fileHideWindow->setText(tr("Hide window"));
}

void MainLayoutWrapper::slotToolsTransfer(bool toggled){
    if (toggled){
        transfer_dock->setVisible(true);
        transfer_dock->setWidget(TransferView::getInstance());
    }
    else {
        transfer_dock->setWidget(NULL);
        transfer_dock->setVisible(false);
    }
}

void MainLayoutWrapper::slotChatClear(){
    /*HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->clearChat();
    else{
        QWidget *wg = arena->widget();

        bool pmw = false;

        if (wg != 0)
            pmw = (typeid(*wg) == typeid(PMWindow));

        if(pmw){
            PMWindow *pm = qobject_cast<PMWindow *>(wg);

            if (pm){
                pm->textEdit_CHAT->setHtml("");

                pm->addStatus(tr("Chat cleared."));
            }
        }
    }*/
}

void MainLayoutWrapper::slotFindInChat(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->slotHideFindFrame();
}

void MainLayoutWrapper::slotChatDisable(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->disableChat();
}

void MainLayoutWrapper::slotWidgetsToggle(){
    QAction *act = reinterpret_cast<QAction*>(sender());
    QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.find(act);

    if (it == menuWidgetsHash.end())
        return;

    mapWidgetOnArena(it.value());
}

void MainLayoutWrapper::slotQC(){
    QuickConnect qc;

    qc.exec();
}

void MainLayoutWrapper::slotHideWindow(){
    /*HubFrame *fr = HubManager::getInstance()->activeHub();
    if (fr){
        if (fr->lineEdit_FIND->hasFocus() && WBGET(WB_TRAY_ENABLED)){
            fr->slotHideFindFrame();
            return;
        }
        else if (!WBGET(WB_TRAY_ENABLED)){
            fr->slotHideFindFrame();
            return;
        }
    }*/
}

void MainLayoutWrapper::slotHideProgressSpace() {
    if (WBGET(WB_SHOW_FREE_SPACE)) {
        progressSpace->hide();
        toolsHideProgressSpace->setText(tr("Show free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, false);
    } else {
        progressSpace->show();
        toolsHideProgressSpace->setText(tr("Hide free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, true);
    }
}

void MainLayoutWrapper::slotAboutClient(){
    About a(this);

    qulonglong app_total_down = WSGET(WS_APP_TOTAL_DOWN).toULongLong();
    qulonglong app_total_up   = WSGET(WS_APP_TOTAL_UP).toULongLong();

#ifndef DCPP_REVISION
    a.label->setText(QString("<b>%1</b> %2 (%3)")
                     .arg(EISKALTDCPP_WND_TITLE)
                     .arg(EISKALTDCPP_VERSION)
                     .arg(EISKALTDCPP_VERSION_SFX));
#else
    a.label->setText(QString("<b>%1</b> %2 - %3 %4")
                     .arg(EISKALTDCPP_WND_TITLE)
                     .arg(EISKALTDCPP_VERSION)
                     .arg(EISKALTDCPP_VERSION_SFX)
                     .arg(DCPP_REVISION));
#endif
    a.label_ABOUT->setTextFormat(Qt::RichText);
    a.label_ABOUT->setText(QString("%1<br/><br/> %2 %3 %4<br/><br/> %5 %6<br/><br/> %7 <b>%8</b> <br/> %9 <b>%10</b>")
                           .arg(tr("EiskaltDC++ is a graphical client for Direct Connect and ADC protocols."))
                           .arg(tr("DC++ core version:"))
                           .arg(DCVERSIONSTRING)
                           .arg(tr("(modified)"))
                           .arg(tr("Home page:"))
                           .arg("<a href=\"http://code.google.com/p/eiskaltdc/\">"
                                "http://code.google.com/p/eiskaltdc/</a>")
                           .arg(tr("Total up:"))
                           .arg(_q(Util::formatBytes(app_total_up)))
                           .arg(tr("Total down:"))
                           .arg(_q(Util::formatBytes(app_total_down))));

    a.exec();
}

void MainLayoutWrapper::slotUnixSignal(int sig){
    printf("%i\n");
}

void MainLayoutWrapper::slotCloseCurrentWidget(){
    /*if (arena->widget())
        arena->widget()->close();*/
}

void MainLayoutWrapper::slotAboutQt(){
    QMessageBox::aboutQt(this);
}

void MainLayoutWrapper::on(dcpp::LogManagerListener::Message, time_t t, const std::string& m) throw(){
    QTextCodec *codec = QTextCodec::codecForLocale();

    typedef Func1<MainLayoutWrapper, QString> FUNC;
    FUNC *func = new FUNC(this, &MainLayoutWrapper::setStatusMessage, codec->toUnicode(m.c_str()));

    QApplication::postEvent(this, new MainLayoutWrapperCustomEvent(func));
}

void MainLayoutWrapper::on(dcpp::QueueManagerListener::Finished, QueueItem *item, const std::string &dir, int64_t) throw(){
    if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST)){
        UserPtr user = item->getDownloads()[0]->getUser();
        QString listName = QString::fromStdString(item->getListName());

        typedef Func3<MainLayoutWrapper, UserPtr, QString, QString> FUNC;
        FUNC *func = new FUNC(this, &MainLayoutWrapper::showShareBrowser, user, listName, QString::fromStdString(dir));

        QApplication::postEvent(this, new MainLayoutWrapperCustomEvent(func));
    }
}

void MainLayoutWrapper::on(dcpp::TimerManagerListener::Second, uint32_t ticks) throw(){
    static quint32 lastUpdate = 0;
    static quint64 lastUp = 0, lastDown = 0;

    quint64 now = GET_TICK();

    quint64 diff = now - lastUpdate;
    quint64 downBytes = 0;
    quint64 upBytes = 0;

    if (diff < 100U)
        diff = 1U;

    quint64 downDiff = Socket::getTotalDown() - lastDown;
    quint64 upDiff = Socket::getTotalUp() - lastUp;

    downBytes = (downDiff * 1000) / diff;
    upBytes = (upDiff * 1000) / diff;

    QMap<QString, QString> map;

    map["STATS"]    = _q(Client::getCounts());
    map["DSPEED"]   = _q(Util::formatBytes(downBytes));
    map["DOWN"]     = _q(Util::formatBytes(Socket::getTotalDown()));
    map["USPEED"]   = _q(Util::formatBytes(upBytes));
    map["UP"]       = _q(Util::formatBytes(Socket::getTotalUp()));

    qulonglong app_total_down = WSGET(WS_APP_TOTAL_DOWN).toULongLong()+downDiff;
    qulonglong app_total_up   = WSGET(WS_APP_TOTAL_UP).toULongLong()+upDiff;

    WSSET(WS_APP_TOTAL_DOWN, QString().setNum(app_total_down));
    WSSET(WS_APP_TOTAL_UP, QString().setNum(app_total_up));

    lastUpdate = ticks;
    lastUp = Socket::getTotalUp();
    lastDown = Socket::getTotalDown();

    typedef Func1<MainLayoutWrapper, QMap<QString, QString> > FUNC;
    FUNC *func = new FUNC(this, &MainLayoutWrapper::updateStatus, map);

    QApplication::postEvent(this, new MainLayoutWrapperCustomEvent(func));
}

Q_EXPORT_PLUGIN2 (leechcraft_eiskaltdcpp, MainLayoutWrapper);

