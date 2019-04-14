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

    pIdxInfo->idxStr = (char*) sqlite3_malloc(2 * pIdxInfo->nConstraint);

    int i = 0;
    for ( ; i < pIdxInfo->nConstraint; ++i)
    {
        pIdxInfo->aConstraintUsage[i].argvIndex = i + 1;
        pIdxInfo->aConstraintUsage[i].omit = pIdxInfo->aConstraint[i].usable;

        pIdxInfo->idxStr[2 * i] = pIdxInfo->aConstraint[i].iColumn;
        pIdxInfo->idxStr[2 * i + 1] = pIdxInfo->aConstraint[i].op;
    }

    pIdxInfo->needToFreeIdxStr = 1;

    return rc;
}

static int btreesModsDisconnect(sqlite3_vtab* pVtab)
{
    close();
    freeParams(params);
    sqlite3_free(pVtab);
    return SQLITE_OK;
}

static int btreesModsDestroy(sqlite3_vtab* pVtab)
{
    close();
    freeParams(params);
    sqlite3_free(pVtab);
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

    if (customCursor->currentRowIdIdx == -1)
        return rc;

    if (customCursor->currentRowIdIdx == customCursor->rowsIdsCount - 1)
    {
        customCursor->currentRowIdIdx = -1;
        return rc;
    }

    ++customCursor->currentRowIdIdx;

    return rc;
}

static int btreesModsEof(sqlite3_vtab_cursor* cursor)
{
    btreesModsCursor* customCursor = (btreesModsCursor*) cursor;
    return customCursor->currentRowIdIdx == -1;
}

static int btreesModsColumn(sqlite3_vtab_cursor* cursor, sqlite3_context* context, int n)
{
    int rc = SQLITE_OK;

    btreesModsCursor* customCursor = (btreesModsCursor*) cursor;
    virtualTableInfo* vTInfo = (virtualTableInfo*) cursor->pVtab;

    sqlite3_str* pSql = sqlite3_str_new(vTInfo->db);
    sqlite3_str_appendf(pSql, "SELECT * FROM %s_real WHERE rowid = %d;", vTInfo->tableName,
            customCursor->rowsIds[customCursor->currentRowIdIdx]);
    sqlite3_stmt* stmt = NULL;

    rc = executeSql(vTInfo->db, sqlite3_str_finish(pSql), &stmt);

    if (rc)
        return rc;

    sqlite3_result_value(context, sqlite3_column_value(stmt, n));

    rc = sqlite3_finalize(stmt);

    return rc;
}

static int btreesModsHandleConstraint(btreesModsCursor* cursor, int columnNum, unsigned char operation,
        sqlite3_value* exprValue)
{
    if (columnNum == params.indexColNumber)
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
    Byte* searchedKey = createTreeKey(exprValue);

    firstPartByteComparator.firstPartBytes = params.indexDataSize;
    setFirstPartByteComparator();

    Byte** keys = NULL;
    int keysCount = searchAll(searchedKey, &keys);

    cursor->rowsIdsCount = keysCount;

    if (keysCount == 0)
        return SQLITE_OK;

    cursor->rowsIds = (sqlite_int64*) malloc(keysCount * sizeof(sqlite_int64));

    for (int i = 0; i < keysCount; ++i)
    {
        memcpy(&cursor->rowsIds[i], &keys[i][params.indexDataSize], sizeof(sqlite_int64));
        free(keys[i]);
    }

    cursor->currentRowIdIdx = 0;

    free(keys);

    return SQLITE_OK;
}

static int btreesModsUpdate(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid)
{
    if (argc == 1 && sqlite3_value_type(argv[0]) != SQLITE_NULL)
        return btreesModsDelete(pVTab, sqlite3_value_int64(argv[0]));
    else if (argc > 1 && sqlite3_value_type(argv[0]) == SQLITE_NULL)
        return btreesModsInsert(pVTab, argc, argv, pRowid);
    else if (argc > 1 && sqlite3_value_type(argv[0]) != SQLITE_NULL &&
        sqlite3_value_int64(argv[0]) == sqlite3_value_int64(argv[1]))
        return btreesModsDoUpdate(pVTab, argc, argv, pRowid);
    else if (argc > 1 && sqlite3_value_type(argv[0]) != SQLITE_NULL &&
        sqlite3_value_int64(argv[0]) != sqlite3_value_int64(argv[1]))
        return btreesModsDoUpdateWithRowIdChange(pVTab, argc, argv, pRowid);
    else
        return -1;
}

