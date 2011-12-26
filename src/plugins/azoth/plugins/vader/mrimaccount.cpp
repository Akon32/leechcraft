/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "mrimaccount.h"
#include <QDataStream>
#include <interfaces/iproxyobject.h>
#include "proto/connection.h"
#include "proto/message.h"
#include "mrimprotocol.h"
#include "mrimaccountconfigwidget.h"
#include "mrimbuddy.h"
#include "mrimmessage.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	MRIMAccount::MRIMAccount (const QString& name, MRIMProtocol *proto)
	: QObject (proto)
	, Proto_ (proto)
	, Name_ (name)
	, Conn_ (new Proto::Connection (this))
	{
		connect (Conn_,
				SIGNAL (gotGroups (QStringList)),
				this,
				SLOT (handleGotGroups (QStringList)));
		connect (Conn_,
				SIGNAL (gotContacts (QList<Proto::ContactInfo>)),
				this,
				SLOT (handleGotContacts (QList<Proto::ContactInfo>)));
		connect (Conn_,
				SIGNAL (gotMessage (Proto::Message)),
				this,
				SLOT (handleGotMessage (Proto::Message)));
	}

	void MRIMAccount::FillConfig (MRIMAccountConfigWidget *w)
	{
		Login_ = w->GetLogin ();

		const QString& pass = w->GetPassword ();
		if (!pass.isEmpty ())
			Core::Instance ().GetProxy ()->SetPassword (pass, this);
	}

	Proto::Connection* MRIMAccount::GetConnection () const
	{
		return Conn_;
	}

	QObject* MRIMAccount::GetObject ()
	{
		return this;
	}

	QObject* MRIMAccount::GetParentProtocol () const
	{
		return Proto_;
	}

	IAccount::AccountFeatures MRIMAccount::GetAccountFeatures () const
	{
		return FHasConfigurationDialog;
	}

	QList<QObject*> MRIMAccount::GetCLEntries ()
	{
		QList<QObject*> result;
		Q_FOREACH (auto b, Buddies_.values ())
			result << b;
		return result;
	}

	QString MRIMAccount::GetAccountName () const
	{
		return Name_;
	}

	QString MRIMAccount::GetOurNick () const
	{
		return Login_.split ('@', QString::SkipEmptyParts).value (0);
	}

	void MRIMAccount::RenameAccount (const QString& name)
	{
		Name_ = name;
	}

	QByteArray MRIMAccount::GetAccountID () const
	{
		return Proto_->GetProtocolID () + "_" + Login_.toUtf8 ();
	}

	QList<QAction*> MRIMAccount::GetActions () const
	{
		return QList<QAction*> ();
	}

	void MRIMAccount::QueryInfo (const QString&)
	{
	}

	void MRIMAccount::OpenConfigurationDialog ()
	{
	}

	EntryStatus MRIMAccount::GetState () const
	{
		return EntryStatus ();
	}

	void MRIMAccount::ChangeState (const EntryStatus& status)
	{
		if (!Conn_->IsConnected ())
		{
			const QString& pass = Core::Instance ().GetProxy ()->GetAccountPassword (this);
			Conn_->SetCredentials (Login_, pass);
		}

		Conn_->SetState (status);
	}

	void MRIMAccount::Synchronize ()
	{
	}

	void MRIMAccount::Authorize (QObject*)
	{
	}

	void MRIMAccount::DenyAuth (QObject*)
	{
	}

	void MRIMAccount::RequestAuth (const QString& , const QString& , const QString& , const QStringList&)
	{
	}

	void MRIMAccount::RemoveEntry (QObject*)
	{
	}

	QObject* MRIMAccount::GetTransferManager () const
	{
		return 0;
	}

	QByteArray MRIMAccount::Serialize () const
	{
		QByteArray result;
		QDataStream str (&result, QIODevice::WriteOnly);
		str << static_cast<quint8> (1)
			<< Name_
			<< Login_;

		return result;
	}

	MRIMAccount* MRIMAccount::Deserialize (const QByteArray& ba, MRIMProtocol *proto)
	{
		QDataStream str (ba);
		quint8 ver = 0;
		str >> ver;
		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return 0;
		}

		QString name;
		str >> name;
		MRIMAccount *result = new MRIMAccount (name, proto);
		str >> result->Login_;
		return result;
	}

	void MRIMAccount::handleGotGroups (const QStringList& groups)
	{
		AllGroups_ = groups;
	}

	void MRIMAccount::handleGotContacts (const QList<Proto::ContactInfo>& contacts)
	{
		QList<QObject*> objs;
		Q_FOREACH (const Proto::ContactInfo& contact, contacts)
		{
			MRIMBuddy *buddy = new MRIMBuddy (contact, this);
			objs << buddy;
			Buddies_ [contact.Email_] = buddy;
		}

		emit gotCLItems (objs);
	}

	void MRIMAccount::handleGotMessage (const Proto::Message& msg)
	{
		auto buddy = Buddies_ [msg.From_];
		if (!buddy)
		{
			qWarning () << Q_FUNC_INFO
					<< "incoming message from unknown buddy"
					<< msg.From_;
			return;
		}

		MRIMMessage *obj = new MRIMMessage (IMessage::DIn, IMessage::MTChatMessage, buddy);
		buddy->HandleMessage (obj);
	}
}
}
}