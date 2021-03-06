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

#include "player.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include "core.h"
#include "mediainfo.h"
#include "localfileresolver.h"
#include "util.h"
#include "localcollection.h"
#include "playlistmanager.h"
#include "staticplaylistmanager.h"

Q_DECLARE_METATYPE (Phonon::MediaSource);

namespace Phonon
{
	uint qHash (const Phonon::MediaSource& src)
	{
		uint hash = 0;
		switch (src.type ())
		{
		case Phonon::MediaSource::LocalFile:
			hash = qHash (src.fileName ());
			break;
		case Phonon::MediaSource::Url:
			hash = qHash (src.url ());
			break;
		case Phonon::MediaSource::Disc:
			hash = src.discType ();
			break;
		case Phonon::MediaSource::Stream:
			hash = qHash (src.deviceName ());
			break;
		default:
			hash = 0;
			break;
		}
		return hash << src.type ();
	}
}

namespace LeechCraft
{
namespace LMP
{
	Player::Player (QObject *parent)
	: QObject (parent)
	, PlaylistModel_ (new QStandardItemModel (this))
	, Source_ (new Phonon::MediaObject (this))
	, Path_ (Phonon::createPath (Source_, new Phonon::AudioOutput (Phonon::MusicCategory, this)))
	, PlayMode_ (PlayMode::Sequential)
	{
		connect (Source_,
				SIGNAL (currentSourceChanged (Phonon::MediaSource)),
				this,
				SLOT (handleCurrentSourceChanged (Phonon::MediaSource)));
		connect (Source_,
				SIGNAL (aboutToFinish ()),
				this,
				SLOT (handleSourceAboutToFinish ()));

		auto staticMgr = Core::Instance ().GetPlaylistManager ()->GetStaticManager ();
		Enqueue (staticMgr->GetOnLoadPlaylist ());
	}

	QAbstractItemModel* Player::GetPlaylistModel () const
	{
		return PlaylistModel_;
	}

	Phonon::MediaObject* Player::GetSourceObject () const
	{
		return Source_;
	}

	void Player::SetPlayMode (Player::PlayMode playMode)
	{
		PlayMode_ = playMode;
	}

	void Player::Enqueue (const QStringList& paths, bool sort)
	{
		QList<Phonon::MediaSource> sources;
		std::transform (paths.begin (), paths.end (), std::back_inserter (sources),
				[] (decltype (paths.front ()) path) { return Phonon::MediaSource (path); });
		Enqueue (sources, sort);
	}

	void Player::Enqueue (const QList<Phonon::MediaSource>& sources, bool sort)
	{
		AddToPlaylistModel (sources, sort);
	}

	QList<Phonon::MediaSource> Player::GetQueue () const
	{
		return CurrentQueue_;
	}

	void Player::Dequeue (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		QList<Phonon::MediaSource> sources;
		if (index.data (Role::IsAlbum).toBool ())
			for (int i = 0; i < PlaylistModel_->rowCount (index); ++i)
				sources << PlaylistModel_->index (i, 0, index).data (Role::Source).value<Phonon::MediaSource> ();
		else
			sources << index.data (Role::Source).value<Phonon::MediaSource> ();

		Q_FOREACH (const auto& source, sources)
		{
			CurrentQueue_.removeAll (source);
			auto item = Items_.take (source);
			auto parent = item->parent ();
			parent->removeRow (item->row ());
			if (!parent->rowCount ())
			{
				AlbumRoots_.remove (AlbumRoots_.key (parent));
				PlaylistModel_->removeRow (parent->row ());
			}
		}

		Core::Instance ().GetPlaylistManager ()->
				GetStaticManager ()->SetOnLoadPlaylist (CurrentQueue_);
	}

	namespace
	{
		void FillItem (QStandardItem *item, const MediaInfo& info)
		{
			item->setText (QString ("%1 - %2 - %3").arg (info.Artist_).arg (info.Album_).arg (info.Title_));
			item->setData (QVariant::fromValue (info), Player::Role::Info);
		}

