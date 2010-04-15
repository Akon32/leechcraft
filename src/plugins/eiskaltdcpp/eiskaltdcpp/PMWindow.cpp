#include "PMWindow.h"
#include "WulforSettings.h"
#include "WulforUtil.h"
#include "HubManager.h"
#include "MainLayoutWrapper.h"
#include "Notification.h"
#include "EmoticonFactory.h"
#include "EmoticonDialog.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ClientManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/User.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>

using namespace dcpp;

int PMWindow::unread = 0;

PMWindow::PMWindow(QString cid, QString hubUrl):
        cid(cid),
        hubUrl(hubUrl),
        arena_menu(NULL),
        hasMessages(false)
{
    setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    this->installEventFilter(this);
    plainTextEdit_INPUT->installEventFilter(this);
    textEdit_CHAT->viewport()->installEventFilter(this);
    textEdit_CHAT->viewport()->setMouseTracking(true);

    if (WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        EmoticonFactory::getInstance()->addEmoticons(textEdit_CHAT->document());

    toolButton_SMILE->setVisible(WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance());
    toolButton_SMILE->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEMOTICON));

    arena_menu = new QMenu(tr("Private message"));
    QAction *close_wnd = new QAction(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    connect(close_wnd, SIGNAL(triggered()), this, SLOT(close()));
    connect(pushButton_HUB, SIGNAL(clicked()), this, SLOT(slotHub()));
    connect(pushButton_SHARE, SIGNAL(clicked()), this, SLOT(slotShare()));
    connect(toolButton_SMILE, SIGNAL(clicked()), this, SLOT(slotSmile()));
}

PMWindow::~PMWindow(){
    delete arena_menu;
}

bool PMWindow::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if ((static_cast<QPlainTextEdit*>(obj) == plainTextEdit_INPUT) &&
            (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) &&
            (k_e->modifiers() == Qt::NoModifier))
        {
            return true;
        }
    }
    else if (e->type() == QEvent::KeyPress){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if ((static_cast<QPlainTextEdit*>(obj) == plainTextEdit_INPUT) &&
            ((k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) && k_e->modifiers() == Qt::NoModifier) ||
             (k_e->key() == Qt::Key_Enter && k_e->modifiers() == Qt::KeypadModifier))
        {
            sendMessage(plainTextEdit_INPUT->toPlainText());

            plainTextEdit_INPUT->setPlainText("");

            return true;
        }

        if (k_e->modifiers() == Qt::ControlModifier){
            if (k_e->key() == Qt::Key_Equal || k_e->key() == Qt::Key_Plus){
                textEdit_CHAT->zoomIn();

                return true;
            }
            else if (k_e->key() == Qt::Key_Minus){
                textEdit_CHAT->zoomOut();

                return true;
            }
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        if ((static_cast<QWidget*>(obj) == textEdit_CHAT->viewport()) && (m_e->button() == Qt::LeftButton)){
            QString pressedParagraph = textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos()));

            WulforUtil::getInstance()->openUrl(pressedParagraph);
        }
    }
    else if (e->type() == QEvent::MouseMove && (static_cast<QWidget*>(obj) == textEdit_CHAT->viewport())){
        QString str = textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos()));

        if (!str.isEmpty())
            textEdit_CHAT->viewport()->setCursor(Qt::PointingHandCursor);
        else
            textEdit_CHAT->viewport()->setCursor(Qt::IBeamCursor);
    }

    return QWidget::eventFilter(obj, e);
}

void PMWindow::closeEvent(QCloseEvent *c_e){
    emit privateMessageClosed(cid);

    MainLayoutWrapper::getInstance()->remArenaWidgetFromToolbar(this);
    MainLayoutWrapper::getInstance()->remArenaWidget(this);

    if (hasMessages)
        unread--;

    hasMessages = false;
    MainLayoutWrapper::getInstance()->redrawToolPanel();

    if (unread == 0)
        Notify->resetTrayIcon();

    c_e->accept();
}

void PMWindow::showEvent(QShowEvent *e){
    e->accept();

    if (isVisible()){
        if (hasMessages)
            unread--;

        hasMessages = false;
        MainLayoutWrapper::getInstance()->redrawToolPanel();

        if (unread == 0)
            Notify->resetTrayIcon();
    }
}

