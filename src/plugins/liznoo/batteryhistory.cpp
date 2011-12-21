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

#include "batteryhistory.h"
#include "batteryinfo.h"

namespace LeechCraft
{
namespace Liznoo
{
	BatteryHistory::BatteryHistory (const BatteryInfo& info)
	: Percentage_ (info.Percentage_)
	, Voltage_ (info.Voltage_)
	, Energy_ (info.Energy_)
	, EnergyRate_ (info.EnergyRate_)
	{
	}
}
}