		QStandardItem* MakeAlbumItem (const MediaInfo& info)
		{
			auto albumItem = new QStandardItem (QString ("%1 - %2")
							.arg (info.Artist_, info.Album_));
			albumItem->setEditable (false);
			albumItem->setData (true, Player::Role::IsAlbum);
			albumItem->setData (QVariant::fromValue (info), Player::Role::Info);
			auto art = FindAlbumArt (info.LocalPath_);
			if (art.isNull ())
				art = QIcon::fromTheme ("media-optical").pixmap (64, 64);
			albumItem->setData (art, Player::Role::AlbumArt);
			albumItem->setData (0, Player::Role::AlbumLength);
			return albumItem;
		}

		void IncAlbumLength (QStandardItem *albumItem, int length)
		{
			const int prevLength = albumItem->data (Player::Role::AlbumLength).toInt ();
			albumItem->setData (length + prevLength, Player::Role::AlbumLength);
		}
	}

	MediaInfo Player::GetMediaInfo (const Phonon::MediaSource& source) const
	{
		return Items_.contains (source) ?
				Items_ [source]->data (Role::Info).value<MediaInfo> () :
				MediaInfo ();
	}

	void Player::AddToPlaylistModel (QList<Phonon::MediaSource> sources, bool sort)
	{
		if (!CurrentQueue_.isEmpty ())
		{
			PlaylistModel_->clear ();
			Items_.clear ();
			AlbumRoots_.clear ();

			auto newList = CurrentQueue_ + sources;
			CurrentQueue_.clear ();
			AddToPlaylistModel (newList, sort);
			return;
		}

		PlaylistModel_->setHorizontalHeaderLabels (QStringList (tr ("Playlist")));

		if (sort)
			ApplyOrdering (sources);
		CurrentQueue_ = sources;

		Core::Instance ().GetPlaylistManager ()->
				GetStaticManager ()->SetOnLoadPlaylist (CurrentQueue_);

		auto resolver = Core::Instance ().GetLocalFileResolver ();

		Q_FOREACH (const auto& source, sources)
		{
			auto item = new QStandardItem ();
			item->setEditable (false);
			item->setData (QVariant::fromValue (source), Role::Source);
			switch (source.type ())
			{
			case Phonon::MediaSource::Stream:
				item->setText (tr ("Stream"));
				PlaylistModel_->appendRow (item);
				break;
			case Phonon::MediaSource::Url:
				item->setText ("URL");
				PlaylistModel_->appendRow (item);
				break;
			case Phonon::MediaSource::LocalFile:
			{
				MediaInfo info;
				try
				{
					info = resolver->ResolveInfo (source.fileName ());
				}
				catch (const ResolveError& error)
				{
					qWarning () << Q_FUNC_INFO
							<< "error resolving info for"
							<< error.GetPath ()
							<< error.what ();
					continue;
				}

				const auto& albumID = qMakePair (info.Artist_, info.Album_);
				FillItem (item, info);
				if (!AlbumRoots_.contains (albumID))
				{
					PlaylistModel_->appendRow (item);
					AlbumRoots_ [albumID] = item;
				}
				else if (AlbumRoots_ [albumID]->data (Role::IsAlbum).toBool ())
				{
					IncAlbumLength (AlbumRoots_ [albumID], info.Length_);
					AlbumRoots_ [albumID]->appendRow (item);
				}
				else
				{
					auto albumItem = MakeAlbumItem (info);

					const int row = AlbumRoots_ [albumID]->row ();
					const auto& existing = PlaylistModel_->takeRow (row);
					albumItem->appendRow (existing);
					albumItem->appendRow (item);
					PlaylistModel_->insertRow (row, albumItem);

					const auto& existingInfo = existing.at (0)->data (Role::Info).value<MediaInfo> ();
					albumItem->setData (existingInfo.Length_, Role::AlbumLength);
					IncAlbumLength (albumItem, info.Length_);

					emit insertedAlbum (albumItem->index ());

					AlbumRoots_ [albumID] = albumItem;
				}
				break;
			}
			default:
				item->setText ("unknown");
				PlaylistModel_->appendRow (item);
				break;
			}

			Items_ [source] = item;
		}
	}

