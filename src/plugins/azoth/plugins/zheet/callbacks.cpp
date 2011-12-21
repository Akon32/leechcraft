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

#include "callbacks.h"
#include <QSslSocket>
#include <QTcpServer>
#include <util/util.h>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	Callbacks::Callbacks (QObject *parent)
	: QObject (parent)
	{
	}

	void Callbacks::SetNotificationServerConnection (MSN::NotificationServerConnection *conn)
	{
		Conn_ = conn;
	}

	void Callbacks::registerSocket (void *sock, int read, int write, bool)
	{
		auto sockObj = Sockets_ [sock];
		if (read)
			connect (sockObj,
					SIGNAL (readyRead ()),
					this,
					SLOT (handleSocketRead ()));
		if (write)
			connect (sockObj,
					SIGNAL (bytesWritten (qint64)),
					this,
					SLOT (handleSocketWrite ()));
	}

	void Callbacks::unregisterSocket (void *sock)
	{
		disconnect (Sockets_ [sock],
				0,
				this,
				SLOT (handleSocketRead ()));
		disconnect (Sockets_ [sock],
				0,
				this,
				SLOT (handleSocketWrite ()));
	}

	void Callbacks::closeSocket (void *sock)
	{
		Sockets_ [sock]->close ();
		delete Sockets_.take (sock);
	}

	void Callbacks::showError (MSN::Connection *conn, std::string msg)
	{
		auto name = conn->myNotificationServer ()->myDisplayName;

		const QString& str = tr ("Error for MSN account %1: %2.")
				.arg (QString::fromUtf8 (name.c_str ()))
				.arg (QString::fromUtf8 (msg.c_str ()));
		const Entity& e = Util::MakeNotification ("MSN Error", str, PWarning_);
		Core::Instance ().SendEntity (e);
	}

	void Callbacks::buddyChangedStatus (MSN::NotificationServerConnection *conn, MSN::Passport buddy,
			std::string friendlyname, MSN::BuddyStatus state, unsigned int clientID, std::string msnobject)
	{

	}

	void Callbacks::buddyOffline (MSN::NotificationServerConnection *conn, MSN::Passport buddy)
	{

	}

	void Callbacks::log (int writing, const char *buf)
	{
		qDebug () << "[MSN]" << (writing ? "->" : "<-") << buf;
	}

	void Callbacks::gotFriendlyName (MSN::NotificationServerConnection *conn, std::string friendlyname)
	{
	}

	void Callbacks::gotBuddyListInfo (MSN::NotificationServerConnection *conn, MSN::ListSyncInfo *data)
	{
		qDebug () << Q_FUNC_INFO;

		std::map<std::string, int> allContacts;

		std::for_each (data->contactList.begin (), data->contactList.end (),
				[&allContacts] (const std::pair<std::string, MSN::Buddy*>& pair)
					{ allContacts [pair.first] = pair.second->lists & (MSN::LST_AB | MSN::LST_AL | MSN::LST_BL); });

		conn->completeConnection (allContacts, data);

		emit finishedConnecting ();
	}

	void Callbacks::buddyChangedPersonalInfo (MSN::NotificationServerConnection *conn, MSN::Passport fromPassport, MSN::personalInfo pInfo)
	{

	}

	void Callbacks::gotLatestListSerial (MSN::NotificationServerConnection *conn, std::string lastChange)
	{

	}

	void Callbacks::gotGTC (MSN::NotificationServerConnection *conn, char c)
	{

	}

	void Callbacks::gotBLP (MSN::NotificationServerConnection *conn, char c)
	{

	}

	void Callbacks::addedListEntry (MSN::NotificationServerConnection *conn, MSN::ContactList list, MSN::Passport buddy, std::string friendlyname)
	{

	}

	void Callbacks::removedListEntry (MSN::NotificationServerConnection *conn, MSN::ContactList list, MSN::Passport buddy)
	{

	}

	void Callbacks::addedGroup (MSN::NotificationServerConnection *conn, bool added, std::string groupName, std::string groupId)
	{

	}

	void Callbacks::removedGroup (MSN::NotificationServerConnection *conn, bool removed, std::string groupId)
	{

	}

	void Callbacks::renamedGroup (MSN::NotificationServerConnection *conn, bool renamed, std::string newGroupName, std::string groupId)
	{

	}

	void Callbacks::addedContactToGroup (MSN::NotificationServerConnection *conn, bool added, std::string groupId, std::string contactId)
	{

	}

	void Callbacks::removedContactFromGroup (MSN::NotificationServerConnection *conn, bool removed, std::string groupId, std::string contactId)
	{

	}

	void Callbacks::addedContactToAddressBook (MSN::NotificationServerConnection *conn, bool added, std::string passport, std::string displayName, std::string guid)
	{

	}

	void Callbacks::removedContactFromAddressBook (MSN::NotificationServerConnection *conn, bool removed, std::string contactId, std::string passport)
	{

	}

	void Callbacks::enabledContactOnAddressBook (MSN::NotificationServerConnection *conn, bool enabled, std::string contactId, std::string passport)
	{

	}

	void Callbacks::disabledContactOnAddressBook (MSN::NotificationServerConnection *conn, bool disabled, std::string contactId)
	{

	}

	void Callbacks::gotSwitchboard (MSN::SwitchboardServerConnection *conn, const void *tag)
	{

	}

	void Callbacks::buddyJoinedConversation (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname, int is_initial)
	{

	}

	void Callbacks::buddyLeftConversation (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy)
	{

	}

	void Callbacks::gotInstantMessage (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname, MSN::Message *msg)
	{

	}

	void Callbacks::gotMessageSentACK (MSN::SwitchboardServerConnection *conn, int trID)
	{

	}

	void Callbacks::gotEmoticonNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string alias, std::string msnobject)
	{

	}

	void Callbacks::failedSendingMessage (MSN::Connection *conn)
	{

	}

	void Callbacks::gotNudge (MSN::SwitchboardServerConnection *conn, MSN::Passport username)
	{

	}

	void Callbacks::gotVoiceClipNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string msnobject)
	{

	}

	void Callbacks::gotWinkNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string msnobject)
	{

	}

	void Callbacks::gotInk (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string image)
	{

	}

	void Callbacks::gotActionMessage (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string message)
	{

	}

	void Callbacks::buddyTyping (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname)
	{

	}

	void Callbacks::gotInitialEmailNotification (MSN::NotificationServerConnection *conn, int msgs_inbox, int unread_inbox, int msgs_folders, int unread_folders)
	{

	}

	void Callbacks::gotNewEmailNotification (MSN::NotificationServerConnection *conn, std::string from, std::string subject)
	{

	}

	void Callbacks::fileTransferProgress (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, long long unsigned int transferred, long long unsigned int total)
	{

	}

	void Callbacks::fileTransferFailed (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, MSN::fileTransferError error)
	{

	}

	void Callbacks::fileTransferSucceeded (MSN::SwitchboardServerConnection *conn, unsigned int sessionID)
	{

	}

	void Callbacks::fileTransferInviteResponse (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, bool response)
	{

	}

	void Callbacks::gotVoiceClipFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string file)
	{

	}

	void Callbacks::gotEmoticonFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string alias, std::string file)
	{

	}

	void Callbacks::gotWinkFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string file)
	{

	}

	void Callbacks::gotNewConnection (MSN::Connection *conn)
	{
		auto nsc = dynamic_cast<MSN::NotificationServerConnection*> (conn);
		qDebug () << Q_FUNC_INFO << nsc;
		if (nsc)
			nsc->synchronizeContactList ();
	}

	void Callbacks::gotOIMList (MSN::NotificationServerConnection *conn, std::vector<MSN::eachOIM> OIMs)
	{

	}

	void Callbacks::gotOIM (MSN::NotificationServerConnection *conn, bool success, std::string id, std::string message)
	{

	}

	void Callbacks::gotOIMSendConfirmation (MSN::NotificationServerConnection *conn, bool success, int id)
	{

	}

	void Callbacks::gotOIMDeleteConfirmation (MSN::NotificationServerConnection *conn, bool success, std::string id)
	{

	}

	void Callbacks::gotContactDisplayPicture (MSN::SwitchboardServerConnection *conn, MSN::Passport passport, std::string filename)
	{

	}

	void Callbacks::connectionReady (MSN::Connection *conn)
	{

	}

	void Callbacks::closingConnection (MSN::Connection *conn)
	{

	}

	void Callbacks::changedStatus (MSN::NotificationServerConnection *conn, MSN::BuddyStatus state)
	{

	}

	void* Callbacks::connectToServer (std::string server, int port, bool *connected, bool isSSL)
	{
		const QString& servStr = QString::fromUtf8 (server.c_str ());
		qDebug () << Q_FUNC_INFO << servStr << port << isSSL;
		QTcpSocket *sock = 0;
		if (isSSL)
		{
			auto ssl = new QSslSocket (this);
			connect (ssl,
					SIGNAL (sslErrors (QList<QSslError>)),
					ssl,
					SLOT (ignoreSslErrors ()));
			ssl->connectToHostEncrypted (servStr, port);
			sock = ssl;
		}
		else
		{
			sock = new QTcpSocket (this);
			sock->connectToHost (servStr, port);
		}

		connect (sock,
				SIGNAL (connected ()),
				this,
				SLOT (handleSocketConnected ()));

		*connected = false;

		Sockets_ [sock] = sock;
		return sock;
	}

	void Callbacks::askFileTransfer (MSN::SwitchboardServerConnection *conn, MSN::fileTransferInvite ft)
	{

	}

	int Callbacks::listenOnPort (int port)
	{
		QTcpServer *serv = new QTcpServer (this);
		serv->listen (QHostAddress::Any, port);
		return serv->socketDescriptor ();
	}

	std::string Callbacks::getOurIP ()
	{
		// TODO
		return "127.0.0.1";
	}

	std::string Callbacks::getSecureHTTPProxy ()
	{
		return "";
	}

	int Callbacks::getSocketFileDescriptor (void *sock)
	{
		return Sockets_ [sock]->socketDescriptor ();
	}

	size_t Callbacks::getDataFromSocket (void *sock, char *data, size_t size)
	{
		return Sockets_ [sock]->read (data, size);
	}

	size_t Callbacks::writeDataToSocket (void *sock, char *data, size_t size)
	{
		auto res = Sockets_ [sock]->write (data, size);
		Sockets_ [sock]->flush ();
		return res;
	}

	void Callbacks::gotInboxUrl (MSN::NotificationServerConnection*, MSN::hotmailInfo)
	{
		// TODO
	}

	void Callbacks::handleSocketRead ()
	{
		auto c = Conn_->connectionWithSocket (sender ());
		if (!c)
			return;

		c->dataArrivedOnSocket ();
	}

	void Callbacks::handleSocketWrite ()
	{
		auto c = Conn_->connectionWithSocket (sender ());
		if (!c)
			return;

		c->socketIsWritable ();
	}

	void Callbacks::handleSocketConnected ()
	{
		qDebug () << Q_FUNC_INFO << qobject_cast<QAbstractSocket*> (sender ())->socketDescriptor ();
		auto c = Conn_->connectionWithSocket (sender ());
		if (!c)
			return;

		c->socketConnectionCompleted ();
	}
}
}
}
