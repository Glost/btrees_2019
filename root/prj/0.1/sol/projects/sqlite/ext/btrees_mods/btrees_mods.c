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

static int btreesModsCreate(sqlite3* db, void* pAux, int argc, char* const* argv, sqlite3_vtab** ppVTab, char** pzErr)
{
    return btreesModsInit(db, pAux, argc, argv, ppVTab, pzErr, 1);
}

static int btreesModsConnect(sqlite3* db, void* pAux, int argc, char* const* argv, sqlite3_vtab** ppVTab, char** pzErr);
{
    return btreesModsInit(db, pAux, argc, argv, ppVTab, pzErr, 0);
}

static int btreesModsBestIndex(sqlite3_vtab* tab, sqlite3_index_info* pIdxInfo)
{
    int rc = SQLITE_OK;
    pIdxInfo->idxNum = 1;
    return rc;
}

static btreesModsDisconnect(sqlite3_vtab* pVtab)
{
    sqlite3_free(pVtab);
    return SQLITE_OK;
}

static int btreesModsInit(sqlite3* db, void* pAux, int argc, char* const* argv, sqlite3_vtab** ppVTab, char** pzErr,
                          int isCreate)
{
    int rc = SQLITE_OK;

    srand(time(NULL));

    // TODO: implement the method.

    return rc;
}

static int createIndex(sqlite3_index_info* pIdxInfo, int order, int keySize)
{
    char treeFileName[100];

    char* treePrefix = "tree_";
    strcpy(treeFileName, treePrefix);

    char treeRandomId[20];
    itoa(rand(), treeRandomId, 10);
    strcat(treeFileName, treeRandomId);

    char treeTimeStamp[20];
    itoa(time(), treeTimeStamp, 10);
    strcat(treeFileName, treeTimeStamp);

    char* treeFileExtension = ".btree";
    strcat(treeFileName, treeFileExtension);

    switch (pIdxInfo->idxNum)
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
        NULL,
        NULL//, // TODO: complete the module struct instance.
};

int sqlite3BtreesModsInit(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
    int rc = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);

    rc = sqlite3_create_module(db, "btrees_mods", &btreesModsModule, 0);

    return rc;
}
