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

#include "mainwidget.h"
#include <QMenu>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QToolButton>
#include <QInputDialog>
#include <QTimer>
#include "interfaces/iclentry.h"
#include "core.h"
#include "sortfilterproxymodel.h"
#include "setstatusdialog.h"
#include "contactlistdelegate.h"
#include "xmlsettingsmanager.h"
#include "addcontactdialog.h"
#include "joinconferencedialog.h"
#include "bookmarksmanagerdialog.h"
#include "chattabsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	MainWidget::MainWidget (QWidget *parent)
	: QWidget (parent)
	, MainMenu_ (new QMenu (tr ("Azoth menu")))
	, MenuButton_ (new QToolButton (this))
	, ProxyModel_ (new SortFilterProxyModel ())
	{
		MainMenu_->setIcon (QIcon (":/plugins/azoth/resources/images/azoth.svg"));

		Ui_.setupUi (this);
		Ui_.BottomLayout_->insertWidget (0, MenuButton_);
#if QT_VERSION >= 0x040700
		Ui_.FilterLine_->setPlaceholderText (tr ("Search..."));
#endif
		Ui_.CLTree_->setFocusProxy (Ui_.FilterLine_);

		Ui_.CLTree_->setItemDelegate (new ContactListDelegate (Ui_.CLTree_));
		ProxyModel_->setSourceModel (Core::Instance ().GetCLModel ());
		Ui_.CLTree_->setModel (ProxyModel_);
		
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (entryMadeCurrent (QObject*)),
				this,
				SLOT (handleEntryMadeCurrent (QObject*)),
				Qt::QueuedConnection);
		
		connect (Ui_.CLTree_,
				SIGNAL (activated (const QModelIndex&)),
				this,
				SLOT (clearFilter ()));
		
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (const QString&)),
				ProxyModel_,
				SLOT (setFilterFixedString (const QString&)));

		connect (ProxyModel_,
				SIGNAL (rowsInserted (const QModelIndex&, int, int)),
				this,
				SLOT (handleRowsInserted (const QModelIndex&, int, int)));
		connect (ProxyModel_,
				SIGNAL (rowsRemoved (const QModelIndex&, int, int)),
				this,
				SLOT (rebuildTreeExpansions ()));
		connect (ProxyModel_,
				SIGNAL (modelReset ()),
				this,
				SLOT (rebuildTreeExpansions ()));

		QMetaObject::invokeMethod (Ui_.CLTree_,
				"expandToDepth",
				Qt::QueuedConnection,
				Q_ARG (int, 0));
		
		if (Core::Instance ().GetCLModel ()->rowCount ())
			QMetaObject::invokeMethod (this,
					"handleRowsInserted",
					Qt::QueuedConnection,
					Q_ARG (QModelIndex, QModelIndex ()),
					Q_ARG (int, 0),
					Q_ARG (int, Core::Instance ().GetCLModel ()->rowCount () - 1));

		CreateMenu ();
		MenuButton_->setMenu (MainMenu_);
		MenuButton_->setIcon (MainMenu_->icon ());
		MenuButton_->setPopupMode (QToolButton::InstantPopup);

		MenuChangeStatus_ = CreateStatusChangeMenu (SLOT (handleChangeStatusRequested ()), true);
		TrayChangeStatus_ = CreateStatusChangeMenu (SLOT (handleChangeStatusRequested ()), true);
		
		Ui_.FastStatusButton_->setMenu (CreateStatusChangeMenu (SLOT (fastStateChangeRequested ())));
		Ui_.FastStatusButton_->setDefaultAction (new QAction (tr ("Set status"), this));
		UpdateFastStatusButton (SOnline);
		connect (Ui_.FastStatusButton_->defaultAction (),
				SIGNAL (triggered ()),
				this,
				SLOT (applyFastStatus ()));
		connect (Ui_.FastStatusText_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (applyFastStatus ()));
		
		AccountJoinConference_ = new QAction (tr ("Join conference..."), this);
		connect (AccountJoinConference_,
				SIGNAL (triggered ()),
				this,
				SLOT (joinAccountConference ()));
		
		AccountAddContact_ = new QAction (tr ("Add contact..."), this);
		connect (AccountAddContact_,
				SIGNAL (triggered ()),
				this,
				SLOT (addAccountContact ()));
		
		XmlSettingsManager::Instance ().RegisterObject ("ShowMenuBar",
				this, "menuBarVisibilityToggled");
		menuBarVisibilityToggled ();
	}
	
	QList<QAction*> MainWidget::GetMenuActions()
	{
		return QList<QAction*> () << MainMenu_->actions ();
	}
	
	QMenu* MainWidget::GetChangeStatusMenu () const
	{
		return TrayChangeStatus_;
	}

	void MainWidget::CreateMenu ()
	{
		MainMenu_->addSeparator ();
		MainMenu_->addAction (tr ("Add contact..."),
				this,
				SLOT (handleAddContactRequested ()));
		MainMenu_->addAction (tr ("Join conference..."),
				&Core::Instance (),
				SLOT (handleMucJoinRequested ()));
		MainMenu_->addSeparator ();
		MainMenu_->addAction (tr ("Manage bookmarks..."),
				this,
				SLOT (handleManageBookmarks ()));

		MainMenu_->addSeparator ();

		QAction *showOffline = MainMenu_->addAction (tr ("Show offline contacts"));
		showOffline->setProperty ("ActionIcon", "azoth_showoffline");
		showOffline->setCheckable (true);
		bool show = XmlSettingsManager::Instance ()
				.Property ("ShowOfflineContacts", true).toBool ();
		ProxyModel_->showOfflineContacts (show);
		showOffline->setChecked (show);
		connect (showOffline,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleShowOffline (bool)));
	}
	
	QMenu* MainWidget::CreateStatusChangeMenu (const char *slot, bool withCustom)
	{
		QMenu *result = new QMenu (tr ("Change status"));
		result->addAction (Core::Instance ().GetIconForState (SOnline),
				tr ("Online"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOnline));
		result ->addAction (Core::Instance ().GetIconForState (SChat),
				tr ("Free to chat"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SChat));
		result ->addAction (Core::Instance ().GetIconForState (SAway),
				tr ("Away"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SAway));
		result ->addAction (Core::Instance ().GetIconForState (SDND),
				tr ("DND"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SDND));
		result ->addAction (Core::Instance ().GetIconForState (SXA),
				tr ("Extended away"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SXA));
		result ->addAction (Core::Instance ().GetIconForState (SOffline),
				tr ("Offline"), this, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOffline));
	
		if (withCustom)
		{
			result->addSeparator ();
			result->addAction (tr ("Custom..."),
					this,
					SLOT (handleChangeStatusRequested ()));
		}
		return result;
	}
	
	void MainWidget::UpdateFastStatusButton (State state)
	{
		Ui_.FastStatusButton_->defaultAction ()->setIcon (Core::Instance ().GetIconForState (state));
		Ui_.FastStatusButton_->setProperty ("Azoth/TargetState",
				QVariant::fromValue<State> (state));
	}
	
	IAccount* MainWidget::GetAccountFromSender (const char *func)
	{
		if (!sender ())
		{
			qWarning () << func
					<< "no sender";
			return 0;
		}
		
		const QVariant& objVar = sender ()->property ("Azoth/AccountObject");
		QObject *object = objVar.value<QObject*> ();
		if (!object)
		{
			qWarning () << func
					<< "no object in Azoth/AccountObject property of the sender"
					<< sender ()
					<< objVar;
			return 0;
		}
		
		IAccount *account = qobject_cast<IAccount*> (object);
		if (!account)
			qWarning () << func
					<< "object"
					<< object
					<< "could not be cast to IAccount";
		
		return account;
	}

	void MainWidget::on_CLTree__activated (const QModelIndex& index)
	{
		if (index.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
			return;

		Core::Instance ().OpenChat (ProxyModel_->mapToSource (index));
	}

	void MainWidget::on_CLTree__customContextMenuRequested (const QPoint& pos)
	{
		const QModelIndex& index = Ui_.CLTree_->indexAt (pos);
		if (!index.isValid ())
			return;

		QList<QAction*> actions;
		switch (index.data (Core::CLREntryType).value<Core::CLEntryType> ())
		{
		case Core::CLETContact:
		{
			QObject *obj = index.data (Core::CLREntryObject).value<QObject*> ();
			ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
			const QList<QAction*>& allActions = Core::Instance ().GetEntryActions (entry);
			Q_FOREACH (QAction *action, allActions)
				if (Core::Instance ().GetAreasForAction (action)
						.contains (Core::CLEAAContactListCtxtMenu) ||
					action->isSeparator ())
					actions << action;
			break;
		}
		case Core::CLETCategory:
		{
			QAction *rename = new QAction (tr ("Rename group..."), this);
			QVariant objVar = index.parent ().data (Core::CLRAccountObject);;
			rename->setProperty ("Azoth/OldGroupName", index.data ());
			rename->setProperty ("Azoth/AccountObject", objVar);
			connect (rename,
					SIGNAL (triggered ()),
					this,
					SLOT (handleCatRenameTriggered ()));
			actions << rename;
			break;
		}
		case Core::CLETAccount:
		{
			QVariant objVar = index.data (Core::CLRAccountObject);

			AccountJoinConference_->setProperty ("Azoth/AccountObject", objVar);
			AccountAddContact_->setProperty ("Azoth/AccountObject", objVar);
			actions << AccountJoinConference_;
			actions << AccountAddContact_;

			Q_FOREACH (QAction *act, MenuChangeStatus_->actions ())
			{
				if (act->isSeparator ())
					continue;

				act->setData (objVar);

				QVariant stateVar = act->property ("Azoth/TargetState");
				if (!stateVar.isNull ())
				{
					State state = stateVar.value<State> ();
					act->setIcon (Core::Instance ().GetIconForState (state));
				}
			}
			actions << MenuChangeStatus_->menuAction ();
			break;
		}
		default:
			break;
		}
		if (!actions.size ())
			return;

		QMenu *menu = new QMenu (tr ("Entry context menu"));
		menu->addActions (actions);
		menu->exec (Ui_.CLTree_->mapToGlobal (pos));
	}

	void MainWidget::handleChangeStatusRequested ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an action";
			return;
		}

		QObject *obj = action->data ().value<QObject*> ();
		IAccount *acc = qobject_cast<IAccount*> (obj);
		if (obj && !acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IAccount";
			return;
		}

		QVariant stateVar = action->property ("Azoth/TargetState");
		EntryStatus status;
		if (!stateVar.isNull ())
		{
			State state = stateVar.value<State> ();
			const QString& propName = "DefaultStatus" + QString::number (state);
			const QString& text = XmlSettingsManager::Instance ()
					.property (propName.toLatin1 ()).toString ();
			status = EntryStatus (state, text);
		}
		else
		{
			SetStatusDialog *ssd = new SetStatusDialog (this);
			if (ssd->exec () != QDialog::Accepted)
				return;

			status = EntryStatus (ssd->GetState (), ssd->GetStatusText ());
		}
		
		if (acc)
			acc->ChangeState (status);
		else
			Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
				acc->ChangeState (status);
	}
	
	void MainWidget::fastStateChangeRequested ()
	{
		UpdateFastStatusButton (sender ()->
					property ("Azoth/TargetState").value<State> ());
		applyFastStatus ();
	}
	
	void MainWidget::applyFastStatus ()
	{
		const QString& text = Ui_.FastStatusText_->text ();
		Ui_.FastStatusText_->setText (QString ());
		State state = Ui_.FastStatusButton_->
				property ("Azoth/TargetState").value<State> ();
				
		EntryStatus status (state, text);
		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
			acc->ChangeState (status);
	}
	
	void MainWidget::handleCatRenameTriggered ()
	{
		if (!sender ())
		{
			qWarning () << Q_FUNC_INFO
					<< "null sender()";
			return;
		}
		sender ()->deleteLater ();

		const QString& group = sender ()->property ("Azoth/OldGroupName").toString ();

		const QString& newGroup = QInputDialog::getText (this,
				tr ("Rename group"),
				tr ("Enter new group name for %1:")
					.arg (group),
				QLineEdit::Normal,
				group);
		if (newGroup.isEmpty () || newGroup == group)
			return;

		QObject *accObj = sender ()->property ("Azoth/AccountObject").value<QObject*> ();
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< accObj
					<< "to IAccount";
			return;
		}
		
		Q_FOREACH (QObject *entryObj, acc->GetCLEntries ())
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< entryObj
						<< "to ICLEntry";
				continue;
			}
			
			QStringList groups = entry->Groups ();
			if (groups.removeAll (group))
			{
				groups << newGroup;
				entry->SetGroups (groups);
			}
		}
	}
	
	void MainWidget::joinAccountConference ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
		if (!account)
			return;
		
		QList<IAccount*> accounts;
		accounts << account;
		JoinConferenceDialog *dia = new JoinConferenceDialog (accounts,
				Core::Instance ().GetProxy ()->GetMainWindow ());
		dia->show ();
	}
	
	void MainWidget::addAccountContact ()
	{
		IAccount *account = GetAccountFromSender (Q_FUNC_INFO);
		if (!account)
			return;
		
		std::auto_ptr<AddContactDialog> dia (new AddContactDialog (account, this));
		if (dia->exec () != QDialog::Accepted)
			return;
		
		dia->GetSelectedAccount ()->RequestAuth (dia->GetContactID (),
					dia->GetReason (),
					dia->GetNick (),
					dia->GetGroups ());
	}
	
	void MainWidget::handleManageBookmarks ()
	{
		BookmarksManagerDialog *dia = new BookmarksManagerDialog (this);
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
		dia->show ();
	}

	void MainWidget::handleAddContactRequested ()
	{
		std::auto_ptr<AddContactDialog> dia (new AddContactDialog (0, this));
		if (dia->exec () != QDialog::Accepted)
			return;

		dia->GetSelectedAccount ()->RequestAuth (dia->GetContactID (),
					dia->GetReason (),
					dia->GetNick (),
					dia->GetGroups ());
	}

	void MainWidget::handleShowOffline (bool show)
	{
		XmlSettingsManager::Instance ().setProperty ("ShowOfflineContacts", show);
		ProxyModel_->showOfflineContacts (show);
	}
	
	void MainWidget::clearFilter ()
	{
		if (!Ui_.FilterLine_->text ().isEmpty ())
			Ui_.FilterLine_->setText (QString ());
	}
	
	void MainWidget::handleEntryMadeCurrent (QObject *obj)
	{
		if (qobject_cast<IMUCEntry*> (obj))
		{
			ProxyModel_->SetMUC (obj);
			if (Ui_.RosterMode_->currentIndex () == 1)
				QTimer::singleShot (100,
						Ui_.CLTree_,
						SLOT (expandAll ()));
		}
	}
	
	void MainWidget::on_RosterMode__currentIndexChanged (int index)
	{
		ProxyModel_->SetMUCMode (index == 1);
	}

	void MainWidget::menuBarVisibilityToggled ()
	{
		MenuButton_->setVisible (XmlSettingsManager::Instance ().property ("ShowMenuBar").toBool ());
	}

	namespace
	{
		QString BuildPath (const QModelIndex& index)
		{
			QString path = "CLTreeState/Expanded/" + index.data ().toString ();
			QModelIndex parent = index;
			while ((parent = parent.parent ()).isValid ())
				path.prepend (parent.data ().toString () + "/");
			path = path.toUtf8 ().toBase64 ().replace ('/', '_');
			return path;
		}
	}

	void MainWidget::handleRowsInserted (const QModelIndex& parent, int begin, int end)
	{
		const QAbstractItemModel *clModel = ProxyModel_;
		for (int i = begin; i <= end; ++i)
		{
			const QModelIndex& index = clModel->index (i, 0, parent);
			const Core::CLEntryType type =
					index.data (Core::CLREntryType).value<Core::CLEntryType> ();
			if (type == Core::CLETCategory)
			{
				const QString& path = BuildPath (index);

				const bool expanded = XmlSettingsManager::Instance ().Property (path, true).toBool ();
				if (expanded)
					QMetaObject::invokeMethod (Ui_.CLTree_,
							"expand",
							Qt::QueuedConnection,
							Q_ARG (QModelIndex, index));

				if (clModel->rowCount (index))
					handleRowsInserted (index, 0, ProxyModel_->rowCount (index) - 1);
			}
			else if (type == Core::CLETAccount)
				QMetaObject::invokeMethod (Ui_.CLTree_,
						"expand",
						Qt::QueuedConnection,
						Q_ARG (QModelIndex, index));
		}
	}

	void MainWidget::rebuildTreeExpansions ()
	{
		if (Core::Instance ().GetCLModel ()->rowCount ())
			handleRowsInserted (QModelIndex (),
					0, Core::Instance ().GetCLModel ()->rowCount () - 1);
	}

	namespace
	{
		void SetExpanded (const QModelIndex& idx, bool expanded)
		{
			XmlSettingsManager::Instance ().setProperty (BuildPath (idx).toUtf8 (), expanded);
		}
	}

	void MainWidget::on_CLTree__collapsed (const QModelIndex& idx)
	{
		SetExpanded (idx, false);
	}

	void MainWidget::on_CLTree__expanded (const QModelIndex& idx)
	{
		SetExpanded (idx, true);
	}
}
}
