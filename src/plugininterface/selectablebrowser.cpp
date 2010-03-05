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

#include "selectablebrowser.h"
#include <QVBoxLayout>

namespace LeechCraft
{
	namespace Util
	{
		SelectableBrowser::SelectableBrowser (QWidget *parent)
		: QWidget (parent)
		, Internal_ (true)
		{
			QVBoxLayout *lay = new QVBoxLayout ();
			lay->setContentsMargins (0, 0, 0, 0);
			setLayout (lay);
		}
		
		void SelectableBrowser::Construct (IWebBrowser *browser)
		{
			IWebWidget *webWidget;
			if (browser &&
					(webWidget = browser->GetWidget ()))
			{
				Internal_ = false;
				InternalBrowser_.reset ();
				ExternalBrowser_.reset (webWidget);
				layout ()->addWidget (ExternalBrowser_->Widget ());
			}
			else
			{
				Internal_ = true;
				InternalBrowser_.reset (new QTextBrowser (this));
				InternalBrowser_->setOpenExternalLinks (true);
				ExternalBrowser_.reset ();
				layout ()->addWidget (InternalBrowser_.get ());
			}
		}

		void SelectableBrowser::Release ()
		{
			InternalBrowser_.reset ();
			ExternalBrowser_.reset ();
		}
		
		void SelectableBrowser::SetHtml (const QString& html, const QUrl& base)
		{
			if (Internal_)
				InternalBrowser_->setHtml (html);
			else
				ExternalBrowser_->SetHtml (html, base);
		}

		void SelectableBrowser::SetNavBarVisible (bool visible)
		{
			if (!Internal_)
				ExternalBrowser_->SetNavBarVisible (visible);
		}
	};
};

