/// \file
/// \brief     B-tree and modifications SQLite extension.
/// \authors   Anton Rigin
/// \version   0.1.0
/// \date      03.01.2019 -- 04.05.2019
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

using namespace btree;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
__declspec(dllexport)
#endif

int sqlite3_btreesmods_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi);

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
#define NULL_SIZE 1

#define ROWID_SIZE 8

#define ERROR_CODE -1

#define ROWID_IDX_EOF -1

#define TRUE 1
#define FALSE 0

#define TREE_ORDER 750

#define REBUILD_COEF 0.1
#define REBUILD_SPLIT_PERCENT_POINT 0.7397
#define REBUILD_COUNT 1000
#define REBUILD_MAX_COUNT 10000

/**
 * The virtual table's index params.
 */
struct indexParams {

    /**
     * The best B-tree modification based index structure number.
     */
    int bestIndex;

    /**
     * The number of the index column (the primary key column).
     */
    int indexColNumber;

    /**
     * The name of the index column (the primary key column).
     */
    char* indexColName = NULL;

    /**
     * The data type name of the index column (the primary key column).
     */
    char* indexDataType = NULL;

    /**
     * The data size of the index column (the primary key column).
     */
    int indexDataSize;

    /**
     * The index tree's file name.
     */
    char* treeFileName = NULL;
};

typedef struct indexParams indexParams;

/**
 * The index usage statistics.
 */
struct indexStats {

    /**
     * The index searches count.
     */
    int searchesCount;

    /**
     * The index inserts count.
     */
    int insertsCount;

    /**
     * The index deletes count.
     */
    int deletesCount;

    /**
     * 1 if the statistics was created when the virtual table was created, 0 otherwise.
     */
    int isOriginalStats;
};

typedef struct indexStats indexStats;

/**
 * The btrees_mods module's virtual table.
 */
struct btreesModsVirtualTable {

    /**
     * The base class. Must be first.
     */
    sqlite3_vtab base;

    /**
     * The SQLite DB connection.
     */
    sqlite3* db = NULL;

    /**
     * The virtual table name.
     */
    char* tableName = NULL;

    /**
     * The virtual table index tree.
     */
    FileBaseBTree* tree = NULL;

    /**
     * The virtual table index params.
     */
    indexParams params = {
            BPLUSTREE_NUM,
            -1,
            NULL,
            NULL,
            0,
            NULL
    };

    /**
     * The virtual table index statistics.
     */
    indexStats stats = {
            0,
            0,
            0,
            FALSE
    };
};

typedef struct btreesModsVirtualTable btreesModsVirtualTable;

/**
 * The btrees_mods module cursor containing the search results.
 */
struct btreesModsCursor {

    /**
     * The base class. Must be first.
     */
    sqlite3_vtab_cursor base;

    /**
     * The ids of the found rows.
     */
    sqlite_int64* rowsIds = NULL;

    /**
     * The current row id index in the rowsIds array.
     */
    int currentRowIdIdx;

    /**
     * The rowsIds array size.
     */
    int rowsIdsCount;
};

typedef struct btreesModsCursor btreesModsCursor;