QString PMWindow::getArenaTitle(){
    return WulforUtil::getInstance()->getNicks(CID(cid.toStdString())) + tr(" on hub ") + hubUrl;
}

QString PMWindow::getArenaShortTitle(){
    return WulforUtil::getInstance()->getNicks(CID(cid.toStdString()));
}

QWidget *PMWindow::getWidget(){
    return this;
}

QMenu *PMWindow::getMenu(){
    return arena_menu;
}

void PMWindow::Remove(){
    close();
}

QList<QAction*> PMWindow::GetTabBarContextMenuActions() const{
    return ( arena_menu? arena_menu->actions() : QList<QAction*>() );
}

const QPixmap &PMWindow::getPixmap(){
    if (hasMessages)
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiMESSAGE);
    else
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiSERVER);
}

void PMWindow::addStatusMessage(QString msg){
    QString status = " * ";

    QString nick = "DC-CORE";
    QString time = "[" + QDateTime::currentDateTime().toString("hh:mm:ss") + "]";

    status = time + status;
    status += "<font color=\"" + WulforSettings::getInstance()->getStr(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>: ";
    status += msg;

    addOutput(status);
}

void PMWindow::addStatus(QString msg){
    QString status = "";
    QString nick    = " * ";

    WulforUtil::getInstance()->textToHtml(msg);
    WulforUtil::getInstance()->textToHtml(nick);

    msg             = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + msg + "</font>";
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + QDateTime::currentDateTime().toString("hh:mm:ss") + "]</font>";

    status = time + "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";
    status += msg;

    WulforUtil::getInstance()->textToHtml(status, false);

    addOutput(status);
}

void PMWindow::addOutput(QString msg){

    /* This is temporary block. Later we must make it more wise. */
    msg.replace("\r", "");
    msg.replace("\n", "\n<br/>");
    msg.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");

    textEdit_CHAT->append(msg);

    if (!isVisible()){
        hasMessages = true;
        unread++;
        MainLayoutWrapper::getInstance()->redrawToolPanel();
    }
}

void PMWindow::sendMessage(QString msg, bool stripNewLines){
    UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

    if (user && user->isOnline()){

        if (stripNewLines)
            msg.replace("\n", "");

        if (msg.isEmpty() || msg == "\n")
            return;

        ClientManager::getInstance()->privateMessage(user, msg.toStdString(), false, hubUrl.toStdString());
    }
    else {
        addStatusMessage(tr("User went offline"));
    }
}

void PMWindow::slotHub(){
    HubFrame *fr = HubManager::getInstance()->getHub(hubUrl);

    if (fr)
        MainLayoutWrapper::getInstance()->mapWidgetOnArena(fr);
}

void PMWindow::slotShare(){
    string cid = this->cid.toStdString();

    if (!cid.empty()){
        try{
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

            if (user){
                if (user == ClientManager::getInstance()->getMe())
                    MainLayoutWrapper::getInstance()->browseOwnFiles();
                else
                    QueueManager::getInstance()->addList(user, _tq(hubUrl), QueueItem::FLAG_CLIENT_VIEW);
            }
        }
        catch (const Exception &e){}
    }
}

void PMWindow::slotSmile(){
    if (!(WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance()))
        return;

    int x, y;
    EmoticonDialog *dialog = new EmoticonDialog(this);
    QPixmap p = QPixmap::fromImage(EmoticonFactory::getInstance()->getImage());
    dialog->SetPixmap(p);

    if (dialog->exec() == QDialog::Accepted) {

        dialog->GetXY(x, y);

        QString smiley = EmoticonFactory::getInstance()->textForPos(x, y);

        if (!smiley.isEmpty()) {

            smiley.replace("&lt;", "<");
            smiley.replace("&gt;", ">");
            smiley.replace("&amp;", "&");
            smiley.replace("&apos;", "\'");
            smiley.replace("&quot;", "\"");

            smiley += " ";

            plainTextEdit_INPUT->textCursor().insertText(smiley);
            plainTextEdit_INPUT->setFocus();
        }
    }

    delete dialog;
}