static int btreesModsDoUpdate(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid)
{

}

static int btreesModsDoUpdateWithRowIdChange(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid)
{

}

static int btreesModsDelete(sqlite3_vtab* pVTab, sqlite_int64 rowId)
{
    int rc = SQLITE_OK;
    virtualTableInfo* vTInfo = (virtualTableInfo*) pVTab;
    sqlite3_str* pSql = sqlite3_str_new(vTInfo->db);
    sqlite3_str_appendf(pSql, "DELETE FROM %s_real WHERE rowid = %d;", vTInfo->tableName, rowId);
    rc = executeSqlAndFinalize(vTInfo->db, sqlite3_str_finish(pSql));

    if (rc)
        return rc;

    secondPartByteComparator.firstPartBytes = params.indexDataSize;
    setSecondPartByteComparator();

    rc = (int) !removeKey(createTreeKey(rowId));

    setFirstPartByteComparator();

    return rc;
}

static int btreesModsInsert(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid)
{
    virtualTableInfo* vTInfo = (virtualTableInfo*) pVTab;
    sqlite3_str* pSql = sqlite3_str_new(vTInfo->db);
    char* strValue = NULL;
    convertSqliteValueToString(argv[2], &strValue);
    sqlite3_str_appendf(pSql, "INSERT INTO %s_real VALUES (%s", vTInfo->tableName, strValue);

    int i = 3;
    for ( ; i < argc; ++i)
    {
        convertSqliteValueToString(argv[i], &strValue);
        sqlite3_str_appendf(pSql, ", %s", strValue);
    }

    sqlite3_str_appendf(pSql, ");");

    int rc = executeSqlAndFinalize(vTInfo->db, sqlite3_str_finish(pSql));

    if (rc)
        return rc;

    sqlite3_str* selectSql = sqlite3_str_new(vTInfo->db);
    convertSqliteValueToString(argv[params.indexColNumber + 2], &strValue);
    sqlite3_str_appendf(selectSql, "SELECT rowid FROM %s_real WHERE %s = %s;", vTInfo->tableName, params.indexColName,
            strValue);
    free(strValue);
    sqlite3_stmt* stmt = NULL;
    rc = executeSql(vTInfo->db, sqlite3_str_finish(selectSql), &stmt);

    if (rc)
        return rc;

    *pRowid = sqlite3_column_int64(stmt, 0);

    rc = sqlite3_finalize(stmt);

    insert(createTreeKey(argv[params.indexColNumber + 2], *pRowid));

    return rc;
}

