#ifndef STARTNODEDIALOG_H
#define STARTNODEDIALOG_H

#include <QDialog>
#include "ui_startnodedialog.h"

namespace Ui {
class StartNodeDialog;
}

class StartNodeDialog : public QDialog, public Ui::StartNodeDialog
{
    Q_OBJECT

public:
    explicit StartNodeDialog(QWidget *parent = nullptr);
    ~StartNodeDialog();

private Q_SLOTS:
    void on_btnLaunch_clicked();
};

#endif // STARTNODEDIALOG_H
