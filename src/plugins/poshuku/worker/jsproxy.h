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

#ifndef PLUGINS_POSHUKU_WORKER_JSPROXY_H
#define PLUGINS_POSHUKU_WORKER_JSPROXY_H
#include <QObject>
#include <QVariant>
#include "customwebpage.h"
#include "pageformsdata.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Worker
			{
				class JSProxy : public QObject
				{
					Q_OBJECT

					PageFormsData_t Current_;
				public:
					JSProxy (QObject* = 0);

					PageFormsData_t GetForms () const;
					void SetForms (const PageFormsData_t&);
					void ClearForms ();
				public slots:
					void debug (const QString& str);
					void warning (const QString& str);

					/** Makes Core remember the form element.
					 */
					void setFormElement (const QString& url,
							int formId,
							const QString& elemName,
							const QString& elemType,
							const QVariant& value);

					/** @brief Returns the stored element's value (or empty QVariant if
					 * none).
					 *
					 * If there is only one element with given elemName, then it's
					 * returned regardless formId, otherwise formId is taken into
					 * account.
					 */
					QVariant getFormElement (int formId,
							const QString& elemName,
							const QString& elemType) const;
				};
			};
		};
	};
};

#endif

