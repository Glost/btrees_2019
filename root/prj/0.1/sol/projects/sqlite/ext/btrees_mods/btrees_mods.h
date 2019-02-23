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

static int btreesModsBestIndex(sqlite3_vtab* tab, sqlite3_index_info* pIdxInfo);

static int btreesModsDisconnect(sqlite3_vtab* pVtab);

static int btreesModsDestroy(sqlite3_vtab* pVtab);

static char* getTreeFileName(char* treeFileName);

static int createIndex(int order, int keySize, const char* treeFileName);

static void registerIndexColumn(sqlite3* db, sqlite3_stmt* stmt, const char* tableName, const char* treeFileName);

static int getDataSizeByType(const char* dataType);

void errorLogCallback(void* pArg, int iErrCode, const char* zMsg);

static sqlite3_module btreesModsModule = {
        0, // iVersion
        btreesModsCreate,
        btreesModsConnect,
        btreesModsBestIndex,
        btreesModsDisconnect,
        btreesModsDestroy,
//        // TODO: complete the module implementation.
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct indexParams {
    int bestIndex;
    int indexColNumber;
    const char* indexColName;
    const char* indexDataType;
    int indexDataSize;
    const char* treeFileName;
};

static indexParams params = {
        0,
        -1,
        NULL,
        NULL,
        0,
        NULL
};

#endif //BTREES_BTREES_MODS_H