/**
 * Creates the virtual table (after the "CREATE TABLE" query).
 *
 * @param db The SQLite DB connection.
 * @param pAux The copy of the client data pointer that was the fourth argument to the sqlite3_create_module() call
 * that registered the virtual table module.
 * @param argc The count of the arguments for the virtual table creating.
 * @param argv The arguments for the virtual table creating.
 * @param ppVTab The pointer for saving the virtual table.
 * @param pzErr The pointer for writing the error message.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsCreate(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr);

/**
 * Connects to the existing virtual table.
 *
 * @param db The SQLite DB connection.
 * @param pAux The copy of the client data pointer that was the fourth argument to the sqlite3_create_module() call
 * that registered the virtual table module.
 * @param argc The count of the arguments for the virtual table creating.
 * @param argv The arguments for the virtual table creating.
 * @param ppVTab The pointer for saving the virtual table.
 * @param pzErr The pointer for writing the error message.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsConnect(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr);

/**
 * Initializes the virtual table.
 *
 * @param db The SQLite DB connection.
 * @param pAux The copy of the client data pointer that was the fourth argument to the sqlite3_create_module() call
 * that registered the virtual table module.
 * @param argc The count of the arguments for the virtual table creating.
 * @param argv The arguments for the virtual table creating.
 * @param ppVTab The pointer for saving the virtual table.
 * @param pzErr The pointer for writing the error message.
 * @param isCreate 1 is the virtual tables is creating, 0 otherwise.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsInit(sqlite3* db, void* pAux, int argc, const char* const* argv,
        sqlite3_vtab** ppVTab, char** pzErr, int isCreate);

/**
 * Prepares the virtual table for searching.
 *
 * @param tab The virtual table instance.
 * @param pIdxInfo The virtual table index info instance.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsBestIndex(sqlite3_vtab* tab, sqlite3_index_info* pIdxInfo);

/**
 * Disconnects from the virtual table.
 *
 * @param pVtab The virtual table instance.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsDisconnect(sqlite3_vtab* pVtab);

/**
 * Destroys the virtual table (after the "DROP TABLE" query).
 *
 * @param pVtab The virtual table instance.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsDestroy(sqlite3_vtab* pVtab);

/**
 * Searches for rows in the table.
 *
 * @param cursor The SQLite virtual table cursor instance.
 * @param idxNum The best index structure number.
 * @param idxStr The idxStr string prepared by btreesModsBestIndex();
 * @param argc The count of the constraint values for searching.
 * @param argv The constraint values for searching.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsFilter(sqlite3_vtab_cursor* cursor, int idxNum, const char* idxStr,
        int argc, sqlite3_value** argv);

/**
 * Searches for the next row matching the search constraints.
 *
 * @param cursor The SQLite virtual table cursor instance.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsNext(sqlite3_vtab_cursor* cursor);

/**
 * Returns 1 if no more rows match the search constraints, 0 otherwise.
 *
 * @param cursor The SQLite virtual table cursor instance.
 * @return 1 if no more rows match the search constraints, 0 otherwise.
 */
static int btreesModsEof(sqlite3_vtab_cursor* cursor);

/**
 * Extracts the @param n-th column from the found row.
 *
 * @param cursor The SQLite virtual table cursor instance.
 * @param context The SQLite context instance for saving the cell value.
 * @param n The number of column to be extracted from the row.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsColumn(sqlite3_vtab_cursor* cursor, sqlite3_context* context, int n);

/**
 * Handles the search constraint.
 *
 * @param cursor The btrees_mods module cursor instance.
 * @param columnNum The constraint's column number.
 * @param operation The constraint's operation.
 * @param exprValue The constraint's right side expression value.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsHandleConstraint(btreesModsCursor* cursor, int columnNum, unsigned char operation,
        sqlite3_value* exprValue);

/**
 * Handles the equality search constraint.
 *
 * @param cursor The btrees_mods module cursor instance.
 * @param exprValue The constraint's right side expression value.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsHandleConstraintEq(btreesModsCursor* cursor, sqlite3_value* exprValue);

/**
 * Updates the row in the virtual table (inserts, updates or deletes the row).
 *
 * @param pVTab The virtual table instance.
 * @param argc The count of the arguments for updating.
 * @param argv The arguments for updating.
 * @param pRowid The pointer for saving the row id of the updated row.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsUpdate(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid);

/**
 * Updates the row in the virtual table (after the "UPDATE" query).
 *
 * @param pVTab The virtual table instance.
 * @param argc The count of the arguments for updating.
 * @param argv The arguments for updating.
 * @param pRowid The pointer for saving the row id of the updated row.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsDoUpdate(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid);

/**
 * Deletes the row from the virtual table (after the "DELETE FROM" query).
 *
 * @param pVTab The virtual table instance.
 * @param primaryKeyValue The primary key value of the deleted row.
 * @param pRowid The pointer for saving the row id of the deleted row.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsDelete(sqlite3_vtab* pVTab, sqlite3_value* primaryKeyValue, sqlite_int64* pRowid);

/**
 * Inserts the row into the virtual table (after the "INSERT INTO" query).
 *
 * @param pVTab The virtual table instance.
 * @param argc The count of the arguments for inserting.
 * @param argv The arguments for inserting.
 * @param pRowid The pointer for saving the row id of the inserted row.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsInsert(sqlite3_vtab* pVTab, int argc, sqlite3_value** argv, sqlite_int64* pRowid);

/**
 * Opens the cursor for the searching in the virtual table.
 *
 * @param pVTab The virtual table instance.
 * @param ppCursor The pointer for saving the opened cursor.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsOpen(sqlite3_vtab* pVTab, sqlite3_vtab_cursor** ppCursor);

/**
 * The closes the cursor after the searching in the virtual table.
 *
 * @param pCur The cursor for closing.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsClose(sqlite3_vtab_cursor* pCur);

/**
 * Determines the row id of the row pointed by the cursor.
 *
 * @param pCur The cursor instance.
 * @param pRowid The pointer for saving the determined row id.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsRowid(sqlite3_vtab_cursor *pCur, sqlite_int64 *pRowid);

/**
 * Renames the virtual table.
 *
 * @param pVtab The virtual table instance.
 * @param zNew The new name for the virtual table.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int btreesModsRename(sqlite3_vtab* pVtab, const char* zNew);

/**
 * Generates the tree file name.
 *
 * @param treeFileName The pointer for saving the generated tree file name.
 * @return The generated tree file name.
 */
