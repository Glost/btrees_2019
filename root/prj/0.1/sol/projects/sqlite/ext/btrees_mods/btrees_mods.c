/// \file
/// \brief     B-tree and modifications SQLite extension.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      03.01.2019 -- 04.05.2019
///            The bachelor thesis of Anton Rigin,
///            the HSE Software Engineering 4-th year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#include "btrees_mods.h"

static int btreesModsCreate(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr)
{
    return btreesModsInit(db, pAux, argc, argv, ppVTab, pzErr, TRUE);
}

static int btreesModsConnect(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr)
{
    return btreesModsInit(db, pAux, argc, argv, ppVTab, pzErr, FALSE);
}

static int btreesModsBestIndex(sqlite3_vtab* tab, sqlite3_index_info* pIdxInfo)
{
    int rc = SQLITE_OK;

    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) tab;
    rebuildIndexIfNecessary(virtualTable);
    pIdxInfo->idxNum = virtualTable->params.bestIndex;

    pIdxInfo->idxStr = (char*) sqlite3_malloc(2 * pIdxInfo->nConstraint);

    int i = 0;
    for ( ; i < pIdxInfo->nConstraint; ++i)
    {
        pIdxInfo->aConstraintUsage[i].argvIndex = i + 1;
        pIdxInfo->aConstraintUsage[i].omit = pIdxInfo->aConstraint[i].usable;

        pIdxInfo->idxStr[2 * i] = pIdxInfo->aConstraint[i].iColumn;
        pIdxInfo->idxStr[2 * i + 1] = pIdxInfo->aConstraint[i].op;
    }

    pIdxInfo->needToFreeIdxStr = TRUE;

    return rc;
}

static int btreesModsDisconnect(sqlite3_vtab* pVtab)
{
    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) pVtab;
    close(&virtualTable->tree);
    freeParams(pVtab);
    return SQLITE_OK;
}

static int btreesModsDestroy(sqlite3_vtab* pVtab)
{
    int rc = SQLITE_OK;

    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) pVtab;

    sqlite3_str* dropSql = sqlite3_str_new(virtualTable->db);
    sqlite3_str_appendf(dropSql, "DROP TABLE %s_real;", virtualTable->tableName);
    rc = executeSqlAndFinalize(virtualTable->db, sqlite3_str_finish(dropSql));

    if (rc)
        return rc;

    sqlite3_str* deleteSql = sqlite3_str_new(virtualTable->db);
    sqlite3_str_appendf(deleteSql, "DELETE FROM btrees_mods_idxinfo WHERE tableName = \"%s\";", virtualTable->tableName);
    rc = executeSqlAndFinalize(virtualTable->db, sqlite3_str_finish(deleteSql));

    if (rc)
        return rc;

    close(&virtualTable->tree);
    remove(virtualTable->params.treeFileName);
    freeParams(pVtab);

    return SQLITE_OK;
}

static int btreesModsFilter(sqlite3_vtab_cursor* cursor, int idxNum, const char* idxStr,
        int argc, sqlite3_value** argv)
{
    int rc = SQLITE_OK;

    btreesModsCursor* customCursor = (btreesModsCursor*) cursor;
    customCursor->rowsIdsCount = -1;
    customCursor->currentRowIdIdx = -1;

    int i = 0;
    for ( ; i < argc; ++i)
    {
        int columnNum = idxStr[2 * i];
        unsigned char operation = idxStr[2 * i + 1];

        rc = btreesModsHandleConstraint(customCursor, columnNum, operation, argv[i]);

        if (customCursor->currentRowIdIdx >= 0)
            return rc;

        if (customCursor->rowsIdsCount >= 0)
            return rc;

        if (rc)
            return rc;
    }

    printf("No primary key value given in the query\n");

    return SQLITE_MISUSE;
}

static int btreesModsNext(sqlite3_vtab_cursor* cursor)
{
    int rc = SQLITE_OK;

    btreesModsCursor* customCursor = (btreesModsCursor*) cursor;

    if (customCursor->currentRowIdIdx == ROWID_IDX_EOF)
        return rc;

    if (customCursor->currentRowIdIdx == customCursor->rowsIdsCount - 1)
    {
        customCursor->currentRowIdIdx = ROWID_IDX_EOF;
        return rc;
    }

    ++customCursor->currentRowIdIdx;

    return rc;
}

static int btreesModsEof(sqlite3_vtab_cursor* cursor)
{
    btreesModsCursor* customCursor = (btreesModsCursor*) cursor;
    return customCursor->currentRowIdIdx == ROWID_IDX_EOF;
}

static int btreesModsColumn(sqlite3_vtab_cursor* cursor, sqlite3_context* context, int n)
{
    int rc = SQLITE_OK;

    btreesModsCursor* customCursor = (btreesModsCursor*) cursor;
    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) cursor->pVtab;

    sqlite3_str* pSql = sqlite3_str_new(virtualTable->db);
    sqlite3_str_appendf(pSql, "SELECT * FROM %s_real WHERE rowid = %d;", virtualTable->tableName,
            customCursor->rowsIds[customCursor->currentRowIdIdx]);
    sqlite3_stmt* stmt = NULL;

    rc = executeSql(virtualTable->db, sqlite3_str_finish(pSql), &stmt);

    if (rc)
        return rc;

    sqlite3_result_value(context, sqlite3_column_value(stmt, n));

    rc = sqlite3_finalize(stmt);

    return rc;
}

