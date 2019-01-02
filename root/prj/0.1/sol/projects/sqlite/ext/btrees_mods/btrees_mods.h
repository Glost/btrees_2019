/// \file
/// \brief     B-tree and modifications SQLite extension.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      03.01.2019 -- 03.01.2019
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREES_BTREES_MODS_H
#define BTREES_BTREES_MODS_H

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

#ifdef _WIN32
__declspec(dllexport)
#endif

int sqlite3BtreesModsInit(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi);

#endif //BTREES_BTREES_MODS_H
