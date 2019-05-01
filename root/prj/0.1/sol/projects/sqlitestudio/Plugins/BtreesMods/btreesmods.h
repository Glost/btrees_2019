#ifndef BTREESMODS_H
#define BTREESMODS_H

#include <btreesmods_global.h>
#include "config_builder.h"
#include "plugins/genericplugin.h"
#include "plugins/generalpurposeplugin.h"
#include "plugins/uiconfiguredplugin.h"
#include <QObject>

CFG_CATEGORIES(BtreesModsConfig,
    CFG_CATEGORY(BtreesMods,
         CFG_ENTRY(QString, BTreeImageFileFormat, QString("PNG"))
    )
)

class BTREESMODSSHARED_EXPORT BtreesMods : public GenericPlugin, public GeneralPurposePlugin //, public UiConfiguredPlugin
{

    Q_OBJECT
    SQLITESTUDIO_PLUGIN("btreesmods.json")

public:

    BtreesMods();

    bool init();
    void deinit();

};

#endif // BTREESMODS_H
