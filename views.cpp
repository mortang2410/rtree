#include "views.h"


QString giveName(int i)
{
    int k='Z' - 'A';
    int alpha = (i % k) + 'A';
    int numeric = i / k;
    return QString("%1%2").arg(char(alpha)).arg(QString::number(numeric));
}


RectScene::RectScene(QWidget *parent)
    :QGraphicsScene(parent)
{    
    mode = Add;
    namePool = 1;
    insearch = false;
}

void RectScene::setMode(RectScene::Mode newmode)
{
    mode = newmode;
    buffer.clear();
    clearSelection();
    while (searchBuf.size())
    {
        auto x = searchBuf.back();
        removeItem(x);
        searchBuf.pop_back();
        delete x;
    }
}

void RectScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->scenePos();
    MyRect * item = dynamic_cast<MyRect *>(itemAt(pos,QTransform()));
    switch (mode)
    {
    case Add:
        {
            buffer.push_back(pos);

            if (buffer.size() == 2)
            {
                MyRect * newrect = new MyRect(buffer[0],buffer[1]);
                addItem(newrect);
                buffer.clear();
                newrect->label = giveName(namePool++);
                auto x = new QGraphicsTextItem(newrect->label,newrect);
                x->setPos(newrect->rect().center());
                treescene->tree->insert(newrect->entry);
                update();
                treescene->drawTree();
            }

        }
        break;
    case Remove:
        {
            if (item)
            {
                removeItem(item);
                treescene->tree->deleteEntry(item->entry);
                delete item;
                update();
                treescene->drawTree();

            }
        }
        break;
    case Search:
        {
            if (!insearch)
            {
                insearch = true;
                buffer.push_back(pos);
            }
            else
            {
                insearch = false;
                QRectF rect(pos,buffer[0]);
                searchBuf.push_back(new QGraphicsRectItem(rect));
                addItem(searchBuf.back());
                deque<LeafEntry*> x = treescene->tree->searchOverlap(to2D(rect));
                MyRect * rectpt;
                int n = x.size();                
                for (int i = 0; i<n; ++i)
                {

                    rectpt =(MyRect*) x[i]->data;
                    rectpt->setSelected(true);
                }
                buffer.clear();

            }
        }
        break;
    }
}

void RectScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
//    switch (mode)
//    {
//    case Add:
//        {

//        }
//        break;
//    case Remove:
//        {

//        }
//        break;
//    }
}

void RectScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
//    switch (mode)
//    {
//    case Add:
//        {

//        }
//        break;
//    case Remove:
//        {

//        }
//        break;
//    }
}

void dealloc(void *data)
{

}

RTreeScene::RTreeScene(QWidget *parent)
    :QGraphicsScene(parent)
{
    tree = new RTree(MAXENT,dealloc);
    decor = new QGraphicsPathItem;

    jump = 50;
}

RTreeScene::~RTreeScene()
{
    delete tree;
    delete decor;
}

void RTreeScene::drawTree()
{
    if (decor->scene()) removeItem(decor);
    clear();
    addItem(decor);
    path = new QPainterPath;
    path->moveTo(0,0);
    path->lineTo(jump,0);
    drawNode(tree->root);    
    decor->setPath(*path);
    delete path;

    update();
}

void RTreeScene::healthCheck()
{

}


qreal RTreeScene::drawNode(TreeNode *node)
{
    qreal down = 0;
    if (!node)
    {
        return down+jump;
    }    
    auto  isleaf = new QGraphicsTextItem(
                (node == tree->root) ? "root" :
                                 (node->isLeaf? "leaf" : "nonleaf" ));
    addItem(isleaf);
    isleaf->setPos(path->currentPosition());
    QPointF point = path->currentPosition();
    if (node->isLeaf)
    {
        int n = node->entries.size();
        for (int i = 0; i<n; ++i)
        {
            path->lineTo(path->currentPosition() + QPointF(jump,0));
            LeafEntry * en = (LeafEntry *) node->entries[i];
            auto label = new QGraphicsTextItem(((MyRect*)en->data)->label);
            label->setPos(path->currentPosition());            
            addItem(label);
            if (i!=(n-1))
            {
                path->moveTo(path->currentPosition() + QPointF(-jump,0));
                path->lineTo(path->currentPosition()+ QPointF(0,jump));
                down += jump;
            }
        }
    }
    else
    {
        qreal downsub = 0;
        int n = node->entries.size();
        for (int i = 0; i<n; ++i)
        {
            path->lineTo(path->currentPosition() + QPointF(jump,0));
            NonLeafEntry * en = (NonLeafEntry *) node->entries[i];
            downsub = drawNode(en->childNode);
            down+=downsub;
            if (i!=(n-1))
            {
                path->moveTo(path->currentPosition() + QPointF(-jump,0));
                path->lineTo(path->currentPosition()+ QPointF(0,downsub));
            }
        }
    }
    path->moveTo(point);
    return down+jump;
}

RectView::RectView(QWidget *parent)
    :QGraphicsView(parent)
{
    scene = new RectScene(this);
    setScene(scene);
    setCacheMode(QGraphicsView::CacheNone);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontSavePainterState,true);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}


RTreeView::RTreeView(QWidget *parent)
    :QGraphicsView(parent)
{
    scene = new RTreeScene(this);
    setScene(scene);
}

MyRect::MyRect(const QPointF &p1, const QPointF &p2)
    :QGraphicsRectItem(QRectF(p1,p2))
{
    setBrush(QColor(0,0,120,120));
    setFlag(ItemIsSelectable);
    postinit();
}

QVariant MyRect::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged)
    {
        if( isSelected())
        {
            setBrush(QColor(120,0,0,120));
        }
        else setBrush(QColor(0,0,120,120));
    }
    return QGraphicsItem::itemChange(change, value);
}

void MyRect::postinit()
{
    entry = new LeafEntry(this,to2D(rect()));    
}


MyRect::~MyRect()
{

}
