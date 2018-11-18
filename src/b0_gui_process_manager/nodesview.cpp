#include "nodesview.h"

#include <QDebug>
#include <QtMath>
#include <QTemporaryFile>
#include <QProcess>
#include <QContextMenuEvent>
#include <QApplication>

AbstractItem::AbstractItem(NodesView *nodeView)
    : nodesView_(nodeView)
{
    outlineColor_ = nodeView->palette().color(nodeView->foregroundRole());
}

AbstractItem::~AbstractItem()
{
}

AbstractVertex::AbstractVertex(NodesView *nodeView, const QString &text, const QColor &color)
    : AbstractItem(nodeView),
      text_(text),
      color_(color)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemIsSelectable);
    setZValue(1.0);
    computePath();
}

AbstractVertex::~AbstractVertex()
{
    while(!connections_.isEmpty())
        delete connections_[0];
}

QPainterPath AbstractVertex::shape() const
{
    return path_;
}

QRectF AbstractVertex::boundingRect() const
{
    return path_.boundingRect().marginsAdded(QMargins(1, 1, 1, 1));
}

void AbstractVertex::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    QColor norm(color_), sel(color_);
    norm.setAlpha(50);
    sel.setAlpha(120);
    painter->fillPath(path_, isSelected() ? sel : norm);
    painter->setPen(QPen(outlineColor_));
    painter->drawPath(path_);
    QFontMetrics fm(scene()->font());
    int w = fm.horizontalAdvance(text_);
    int h = fm.height();
    painter->drawText(-w / 2, h / 2, text_);
}

QVariant AbstractVertex::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionHasChanged && scene())
    {
        for(Connection *conn : connections_)
            conn->update();
    }
    else if(change == ItemSceneHasChanged)
    {
        computePath();
    }
    else if(change == ItemSelectedHasChanged)
    {
        nodesView_->raiseItem(this);
    }
    return QGraphicsItem::itemChange(change, value);
}

QString AbstractVertex::text() const
{
    return text_;
}

QPointF AbstractVertex::pointOnBorderAtAngle(qreal angle) const
{
    // normalize angle between -M_PI and +M_PI
    angle = fmod(fmod(angle + M_PI, 2 * M_PI) + 2 * M_PI, 2 * M_PI) - M_PI;

    const QRectF &rect(boundingRect());
    qreal w2 = rect.width() / 2,
        h2 = rect.height() / 2;

    double a1 = qAtan2(h2, w2),
        a2 = qAtan2(h2, -w2),
        a3 = qAtan2(-h2, -w2),
        a4 = qAtan2(-h2, w2);

    if(angle >= a3 && angle < a4)
        // bottom side
        return QPointF{-h2 * qTan(M_PI / 2 - angle), h2};
    else if(angle >= a4 && angle < a1)
        // right side
        return QPointF{w2, -w2 * qTan(angle)};
    else if(angle >= a1 && angle < a2)
        // top side
        return QPointF{h2 * qTan(M_PI / 2 - angle), -h2};
    else
        // left side
        return QPointF{-w2, w2 * qTan(angle)};
}

void AbstractVertex::computePath()
{
    if(!scene()) return;
    QFontMetrics fm(scene()->font());
    int w = fm.horizontalAdvance(text_);
    int h = fm.height();
    QRectF rect(-w / 2, -h / 2, w, h);
    path_ = QPainterPath();
    path_.addRect(rect.adjusted(-10, -10, 10, 10));
}

Node::Node(NodesView *nodeView, const QString &text)
    : AbstractVertex(nodeView, text, Qt::black)
{
}

AbstractSocket::AbstractSocket(NodesView *nodeView, const QString &text, const QColor &color)
    : AbstractVertex(nodeView, text, color)
{
}

Topic::Topic(NodesView *nodeView, const QString &text)
    : AbstractSocket(nodeView, text, Qt::blue)
{
}

Service::Service(NodesView *nodeView, const QString &text)
    : AbstractSocket(nodeView, text, Qt::red)
{
}

Connection::Connection(NodesView *nodeView, Node *node, AbstractSocket *socket, Direction dir)
    : AbstractItem(nodeView),
      node_(node),
      socket_(socket),
      dir_(dir)
{
    color_ = nodeView->palette().color(nodeView->foregroundRole());
    node_->connections_.append(this);
    socket_->connections_.append(this);
    update();
}

Connection::~Connection()
{
    node_->connections_.removeAll(this);
    socket_->connections_.removeAll(this);
}

