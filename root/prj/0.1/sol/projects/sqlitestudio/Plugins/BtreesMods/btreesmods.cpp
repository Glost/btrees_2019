#include "btreesmods.h"
#include "btreesmodsdialog.h"
#include "dataview.h"
#include "common/extactionprototype.h"
#include "mainwindow.h"

BtreesMods::BtreesMods()
{
}

bool BtreesMods::init()
{
//    Q_INIT_RESOURCE(btreesmods);

    ExtActionPrototype* visualizeBTreeAction = new ExtActionPrototype(tr("Visualize B-tree"), this);
    ExtActionPrototype* separatorAction = new ExtActionPrototype(this);

    connect(visualizeBTreeAction, SIGNAL(triggered(ExtActionContainer*,int)), this, SLOT(bTreeVisualizeRequested(ExtActionContainer*)));

    DataView::insertActionAfter(visualizeBTreeAction, DataView::LAST_PAGE);
    DataView::insertActionAfter(separatorAction, DataView::LAST_PAGE);

    return true;
}

void BtreesMods::deinit()
{
//    Q_CLEANUP_RESOURCE(btreesmods);
}

void BtreesMods::bTreeVisualizeRequested(ExtActionContainer* actionContainer)
{
    BtreesModsDialog* dialog = new BtreesModsDialog();
    dialog->show();
}
