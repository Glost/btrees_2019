#ifndef BTREESMODSDIALOG_H
#define BTREESMODSDIALOG_H

#include <QDialog>

namespace Ui {
class BtreesModsDialog;
}

class BtreesModsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BtreesModsDialog(QWidget *parent = nullptr);
    ~BtreesModsDialog();

private:
    Ui::BtreesModsDialog *ui;
};

#endif // BTREESMODSDIALOG_H