static int btreesModsHandleConstraint(btreesModsCursor* cursor, int columnNum, unsigned char operation,
        sqlite3_value* exprValue)
{
    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) cursor->base.pVtab;

    if (columnNum == virtualTable->params.indexColNumber)
    {
        switch (operation)
        {
            case SQLITE_INDEX_CONSTRAINT_EQ:
                return btreesModsHandleConstraintEq(cursor, exprValue);
            default:
                printf("Only equality comparing is supported\n");
                return SQLITE_MISUSE;
        }
    }
}

static int btreesModsHandleConstraintEq(btreesModsCursor* cursor, sqlite3_value* exprValue)
{
    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) cursor->base.pVtab;
    Byte* searchedKey = createTreeKey(exprValue, virtualTable);
    byteComparator.firstPartBytes = virtualTable->params.indexDataSize;

    Byte** keys = NULL;
    int keysCount = searchAll(virtualTable->tree, searchedKey, &keys);
    ++virtualTable->stats.searchesCount;

    cursor->rowsIdsCount = keysCount;

    if (keysCount == 0)
        return SQLITE_OK;

    cursor->rowsIds = (sqlite_int64*) malloc(keysCount * sizeof(sqlite_int64));

    for (int i = 0; i < keysCount; ++i)
    {
        memcpy(&cursor->rowsIds[i], &keys[i][virtualTable->params.indexDataSize], sizeof(sqlite_int64));
        free(keys[i]);
    }

    cursor->currentRowIdIdx = 0;

    free(keys);

    return SQLITE_OK;
}

static int btreesModsUpdate(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid)
{
    if (argc == 1 && sqlite3_value_type(argv[0]) != SQLITE_NULL)
        return btreesModsDelete(pVTab, argv[0], pRowid);
    else if (argc > 1 && sqlite3_value_type(argv[0]) == SQLITE_NULL)
        return btreesModsInsert(pVTab, argc, argv, pRowid);
    else if (argc > 1 && sqlite3_value_type(argv[0]) != SQLITE_NULL)
        return btreesModsDoUpdate(pVTab, argc, argv, pRowid);
    else
        return -1;
}

static int btreesModsDoUpdate(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid)
{
    int rc = SQLITE_OK;
    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) pVTab;

    sqlite3_str* selectSql = sqlite3_str_new(virtualTable->db);
    sqlite3_str_appendf(selectSql, "SELECT * FROM %s_real;", virtualTable->tableName);
    sqlite3_stmt* selectStmt = NULL;
    rc = executeSql(virtualTable->db, sqlite3_str_finish(selectSql), &selectStmt);

    if (rc)
        return rc;

    sqlite3_str* pSql = sqlite3_str_new(virtualTable->db);
    char* strValue = NULL;
    convertSqliteValueToString(argv[2], &strValue);
    sqlite3_str_appendf(pSql, "UPDATE %s_real SET %s = %s", virtualTable->tableName,
            sqlite3_column_name(selectStmt, 0), strValue);

    int i = 3;
    for ( ; i < argc; ++i)
    {
        convertSqliteValueToString(argv[i], &strValue);
        sqlite3_str_appendf(pSql, ", %s = %s", sqlite3_column_name(selectStmt, i - 2), strValue);
    }

    convertSqliteValueToString(argv[0], &strValue);
    sqlite3_str_appendf(pSql, " WHERE %s = %s;", virtualTable->params.indexColName, strValue);

    free(strValue);
    rc = sqlite3_finalize(selectStmt);

    if (rc)
        return rc;

    rc = executeSqlAndFinalize(virtualTable->db, sqlite3_str_finish(pSql));

    *pRowid = getRowId(virtualTable, argv[virtualTable->params.indexColNumber + 2]);

    if (strcmp((char*) sqlite3_value_text(argv[0]),
            (char*) sqlite3_value_text(argv[virtualTable->params.indexColNumber + 2])) != 0)
    {
        byteComparator.firstPartBytes = virtualTable->params.indexDataSize;

        rc = (int) !removeKey(virtualTable->tree, createTreeKey(argv[0], virtualTable));
        ++virtualTable->stats.searchesCount;
        ++virtualTable->stats.deletesCount;

        if (rc)
            return rc;

        insert(virtualTable->tree, createTreeKey(argv[virtualTable->params.indexColNumber + 2],
                *pRowid, virtualTable));
        ++virtualTable->stats.searchesCount;
        ++virtualTable->stats.insertsCount;
    }

    return rc;
}