static char* getTreeFileName(char* treeFileName);

/**
 * Creates the B-tree modification based index for the virtual table.
 *
 * @param virtualTable The virtual table instance.
 * @param order The tree order.
 * @param keySize The tree key size.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int createIndex(btreesModsVirtualTable* virtualTable, int order, int keySize);

/**
 * Creates the virtual table's B-tree modification based index.
 *
 * @param virtualTable The virtual table instance.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int openIndex(btreesModsVirtualTable* virtualTable);

/**
 * Registers the virtual table's index column (the primary key column).
 *
 * @param db The SQLite DB connection.
 * @param stmt The SQLite SELECT statement performed on the real table representation of the virtual table.
 * @param virtualTable The virtual table instance.
 * @param tableName The virtual table name.
 * @param treeFileName The index tree file name.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int registerIndexColumn(sqlite3* db, sqlite3_stmt* stmt, btreesModsVirtualTable* virtualTable,
        const char* tableName, const char* treeFileName);

/**
 * Converts the data type name to the data size.
 *
 * @param dataType The data type name.
 * @return The data size.
 */
static int getDataSizeByType(const char* dataType);

/**
 * Converts the data type number to the data type name.
 *
 * @param dataType The data type number.
 * @return The data type name.
 */
static const char* getDataTypeByInt(int dataType);

/**
 * Converts the data type name to the data type number.
 *
 * @param dataType The data type name.
 * @return The data type number.
 */
static int getIntByDataType(const char* dataType);

/**
 * Gets the row id for the given primary key value.
 *
 * @param virtualTable The virtual table instance.
 * @param primaryKeyValue The primary key value.
 * @return The row id.
 */
static sqlite3_int64 getRowId(btreesModsVirtualTable* virtualTable, sqlite3_value* primaryKeyValue);

/**
 * Executes the SQL query and finalizes its SQLite statement.
 *
 * @param db The SQLite DB connection.
 * @param sql The SQL query.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int executeSqlAndFinalize(sqlite3* db, char* sql);

/**
 * Executes the SQL query.
 *
 * @param db The SQLite DB connection.
 * @param sql The SQL query.
 * @param stmt The pointer for saving the query's SQLite statement.
 * @return SQLITE_OK if successful, SQLite error code otherwise.
 */
static int executeSql(sqlite3* db, char* sql, sqlite3_stmt** stmt);

/**
 * Creates the tree key for the given primary key value.
 *
 * @param primaryKeyValue The primary key value.
 * @param virtualTable The virtual table instance.
 * @return The tree key.
 */
static Byte* createTreeKey(sqlite3_value* primaryKeyValue, btreesModsVirtualTable* virtualTable);

/**
 * Creates the tree key for the given row id.
 *
 * @param rowId The row id.
 * @param virtualTable The virtual table instance.
 * @return The tree key.
 */
