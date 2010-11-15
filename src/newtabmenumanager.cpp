/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "newtabmenumanager.h"
#include <QMenu>
#include <QtDebug>
#include "interfaces/iinfo.h"
#include "interfaces/imultitabs.h"

namespace LeechCraft
{
	NewTabMenuManager::NewTabMenuManager (QObject *parent)
	: QObject (parent)
	, NewTabMenu_ (new QMenu (tr ("New tab menu")))
	{
	}

	void NewTabMenuManager::AddObject (QObject *obj)
	{
		IMultiTabs *imt = qobject_cast<IMultiTabs*> (obj);
		if (imt && !RegisteredMultiTabs_.contains (obj))
		{
			RegisteredMultiTabs_ << obj;

			IInfo *ii = qobject_cast<IInfo*> (obj);
			try
			{
				const QString& name = ii->GetName ();
				const QString& info = ii->GetInfo ();
				const QIcon& icon = ii->GetIcon ();
				NewTabMenu_->addAction (icon,
						name,
						obj,
						SLOT (newTabRequested ()))->setToolTip (info);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< e.what ()
					<< obj;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< obj;
			}
		}
	}

	void NewTabMenuManager::HandleEmbedTabRemoved (QObject *obj)
	{
		IInfo *ii = qobject_cast<IInfo*> (obj);

		try
		{
			const QString& name = ii->GetName ();
			const QIcon& icon = ii->GetIcon ();

			QAction *action = 0;
			Q_FOREACH (QAction *act, NewTabMenu_->actions ())
				if (act->text () == name)
				{
					action = new QAction (icon, name, this);
					connect (action,
							SIGNAL (triggered ()),
							this,
							SLOT (restoreEmbedTab ()));
					NewTabMenu_->insertAction (act, action);
					NewTabMenu_->removeAction (act);
					ReaddOnRestore_ [name] = act;
					break;
				}

			if (!action)
				action = NewTabMenu_->addAction (icon, name,
						this,
						SLOT (restoreEmbedTab ()));
			action->setData (QVariant::fromValue<QObject*> (obj));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ()
				<< obj;
			throw;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< obj;
			throw;
		}
	}

	inline bool toolbarActionsLessThan (const QList<QAction*>& l1, const QList<QAction*>& l2)
	{
		return l1.size () < l2.size ();
	}

	void NewTabMenuManager::SetToolbarActions (QList<QList<QAction*> > lists)
	{
		qSort (lists.begin (), lists.end (), toolbarActionsLessThan);

		Q_FOREACH (const QList<QAction*>& list, lists)
		{
			if (list.isEmpty ())
				continue;

			/*
			Q_FOREACH (QAction *act, list)
				act->setParent (this);
				*/

			NewTabMenu_->addSeparator ();
			NewTabMenu_->addActions (list);
		}
	}

	QMenu* NewTabMenuManager::GetNewTabMenu () const
	{
		return NewTabMenu_;
	}

	void NewTabMenuManager::restoreEmbedTab ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
				<< "null action, damn"
				<< sender ();
			return;
		}

		QObject *obj = action->data ().value<QObject*> ();
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
				<< "action's data is not a QObject*"
				<< action
				<< action->data ();
			return;
		}

		try
		{
			if (ReaddOnRestore_.contains (action->text ()))
			{
				QAction *readd = ReaddOnRestore_ [action->text ()];
				NewTabMenu_->insertAction (action, readd);
			}

			action->deleteLater ();
			emit restoreEmbedTabRequested (obj);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ()
				<< obj;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< obj;
		}
	}
}