static int btreesModsDelete(sqlite3_vtab* pVTab, sqlite3_value* primaryKeyValue, sqlite_int64* pRowid)
{
    int rc = SQLITE_OK;
    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) pVTab;
    sqlite3_str* pSql = sqlite3_str_new(virtualTable->db);
    char* strValue = NULL;
    convertSqliteValueToString(primaryKeyValue, &strValue);
    sqlite3_str_appendf(pSql, "DELETE FROM %s_real WHERE %s = %s;",
            virtualTable->tableName,
            virtualTable->params.indexColName,
            strValue);
    free(strValue);
    rc = executeSqlAndFinalize(virtualTable->db, sqlite3_str_finish(pSql));

    if (rc)
        return rc;

    byteComparator.firstPartBytes = virtualTable->params.indexDataSize;

    rc = (int) !removeKey(virtualTable->tree, createTreeKey(primaryKeyValue, virtualTable));
    ++virtualTable->stats.searchesCount;
    ++virtualTable->stats.deletesCount;

    *pRowid = getRowId(virtualTable, primaryKeyValue);

    return rc;
}

static int btreesModsInsert(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid)
{
    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) pVTab;
    rebuildIndexIfNecessary(virtualTable);
    sqlite3_str* pSql = sqlite3_str_new(virtualTable->db);
    char* strValue = NULL;
    convertSqliteValueToString(argv[2], &strValue);
    sqlite3_str_appendf(pSql, "INSERT INTO %s_real VALUES (%s", virtualTable->tableName, strValue);

    int i = 3;
    for ( ; i < argc; ++i)
    {
        convertSqliteValueToString(argv[i], &strValue);
        sqlite3_str_appendf(pSql, ", %s", strValue);
    }

    sqlite3_str_appendf(pSql, ");");

    int rc = executeSqlAndFinalize(virtualTable->db, sqlite3_str_finish(pSql));

    if (rc)
        return rc;

    *pRowid = getRowId(virtualTable, argv[virtualTable->params.indexColNumber + 2]);

    byteComparator.firstPartBytes = virtualTable->params.indexDataSize;

    insert(virtualTable->tree, createTreeKey(argv[virtualTable->params.indexColNumber + 2], *pRowid, virtualTable));
    ++virtualTable->stats.searchesCount;
    ++virtualTable->stats.insertsCount;

    return rc;
}

static int btreesModsOpen(sqlite3_vtab* pVTab, sqlite3_vtab_cursor** ppCursor)
{
    btreesModsCursor* cursor = (btreesModsCursor*) sqlite3_malloc(sizeof(btreesModsCursor));
    memset(cursor, 0, sizeof(btreesModsCursor));
    cursor->base.pVtab = pVTab;
    cursor->currentRowIdIdx = ROWID_IDX_EOF;
    *ppCursor = (sqlite3_vtab_cursor*) cursor;

    return SQLITE_OK;
}

static int btreesModsClose(sqlite3_vtab_cursor* pCur)
{
    sqlite3_free(pCur);
    return SQLITE_OK;
}

static int btreesModsRowid(sqlite3_vtab_cursor *pCur, sqlite_int64 *pRowid)
{
    btreesModsCursor* cursor = (btreesModsCursor*) pCur;

    if (cursor->currentRowIdIdx == ROWID_IDX_EOF)
        pRowid = NULL;
    else
        *pRowid = cursor->rowsIds[cursor->currentRowIdIdx];

    return SQLITE_OK;
}

static int btreesModsRename(sqlite3_vtab* pVtab, const char* zNew)
{
    int rc = SQLITE_OK;

    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) pVtab;

    sqlite3_str* renameSql = sqlite3_str_new(virtualTable->db);
    sqlite3_str_appendf(renameSql, "ALTER TABLE %s_real RENAME TO %s_real;", virtualTable->tableName, zNew);
    rc = executeSqlAndFinalize(virtualTable->db, sqlite3_str_finish(renameSql));

    if (rc)
        return rc;

    sqlite3_str* updateSql = sqlite3_str_new(virtualTable->db);
    sqlite3_str_appendf(updateSql, "UPDATE btrees_mods_idxinfo SET tableName = \"%s\" WHERE tableName = \"%s\";",
            zNew, virtualTable->tableName);
    rc = executeSqlAndFinalize(virtualTable->db, sqlite3_str_finish(updateSql));

    if (rc)
        return rc;

    if (virtualTable->tableName)
    {
        free(virtualTable->tableName);
        virtualTable->tableName = NULL;
    }

    size_t newStrLen = strlen(zNew);
    virtualTable->tableName = (char*) malloc(newStrLen);
    strcpy(virtualTable->tableName, zNew);

    return rc;
}

