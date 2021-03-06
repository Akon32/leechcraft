/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "proxiesconfigwidget.h"
#include <QStandardItemModel>
#include <QSettings>

Q_DECLARE_METATYPE (QList<LeechCraft::XProxy::Entry_t>);

namespace LeechCraft
{
namespace XProxy
{
	Proxy::operator QNetworkProxy () const
	{
		return QNetworkProxy (Type_, Host_, Port_, User_, Pass_);
	}

	namespace
	{
		QString ProxyType2Str (QNetworkProxy::ProxyType type)
		{
			switch (type)
			{
			case QNetworkProxy::ProxyType::Socks5Proxy:
				return "SOCKS5";
				break;
			case QNetworkProxy::ProxyType::HttpProxy:
				return "HTTP";
				break;
			case QNetworkProxy::ProxyType::HttpCachingProxy:
				return ProxiesConfigWidget::tr ("caching HTTP");
				break;
			case QNetworkProxy::ProxyType::FtpCachingProxy:
				return ProxiesConfigWidget::tr ("caching FTP");
				break;
			default:
				return ProxiesConfigWidget::tr ("other type");
				break;
			}
		}

		QList<QStandardItem*> Entry2Row (const Entry_t& entry)
		{
			QList<QStandardItem*> row;

			const auto& req = entry.first;
			if (req.Protocols_.isEmpty ())
				row << new QStandardItem (ProxiesConfigWidget::tr ("any"));
			else
				row << new QStandardItem (req.Protocols_.join ("; "));
			const QString& targetStr = req.Host_.pattern () +
					":" +
					(req.Port_ > 0 ?
							QString::number (req.Port_) :
							ProxiesConfigWidget::tr ("any"));
			row << new QStandardItem (targetStr);

			const auto& proxy = entry.second;
			row << new QStandardItem (ProxyType2Str (proxy.Type_));
			row << new QStandardItem (proxy.Host_ + ":" + QString::number (proxy.Port_));
			row << new QStandardItem (proxy.User_);

			return row;
		}
	}

	ProxiesConfigWidget::ProxiesConfigWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ProxiesList_->setModel (Model_);

		connect (Ui_.ProxiesList_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleItemSelected (QModelIndex)));

