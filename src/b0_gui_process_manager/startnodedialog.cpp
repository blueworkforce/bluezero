#include "startnodedialog.h"

#include <QMessageBox>
#include <QDebug>

StartNodeDialog::StartNodeDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
}

StartNodeDialog::~StartNodeDialog()
{
}

void StartNodeDialog::on_btnLaunch_clicked()
{
    QString s(editArguments->toPlainText());
    QString cur;
    QStringList args;
    bool inQuotes = false;
    bool escaping = false;
    bool curEmpty = true;
    for(int i = 0; i < s.length(); i++)
    {
        QChar c(s[i]);
        if(escaping)
        {
            if(c == 'n') cur += '\n';
            else if(c == 'r') cur += '\r';
            else if(c == 't') cur += '\t';
            else cur += c;
            escaping = false;
        }
        else if(c == '"')
        {
            inQuotes = !inQuotes;
            curEmpty = false;
        }
        else if(c == '\\')
        {
            escaping = true;
        }
        else if(c == ' ' && !inQuotes)
        {
            args << cur;
            cur = "";
            curEmpty = true;
        }
        else
        {
            cur += c;
        }
    }
    if(!cur.isEmpty() || !curEmpty)
    {
        args << cur;
    }
    qDebug() << comboHost->currentText() << editProgram->text() << args;
}