static int btreesModsInit(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr, int isCreate)
{
    int rc = SQLITE_OK;

    srand(time(NULL));

    sqlite3_str* realSql = sqlite3_str_new(db);
    sqlite3_str* virtualSql = sqlite3_str_new(db);
    sqlite3_str_appendf(realSql, "CREATE TABLE %s_real(%s", argv[2], argv[3]);
    sqlite3_str_appendf(virtualSql, "CREATE TABLE %s(%s", argv[2], argv[3]);

    int i = 4;
    for (; i < argc; ++i)
    {
        sqlite3_str_appendf(realSql, ",%s", argv[i]);
        sqlite3_str_appendf(virtualSql, ",%s", argv[i]);
    }

    sqlite3_str_appendf(realSql, ");");
    sqlite3_str_appendf(virtualSql, ");");
    char* zRealSql = sqlite3_str_finish(realSql);
    char* zVirtualSql = sqlite3_str_finish(virtualSql);

    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) sqlite3_malloc(sizeof(btreesModsVirtualTable));
    virtualTable->db = NULL;
    virtualTable->tree = NULL;
    virtualTable->tableName = NULL;
    virtualTable->params.indexColName = NULL;
    virtualTable->params.indexDataType = NULL;
    virtualTable->params.treeFileName = NULL;
    virtualTable->stats.searchesCount = 0;
    virtualTable->stats.insertsCount = 0;
    virtualTable->stats.deletesCount = 0;
    virtualTable->stats.isOriginalStats = FALSE;

    if (isCreate)
    {
        virtualTable->stats.isOriginalStats = TRUE;

        rc = executeSqlAndFinalize(db, zRealSql);

        if (rc)
            return rc;

        sqlite3_str* selectSql = sqlite3_str_new(db);
        sqlite3_str_appendf(selectSql, "SELECT * FROM %s_real;", argv[2]);
        char* selectZSql = sqlite3_str_finish(selectSql);
        sqlite3_stmt* selectStmt = NULL;
        rc = sqlite3_prepare_v2(db, selectZSql, -1, &selectStmt, 0);
        *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));

        if (rc)
            return rc;

        char treeFileName[CHAR_BUFFER_SIZE];
        char* filledTreeFileName = getTreeFileName(treeFileName);

        rc = registerIndexColumn(db, selectStmt, virtualTable, argv[2], filledTreeFileName);

        if (rc)
            return rc;

        rc = createIndex(virtualTable, TREE_ORDER, virtualTable->params.indexDataSize + sizeof(sqlite_int64));

        if (rc)
            return rc;

        sqlite3_finalize(selectStmt);
    }
    else
    {
        sqlite3_str* getParamsSql = sqlite3_str_new(db);
        sqlite3_str_appendf(getParamsSql, "SELECT * FROM btrees_mods_idxinfo WHERE tableName = \"%s\";", argv[2]);
        sqlite3_stmt* getParamsStmt = NULL;
        executeSql(db, sqlite3_str_finish(getParamsSql), &getParamsStmt);

        int colNum = 1;

        virtualTable->params.bestIndex = sqlite3_column_int(getParamsStmt, colNum++);
        virtualTable->params.indexColNumber = sqlite3_column_int(getParamsStmt, colNum++);
        copyString(&virtualTable->params.indexColName, (char*) sqlite3_column_text(getParamsStmt, colNum++));
        copyString(&virtualTable->params.indexDataType, (char*) sqlite3_column_text(getParamsStmt, colNum++));
        virtualTable->params.indexDataSize = sqlite3_column_int(getParamsStmt, colNum++);
        copyString(&virtualTable->params.treeFileName, (char*) sqlite3_column_text(getParamsStmt, colNum++));

        rc = openIndex(virtualTable);

        if (rc)
            return rc;

        sqlite3_finalize(getParamsStmt);
    }

    rc = sqlite3_declare_vtab(db, zVirtualSql);

    if (rc)
        return rc;

    virtualTable->base.pModule = &btreesModsModule;
    virtualTable->base.nRef = 1;
    virtualTable->base.zErrMsg = NULL;
    virtualTable->db = db;
    size_t tableNameStrLen = strlen(argv[2]);
    virtualTable->tableName = (char*) malloc(tableNameStrLen);
    strcpy(virtualTable->tableName, argv[2]);
    *ppVTab = (sqlite3_vtab*) virtualTable;

    sqlite3_free(zRealSql);
    sqlite3_free(zVirtualSql);

    return rc;
}

static char* getTreeFileName(char* treeFileName)
{
    const char* treePrefix = "tree_";
    strcpy(treeFileName, treePrefix);

    char treeRandomId[CHAR_BUFFER_SIZE];
    sprintf(treeRandomId, "%d", rand());
    strcat(treeFileName, treeRandomId);

    char treeTimeStamp[CHAR_BUFFER_SIZE];
    sprintf(treeTimeStamp, "%d", (int) time(NULL));
    strcat(treeFileName, treeTimeStamp);

    const char* treeFileExtension = ".btree";
    strcat(treeFileName, treeFileExtension);

    return treeFileName;
}

static int createIndex(btreesModsVirtualTable* virtualTable, int order, int keySize)
{
    switch (virtualTable->params.bestIndex)
    {
        case BTREE_NUM:
            create(&virtualTable->tree, BaseBTree::TreeType::B_TREE,
                    order, keySize, virtualTable->params.treeFileName);
            break;
        case BPLUSTREE_NUM:
            create(&virtualTable->tree, BaseBTree::TreeType::B_PLUS_TREE,
                order, keySize, virtualTable->params.treeFileName);
            break;
        case BSTARTREE_NUM:
            create(&virtualTable->tree, BaseBTree::TreeType::B_STAR_TREE,
                order, keySize, virtualTable->params.treeFileName);
            break;
        case BSTARPLUSTREE_NUM:
            create(&virtualTable->tree, BaseBTree::TreeType::B_STAR_PLUS_TREE,
                order, keySize, virtualTable->params.treeFileName);
            break;
        default:
            return ERROR_CODE;
    }

    if (virtualTable->tree)
        return SQLITE_OK;
    else
        return ERROR_CODE;
}