static int btreesModsOpen(sqlite3_vtab* pVTab, sqlite3_vtab_cursor** ppCursor)
{
    btreesModsCursor* cursor = (btreesModsCursor*) sqlite3_malloc(sizeof(btreesModsCursor));
    memset(cursor, 0, sizeof(btreesModsCursor));
    cursor->base.pVtab = pVTab;
    cursor->currentRowIdIdx = -1;
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

    if (cursor->currentRowIdIdx == -1)
        pRowid = NULL;
    else
        *pRowid = cursor->rowsIds[cursor->currentRowIdIdx];

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

    executeSqlAndFinalize(db, zSql);

    if (isCreate)
    {
        sqlite3_str* selectSql = sqlite3_str_new(db);
        sqlite3_str_appendf(selectSql, "SELECT * FROM %s_real;", argv[2]);
        char* selectZSql = sqlite3_str_finish(selectSql);
        sqlite3_stmt* selectStmt = NULL;
        rc = sqlite3_prepare_v2(db, selectZSql, -1, &selectStmt, 0);
        *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));

        if (rc)
            return rc;

        char treeFileName[100];
        char* filledTreeFileName = getTreeFileName(treeFileName);
        rc = registerIndexColumn(db, selectStmt, argv[2], filledTreeFileName);

        if (rc)
            return rc;

        rc = createIndex(2, params.indexDataSize + sizeof(sqlite_int64));

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

        params.bestIndex = sqlite3_column_int(getParamsStmt, colNum++);
        params.indexColNumber = sqlite3_column_int(getParamsStmt, colNum++);
        copyString(&params.indexColName, (char*) sqlite3_column_text(getParamsStmt, colNum++));
        copyString(&params.indexDataType, (char*) sqlite3_column_text(getParamsStmt, colNum++));
        params.indexDataSize = sqlite3_column_int(getParamsStmt, colNum++);
        copyString(&params.treeFileName, (char*) sqlite3_column_text(getParamsStmt, colNum++));

        rc = openIndex();

        if (rc)
            return rc;

        sqlite3_finalize(getParamsStmt);
    }

    rc = sqlite3_declare_vtab(db, zSql);

    virtualTableInfo* vTInfo = (virtualTableInfo*) sqlite3_malloc(sizeof(virtualTableInfo));
    vTInfo->base.pModule = &btreesModsModule;
    vTInfo->base.nRef = 1;
    vTInfo->base.zErrMsg = NULL;
    vTInfo->db = db;
    vTInfo->tableName = argv[2];
    *ppVTab = (sqlite3_vtab*) vTInfo;

    sqlite3_free(zSql);

    return rc;
}

static char* getTreeFileName(char* treeFileName)
{
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

    return treeFileName;
}

static int createIndex(int order, int keySize)
{
    switch (params.bestIndex)
    {
        case 1: create(BaseBTree::TreeType::B_TREE, order, keySize, params.treeFileName); break;
        case 2: create(BaseBTree::TreeType::B_PLUS_TREE, order, keySize, params.treeFileName); break;
        case 3: create(BaseBTree::TreeType::B_STAR_TREE, order, keySize, params.treeFileName); break;
        case 4: create(BaseBTree::TreeType::B_STAR_PLUS_TREE, order, keySize, params.treeFileName); break;
        default: return -1;
    }

    return 0;
}

static int openIndex()
{
    switch (params.bestIndex)
    {
        case 1: open(BaseBTree::TreeType::B_TREE, params.treeFileName); break;
        case 2: open(BaseBTree::TreeType::B_PLUS_TREE, params.treeFileName); break;
        case 3: open(BaseBTree::TreeType::B_STAR_TREE, params.treeFileName); break;
        case 4: open(BaseBTree::TreeType::B_STAR_PLUS_TREE, params.treeFileName); break;
        default: return -1;
    }

    return 0;
}

static int registerIndexColumn(sqlite3* db, sqlite3_stmt* stmt, const char* tableName, const char* treeFileName)
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

    params.indexColNumber = -1;
    copyString(&params.indexColName, "rowid");
    copyString(&params.indexDataType, "INTEGER");
    params.indexDataSize = getDataSizeByType(params.indexDataType);
    params.bestIndex = 1;
    copyString(&params.treeFileName, treeFileName);

    int i = 0;
    for ( ; (columnName = sqlite3_column_name(stmt, i)) != NULL; ++i)
    {
        rc = sqlite3_table_column_metadata(db, NULL, sqlite3_mprintf("%s_real", tableName), columnName,
                &dataType, &collSeq, &notNull, &primaryKey, &autoInc);

        if (rc)
            return rc;

        if (primaryKey)
        {
            params.indexColNumber = i;
            copyString(&params.indexColName, columnName);
            copyString(&params.indexDataType, dataType);
            params.indexDataSize = getDataSizeByType(dataType);

            break;
        }
    }

    sqlite3_str* insertSql = sqlite3_str_new(db);
    sqlite3_str_appendf(insertSql, "INSERT INTO btrees_mods_idxinfo "
                                   "VALUES (\"%s\", %d, %d, \"%s\", \"%s\", %d, \"%s\");",
                        tableName, params.bestIndex, params.indexColNumber, params.indexColName,
                        params.indexDataType, params.indexDataSize, params.treeFileName);
    rc = executeSqlAndFinalize(db, sqlite3_str_finish(insertSql));

    return rc;
}

