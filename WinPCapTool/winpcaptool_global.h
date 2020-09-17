#pragma once

#include <QtCore/qglobal.h>
#include <guiddef.h>
#include <windows.h>

#ifndef BUILD_STATIC
# if defined(WINPCAPTOOL_LIB)
#  define WINPCAPTOOL_EXPORT Q_DECL_EXPORT
# else
#  define WINPCAPTOOL_EXPORT Q_DECL_IMPORT
# endif
#else
# define WINPCAPTOOL_EXPORT
#endif