static int openIndex(btreesModsVirtualTable* virtualTable)
{
    switch (virtualTable->params.bestIndex)
    {
        case BTREE_NUM:
            open(&virtualTable->tree, BaseBTree::TreeType::B_TREE, virtualTable->params.treeFileName);
            break;
        case BPLUSTREE_NUM:
            open(&virtualTable->tree, BaseBTree::TreeType::B_PLUS_TREE, virtualTable->params.treeFileName);
            break;
        case BSTARTREE_NUM:
            open(&virtualTable->tree, BaseBTree::TreeType::B_STAR_TREE, virtualTable->params.treeFileName);
            break;
        case BSTARPLUSTREE_NUM:
            open(&virtualTable->tree, BaseBTree::TreeType::B_STAR_PLUS_TREE, virtualTable->params.treeFileName);
            break;
        default:
            return ERROR_CODE;
    }

    if (virtualTable->tree)
        return SQLITE_OK;
    else
        return ERROR_CODE;
}

static int registerIndexColumn(sqlite3* db, sqlite3_stmt* stmt, btreesModsVirtualTable* virtualTable,
        const char* tableName, const char* treeFileName)
{
    int rc = SQLITE_OK;

    const char* columnName = NULL;

    const char* dataType = NULL;
    const char* collSeq = NULL;

    int notNull, primaryKey, autoInc;

    sqlite3_str* createSql = sqlite3_str_new(db);
    sqlite3_str_appendf(createSql, "CREATE TABLE IF NOT EXISTS btrees_mods_idxinfo("
                                   "tableName TEXT PRIMARY KEY, "
                                   "bestIndex INTEGER, "
                                   "indexColNumber INTEGER, "
                                   "indexColName TEXT, "
                                   "indexDataType TEXT, "
                                   "indexDataSize INTEGER, "
                                   "treeFileName TEXT);");
    rc = executeSqlAndFinalize(db, sqlite3_str_finish(createSql));

    if (rc)
        return rc;

    virtualTable->params.indexColNumber = -1;
    copyString(&virtualTable->params.indexColName, "rowid");
    copyString(&virtualTable->params.indexDataType, "INTEGER");
    virtualTable->params.indexDataSize = getDataSizeByType(virtualTable->params.indexDataType);
    virtualTable->params.bestIndex = BTREE_NUM;
    copyString(&virtualTable->params.treeFileName, treeFileName);

    int i = 0;
    for ( ; (columnName = sqlite3_column_name(stmt, i)) != NULL; ++i)
    {
        rc = sqlite3_table_column_metadata(db, NULL, sqlite3_mprintf("%s_real", tableName), columnName,
                &dataType, &collSeq, &notNull, &primaryKey, &autoInc);

        if (rc)
            return rc;

        if (primaryKey)
        {
            virtualTable->params.indexColNumber = i;
            copyString(&virtualTable->params.indexColName, columnName);
            copyString(&virtualTable->params.indexDataType, dataType);
            virtualTable->params.indexDataSize = getDataSizeByType(dataType);

            break;
        }
    }

    sqlite3_str* insertSql = sqlite3_str_new(db);
    sqlite3_str_appendf(insertSql, "INSERT INTO btrees_mods_idxinfo "
                                   "VALUES (\"%s\", %d, %d, \"%s\", \"%s\", %d, \"%s\");",
                        tableName,
                        virtualTable->params.bestIndex,
                        virtualTable->params.indexColNumber,
                        virtualTable->params.indexColName,
                        virtualTable->params.indexDataType,
                        virtualTable->params.indexDataSize,
                        virtualTable->params.treeFileName);
    rc = executeSqlAndFinalize(db, sqlite3_str_finish(insertSql));

    return rc;
}

static int getDataSizeByType(const char* dataType)
{
    if (strcmp(dataType, "INTEGER") == 0)
        return INTEGER_SIZE;
    else if (strcmp(dataType, "FLOAT") == 0)
        return FLOAT_SIZE;
    else if (strcmp(dataType, "TEXT") == 0)
        return TEXT_SIZE;
    else if (strcmp(dataType, "BLOB") == 0)
        return BLOB_SIZE;
    else if (strcmp(dataType, "NULL") == 0)
        return NULL_SIZE;
    else
        return ERROR_CODE;
}

static const char* getDataTypeByInt(int dataType)
{
    switch (dataType)
    {
        case SQLITE_INTEGER:
            return "INTEGER";
        case SQLITE_FLOAT:
            return "FLOAT";
        case SQLITE_TEXT:
            return "TEXT";
        case SQLITE_BLOB:
            return "BLOB";
        case SQLITE_NULL:
            return "NULL";
        default:
            return NULL;
    }
}

static int getIntByDataType(const char* dataType)
{
    if (strcmp(dataType, "INTEGER") == 0)
        return SQLITE_INTEGER;
    else if (strcmp(dataType, "FLOAT") == 0)
        return SQLITE_FLOAT;
    else if (strcmp(dataType, "TEXT") == 0)
        return SQLITE_TEXT;
    else if (strcmp(dataType, "BLOB") == 0)
        return SQLITE_BLOB;
    else if (strcmp(dataType, "NULL") == 0)
        return SQLITE_NULL;
    else
        return -1;
}

