#ifndef VIEWS_H
#define VIEWS_H
#include "qtmodules.h"
#include "rtree.h"


QString giveName(int i);


class MyRect : public QGraphicsRectItem
{
public:
//    MyRect( QGraphicsItem * parent = 0 );
    MyRect( const QPointF & p1, const QPointF & p2 );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    ~MyRect();
    void postinit();

    QString label;
    LeafEntry * entry;
};

void dealloc(void *data);
class RTreeScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum {MAXENT=4};
    RTreeScene(QWidget *parent = 0);
    ~RTreeScene();
    void drawTree();
    void healthCheck();
    qreal drawNode(TreeNode * node);
    RTree * tree;
    QGraphicsPathItem  * decor;
    QPainterPath * path;

    qreal jump;
};

class RectScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum Mode{Add, Remove,Search};
    RectScene(QWidget *parent = 0);
    void setMode(Mode newmode);

    RTreeScene * treescene;
    int namePool;
protected:
//    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
//    void wheelEvent(QGraphicsSceneWheelEvent *event);
    bool insearch;
    Mode mode;
    QList<QPointF> buffer;
    QList<QGraphicsRectItem*> searchBuf;
};


class RectView : public QGraphicsView
{
    Q_OBJECT
public:
    RectView(QWidget *parent = 0);

    RectScene * scene;
};


class RTreeView : public QGraphicsView
{
    Q_OBJECT
public:
    RTreeView(QWidget *parent = 0);

    RTreeScene * scene;
};

#endif // VIEWS_H
