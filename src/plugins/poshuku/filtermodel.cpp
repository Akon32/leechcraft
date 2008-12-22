#include "filtermodel.h"
#include <QStringList>
#include "favoritesmodel.h"

FilterModel::FilterModel (QObject *parent)
: LeechCraft::Util::TagsFilterModel (parent)
{
}

FilterModel::~FilterModel ()
{
}

QStringList FilterModel::GetTagsForIndex (int row) const
{
	return sourceModel ()->data (sourceModel ()->index (row, 0),
			FavoritesModel::TagsRole).toStringList ();
}