static sqlite3_int64 getRowId(btreesModsVirtualTable* virtualTable, sqlite3_value* primaryKeyValue)
{
    char* strValue = NULL;

    sqlite3_str* selectSql = sqlite3_str_new(virtualTable->db);
    convertSqliteValueToString(primaryKeyValue, &strValue);
    sqlite3_str_appendf(selectSql, "SELECT rowid FROM %s_real WHERE %s = %s;",
                        virtualTable->tableName,
                        virtualTable->params.indexColName,
                        strValue);
    free(strValue);
    sqlite3_stmt* stmt = NULL;
    executeSql(virtualTable->db, sqlite3_str_finish(selectSql), &stmt);

    sqlite3_int64 rowId = sqlite3_column_int64(stmt, 0);

    sqlite3_finalize(stmt);

    return rowId;
}

static int executeSqlAndFinalize(sqlite3* db, char* sql)
{
    sqlite3_stmt* stmt = NULL;
    int rc = executeSql(db, sql, &stmt);
    sqlite3_finalize(stmt);
    return rc;
}

static int executeSql(sqlite3* db, char* sql, sqlite3_stmt** stmt)
{
    int rc = SQLITE_OK;

    if (!sql)
        rc = SQLITE_NOMEM;
    else if (SQLITE_OK == (rc = sqlite3_prepare_v2(db, sql, -1, stmt, 0)))
        sqlite3_step(*stmt);

    return rc;
}

static Byte* createTreeKey(sqlite3_value* primaryKeyValue, btreesModsVirtualTable* virtualTable)
{
    return createTreeKey(primaryKeyValue, 0, virtualTable);
}

static Byte* createTreeKey(sqlite_int64 rowId, btreesModsVirtualTable* virtualTable)
{
    return createTreeKey(NULL, rowId, virtualTable);
}

static Byte* createTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId, btreesModsVirtualTable* virtualTable)
{
    if (primaryKeyValue == NULL)
    {
        Byte* treeKey = (Byte*) malloc(virtualTable->params.indexDataSize + sizeof(sqlite_int64));

        memcpy(treeKey, (Byte*) &rowId, virtualTable->params.indexDataSize);
        memcpy(&treeKey[virtualTable->params.indexDataSize], (Byte*) &rowId, sizeof(sqlite_int64));

        return treeKey;
    }
    else
    {
        int dataType = sqlite3_value_type(primaryKeyValue);

        switch (dataType)
        {
            case SQLITE_INTEGER:
                return createIntTreeKey(primaryKeyValue, rowId);
            case SQLITE_FLOAT:
                return createFloatTreeKey(primaryKeyValue, rowId);
            case SQLITE_TEXT:
                return createTextTreeKey(primaryKeyValue, rowId);
            case SQLITE_BLOB:
                return createBlobTreeKey(primaryKeyValue, rowId);
            case SQLITE_NULL:
                return NULL;
        }
    }
}

static Byte* createIntTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId)
{
    Byte* treeKey = (Byte*) malloc(INTEGER_SIZE + sizeof(sqlite_int64));

    int value = sqlite3_value_int(primaryKeyValue);
    memcpy(treeKey, (Byte*) &value, INTEGER_SIZE);
    memcpy(&treeKey[INTEGER_SIZE], (Byte*) &rowId, sizeof(sqlite_int64));

    return treeKey;
}

static Byte* createFloatTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId)
{
    Byte* treeKey = (Byte*) malloc(FLOAT_SIZE + sizeof(sqlite_int64));

    double value = sqlite3_value_double(primaryKeyValue);
    memcpy(treeKey, (Byte*) &value, FLOAT_SIZE);
    memcpy(&treeKey[FLOAT_SIZE], (Byte*) &rowId, sizeof(sqlite_int64));

    return treeKey;
}

static Byte* createTextTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId)
{
    int size = sqlite3_value_bytes(primaryKeyValue);
    size = size >= TEXT_SIZE ? TEXT_SIZE : size;

    Byte* treeKey = (Byte*) malloc(TEXT_SIZE + sizeof(sqlite_int64));
    memset(treeKey, 0, TEXT_SIZE + sizeof(sqlite_int64));

    memcpy(treeKey, (Byte*) sqlite3_value_text(primaryKeyValue), size);
    memcpy(&treeKey[TEXT_SIZE], (Byte*) &rowId, sizeof(sqlite_int64));

    return treeKey;
}

static Byte* createBlobTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId)
{
    int size = sqlite3_value_bytes(primaryKeyValue);
    size = size >= BLOB_SIZE ? BLOB_SIZE : size;

    Byte* treeKey = (Byte*) malloc(BLOB_SIZE + sizeof(sqlite_int64));
    memset(treeKey, 0, BLOB_SIZE + sizeof(sqlite_int64));

    memcpy(treeKey, (Byte*) sqlite3_value_blob(primaryKeyValue), size);
    memcpy(&treeKey[BLOB_SIZE], (Byte*) &rowId, sizeof(sqlite_int64));

    return treeKey;
}

