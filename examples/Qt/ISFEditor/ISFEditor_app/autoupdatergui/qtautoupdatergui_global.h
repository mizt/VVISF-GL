#ifndef QTAUTOUPDATERGUI_GLOBAL_H
#define QTAUTOUPDATERGUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QT_BUILD_AUTOUPDATERGUI_LIB)
#	define Q_AUTOUPDATERGUI_EXPORT Q_DECL_EXPORT
#else
#	define Q_AUTOUPDATERGUI_EXPORT Q_DECL_IMPORT
#endif

#endif // QTAUTOUPDATERGUI_GLOBAL_H