	void Player::ApplyOrdering (QList<Phonon::MediaSource>& sources)
	{
		auto resolver = Core::Instance ().GetLocalFileResolver ();
		std::sort (sources.begin (), sources.end (),
				[resolver] (const Phonon::MediaSource& s1, const Phonon::MediaSource& s2)
				{
					if (s1.type () != Phonon::MediaSource::LocalFile ||
						s2.type () != Phonon::MediaSource::LocalFile)
						return qHash (s1) < qHash (s2);

					try
					{
						const auto& left = resolver->ResolveInfo (s1.fileName ());
						const auto& right = resolver->ResolveInfo (s2.fileName ());
						if (left.Artist_ != right.Artist_)
							return left.Artist_ < right.Artist_;
						if (left.Year_ != right.Year_)
							return left.Year_ < right.Year_;
						if (left.Album_ != right.Album_)
							return left.Album_ < right.Album_;
						if (left.TrackNumber_ != right.TrackNumber_)
							return left.TrackNumber_ < right.TrackNumber_;
						return left.Title_ < right.Title_;
					}
					catch (...)
					{
						return s1.fileName () < s2.fileName ();
					}
				});
	}

	void Player::play (const QModelIndex& index)
	{
		if (index.data (Role::IsAlbum).toBool ())
		{
			play (index.child (0, 0));
			return;
		}

		if (!index.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index;
			return;
		}

		Source_->stop ();
		const auto& source = index.data (Role::Source).value<Phonon::MediaSource> ();
		Source_->setCurrentSource (source);
		Source_->play ();
	}

	void Player::previousTrack ()
	{
		const auto& current = Source_->currentSource ();
		auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);
		if (pos == CurrentQueue_.end () || pos == CurrentQueue_.begin ())
			return;

		Source_->stop ();
		Source_->setCurrentSource (*(--pos));
		Source_->play ();
	}

	void Player::nextTrack()
	{
		const auto& current = Source_->currentSource ();
		auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);
		if (pos == CurrentQueue_.end () || pos == CurrentQueue_.end () - 1)
			return;

		Source_->stop ();
		Source_->setCurrentSource (*(++pos));
		Source_->play ();
	}

	void Player::togglePause ()
	{
		if (Source_->state () == Phonon::PlayingState)
			Source_->pause ();
		else
			Source_->play ();
	}

	void Player::stop ()
	{
		Source_->stop ();
		emit songChanged (MediaInfo ());
	}

	void Player::clear ()
	{
		PlaylistModel_->clear ();
		Items_.clear ();
		AlbumRoots_.clear ();
		CurrentQueue_.clear ();
		Source_->clearQueue ();

		Core::Instance ().GetPlaylistManager ()->
				GetStaticManager ()->SetOnLoadPlaylist (CurrentQueue_);
	}

	void Player::handleSourceAboutToFinish ()
	{
		const auto& current = Source_->currentSource ();
		const auto& path = current.fileName ();
		if (!path.isEmpty ())
			QMetaObject::invokeMethod (Core::Instance ().GetLocalCollection (),
					"recordPlayedTrack",
					Qt::QueuedConnection,
					Q_ARG (QString, path));

		auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);
		switch (PlayMode_)
		{
		case PlayMode::Sequential:
			if (pos != CurrentQueue_.end () && ++pos != CurrentQueue_.end ())
				Source_->enqueue (*pos);
			break;
		case PlayMode::Shuffle:
			Source_->enqueue (CurrentQueue_.at (qrand () % CurrentQueue_.size ()));
			break;
		case PlayMode::RepeatTrack:
			Source_->enqueue (current);
			break;
		case PlayMode::RepeatAlbum:
		{
			if (pos == CurrentQueue_.end ())
				return;
			const auto& curAlbum = GetMediaInfo (*pos).Album_;
			if (++pos == CurrentQueue_.end () ||
					GetMediaInfo (*pos).Album_ != curAlbum)
				while (pos >= CurrentQueue_.begin () &&
						GetMediaInfo (*pos).Album_ == curAlbum)
					--pos;
			Source_->enqueue (*pos);
			break;
		}
		case PlayMode::RepeatWhole:
			if (pos == CurrentQueue_.end () || ++pos == CurrentQueue_.end ())
				pos = CurrentQueue_.begin ();
			Source_->enqueue (*pos);
			break;
		}
	}

	void Player::handleCurrentSourceChanged (const Phonon::MediaSource& source)
	{
		auto curItem = Items_ [source];
		curItem->setData (true, Role::IsCurrent);

		emit songChanged (curItem->data (Role::Info).value<MediaInfo> ());

		Q_FOREACH (auto item, Items_.values ())
		{
			if (item == curItem)
				continue;
			if (item->data (Role::IsCurrent).toBool ())
			{
				item->setData (false, Role::IsCurrent);
				break;
			}
		}
	}
}
}