static void convertSqliteValueToString(sqlite3_value* value, char** pString)
{
    if (*pString)
    {
        free(*pString);
        *pString = NULL;
    }

    *pString = (char*) malloc(TEXT_SIZE);

    switch (sqlite3_value_type(value))
    {
        case SQLITE_INTEGER:
        case SQLITE_FLOAT:
            strcpy(*pString, (const char*) sqlite3_value_text(value));
            break;
        case SQLITE_TEXT:
            convertSqliteTextValueToString(value, pString);
            break;
        case SQLITE_BLOB:
            strcpy(*pString, (const char*) sqlite3_value_blob(value));
            break;
        case SQLITE_NULL:
            sprintf(*pString, "NULL");
            break;
    }
}

static void convertSqliteTextValueToString(sqlite3_value* value, char** pString)
{
    snprintf(*pString, TEXT_SIZE - 1, "\"%s", (const char*) sqlite3_value_text(value));
    strcat(*pString, "\"");
}

static void copyString(char** pDestination, const char* source)
{
    if (*pDestination)
        freeString(pDestination);

    *pDestination = (char*) malloc(strlen(source));
    strcpy(*pDestination, source);
}

static void freeString(char** pString)
{
    free(*pString);
    *pString = NULL;
}

static void freeParams(sqlite3_vtab* pVTab)
{
    btreesModsVirtualTable* virtualTable = (btreesModsVirtualTable*) pVTab;

    free(virtualTable->params.indexColName);
    virtualTable->params.indexColName = NULL;
    free(virtualTable->params.indexDataType);
    virtualTable->params.indexDataType = NULL;
    free(virtualTable->params.treeFileName);
    virtualTable->params.treeFileName = NULL;

    free(virtualTable->tableName);

    sqlite3_free(pVTab);
}

static void rebuildIndexIfNecessary(btreesModsVirtualTable* virtualTable)
{
    int updatesCount = virtualTable->stats.insertsCount + virtualTable->stats.deletesCount;
    int totalOperationsCount = updatesCount + virtualTable->stats.searchesCount;

    if (!virtualTable->stats.isOriginalStats)
        return;

    if (totalOperationsCount == 0)
        return;

    if (totalOperationsCount > REBUILD_MAX_COUNT)
        return;

    if (totalOperationsCount % REBUILD_COUNT != 0)
        return;

    if (updatesCount < REBUILD_COEF * totalOperationsCount)
        return;

    int oldBestIndex = virtualTable->params.bestIndex;

    if (virtualTable->stats.insertsCount > (1 - REBUILD_COEF) * updatesCount)
        virtualTable->params.bestIndex = BSTARTREE_NUM;
    else if (virtualTable->stats.insertsCount >= REBUILD_COEF * updatesCount)
        virtualTable->params.bestIndex = BSTARPLUSTREE_NUM;
    else
        virtualTable->params.bestIndex = BPLUSTREE_NUM;

    if (virtualTable->params.bestIndex != oldBestIndex)
        rebuildIndex(virtualTable);
}

static void rebuildIndex(btreesModsVirtualTable* virtualTable)
{
    searchAllByteComparator.firstPartBytes = virtualTable->params.indexDataSize;
    setSearchAllByteComparator(virtualTable->tree);
    Byte** keys = NULL;
    int keysCount = searchAll(virtualTable->tree, createTreeKey((sqlite_int64) 0, virtualTable), &keys);
    setByteComparator(virtualTable->tree);

    close(&virtualTable->tree);
    remove(virtualTable->params.treeFileName);

    createIndex(virtualTable, TREE_ORDER, virtualTable->params.indexDataSize + sizeof(sqlite_int64));

    int i = 0;
    for ( ; i < keysCount; ++i)
        insert(virtualTable->tree, keys[i]);

    sqlite3_str* updateParamsSql = sqlite3_str_new(virtualTable->db);
    sqlite3_str_appendf(updateParamsSql, "UPDATE btrees_mods_idxinfo SET bestIndex = %d WHERE tableName = \"%s\";",
            virtualTable->params.bestIndex, virtualTable->tableName);
    executeSqlAndFinalize(virtualTable->db, sqlite3_str_finish(updateParamsSql));
}

static void btreesModsVisualize(sqlite3_context* ctx, int argc, sqlite3_value** argv)
{
    if (argc != 2)
    {
        sqlite3_result_text(ctx, "Arguments count should be equal to 2", -1, NULL);
        return;
    }

    if (sqlite3_value_type(argv[0]) != SQLITE3_TEXT || sqlite3_value_type(argv[1]) != SQLITE3_TEXT)
    {
        sqlite3_result_text(ctx, "Arguments types should be TEXT", -1, NULL);
        return;
    }

    sqlite3* db = sqlite3_context_db_handle(ctx);
    FileBaseBTree* tree = NULL;
    int dataType;
    int isFound = openTreeForTable(&tree, db, (const char*) sqlite3_value_text(argv[0]), dataType);

    if (!isFound)
    {
        sqlite3_result_text(ctx, "Table not found or table metadata is invalid", -1, NULL);
        return;
    }

    switch (dataType)
    {
        case SQLITE_INTEGER:
            setIntBytePrinter(tree);
            break;
        case SQLITE_FLOAT:
            setFloatBytePrinter(tree);
            break;
        case SQLITE_TEXT:
        case SQLITE_BLOB:
            setBytePrinter(tree);
            break;
        case SQLITE_NULL:
            setNullBytePrinter(tree);
            break;
        default:
            printf("Invalid data type\n");
            break;
    }

    if (visualize(tree, (const char*) sqlite3_value_text(argv[1])))
        sqlite3_result_text(ctx, "File written", -1, NULL);
    else
        printf("Cannot open DOT file for writing\n");
}