static int getDataSizeByType(const char* dataType)
{
    if (strcmp(dataType, "INTEGER") == 0)
        return 4;
    else if (strcmp(dataType, "FLOAT") == 0)
        return 8;
    else if (strcmp(dataType, "TEXT") == 0)
        return 8;
    else if (strcmp(dataType, "BLOB") == 0)
        return 8;
    else if (strcmp(dataType, "NULL") == 0)
        return 1;
    else
        return -1;
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

static Byte* createTreeKey(sqlite3_value* primaryKeyValue)
{
    return createTreeKey(primaryKeyValue, 0);
}

static Byte* createTreeKey(sqlite_int64 rowId)
{
    return createTreeKey(NULL, rowId);
}

static Byte* createTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId)
{
    if (primaryKeyValue == NULL)
    {
        Byte* treeKey = (Byte*) malloc(params.indexDataSize + sizeof(sqlite_int64));

        memcpy(treeKey, (Byte*) &rowId, params.indexDataSize);
        memcpy(&treeKey[params.indexDataSize], (Byte*) &rowId, sizeof(sqlite_int64));

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
    Byte* treeKey = (Byte*) malloc(4 + sizeof(sqlite_int64));

    int value = sqlite3_value_int(primaryKeyValue);
    memcpy(treeKey, (Byte*) &value, 4);
    memcpy(&treeKey[4], (Byte*) &rowId, sizeof(sqlite_int64));

    return treeKey;
}

static Byte* createFloatTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId)
{
    Byte* treeKey = (Byte*) malloc(8 + sizeof(sqlite_int64));

    double value = sqlite3_value_double(primaryKeyValue);
    memcpy(treeKey, (Byte*) &value, 8);
    memcpy(&treeKey[8], (Byte*) &rowId, sizeof(sqlite_int64));

    return treeKey;
}

static Byte* createTextTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId)
{
    int size = sqlite3_value_bytes(primaryKeyValue);
    size = size >= 8 ? 8 : size;

    Byte* treeKey = (Byte*) malloc(8 + sizeof(sqlite_int64));
    memset(treeKey, 0, 8 + sizeof(sqlite_int64));

    memcpy(treeKey, (Byte*) sqlite3_value_text(primaryKeyValue), size);
    memcpy(&treeKey[8], (Byte*) &rowId, sizeof(sqlite_int64));

    return treeKey;
}

static Byte* createBlobTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId)
{
    int size = sqlite3_value_bytes(primaryKeyValue);
    size = size >= 8 ? 8 : size;

    Byte* treeKey = (Byte*) malloc(8 + sizeof(sqlite_int64));
    memset(treeKey, 0, 8 + sizeof(sqlite_int64));

    memcpy(treeKey, (Byte*) sqlite3_value_blob(primaryKeyValue), size);
    memcpy(&treeKey[8], (Byte*) &rowId, sizeof(sqlite_int64));

    return treeKey;
}

static void convertSqliteValueToString(sqlite3_value* value, char** pString)
{
    if (*pString)
    {
        free(*pString);
        *pString = NULL;
    }

    *pString = (char*) malloc(256);

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
    snprintf(*pString, 255, "\"%s", (const char*) sqlite3_value_text(value));
    strcat(*pString, "\"");
}

static const char* copyString(char** pDestination, const char* source)
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

static void freeParams(indexParams& paramsInstance)
{
    free(paramsInstance.indexColName);
    paramsInstance.indexColName = NULL;
    free(paramsInstance.indexDataType);
    paramsInstance.indexDataType = NULL;
    free(paramsInstance.treeFileName);
    paramsInstance.treeFileName = NULL;
}

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