QPainterPath Connection::shape() const
{
    qreal angle = qDegreesToRadians(line_.angle());
    const qreal headLength = 12, headAngle = M_PI / 4;
    const qreal a1 = angle + M_PI + headAngle / 2,
            a2 = angle + M_PI - headAngle / 2;
    QPolygonF arrowHeadPoly;
    arrowHeadPoly.append(line_.p2());
    arrowHeadPoly.append(line_.p2() + headLength * QPointF{cos(a1), -sin(a1)});
    arrowHeadPoly.append(line_.p2() + headLength * QPointF{cos(a2), -sin(a2)});
    arrowHeadPoly.append(line_.p2());
    QPolygonF linePoly;
    linePoly.append(line_.p1());
    linePoly.append(line_.p2());
    QPainterPath path;
    path.addPolygon(linePoly);
    path.addPolygon(arrowHeadPoly);
    return path;
}

QRectF Connection::boundingRect() const
{
    const int k = 6;
    return shape().boundingRect().marginsAdded(QMargins(k, k, k, k));
}

void Connection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->fillPath(shape(), color_);
    painter->strokePath(shape(), QPen(color_));
}

void Connection::update()
{
    prepareGeometryChange();
    line_.setP1(source()->pos());
    line_.setP2(destination()->pos());
    qreal a = qDegreesToRadians(line_.angle());
    line_.setP1(source()->pos() + source()->pointOnBorderAtAngle(a));
    line_.setP2(destination()->pos() + destination()->pointOnBorderAtAngle(a + M_PI));
}

AbstractVertex * Connection::source() const
{
    switch(dir_)
    {
    case Direction::In: return socket_;
    case Direction::Out: return node_;
    }
}

AbstractVertex * Connection::destination() const
{
    switch(dir_)
    {
    case Direction::In: return node_;
    case Direction::Out: return socket_;
    }
}

Node * Connection::node() const
{
    return node_;
}

AbstractSocket * Connection::socket() const
{
    return socket_;
}

NodesView::NodesView(QWidget *parent)
    : QGraphicsView(parent),
      startNodeDialog_(new StartNodeDialog)
{
    contextMenu_ = new QMenu();
    actionStartNode_ = contextMenu_->addAction("Start new node...", this, &NodesView::onMenuStartNode);
    actionStopNode_ = contextMenu_->addAction("Stop selected node", this, &NodesView::onMenuStopNode);
    contextMenu_->addSeparator();
    contextMenu_->addAction("Arrange items", this, &NodesView::onMenuArrangeItems);

    setScene(scene_ = new QGraphicsScene(this));
    setRenderHint(QPainter::Antialiasing, false);
    setRenderHint(QPainter::TextAntialiasing, true);
    setMinimumSize(600, 400);

#if 0
    auto n1 = addNode("node-with-long-name-1");
    auto n2 = addNode("node-2");
    auto n3 = addNode("node-4");
    auto t1 = addTopic("topic");
    auto s1 = addService("srv");
    n1->setPos(40, 50);
    n2->setPos(40, 210);
    n3->setPos(240, 210);
    t1->setPos(60, 130);
    s1->setPos(220, 130);
    addConnection(n1, t1, Direction::In);
    addConnection(n2, t1, Direction::Out);
    addConnection(n1, s1, Direction::Out);
    addConnection(n3, s1, Direction::Out);
    centerOn(t1);
    arrangeItems();
#endif
}

Node * NodesView::addNode(const QString &text)
{
    Node *node = new Node(this, text);
    scene()->addItem(node);
    return node;
}

Topic * NodesView::addTopic(const QString &text)
{
    Topic *topic = new Topic(this, text);
    scene()->addItem(topic);
    return topic;
}

Service * NodesView::addService(const QString &text)
{
    Service *service = new Service(this, text);
    scene()->addItem(service);
    return service;
}

Connection * NodesView::addConnection(Node *node, AbstractSocket *socket, Direction dir)
{
    Connection *conn = new Connection(this, node, socket, dir);
    scene()->addItem(conn);
    return conn;
}

void NodesView::raiseItem(QGraphicsItem *item)
{
    for(auto x : items())
    {
        if(x->parentItem() == item->parentItem())
            x->stackBefore(item);
    }
}