static void btreesModsGetTreeOrder(sqlite3_context* ctx, int argc, sqlite3_value** argv)
{
    if (argc != 1)
    {
        sqlite3_result_text(ctx, "Arguments count should be equal to 1", -1, NULL);
        return;
    }

    if (sqlite3_value_type(argv[0]) != SQLITE3_TEXT)
    {
        sqlite3_result_text(ctx, "Argument's type should be TEXT", -1, NULL);
        return;
    }

    sqlite3* db = sqlite3_context_db_handle(ctx);
    FileBaseBTree* tree = NULL;
    int dataType;
    int isFound = openTreeForTable(&tree, db, (const char*) sqlite3_value_text(argv[0]), dataType);

    if (!isFound)
    {
        sqlite3_result_text(ctx, "Table not found or table metadata is invalid or cannot open the tree file", -1, NULL);
        return;
    }

    int treeOrder = getOrder(tree);
    sqlite3_result_int(ctx, treeOrder);
}

static void btreesModsGetTreeType(sqlite3_context* ctx, int argc, sqlite3_value** argv)
{
    if (argc != 1)
    {
        sqlite3_result_text(ctx, "Arguments count should be equal to 1", -1, NULL);
        return;
    }

    if (sqlite3_value_type(argv[0]) != SQLITE3_TEXT)
    {
        sqlite3_result_text(ctx, "Argument's type should be TEXT", -1, NULL);
        return;
    }

    sqlite3* db = sqlite3_context_db_handle(ctx);

    sqlite3_str* sql = sqlite3_str_new(db);
    sqlite3_str_appendf(sql, "SELECT bestIndex FROM btrees_mods_idxinfo WHERE tableName = \"%s\";",
            sqlite3_value_text(argv[0]));
    sqlite3_stmt* stmt = NULL;
    executeSql(db, sqlite3_str_finish(sql), &stmt);

    if (sqlite3_column_type(stmt, 0) == SQLITE_NULL)
    {
        sqlite3_result_text(ctx, "Table not found or table metadata is invalid or cannot open the tree file", -1, NULL);
        return;
    }

    int bestIndex = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    sqlite3_result_int(ctx, bestIndex);
}

static int openTreeForTable(FileBaseBTree** pTree, sqlite3* db, const char* tableName, int& dataType)
{
    sqlite3_str* sql = sqlite3_str_new(db);
    sqlite3_str_appendf(sql,
            "SELECT treeFileName, bestIndex, indexDataType FROM btrees_mods_idxinfo WHERE tableName = \"%s\";",
            tableName);
    sqlite3_stmt* stmt = NULL;
    executeSql(db, sqlite3_str_finish(sql), &stmt);

    if (sqlite3_column_type(stmt, 0) == SQLITE_NULL)
    {
        sqlite3_finalize(stmt);
        return FALSE;
    }

    const char* treeFileName = (const char*) sqlite3_column_text(stmt, 0);
    int bestIndex = sqlite3_column_int(stmt, 1);
    const char* indexDataType = (const char*) sqlite3_column_text(stmt, 2);

    dataType = getIntByDataType(indexDataType);

    switch (bestIndex)
    {
        case BTREE_NUM:
            open(pTree, BaseBTree::TreeType::B_TREE, treeFileName);
            break;
        case BPLUSTREE_NUM:
            open(pTree, BaseBTree::TreeType::B_PLUS_TREE, treeFileName);
            break;
        case BSTARTREE_NUM:
            open(pTree, BaseBTree::TreeType::B_STAR_TREE, treeFileName);
            break;
        case BSTARPLUSTREE_NUM:
            open(pTree, BaseBTree::TreeType::B_STAR_PLUS_TREE, treeFileName);
            break;
        default:
            printf("Invalid index type: %d\n", bestIndex);
            sqlite3_finalize(stmt);
            return FALSE;
    }

    sqlite3_finalize(stmt);

    if (*pTree)
        return TRUE;
    else
        return FALSE;
}

#ifdef __cplusplus
extern "C" {
#endif

int sqlite3_btreesmods_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi) {
    int rc = SQLITE_OK;

    SQLITE_EXTENSION_INIT2(pApi);

    rc = sqlite3_create_function(db, "btreesModsVisualize", 2, SQLITE_UTF8, 0, btreesModsVisualize, 0, 0);

    if (rc)
        return rc;

    rc = sqlite3_create_function(db, "btreesModsGetTreeOrder", 1, SQLITE_UTF8, 0, btreesModsGetTreeOrder, 0, 0);

    if (rc)
        return rc;

    rc = sqlite3_create_function(db, "btreesModsGetTreeType", 1, SQLITE_UTF8, 0, btreesModsGetTreeType, 0, 0);

    if (rc)
        return rc;

    rc = sqlite3_create_module(db, "btrees_mods", &btreesModsModule, 0);

    return rc;
}

#ifdef __cplusplus
}; // extern "C"
#endif
