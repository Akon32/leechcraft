/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "kinotify.h"
#include <boost/bind.hpp>
#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include "kinotifywidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Kinotify
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
			}

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.Kinotify";
			}

			QString Plugin::GetName () const
			{
				return "Kinotify";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Fancy Kinetic notifications.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon ();
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ();
			}

			QStringList Plugin::Needs () const
			{
				return QStringList ();
			}

			QStringList Plugin::Uses () const
			{
				return QStringList ();
			}

			void Plugin::SetProvider (QObject*, const QString&)
			{
			}

			bool Plugin::CouldHandle (const LeechCraft::Entity& e) const
			{
				return e.Mime_ == "x-leechcraft/notification" &&
						e.Additional_ ["Priority"].toInt () != PLog_;
			}

			void Plugin::Handle (LeechCraft::Entity e)
			{
				Priority prio = static_cast<Priority> (e.Additional_ ["Priority"].toInt ());
				if (prio == PLog_)
					return;

				QString header = e.Entity_.toString ();
				QString text = e.Additional_ ["Text"].toString ();
				
				int timeout = Proxy_->GetSettingsManager ()->
						property ("FinishedDownloadMessageTimeout").toInt () * 1000;

 				KinotifyWidget *notificationWidget =
						new KinotifyWidget (timeout, Proxy_->GetMainWindow ());
				
				QStringList actionsNames = e.Additional_ ["NotificationActions"].toStringList ();
				if (!actionsNames.isEmpty ())
				{
					QObject *actionObject = e.Additional_ ["HandlingObject"].value<QObject*> ();
					if (!actionObject)
					{
						qWarning () << Q_FUNC_INFO
								<< "value is not QObject*"
								<< e.Additional_ ["HandlingObject"];
					}
					else
						notificationWidget->SetActions (actionsNames, actionObject);
				}
				
				connect (notificationWidget,
						SIGNAL (checkNotificationQueue ()),
						this,
						SLOT (pushNotification ()));

				QString mi = "information";
				switch (prio)
				{
					case PWarning_:
						mi = "warning";
						break;
					case PCritical_:
						mi = "error";
					default:
						break;
				}

				QString path;
				QMap<int, QString> sizes = Proxy_->GetIconPath (mi);
				if (sizes.size ())
				{
					int size = 0;
					if (!sizes.contains (size))
						size = sizes.keys ().last ();
					path = sizes [size];
				}

				notificationWidget->SetContent (header, text, path);

				if (!ActiveNotifications_.size ())
					notificationWidget->PrepareNotification ();

				ActiveNotifications_ << notificationWidget;
			}

			void Plugin::pushNotification ()
			{
				if (!ActiveNotifications_.size ())
					return;

				ActiveNotifications_.removeFirst ();
				if (ActiveNotifications_.size ())
					ActiveNotifications_.first ()->PrepareNotification ();
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_kinotify, LeechCraft::Plugins::Kinotify::Plugin);

