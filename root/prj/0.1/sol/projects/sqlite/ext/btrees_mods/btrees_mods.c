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

static btreesModsDisconnect(sqlite3_vtab* pVtab)
{
    sqlite3_free(pVtab);
    return SQLITE_OK;
}

static sqlite3_module btreesModsModule = {
        0,
        0//, // TODO: complete the module struct instance.
};

int sqlite3BtreesModsInit(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
    int rc = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);

    rc = sqlite3_create_module(db, "btrees_mods", &btreesModsModule, 0);

    return rc;
}