static Byte* createTreeKey(sqlite_int64 rowId, btreesModsVirtualTable* virtualTable);

/**
 * Creates the tree key for the given primary key value and row id.
 *
 * @param primaryKeyValue The primary key value.
 * @param rowId The row id.
 * @param virtualTable The virtual table instance.
 * @return The tree key.
 */
static Byte* createTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId, btreesModsVirtualTable* virtualTable);

/**
 * Creates the integer tree key for the given primary key value and row id.
 *
 * @param primaryKeyValue The primary key value.
 * @param rowId The row id.
 * @return The tree key.
 */
static Byte* createIntTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId);

/**
 * Creates the float tree key for the given primary key value and row id.
 *
 * @param primaryKeyValue The primary key value.
 * @param rowId The row id.
 * @return The tree key.
 */
static Byte* createFloatTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId);

/**
 * Creates the text tree key for the given primary key value and row id.
 *
 * @param primaryKeyValue The primary key value.
 * @param rowId The row id.
 * @return The tree key.
 */
static Byte* createTextTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId);

/**
 * Creates the blob tree key for the given primary key value and row id.
 *
 * @param primaryKeyValue The primary key value.
 * @param rowId The row id.
 * @return The tree key.
 */
static Byte* createBlobTreeKey(sqlite3_value* primaryKeyValue, sqlite_int64 rowId);

/**
 * Converts the SQLite value to the string.
 *
 * @param value The SQLite value.
 * @param pString The pointer for saving the converted string.
 */
static void convertSqliteValueToString(sqlite3_value* value, char** pString);

/**
 * Converts the text SQLite value to the string.
 *
 * @param value The SQLite value.
 * @param pString The pointer for saving the converted string.
 */
static void convertSqliteTextValueToString(sqlite3_value* value, char** pString);

/**
 * Copies the string from the source to the destination.
 * Frees the destination string's memory if necessary and allocates the memory for it.
 *
 * @param pDestination The pointer to the destination string.
 * @param source The source string.
 */
static void copyString(char** pDestination, const char* source);

/**
 * Frees the given string's memory.
 *
 * @param pString The given string.
 */
static void freeString(char** pString);

/**
 * Clears the virtual table's index params and frees their memory.
 *
 * @param pVTab The virtual table instance.
 */
static void freeParams(sqlite3_vtab* pVTab);

/**
 * Rebuilds the tree index if the special conditions are matched.
 *
 * @param virtualTable The virtual table instance.
 */
static void rebuildIndexIfNecessary(btreesModsVirtualTable* virtualTable);

/**
 * Rebuilds the tree index.
 *
 * @param virtualTable The virtual table instance.
 */
static void rebuildIndex(btreesModsVirtualTable* virtualTable);

/**
 * Measures max memory usage during the last query execution.
 */
static void measureMaxMemoryUsage();

/**
 * Visualizes the B-tree or its modification used in the virtual table to the GraphViz DOT file.
 *
 * @param ctx Context for writing the results.
 * @param argc The arguments count.
 * @param argv The arguments.
 */
static void btreesModsVisualize(sqlite3_context* ctx, int argc, sqlite3_value** argv);

/**
 * Gets the order for the B-tree or its modification used in the virtual table.
 *
 * @param ctx Context for writing the results.
 * @param argc The arguments count.
 * @param argv The arguments.
 */
static void btreesModsGetTreeOrder(sqlite3_context* ctx, int argc, sqlite3_value** argv);

/**
 * Gets the type of the B-tree or its modification used in the virtual table.
 *
 * @param ctx Context for writing the results.
 * @param argc The arguments count.
 * @param argv The arguments.
 */
static void btreesModsGetTreeType(sqlite3_context* ctx, int argc, sqlite3_value** argv);

/**
 * Opens the tree for the given virtual table.
 *
 * @param pTree The pointer to the tree.
 * @param db The SQLite DB connection.
 * @param tableName The virtual table name.
 * @return 1 if the tree is successfully opened, false otherwise.
 */
static int openTreeForTable(FileBaseBTree** pTree, sqlite3* db, const char* tableName, int& dataType);

/**
 * The btrees_mods SQLite module.
 */
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
