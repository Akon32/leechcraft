/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "ircparser.h"
#include <boost/bind.hpp>
#include <QTextCodec>
#include "socketmanager.h"
#include "ircserver.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcParser::IrcParser (IrcServer *server)
	: IrcServer_ (server)
	, Prefix_ (QString ())
	, Command_ (QString ())
	, Parameters_ (QStringList ())
	{
		Init ();
	}

	void IrcParser::AuthCommand (const ServerOptions& server)
	{
		if (!server.ServerPassword_.isEmpty ())
		{
			QString passCmd = QString ("PASS " + server.ServerPassword_ + "\r\n");
			Core::Instance ().GetSocketManager ()->
					SendCommand (passCmd, server.ServerName_, server.ServerPort_);
		}

		UserCommand (server);
		NickCommand (server);
	}

	void IrcParser::UserCommand (const ServerOptions& server)
	{
		QString userCmd = QString ("USER " + 
				server.ServerNicknames_.at (0) + 
				" 0 * :" + 
				server.ServerRealName_ + "\r\n");

		Core::Instance ().GetSocketManager ()->
				SendCommand (userCmd, server.ServerName_, server.ServerPort_);
	}

	void IrcParser::NickCommand (const ServerOptions& server)
	{
		QString nickCmd = QString ("NICK " + server.ServerNicknames_.at (0) + "\r\n");
		Core::Instance ().GetSocketManager ()->
				SendCommand (nickCmd, server.ServerName_, server.ServerPort_);
	} 

	void IrcParser::JoinChannel (const ChannelOptions& channel)
	{
		QString joinCmd = QString ("JOIN " + channel.ChannelName_ + "\r\n");
		Core::Instance ().GetSocketManager ()->SendCommand (joinCmd, 
				IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

	void IrcParser::PrivMessageCommand (const QString& text, const ServerOptions& server, const ChannelOptions& channel)
	{
// 		QTextCodec *codec = QTextCodec::codecForName (server.ServerEncoding_.toUtf8 ());
// 		QString mess =  codec->fromUnicode (text);
// 		QString msg = QString ("PRIVMSG " + channel.ChannelName_ + " :" + mess + "\r\n");
// 		Core::Instance ().GetSocketManager ()->SendCommand (msg, server.ServerName_, server.ServerPort_);
// 		QString  id = QString ("%1@%2")
// 					.arg (channel.ChannelName_, channel.ServerName_);
// 		emit messageReceived (mess, id, server.ServerNicknames_.at (0));
	}

	void IrcParser::HandleServerReply (const QString& result)
	{
		ParseMessage (result);
		if (Command_.toLower () == "privmsg")
			qDebug () << result;
		if (Command2Signal_.contains (Command_.toLower ()))
			Command2Signal_ [Command_.toLower ()] (Parameters_);
		Parameters_.clear ();
	}

	void IrcParser::Init ()
	{
		Command2Signal_ ["001"] = boost::bind (&IrcParser::gotAuthSuccess, this, _1);
		connect (this,
				SIGNAL (gotAuthSuccess (const QStringList&)),
				IrcServer_,
				SLOT (authFinished (const QStringList&)));

		Command2Signal_ ["332"] = boost::bind (&IrcParser::gotTopic, this, _1);
		Command2Signal_ ["topic"] = boost::bind (&IrcParser::gotTopic, this, _1);
		connect (this,
				SIGNAL (gotTopic (const QStringList&)),
				IrcServer_,
				SLOT (setTopic (const QStringList&)));

		Command2Signal_ ["ping"] = boost::bind (&IrcParser::gotPing, this, _1);
		connect (this,
				SIGNAL (gotPing (const QStringList&)),
				this,
				SLOT (pongCommand (const QStringList&)));

		Command2Signal_ ["353"] = boost::bind (&IrcParser::gotCLEntries, this, _1);
		connect (this,
				SIGNAL (gotCLEntries (const QStringList&)),
				IrcServer_,
				SLOT (setCLEntries (const QStringList&)));

		Command2Signal_ ["privmsg"] = boost::bind (&IrcParser::gotMessage, this, _1);
		connect (this,
				SIGNAL (gotMessage (const QStringList&)),
				IrcServer_,
				SLOT (readMessage (const QStringList&)));
	}

	void IrcParser::ParseMessage (const QString& msg)
	{
		int pos = 0;
		int msg_len = msg.length ();
		if (msg.startsWith (':'))
		{
			int delim = msg.indexOf (' ');
			if (delim == -1)
				delim = msg_len;
			Prefix_ = msg.mid (1, delim - 1);
			pos = delim + 1;
			ParsePrefix (Prefix_);
		}
		else
			Prefix_ = QString ("");
		
		int par_start = msg.indexOf (' ', pos);
		if (par_start == -1)
			par_start = msg_len;
		
		Command_ = msg.mid (pos, par_start - pos);
		pos = par_start + 1;
		
		while (pos < msg_len)
		{
			if (msg [pos] == ':') {
				Parameters_.append (msg.mid (pos + 1));
				break;
			}
			else 
			{
				int nextpos = msg.indexOf (' ',pos);
				if (nextpos == -1)
					nextpos = msg_len;
				Parameters_.append (msg.mid (pos,nextpos - pos));
				pos = nextpos + 1;
			}
		}
	}

	void IrcParser::ParsePrefix (const QString& prefix)
	{
		// prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
		// servername =  hostname
		// hostname   =  shortname *( "." shortname )
		// shortname  =  ( letter / digit ) *( letter / digit / "-" )
		//              *( letter / digit )

		// host       =  hostname / hostaddr
		// hostaddr   =  ip4addr / ip6addr
		// ip4addr    =  1*3digit "." 1*3digit "." 1*3digit "." 1*3digit
		// ip6addr    =  1*hexdigit 7( ":" 1*hexdigit )
		// ip6addr    =/ "0:0:0:0:0:" ( "0" / "FFFF" ) ":" ip4addr
		// nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
		// user       =  1*( %x01-09 / %x0B-0C / %x0E-1F / %x21-3F / %x41-FF )
		//                   ; any octet except NUL, CR, LF, " " and "@"
		QString nickname = "([a-zA-Z\\[\\]`_\\^\\{\\|\\}]+[a-zA-Z\\[\\]`_\\^\\{\\|\\}-]{0,8})";
		
		int msgLen = prefix.length ();
		int pos_user = prefix.indexOf ("!");
		if (!pos_user)
		{
			int pos_host = prefix.indexOf ("@");
			if (!pos_host)
			{
				int pos_server = prefix.indexOf (".");
				if (!pos_server)
					Nick_ = prefix;
				else
					ServerName = prefix;
			}
			else
			{
				Nick_ = prefix.mid (0, pos_host - 1);
				Host_ = prefix.mid (pos_host + 1, msgLen - pos_host);
			}
		}
		else
		{
			int pos_server = prefix.indexOf ("@");
			if (!pos_server)
			{
				Nick_ = prefix.mid (0, pos_user - 1);
				User_ = prefix.mid (pos_user + 1, msgLen - pos_user);
			}
			else
				Nick_ = prefix.mid (0, pos_user - 1);
				User_ = prefix.mid (pos_user + 1, pos_server - pos_user);
				Host_ = prefix.mid (pos_server + 1, msgLen - pos_server);
		}
	}

	void IrcParser::pongCommand (const QStringList& params)
	{
		QString pongCmd = QString ("PONG :" + params.at (0) + "\r\n");
		Core::Instance ().GetSocketManager ()
				->SendCommand (pongCmd, IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

};
};
};