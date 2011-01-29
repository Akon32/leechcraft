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

#include "firefoximporter.h"
#include "firefoximportpage.h"
#include "firefoxprofileselectpage.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NewLife
		{
			FirefoxImporter::FirefoxImporter (QWidget *parent)
			: AbstractImporter (parent)
			{				
				ImportPage_ = new FirefoxImportPage ();
				ProfileSelectPage_ = new FirefoxProfileSelectPage (); 
			}

			QStringList FirefoxImporter::GetNames () const
			{
				return QStringList ("Firefox");
			}

			QList<QWizardPage*> FirefoxImporter::GetWizardPages () const
			{
				QList<QWizardPage*> result;
				result << ImportPage_;
				result << ProfileSelectPage_;
				return result;
			}
		};
	};
};

