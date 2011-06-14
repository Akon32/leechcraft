/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
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

#include "quicklaunch.h"
#include <QIcon>

namespace LeechCraft
{
	namespace QuickLaunch
	{
		void Plugin::Init (ICoreProxy_ptr proxy)
		{
		}

		void Plugin::SecondInit ()
		{
		}

		QByteArray Plugin::GetUniqueID () const
		{
			return "org.LeechCraft.QuickLaunch";
		}

		void Plugin::Release ()
		{
		}

		QString Plugin::GetName () const
		{
			return "QuickLaunch";
		}

		QString Plugin::GetInfo () const
		{
			return tr ("");
		}

		QIcon Plugin::GetIcon () const
		{
			return QIcon ();
		}

		LeechCraft::TabClasses_t Plugin::GetTabClasses () const
		{
			TabClasses_t result;
			return result;
		}

		void Plugin::TabOpenRequested (const QByteArray& tabClass)
		{

		}
		Plugin::~Plugin()
		{
		}
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_quicklaunch, LeechCraft::QuickLaunch::Plugin);

