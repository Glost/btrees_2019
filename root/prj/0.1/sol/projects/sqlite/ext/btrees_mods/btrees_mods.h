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

#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "sqlite3ext.h"
#include "btree_c.h"

SQLITE_EXTENSION_INIT1

#ifdef _WIN32
__declspec(dllexport)
#endif

using namespace btree;

#ifdef __cplusplus
extern "C" {
#endif

int sqlite3_btreesmods_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);

#ifdef __cplusplus
}; // extern "C"
#endif

static int btreesModsCreate(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr);

static int btreesModsConnect(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr);

static int btreesModsInit(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr, int isCreate);

static int btreesModsDisconnect(sqlite3_vtab* pVtab);

static int createIndex(sqlite3_index_info* pIdxInfo, int order, int keySize);

#endif //BTREES_BTREES_MODS_H