		reject ();
	}

	QList<Proxy> ProxiesConfigWidget::FindMatching (const QString& reqHost, int reqPort, const QString& proto)
	{
		QList<Proxy> result;
		Q_FOREACH (const auto& pair, Entries_)
		{
			const auto& target = pair.first;
			if (target.Port_ && reqPort > 0 && target.Port_ != reqPort)
				continue;

			if (!target.Protocols_.isEmpty () && !target.Protocols_.contains (proto))
				continue;

			if (!target.Host_.exactMatch (reqHost))
				continue;

			result << pair.second;
		}
		return result;
	}

	void ProxiesConfigWidget::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy");
		settings.beginGroup ("SavedProxies");
		Entries_ = settings.value ("Entries").value<decltype (Entries_)> ();
		settings.endGroup ();

		Q_FOREACH (const auto& entry, Entries_)
			Model_->appendRow (Entry2Row (entry));
	}

	void ProxiesConfigWidget::SaveSettings () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy");
		settings.beginGroup ("SavedProxies");
		settings.setValue ("Entries", QVariant::fromValue<decltype (Entries_)> (Entries_));
		settings.endGroup ();
	}

	Entry_t ProxiesConfigWidget::EntryFromUI () const
	{
		QString rxPat = Ui_.TargetHost_->text ();
		if (!rxPat.contains ("*") && !rxPat.contains ("^") && !rxPat.contains ("$"))
		{
			rxPat.prepend (".*");
			rxPat.append (".*");
		}

		ReqTarget targ =
		{
			QRegExp (rxPat, Qt::CaseInsensitive),
			Ui_.TargetPort_->value (),
			Ui_.TargetProto_->text ().split (' ', QString::SkipEmptyParts)
		};

		QNetworkProxy::ProxyType type = QNetworkProxy::ProxyType::NoProxy;
		switch (Ui_.ProxyType_->currentIndex ())
		{
			case 0:
				type = QNetworkProxy::ProxyType::Socks5Proxy;
				break;
			case 1:
				type = QNetworkProxy::ProxyType::HttpProxy;
				break;
			case 2:
				type = QNetworkProxy::ProxyType::HttpCachingProxy;
				break;
			case 3:
				type = QNetworkProxy::ProxyType::FtpCachingProxy;
				break;
		}
		Proxy proxy =
		{
			type,
			Ui_.ProxyHost_->text (),
			Ui_.ProxyPort_->value (),
			Ui_.ProxyUser_->text (),
			Ui_.ProxyPassword_->text ()
		};

		return qMakePair (targ, proxy);
	}

	void ProxiesConfigWidget::accept ()
	{
		SaveSettings ();
	}

	void ProxiesConfigWidget::reject ()
	{
		Model_->clear ();

		QStringList labels;
		labels << tr ("Protocols")
				<< tr ("Target")
				<< tr ("Proxy type")
				<< tr ("Proxy target")
				<< tr ("User");
		Model_->setHorizontalHeaderLabels (labels);

		LoadSettings ();
	}

	void ProxiesConfigWidget::handleItemSelected (const QModelIndex& idx)
	{
		Ui_.UpdateProxyButton_->setEnabled (idx.isValid ());
		Ui_.RemoveProxyButton_->setEnabled (idx.isValid ());

		const auto& entry = Entries_.value (idx.row ());
		Ui_.TargetHost_->setText (entry.first.Host_.pattern ());
		Ui_.TargetPort_->setValue (entry.first.Port_);
		Ui_.TargetProto_->setText (entry.first.Protocols_.join (" "));

		Ui_.ProxyHost_->setText (entry.second.Host_);
		Ui_.ProxyPort_->setValue (entry.second.Port_);
		Ui_.ProxyUser_->setText (entry.second.User_);
		Ui_.ProxyPassword_->setText (entry.second.Pass_);
		switch (entry.second.Type_)
		{
		case QNetworkProxy::ProxyType::Socks5Proxy:
			Ui_.ProxyType_->setCurrentIndex (0);
			break;
		case QNetworkProxy::ProxyType::HttpProxy:
			Ui_.ProxyType_->setCurrentIndex (1);
			break;
		case QNetworkProxy::ProxyType::HttpCachingProxy:
			Ui_.ProxyType_->setCurrentIndex (2);
			break;
		case QNetworkProxy::ProxyType::FtpCachingProxy:
			Ui_.ProxyType_->setCurrentIndex (3);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown proxy type"
					<< entry.second.Type_;
			break;
		}
	}

	void ProxiesConfigWidget::on_AddProxyButton__released ()
	{
		const auto& entry = EntryFromUI ();
		Entries_ << entry;
		Model_->appendRow (Entry2Row (entry));
	}

	void ProxiesConfigWidget::on_UpdateProxyButton__released ()
	{
		const int row = Ui_.ProxiesList_->currentIndex ().row ();
		if (row < 0 || row >= Entries_.size ())
			return;

		const auto& entry = EntryFromUI ();
		Entries_ [row] = entry;
		Model_->removeRow (row);
		Model_->insertRow (row, Entry2Row (entry));
	}

	void ProxiesConfigWidget::on_RemoveProxyButton__released ()
	{
		const int row = Ui_.ProxiesList_->currentIndex ().row ();
		if (row < 0 || row >= Entries_.size ())
			return;

		Entries_.removeAt (row);
		Model_->removeRow (row);
	}

	QDataStream& operator<< (QDataStream& out, const Proxy& p)
	{
		out << static_cast<quint8> (1);
		out << static_cast<qint8> (p.Type_)
			<< p.Host_
			<< p.Port_
			<< p.User_
			<< p.Pass_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Proxy& p)
	{
		quint8 ver = 0;
		in >> ver;
		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return in;
		}

		qint8 type = 0;
		in >> type
			>> p.Host_
			>> p.Port_
			>> p.User_
			>> p.Pass_;
		p.Type_ = static_cast<QNetworkProxy::ProxyType> (type);

		return in;
	}

	QDataStream& operator<< (QDataStream& out, const ReqTarget& t)
	{
		out << static_cast<quint8> (1);
		out << t.Host_
			<< t.Port_
			<< t.Protocols_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, ReqTarget& t)
	{
		quint8 ver = 0;
		in >> ver;
		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return in;
		}

		in >> t.Host_
			>> t.Port_
			>> t.Protocols_;
		return in;
	}
}
}
