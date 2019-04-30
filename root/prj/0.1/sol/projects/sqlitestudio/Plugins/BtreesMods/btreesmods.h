#ifndef BTREESMODS_H
#define BTREESMODS_H

#include <btreesmods_global.h>
#include "config_builder.h"
#include "plugins/genericplugin.h"
#include "plugins/uiconfiguredplugin.h"
#include <QObject>

class BTREESMODSSHARED_EXPORT BtreesMods : public GenericPlugin, public UiConfiguredPlugin
{

    Q_OBJECT
    SQLITESTUDIO_PLUGIN("btreesmods.json")

public:
    BtreesMods();
};

#endif // BTREESMODS_H
