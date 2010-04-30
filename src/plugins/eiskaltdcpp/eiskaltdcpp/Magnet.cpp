#include "Magnet.h"

#include <QUrl>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"
#include "dcpp/ClientManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/QueueManager.h"

#include "WulforUtil.h"
#include "SearchFrame.h"
#include "MainLayoutWrapper.h"
#include "Func.h"
#include <QtDebug>
using namespace dcpp;

Magnet::Magnet(QWidget *parent) :
    QDialog(parent),
    t(NULL)
{
    setupUi(this);

    pushButton_BROWSE->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFOLDER_BLUE));

    connect(pushButton_CANCEL,  SIGNAL(clicked()), this, SLOT(accept()));
    connect(pushButton_SEARCH,  SIGNAL(clicked()), this, SLOT(search()));
    connect(pushButton_DOWNLOAD,SIGNAL(clicked()), this, SLOT(download()));
    connect(pushButton_BROWSE, SIGNAL(clicked()), SLOT(slotBrowse()));
    if (!SETTING(AUTO_SEARCH)){
        pushButton_DOWNLOAD->setToolTip(tr("Run search alternatives manually."));
    }
    else {
        pushButton_DOWNLOAD->setToolTip(tr("Download file via auto search alternatives"));
    }
}

Magnet::~Magnet(){
    delete t;
}

void Magnet::customEvent(QEvent *e){
    if (e->type() == MagnetCustomEvent::Event){
        MagnetCustomEvent *c_e = reinterpret_cast<MagnetCustomEvent*>(e);

        c_e->func()->call();
    }

    e->accept();
}

void Magnet::setLink(const QString &link){

    QString name = "", tth = "";
    int64_t size = 0;

    lineEdit_SIZE->setReadOnly(true);

    WulforUtil::splitMagnet(link, size, tth, name);

    lineEdit_FNAME->setText(name);

    if (size > 0)
        lineEdit_SIZE->setText(QString("%1 (%2)").arg(size).arg(WulforUtil::formatBytes(size)));
    else
        lineEdit_SIZE->setText("0 (0 MiB)");

    if (lineEdit_FNAME->text().isEmpty())
        lineEdit_FNAME->setText(tth);

    lineEdit_TTH->setText(tth);
    lineEdit_FPATH->setText(_q(SETTING(DOWNLOAD_DIRECTORY)));

    setWindowTitle(lineEdit_FNAME->text());

    if (!MainLayoutWrapper::getInstance()->isVisible()){
        MainLayoutWrapper::getInstance()->show();
        MainLayoutWrapper::getInstance()->raise();
    }

    if (WIGET(WI_DEF_MAGNET_ACTION) != 0) {
        checkBox_Remember->setChecked(true);
        if (WIGET(WI_DEF_MAGNET_ACTION) == 2)
            Magnet::download();
        else if (WIGET(WI_DEF_MAGNET_ACTION) == 1)
            Magnet::search();
    } else
        checkBox_Remember->setChecked(false);

}

void Magnet::search(){
    QString tth = lineEdit_TTH->text();

    if (checkBox_Remember->isChecked() && WIGET(WI_DEF_MAGNET_ACTION) != 1)
        WISET(WI_DEF_MAGNET_ACTION,1);

    if (tth.isEmpty())
        return;

    SearchFrame *fr = new SearchFrame();
    fr->setAttribute(Qt::WA_DeleteOnClose);

    fr->searchAlternates(tth);

    typedef Func0<Magnet> FUNC;
    FUNC *f = new FUNC(this, &Magnet::accept);

    QApplication::postEvent(this, new MagnetCustomEvent(f));
}

void Magnet::download(){
    QString tth = lineEdit_TTH->text();

    if (checkBox_Remember->isChecked() && WIGET(WI_DEF_MAGNET_ACTION) != 2)
        WISET(WI_DEF_MAGNET_ACTION,2);
    if (tth.isEmpty())
        return;
    QString fname = lineEdit_FNAME->text();
    QString path = lineEdit_FPATH->text();
    QString size_str = lineEdit_SIZE->text();

    QString name = path + (path.endsWith(QDir::separator())? QString("") : QDir::separator()) + fname.split(QDir::separator(), QString::SkipEmptyParts).last();
    qulonglong size = size_str.left(size_str.indexOf(" (")).toULongLong();
    try {
        UserPtr dummyuser(new User(CID::generate()));
        QueueManager::getInstance()->add(_tq(name), size, TTHValue(_tq(tth)), dummyuser, "");
        QueueManager::getInstance()->removeSource(_tq(name), dummyuser, QueueItem::Source::FLAG_REMOVED);
    }
    catch (const std::exception& e){
        QMessageBox::critical(this, tr("Error"), tr("Some error ocurred when starting download:\n %1").arg(e.what()));
    }
    typedef Func0<Magnet> FUNC;
    FUNC *f = new FUNC(this, &Magnet::accept);

    QApplication::postEvent(this, new MagnetCustomEvent(f));


}

void Magnet::timeout(){
    QMessageBox::information(this, tr(""), tr("Search Manager not ready. Please, try again later."));

    accept();
}

void Magnet::slotBrowse(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());

    if (dir.isEmpty())
        return;

        lineEdit_FPATH->setText(dir + PATH_SEPARATOR_STR);
}
