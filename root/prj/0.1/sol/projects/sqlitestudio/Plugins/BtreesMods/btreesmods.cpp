#include "btreesmods.h"
#include "dataview.h"

BtreesMods::BtreesMods()
{
}

bool BtreesMods::init()
{
//    Q_INIT_RESOURCE(btreesmods);

    ExtActionPrototype* visualizeBTreeAction = new ExtActionPrototype(tr("Visualize B-tree"), this);
    ExtActionPrototype* separatorAction = new ExtActionPrototype(this);

    DataView::insertActionAfter(visualizeBTreeAction, DataView::LAST_PAGE);
    DataView::insertActionAfter(separatorAction, DataView::LAST_PAGE);

    return true;
}

void BtreesMods::deinit()
{
//    Q_CLEANUP_RESOURCE(btreesmods);
}
