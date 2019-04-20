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
#include "stdio.h"
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

#define CHAR_BUFFER_SIZE 256

#define BTREE_NUM 1
#define BPLUSTREE_NUM 2
#define BSTARTREE_NUM 3
#define BSTARPLUSTREE_NUM 4

#define INTEGER_SIZE 4
#define FLOAT_SIZE 8
#define TEXT_SIZE 256
#define BLOB_SIZE 256
#define NULL_SIZE 256

#define ROWID_SIZE 8

#define ERROR_CODE -1

#define ROWID_IDX_EOF -1

#define TRUE 1
#define FALSE 0

#define TREE_ORDER 200

#define REBUILD_COEF 0.25
#define REBUILD_COUNT 1000
#define REBUILD_MAX_COUNT 10000

struct indexParams {
    int bestIndex;
    int indexColNumber;
    char* indexColName = NULL;
    char* indexDataType = NULL;
    int indexDataSize;
    char* treeFileName = NULL;
};

typedef struct indexParams indexParams;

struct indexStats {
    int searchesCount;
    int insertsCount;
    int deletesCount;
    int isOriginalStats;
};

typedef struct indexStats indexStats;

struct btreesModsVirtualTable {
    sqlite3_vtab base;
    sqlite3* db = NULL;
    char* tableName = NULL;
    FileBaseBTree* tree = NULL;
    indexParams params = {
            BTREE_NUM,
            -1,
            NULL,
            NULL,
            0,
            NULL
    };
    indexStats stats = {
            0,
            0,
            0,
            FALSE
    };
};

typedef struct btreesModsVirtualTable btreesModsVirtualTable;

struct btreesModsCursor {
    sqlite3_vtab_cursor base;
    sqlite_int64* rowsIds = NULL;
    int currentRowIdIdx;
    int rowsIdsCount;
};

typedef struct btreesModsCursor btreesModsCursor;

static int btreesModsCreate(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr);

static int btreesModsConnect(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr);

static int btreesModsInit(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr, int isCreate);

static int btreesModsBestIndex(sqlite3_vtab* tab, sqlite3_index_info* pIdxInfo);

static int btreesModsDisconnect(sqlite3_vtab* pVtab);

static int btreesModsDestroy(sqlite3_vtab* pVtab);

static int btreesModsFilter(sqlite3_vtab_cursor* cursor, int idxNum, const char* idxStr,
        int argc, sqlite3_value** argv);

static int btreesModsNext(sqlite3_vtab_cursor* cursor);

static int btreesModsEof(sqlite3_vtab_cursor* cursor);

static int btreesModsColumn(sqlite3_vtab_cursor* cursor, sqlite3_context* context, int n);

static int btreesModsHandleConstraint(btreesModsCursor* cursor, int columnNum, unsigned char operation,
        sqlite3_value* exprValue);

static int btreesModsHandleConstraintEq(btreesModsCursor* cursor, sqlite3_value* exprValue);

static int btreesModsUpdate(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid);

static int btreesModsDoUpdate(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid);

static int btreesModsDelete(sqlite3_vtab* pVTab, sqlite3_value* primaryKeyValue, sqlite_int64* pRowid);

static int btreesModsInsert(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid);

static int btreesModsOpen(sqlite3_vtab* pVTab, sqlite3_vtab_cursor** ppCursor);

static int btreesModsClose(sqlite3_vtab_cursor* pCur);

static int btreesModsRowid(sqlite3_vtab_cursor *pCur, sqlite_int64 *pRowid);

static int btreesModsRename(sqlite3_vtab* pVtab, const char* zNew);

static char* getTreeFileName(char* treeFileName);

static int createIndex(btreesModsVirtualTable* virtualTable, int order, int keySize);

static int openIndex(btreesModsVirtualTable* virtualTable);

static int registerIndexColumn(sqlite3* db, sqlite3_stmt* stmt, btreesModsVirtualTable* virtualTable,
        const char* tableName, const char* treeFileName);

static int getDataSizeByType(const char* dataType);

static const char* getDataTypeByInt(int dataType);

static sqlite3_int64 getRowId(btreesModsVirtualTable* virtualTable, sqlite3_value* primaryKeyValue);

static int executeSqlAndFinalize(sqlite3* db, char* sql);

static int executeSql(sqlite3* db, char* sql, sqlite3_stmt** stmt);

static Byte* createTreeKey(sqlite3_value* primaryKeyValue, btreesModsVirtualTable* virtualTable);

static Byte* createTreeKey(sqlite_int64 rowId, btreesModsVirtualTable* virtualTable);

static Byte* createTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId, btreesModsVirtualTable* virtualTable);

static Byte* createIntTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId);

static Byte* createFloatTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId);

static Byte* createTextTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId);

static Byte* createBlobTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId);

static void convertSqliteValueToString(sqlite3_value* value, char** pString);

static void convertSqliteTextValueToString(sqlite3_value* value, char** pString);

static const char* copyString(char** pDestination, const char* source);

static void freeString(char** pString);

static void freeParams(sqlite3_vtab* pVTab);

static void rebuildIndexIfNecessary(btreesModsVirtualTable* virtualTable);

static void rebuildIndex(btreesModsVirtualTable* virtualTable);

static sqlite3_module btreesModsModule = {
        0, // iVersion
        btreesModsCreate,
        btreesModsConnect,
        btreesModsBestIndex,
        btreesModsDisconnect,
        btreesModsDestroy,
        btreesModsOpen,
        btreesModsClose,
        btreesModsFilter,
        btreesModsNext,
        btreesModsEof,
        btreesModsColumn,
        btreesModsRowid,
        btreesModsUpdate,
        NULL, // xBegin
        NULL, // xSync
        NULL, // xCommit
        NULL, // xRollback
        NULL, // xFindFunction
        btreesModsRename,
        NULL, // xSavepoint
        NULL, // xRelease
        NULL, // xRollbackTo
        NULL  // xShadowName
};

#endif //BTREES_BTREES_MODS_H
