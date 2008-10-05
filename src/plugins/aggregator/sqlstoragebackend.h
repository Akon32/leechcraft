#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include "storagebackend.h"

class SQLStorageBackend : public StorageBackend
{
	QSqlDatabase DB_;
	QSqlQuery FeedFinderByURL_,
			  InsertFeed_,
			  InsertChannel_,
			  InsertItem_;
public:
	SQLStorageBackend ();
	virtual ~SQLStorageBackend ();

	virtual void GetFeeds (feeds_container_t&) const;
	virtual void GetChannels (Feed_ptr,
			channels_container_t&) const;
	virtual void GetItems (Channel_ptr,
			items_container_t&) const;

	virtual void AddFeed (Feed_ptr);
	virtual void UpdateFeed (Feed_ptr);
	virtual void UpdateItem (Item_ptr);
private:
	void AddChannel (Channel_ptr, const QString&);
	void AddItem (Item_ptr, const QString&);
	bool InitializeTables ();
	void DumpError (const QSqlError&) const;
	void DumpError (const QSqlQuery&) const;
	QByteArray SerializePixmap (const QPixmap&) const;
	QPixmap UnserializePixmap (const QByteArray&) const;
};

#endif

