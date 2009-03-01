#ifndef SEEKTHRU_H
#define SEEKTHRU_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>

class SeekThru : public QObject
			   , public IInfo
			   , public IFinder
{
	Q_OBJECT
	Q_INTERFACES (IInfo IFinder)
public:
	void Init ();
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	QIcon GetIcon () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);

	QStringList GetCategories () const;
	boost::shared_ptr<IFindProxy> GetProxy (const LeechCraft::Request&);
};

#endif

