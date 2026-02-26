#ifndef CODE_UTILS_H
#define CODE_UTILS_H

#include <QString>

namespace CodeUtils
{

/// Load a custom function or element template code from a standard templates directory.
/// Returns an empty string if that does not exists or can not be loaded.
QString loadCodeTemplate(const QString &templateName);

}

#endif // CODE_UTILS_H
