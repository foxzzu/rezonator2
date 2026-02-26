#include "CodeUtils.h"

#include <QApplication>
#include <QDebug>
#include <QFile>

namespace CodeUtils
{

QString loadCodeTemplate(const QString &templateName)
{
    QString fileName = templateName;
    if (!templateName.endsWith(".py", Qt::CaseInsensitive))
        fileName += ".py";
        
    QString filePath = qApp->applicationDirPath() + "/templates/" + fileName;

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qWarning() << "Failed to open" << filePath << f.errorString();
        return {};
    }
    return QString::fromUtf8(f.readAll());
}

} // namespace CodeUtils