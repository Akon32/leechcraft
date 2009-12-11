/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_CORE_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_CORE_H
#include <QAbstractItemModel>
#include <QHash>
#include <QStringList>
#include <QNetworkReply>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/pluginbase.h>
#include "filter.h"

class QNetworkRequest;
class QWebPage;
class QWebHitTestResult;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace CleanWeb
				{
					class FlashOnClickPlugin;
					class FlashOnClickWhitelist;
					class UserFiltersModel;

					class Core : public QAbstractItemModel
					{
						Q_OBJECT

						FlashOnClickPlugin *FlashOnClickPlugin_;
						FlashOnClickWhitelist *FlashOnClickWhitelist_;
						UserFiltersModel *UserFilters_;

						QList<Filter> Filters_;
						QObjectList Downloaders_;
						QStringList HeaderLabels_;

						struct PendingJob
						{
							QString FullName_;
							QString FileName_;
							QString Subscr_;
							QUrl URL_;
						};
						QMap<int, PendingJob> PendingJobs_;

						ICoreProxy_ptr Proxy_;

						Core ();
					public:
						static Core& Instance ();
						void Release ();

						void SetProxy (ICoreProxy_ptr);
						ICoreProxy_ptr GetProxy () const;

						int columnCount (const QModelIndex& = QModelIndex ()) const;
						QVariant data (const QModelIndex&, int) const;
						QVariant headerData (int, Qt::Orientation, int) const;
						QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
						QModelIndex parent (const QModelIndex&) const;
						int rowCount (const QModelIndex& = QModelIndex ()) const;

						bool CouldHandle (const DownloadEntity&) const;
						void Handle (DownloadEntity);
						QAbstractItemModel* GetModel ();
						void Remove (const QModelIndex&);
						QNetworkReply* Hook (LeechCraft::IHookProxy_ptr,
								QNetworkAccessManager*,
								QNetworkAccessManager::Operation*,
								QNetworkRequest*,
								QIODevice**);
						bool ShouldReject (const QNetworkRequest&) const;
						void HandleContextMenu (const QWebHitTestResult&,
								QMenu*, PluginBase::WebViewCtxMenuStage);

						UserFiltersModel* GetUserFiltersModel () const;
						FlashOnClickPlugin* GetFlashOnClick ();
						FlashOnClickWhitelist* GetFlashOnClickWhitelist ();
					private:
						bool Matches (const QString&, const Filter&,
								const QString&, const QString&) const;
						void HandleProvider (QObject*);
						void Parse (const QString&);

						/** Loads the subscription from the url with the name
						 * subscrName. Returns true if the load delegation was
						 * successful, otherwise returns false.
						 */
						bool Load (const QUrl& url, const QString& subscrName);

						/** Removes the subscription at
						 * ~/.leechcraft/cleanweb/filename.
						 */
						void Remove (const QString& filename);
						void WriteSettings ();
						void ReadSettings ();
						bool AssignSD (const SubscriptionData&);
					private slots:
						void blockImage ();
						void update ();
						void handleJobFinished (int);
						void handleJobError (int, IDownload::Error);
					signals:
						void delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**);
					};
				};
			};
		};
	};
};

#endif

