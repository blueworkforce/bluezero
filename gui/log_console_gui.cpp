#include <b0/b0.h>

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

#include "logger.pb.h"

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
            layout->addWidget(new QLabel("Node:"));
            layout->addWidget(textNode);
            filterToolBar->setLayout(layout);
        }

        QWidget *centralWidget = new QWidget;
        setCentralWidget(centralWidget);
        {
            QVBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(filterToolBar);
            tableWidget = new QTableWidget;
            layout->addWidget(tableWidget);
            centralWidget->setLayout(layout);
        }

        comboLevel->addItem("TRACE", b0::logger_msgs::trace);
        comboLevel->addItem("DEBUG", b0::logger_msgs::debug);
        comboLevel->addItem("INFO",  b0::logger_msgs::info);
        comboLevel->addItem("WARN",  b0::logger_msgs::warn);
        comboLevel->addItem("ERROR", b0::logger_msgs::error);
        comboLevel->addItem("FATAL", b0::logger_msgs::fatal);
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
        connect(timer, &QTimer::timeout, this, &LogConsoleWindow::spinOnce);
        timer->start(100);
    }

    void spinOnce()
    {
        node_.spinOnce();
    }

    void onLogEntry(const b0::logger_msgs::LogEntry &entry)
    {
        all_entries_.push_back(entry);
        if(!filter(entry))
            addEntry(entry);
    }

    void addEntry(const b0::logger_msgs::LogEntry &entry)
    {
        int n = tableWidget->rowCount();
        tableWidget->setRowCount(n + 1);
        tableWidget->setItem(n, 0, new QTableWidgetItem(QString::number(node_.timeUSec())));
        tableWidget->setItem(n, 1, new QTableWidgetItem(QString::fromStdString(entry.node_name())));
        tableWidget->setItem(n, 2, new QTableWidgetItem(levelStr(entry.level())));
        tableWidget->setItem(n, 3, new QTableWidgetItem(QString::fromStdString(entry.msg())));
    }

    QString levelStr(b0::logger_msgs::LogLevel level)
    {
        switch(level)
        {
        case b0::logger_msgs::trace: return "TRACE";
        case b0::logger_msgs::debug: return "DEBUG";
        case b0::logger_msgs::info:  return "INFO";
        case b0::logger_msgs::warn:  return "WARN";
        case b0::logger_msgs::error: return "ERROR";
        case b0::logger_msgs::fatal: return "FATAL";
        default: return "UNKNOWN";
        }
    }

    void comboLevelChanged(int newIndex)
    {
        filterLevel = (b0::logger_msgs::LogLevel)comboLevel->currentData().toInt();
        refilter();
    }

    void textNodeChanged(const QString &txt)
    {
        filterNodeName = textNode->text().toStdString();
        refilter();
    }

    bool filter(const b0::logger_msgs::LogEntry &entry)
    {
        if(entry.level() < filterLevel) return true;
        if(entry.node_name().find(filterNodeName) == std::string::npos) return true;
        return false;
    }

    void refilter()
    {
        tableWidget->setRowCount(0);
        for(b0::logger_msgs::LogEntry &entry : all_entries_)
            if(!filter(entry))
                addEntry(entry);
    }

private:
    b0::Node &node_;
    QTableWidget *tableWidget;
    QComboBox *comboLevel;
    QLineEdit *textNode;
    std::vector<b0::logger_msgs::LogEntry> all_entries_;
    std::string filterNodeName = "";
    b0::logger_msgs::LogLevel filterLevel = b0::logger_msgs::trace;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    b0::Node logConsoleNode("log_console");

    LogConsoleWindow logConsoleWindow(logConsoleNode);

    b0::Subscriber<b0::logger_msgs::LogEntry> logSub(&logConsoleNode, "log", &LogConsoleWindow::onLogEntry, &logConsoleWindow);

    logConsoleNode.init();

    logConsoleWindow.resize(800, 340);
    logConsoleWindow.show();

    int ret = app.exec();

    logConsoleNode.cleanup();

    return ret;
}

