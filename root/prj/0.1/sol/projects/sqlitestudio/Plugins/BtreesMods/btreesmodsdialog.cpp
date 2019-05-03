#include "btreesmodsdialog.h"
#include "ui_btreesmodsdialog.h"

BtreesModsDialog::BtreesModsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BtreesModsDialog)
{
    ui->setupUi(this);
}

BtreesModsDialog::~BtreesModsDialog()
{
    delete ui;
}
