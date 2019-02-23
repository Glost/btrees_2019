/// \file
/// \brief     B-tree and modifications SQLite extension.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      03.01.2019 -- 03.01.2019
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include "btrees_mods.h"

static int btreesModsCreate(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr)
{
    return btreesModsInit(db, pAux, argc, argv, ppVTab, pzErr, 1);
}

static int btreesModsConnect(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr)
{
    return btreesModsInit(db, pAux, argc, argv, ppVTab, pzErr, 0);
}

static int btreesModsBestIndex(sqlite3_vtab* tab, sqlite3_index_info* pIdxInfo)
{
    int rc = SQLITE_OK;
    pIdxInfo->idxNum = 1;
    params.bestIndex = pIdxInfo->idxNum;
    return rc;
}

static int btreesModsDisconnect(sqlite3_vtab* pVtab)
{
    sqlite3_free(pVtab);
    return SQLITE_OK;
}

static int btreesModsDestroy(sqlite3_vtab* pVtab)
{
    close();
    sqlite3_free(pVtab);
    return SQLITE_OK;
}

static int btreesModsInit(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr, int isCreate)
{
    int rc = SQLITE_OK;

    srand(time(NULL));

    sqlite3_str* pSql = sqlite3_str_new(db);
    sqlite3_str_appendf(pSql, "CREATE TABLE %s_real(%s", argv[2], argv[3]);

    int i = 4;
    for (; i < argc; ++i)
        sqlite3_str_appendf(pSql, ",%s", argv[i]);

    sqlite3_str_appendf(pSql, ");");
    char* zSql = sqlite3_str_finish(pSql);

    sqlite3_stmt* stmt = NULL;

    if (!zSql)
        rc = SQLITE_NOMEM;
    else if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, zSql, -1, &stmt, 0)))
        *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));

    rc = sqlite3_step(stmt);
    rc = sqlite3_finalize(stmt);

    if (isCreate)
    {
        sqlite3_str* selectSql = sqlite3_str_new(db);
        sqlite3_str_appendf(selectSql, "SELECT * FROM %s_real;", argv[2]);
        char* selectZSql = sqlite3_str_finish(selectSql);
        sqlite3_stmt* selectStmt = NULL;
        rc = sqlite3_prepare_v2(db, selectZSql, -1, &selectStmt, 0);
        *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
        registerIndexColumn(db, selectStmt, argv[2]);
        rc = createIndex(2, params.indexDataSize);
        sqlite3_finalize(selectStmt);
    }

    rc = sqlite3_declare_vtab(db, zSql);

    sqlite3_free(zSql);

    return rc;
}

static int createIndex(int order, int keySize)
{
    char treeFileName[100];

    char* treePrefix = "tree_";
    strcpy(treeFileName, treePrefix);

    char treeRandomId[20];
    snprintf(treeRandomId, 20, "%d", rand());
    strcat(treeFileName, treeRandomId);

    char treeTimeStamp[20];
    snprintf(treeTimeStamp, 20, "%d", time(NULL));
    strcat(treeFileName, treeTimeStamp);

    char* treeFileExtension = ".btree";
    strcat(treeFileName, treeFileExtension);

    switch (params.bestIndex)
    {
        case 1: create(BaseBTree::TreeType::B_TREE, order, keySize, treeFileName); break;
        case 2: create(BaseBTree::TreeType::B_PLUS_TREE, order, keySize, treeFileName); break;
        case 3: create(BaseBTree::TreeType::B_STAR_TREE, order, keySize, treeFileName); break;
        case 4: create(BaseBTree::TreeType::B_STAR_PLUS_TREE, order, keySize, treeFileName); break;
        default: return -1;
    }

    return 0;
}

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

#ifdef __cplusplus
extern "C" {
#endif

int sqlite3_btreesmods_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
    int rc = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);

    rc = sqlite3_create_module(db, "btrees_mods", &btreesModsModule, 0);

    return rc;
}

#ifdef __cplusplus
}; // extern "C"
#endif
