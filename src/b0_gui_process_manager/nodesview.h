#ifndef NODESVIEW_H
#define NODESVIEW_H

#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QTextStream>
#include <QMenu>
#include "startnodedialog.h"

class NodesView;

class Connection;

class AbstractItem : public QGraphicsItem
{
public:
    AbstractItem(NodesView *nodeView);
    virtual ~AbstractItem();

protected:
    NodesView *nodesView_;
    QColor outlineColor_;
};

class AbstractVertex : public AbstractItem
{
public:
    AbstractVertex(NodesView *nodeView, const QString &text, const QColor &color);
    virtual ~AbstractVertex();

    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    QString text() const;

protected:
    QPointF pointOnBorderAtAngle(qreal angle) const;

private:
    QString text_;
    QColor color_;
    QPainterPath path_;
    QVector<Connection *> connections_;

    void computePath();

    friend class Connection;
};

class Node : public AbstractVertex
{
public:
    Node(NodesView *nodeView, const QString &text);
};

class AbstractSocket : public AbstractVertex
{
public:
    AbstractSocket(NodesView *nodeView, const QString &text, const QColor &color);
};

class Topic : public AbstractSocket
{
public:
    Topic(NodesView *nodeView, const QString &text);
};

class Service : public AbstractSocket
{
public:
    Service(NodesView *nodeView, const QString &text);
};

enum class Direction
{
    In,
    Out
};

class Connection : public AbstractItem
{
public:
    Connection(NodesView *nodeView, Node *node, AbstractSocket *sock, Direction dir);
    virtual ~Connection();

    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void update();
    AbstractVertex * source() const;
    AbstractVertex * destination() const;
    Node * node() const;
    AbstractSocket * socket() const;

private:
    Node *node_;
    AbstractSocket *socket_;
    Direction dir_;
    QLineF line_;
    QColor color_;
};

class NodesView : public QGraphicsView
{
    Q_OBJECT
public:
    NodesView(QWidget *parent = nullptr);

    Node * addNode(const QString &text);
    Topic * addTopic(const QString &text);
    Service * addService(const QString &text);
    Connection * addConnection(Node *node, AbstractSocket *socket, Direction dir);

    void raiseItem(QGraphicsItem *item);
    void arrangeItems();
    void toGraphviz(QTextStream &stream, QMap<QString, AbstractVertex *> &itemMap) const;

    void contextMenuEvent(QContextMenuEvent *event);

public Q_SLOTS:
    void setGraph(QMap<QString, QString> node_topic, QMap<QString, QString> topic_node, QMap<QString, QString> node_service, QMap<QString, QString> service_node);
    void setActiveNodes(QSet<QString> active_nodes);

private Q_SLOTS:
    void onMenuStartNode();
    void onMenuStopNode();
    void onMenuArrangeItems();

private:
    QGraphicsScene *scene_;
    QMenu *contextMenu_;
    QAction *actionStartNode_;
    QAction *actionStopNode_;
    StartNodeDialog *startNodeDialog_;

    friend class MainWindow;
};

#endif // NODESVIEW_H
