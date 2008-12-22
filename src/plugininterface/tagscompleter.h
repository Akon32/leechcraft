#ifndef TAGSCOMPLETER_H
#define TAGSCOMPLETER_H
#include <QCompleter>
#include "config.h"

class QLineEdit;

namespace LeechCraft
{
	namespace Util
	{
		/** @brief Completer suitable for tag completion.
		 *
		 * Handles tag completions. One would need special class for this
		 * because standard QCompleter doesn't work: tag sequence isn't
		 * hierarchical, it is rather a set.
		 *
		 * @sa TagsCompletionModel
		 * @sa TagsLineEdit
		 */
		class TagsCompleter : public QCompleter
		{
			Q_OBJECT
		public:
			/** @brief Constructs the completer.
			 *
			 * Sets up for completion and prepares line for work with itself.
			 *
			 * @param[in] line The line edit which would be used for tag
			 * completion.
			 * @param[in] parent Parent object.
			 */
			LEECHCRAFT_API TagsCompleter (QLineEdit *line,
					QObject *parent = 0);
			/** @brief Path splitter override.
			 *
			 * Handles sequence of tags considering its set structure. Splits
			 * the path by spaces and returns the resulting string list.
			 *
			 * @param[in] path The tags sequence to split.
			 * @return Splitted sequence.
			 */
			LEECHCRAFT_API virtual QStringList splitPath (const QString& path) const;
		};
	};
};

#endif

