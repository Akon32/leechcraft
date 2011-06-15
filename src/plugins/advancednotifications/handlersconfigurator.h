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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_HANDLERSCONFIGURATOR_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_HANDLERSCONFIGURATOR_H
#include <QObject>
#include <QSet>
#include "concretehandlerbase.h"

namespace LeechCraft
{
struct Entity;

namespace AdvancedNotifications
{
	class HandlersConfigurator : public QObject
	{
		Q_OBJECT
	public:
		HandlersConfigurator (QObject* = 0);
		
		QSet<ConcreteHandlerBase::HandlerType> GetEnabledHandlers (const Entity&) const;
	};
}
}

#endif
