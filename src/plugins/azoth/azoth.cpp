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

#include "azoth.h"
#include <QIcon>
#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QMenu>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/resourceloader.h>
#include <plugininterface/util.h>
#include "core.h"
#include "mainwidget.h"
#include "chattabsmanager.h"
#include "chattab.h"
#include "xmlsettingsmanager.h"
#include "transferjobmanager.h"
#include "servicediscoverywidget.h"
#include "accountslistwidget.h"

namespace LeechCraft
{
namespace Azoth
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("azoth"));

		ChatTab::SetParentMultiTabs (this);

		Core::Instance ().SetProxy (proxy);

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothsettings.xml");

		XmlSettingsDialog_->SetDataSource ("StatusIcons",
				Core::Instance ().GetResourceLoader (Core::RLTStatusIconLoader)->
					GetSubElemModel ());
		XmlSettingsDialog_->SetDataSource ("ClientIcons",
				Core::Instance ().GetResourceLoader (Core::RLTClientIconLoader)->
					GetSubElemModel ());
		XmlSettingsDialog_->SetDataSource ("AffIcons",
				Core::Instance ().GetResourceLoader (Core::RLTAffIconLoader)->
					GetSubElemModel ());
		XmlSettingsDialog_->SetDataSource ("SystemIcons",
				Core::Instance ().GetResourceLoader (Core::RLTSystemIconLoader)->
					GetSubElemModel ());
		
		XmlSettingsDialog_->SetCustomWidget ("AccountsWidget", new AccountsListWidget);

		QMainWindow *mainWin = proxy->GetMainWindow ();
		QDockWidget *dw = new QDockWidget (mainWin);
		MW_ = new MainWidget ();
		dw->setWidget (MW_);
		dw->setWindowTitle ("Azoth");

		mainWin->addDockWidget (Qt::RightDockWidgetArea, dw);

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));

		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (addNewTab (const QString&, QWidget*)),
				this,
				SIGNAL (addNewTab (const QString&, QWidget*)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (changeTabName (QWidget*, const QString&)),
				this,
				SIGNAL (changeTabName (QWidget*, const QString&)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
				this,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (raiseTab (QWidget*)),
				this,
				SIGNAL (raiseTab (QWidget*)));
		
		TabClassInfo chatTab =
		{
			"ChatTab",
			tr ("Chat"),
			tr ("A tab with a chat session"),
			QIcon (),
			0,
			TFEmpty
		};
		TabClassInfo mucTab =
		{
			"MUCTab",
			tr ("MUC"),
			tr ("A multiuser conference"),
			QIcon (),
			50,
			TFOpenableByRequest
		};
		TabClassInfo sdTab =
		{
			"SD",
			tr ("Service discovery"),
			tr ("A service discovery tab that allows to discover "
				"capabilities of remote entries"),
			QIcon (),
			55,
			TFOpenableByRequest
		};
		
		TabClasses_ << chatTab;
		TabClasses_ << mucTab;
		TabClasses_ << sdTab;
	}

	void Plugin::SecondInit ()
	{
		XmlSettingsDialog_->SetDataSource ("SmileIcons",
				Core::Instance ().GetSmilesOptionsModel ());
		XmlSettingsDialog_->SetDataSource ("ChatWindowStyle",
				Core::Instance ().GetChatStylesOptionsModel ());
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth";
	}

	QString Plugin::GetName () const
	{
		return "Azoth";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Extensible IM client for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/azoth/resources/images/azoth.svg");
	}

	QStringList Plugin::Provides () const
	{
		return QStringList (GetUniqueID ());
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Plugin::AddPlugin (QObject *object)
	{
		Core::Instance ().AddPlugin (object);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return Core::Instance ().GetTransferJobManager ()->GetSummaryModel ();
	}
	
	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		switch (aep)
		{
		case AEPTrayMenu:
			result << MW_->GetChangeStatusMenu ()->menuAction ();
			break;
		default:
			break;
		}
		return result;
	}
	
	QMap<QString, QList<QAction*> > Plugin::GetMenuActions () const
	{
		QMap<QString, QList<QAction*> > result;
		result ["Azoth"] << MW_->GetMenuActions ();
		return result;
	}
	
	bool Plugin::CouldHandle (const LeechCraft::Entity& e) const
	{
		return Core::Instance ().CouldHandle (e);
	}
	
	void Plugin::Handle (Entity e)
	{
		Core::Instance ().Handle (e);
	}
	
	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "MUCTab")
			Core::Instance ().handleMucJoinRequested ();
		else if (tabClass == "SD")
		{
			ServiceDiscoveryWidget *sd = new ServiceDiscoveryWidget;
			connect (sd,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));
			emit addNewTab (tr ("Service discovery"), sd);
		}
	}
	
	void Plugin::handleTasksTreeSelectionCurrentRowChanged (const QModelIndex& index, const QModelIndex&)
	{
		QModelIndex si = Core::Instance ().GetProxy ()->MapToSource (index);
		TransferJobManager *mgr = Core::Instance ().GetTransferJobManager ();
		mgr->SelectionChanged (si.model () == mgr->GetSummaryModel () ? si : QModelIndex ());
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth, LeechCraft::Azoth::Plugin);
