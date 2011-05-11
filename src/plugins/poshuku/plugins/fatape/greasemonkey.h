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

#ifndef PLUGINS_POSHUKU_PLUGINS_FATAPE_GREASEMONKEY_H
#define PLUGINS_POSHUKU_PLUGINS_FATAPE_GREASEMONKEY_H
#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QWebFrame>

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	class GreaseMonkey : public QObject
	{
		Q_OBJECT
		
		QWebFrame *Frame_;
		QString ScriptNamespace_;
		QString ScriptName_;
	public:
		GreaseMonkey (QWebFrame *frame,
				const QString& scriptNamespace, const QString& scriptName);
	public slots:
		void addStyle (const QString& css);
		void deleteValue (const QString& name);
		QVariant getValue (const QString& name);
		QVariant getValue (const QString& name, QVariant defVal);
		QVariant listValues ();
		void setValue (const QString& name, QVariant value);
	};
}
}
}
#endif