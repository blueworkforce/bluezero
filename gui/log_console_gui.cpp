#include <iostream>
#include <string>
#include <vector>

#include <b0/node.h>
#include <b0/subscriber.h>
#include <b0/message/log_entry.h>

#include <QRegExp>
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QAction>
#include <QClipboard>

class LogConsoleWindow : public QMainWindow
{
public:
    LogConsoleWindow(b0::Node &node)
        : QMainWindow(),
          node_(node)
    {
        setWindowTitle("BlueZero log console");

        QWidget *filterToolBar = new QWidget;
        {
            QHBoxLayout *layout = new QHBoxLayout;
            comboLevel = new QComboBox;
            layout->addWidget(new QLabel("Level:"));
            layout->addWidget(comboLevel);
            textNode = new QLineEdit;
            layout->addWidget(new QLabel("Node(s):"));
            layout->addWidget(textNode);
            filterToolBar->setLayout(layout);
        }

        QWidget *centralWidget = new QWidget;
        setCentralWidget(centralWidget);
        {
            QVBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(filterToolBar);
            tableWidget = new QTableWidget;
            tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
            tableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
            tableWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
            QAction *action1 = new QAction("Copy selected entries", this);
            connect(action1, &QAction::triggered, this, &LogConsoleWindow::copySelectedEntries);
            tableWidget->insertAction(0, action1);
            layout->addWidget(tableWidget);
            centralWidget->setLayout(layout);
        }

        comboLevel->addItem("TRACE", b0::message::LogLevel::trace);
        comboLevel->addItem("DEBUG", b0::message::LogLevel::debug);
        comboLevel->addItem("INFO",  b0::message::LogLevel::info);
        comboLevel->addItem("WARN",  b0::message::LogLevel::warn);
        comboLevel->addItem("ERROR", b0::message::LogLevel::error);
        comboLevel->addItem("FATAL", b0::message::LogLevel::fatal);
        comboLevel->setCurrentIndex(0);
        connect(comboLevel, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LogConsoleWindow::comboLevelChanged);
        connect(textNode, &QLineEdit::textChanged, this, &LogConsoleWindow::textNodeChanged);

        QStringList labels;
        labels << "Time" << "Node" << "Level" << "Message";
        tableWidget->setColumnCount(labels.size());
        tableWidget->horizontalHeader()->setStretchLastSection(true);
        tableWidget->verticalHeader()->hide();
        tableWidget->setHorizontalHeaderLabels(labels);

        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [this](){this->node_.spinOnce();});
        timer->start(100);
    }

    void copySelectedEntries()
    {
        QModelIndexList selection = tableWidget->selectionModel()->selectedRows();
        QString s;

        foreach(QTableWidgetSelectionRange range, tableWidget->selectedRanges())
        {
            for(int row = range.topRow(); row <= range.bottomRow(); row++)
            {
                if(s.length()) s += "\n";
                s += tableWidget->item(row, 0)->text();
                s += " [";
                s += tableWidget->item(row, 1)->text();
                s += "] ";
                s += tableWidget->item(row, 2)->text();
                s += ": ";
                s += tableWidget->item(row, 3)->text();
            }
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(s);
    }

    void onLogEntry(const b0::message::LogEntry &entry)
    {
        all_entries_.push_back(entry);
        if(!filter(entry))
            addEntry(entry);
    }

    void addEntry(const b0::message::LogEntry &entry)
    {
        int n = tableWidget->rowCount();
        tableWidget->setRowCount(n + 1);
        tableWidget->setItem(n, 0, new QTableWidgetItem(QString::number(node_.timeUSec())));
        tableWidget->setItem(n, 1, new QTableWidgetItem(QString::fromStdString(entry.node_name)));
        tableWidget->setItem(n, 2, new QTableWidgetItem(levelStr(entry.level)));
        tableWidget->setItem(n, 3, new QTableWidgetItem(QString::fromStdString(entry.message)));
    }

    QString levelStr(b0::message::LogLevel level)
    {
        switch(level)
        {
        case b0::message::LogLevel::trace: return "TRACE";
        case b0::message::LogLevel::debug: return "DEBUG";
        case b0::message::LogLevel::info:  return "INFO";
        case b0::message::LogLevel::warn:  return "WARN";
        case b0::message::LogLevel::error: return "ERROR";
        case b0::message::LogLevel::fatal: return "FATAL";
        default: return "UNKNOWN";
        }
    }

    void comboLevelChanged(int newIndex)
    {
        filterLevel = (b0::message::LogLevel)comboLevel->currentData().toInt();
        refilter();
    }

    void textNodeChanged(const QString &txt)
    {
        QStringList words = textNode->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
        filterNodeNames.clear();
        foreach(QString s, words)
            filterNodeNames.push_back(s.toStdString());
        refilter();
    }

    bool filter(const b0::message::LogEntry &entry)
    {
        if(entry.level < filterLevel) return true;

        if(filterNodeNames.empty()) return false;
        else
        {
            for(std::string s : filterNodeNames)
                if(entry.node_name.find(s) != std::string::npos) return false;
            return true;
        }
    }

    void refilter()
    {
        tableWidget->setRowCount(0);
        for(b0::message::LogEntry &entry : all_entries_)
            if(!filter(entry))
                addEntry(entry);
    }

private:
    b0::Node &node_;
    QTableWidget *tableWidget;
    QComboBox *comboLevel;
    QLineEdit *textNode;
    std::vector<b0::message::LogEntry> all_entries_;
    std::vector<std::string> filterNodeNames;
    b0::message::LogLevel filterLevel = b0::message::LogLevel::trace;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    b0::Node logConsoleNode("log_console_gui");

    LogConsoleWindow logConsoleWindow(logConsoleNode);

    b0::Subscriber logSub(&logConsoleNode, "log", &LogConsoleWindow::onLogEntry, &logConsoleWindow);

    logConsoleNode.init();

    logConsoleWindow.resize(800, 340);
    logConsoleWindow.show();

    int ret = app.exec();

    logConsoleNode.cleanup();

    return ret;
}