void NodesView::arrangeItems()
{
    QTemporaryFile file;
    if(!file.open()) {
        qDebug() << "failed to open QTemporaryFile";
        return;
    }
    QTextStream stream(&file);
    QMap<QString, AbstractVertex *> itemMap;
    toGraphviz(stream, itemMap);
    file.close();

    QProcess process;
    process.start("/usr/local/bin/dot", QStringList{"-Tplain", file.fileName()});
    if(!process.waitForStarted())
    {
        qDebug() << "failed to start program" << process.program();
        return;
    }
    process.waitForFinished();
    QString out(process.readAllStandardOutput());
    QStringList lines = out.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    for(auto line : lines)
    {
        QStringList tokens = line.split(QRegExp("\\s"), QString::SkipEmptyParts);
        if(tokens[0] == "node")
        {
            QString id = tokens[1];
            auto it = itemMap.find(id);
            if(it != itemMap.end())
            {
                qreal x = tokens[2].toDouble();
                qreal y = tokens[3].toDouble();
                it.value()->setPos(100 * QPointF(x, y));
            }
        }
    }
}

void NodesView::toGraphviz(QTextStream &stream, QMap<QString, AbstractVertex *> &itemMap) const
{
    stream << "digraph {" << endl;
    stream << "  graph [overlap=false];" << endl;
    stream << "  node [shape=box];" << endl;

    long itemCount = 1;
    QString itemIdFmt("item_%1");
    const int graphvizIdKey = 4242;

    for(auto item : items())
    {
        if(auto abstractItem = dynamic_cast<AbstractVertex *>(item))
        {
            QString id = itemIdFmt.arg(itemCount++);
            abstractItem->setData(graphvizIdKey, id);
            itemMap.insert(id, abstractItem);
            stream << "  " << id << " [label=\"" << abstractItem->text() << "\"]" << endl;
        }
    }

    for(auto item : items())
    {
        if(auto connection = dynamic_cast<Connection *>(item))
        {
            stream << "  "
                << connection->source()->data(graphvizIdKey).toString()
                << " -> "
                << connection->destination()->data(graphvizIdKey).toString()
                << ";" << endl;
        }
    }

    stream << "}" << endl;
}

void NodesView::contextMenuEvent(QContextMenuEvent *event)
{
    auto selection = scene()->selectedItems();

    actionStartNode_->setEnabled(selection.isEmpty());

    actionStopNode_->setEnabled(selection.size() == 1);
    for(auto item : selection)
    {
        if(!dynamic_cast<Node *>(item))
            actionStopNode_->setEnabled(false);
    }

    contextMenu_->popup(event->globalPos());
}

template<typename T>
inline T * getOrCreate(QMap<QString, T*> &map, const QString &name, T * (NodesView::*fnCreate)(const QString&), NodesView *nv)
{
    auto it = map.find(name);
    if(it != map.end()) return it.value();
    T *obj = (nv->*fnCreate)(name);
    map[name] = obj;
    return obj;
}

void NodesView::setGraph(QMap<QString, QString> node_topic, QMap<QString, QString> topic_node, QMap<QString, QString> node_service, QMap<QString, QString> service_node)
{
    using std::placeholders::_1;
    QMap<QString, Node*> nodeByNameMap;
    QMap<QString, Topic*> topicByNameMap;
    QMap<QString, Service*> serviceByNameMap;
    scene()->clear();
    auto node = [&](const QString &n) {return getOrCreate(nodeByNameMap, n, &NodesView::addNode, this);};
    auto topic = [&](const QString &n) {return getOrCreate(topicByNameMap, n, &NodesView::addTopic, this);};
    auto service = [&](const QString &n) {return getOrCreate(serviceByNameMap, n, &NodesView::addService, this);};
    for(auto k : node_topic.keys())
        addConnection(node(k), topic(node_topic.value(k)), Direction::Out);
    for(auto k : topic_node.keys())
        addConnection(node(topic_node.value(k)), topic(k), Direction::In);
    for(auto k : node_service.keys())
        addConnection(node(k), service(node_service.value(k)), Direction::Out);
    for(auto k : service_node.keys())
        addConnection(node(service_node.value(k)), service(k), Direction::In);
    arrangeItems();
}

void NodesView::setActiveNodes(QSet<QString> active_nodes)
{
    QString oldSel = startNodeDialog_->comboHost->currentText();
    startNodeDialog_->comboHost->clear();
    for(auto s : active_nodes)
        startNodeDialog_->comboHost->addItem(s);
    startNodeDialog_->comboHost->setCurrentText(oldSel);
}

void NodesView::onMenuStartNode()
{
    startNodeDialog_->exec();
}

void NodesView::onMenuStopNode()
{
}

void NodesView::onMenuArrangeItems()
{
    arrangeItems();
}
