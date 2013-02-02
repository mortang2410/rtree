#include "rtree.h"
#include "views.h"



Rect2D to2D(const QRectF & rect)
{
    return Rect2D(rect.topLeft().x(),rect.topLeft().y(),
                  rect.bottomRight().x(),rect.bottomRight().y());
}
QRectF toRectF(const Rect2D & rect)
{
    return QRectF(rect.a.x,rect.a.y,rect.b.x-rect.a.x,rect.b.y-rect.a.y);
}
namespace misc
{

inline bool sameSign(real x1, real x2)
{
    return (((x1 >= 0) && (x2 >=0)) || ((x1<=0) && (x2<=0)));
}

inline bool overlap(const interval& i1, const interval& i2)
{
    return !((min(i1.first,i1.second) > max(i2.first,i2.second))
            || (max(i1.first,i1.second) < min(i2.first,i2.second)));

}

inline bool containedIn(const interval& i1, const interval& i2)
{
    real a1 = min(i1.first,i1.second);
    real a2 = max(i1.first,i1.second);
    real b1 = min(i2.first,i2.second);
    real b2 = max(i2.first,i2.second);
    return ( (a1 < b1) == (b2<a2)  );

}

inline void putMinMax(real a, real b, real& min, real& max)
{
    if (a<b) { min = a; max =b;}
    else {min = b; max = a; }
}

//for rtree only
void updateMinMax(real a, real b, real& min, real& max,int& rmin, int& rmax, int target)
{

    if (a<b)
    {
        if (a>min) { min=a; rmin = target;}
        if (b<max) { max =b; rmax=target;}
    }
    else
    {
        if (b>min) { min=b; rmin = target;}
        if (a<max)  { max =a; rmax=target;}
    }
}
}


inline Rect2D Rect2D::unite(const Rect2D& other) const
{
    return sum(*this,other);
}

inline Rect2D& Rect2D::united(const Rect2D& other)
{
    *this = sum(*this,other);
    return (*this);
}

inline Rect2D Rect2D::unite(const vector< Rect2D >& others) const
{
    return sum(*this, sum(others));
}
inline Rect2D& Rect2D::united(const vector< Rect2D >& others)
{
    *this = sum(*this, sum(others));
    return * this;
}

Rect2D sum(const Rect2D &rect, const Rect2D &other)
{
    real minx,miny,maxx,maxy;
    minx=maxx=rect.a.x; miny=maxy=rect.a.y;
    Point2D p[] = {rect.b,other.a,other.b};
    for (int i = 0; i<3; ++i )
    {
        if (p[i].x > maxx) maxx=p[i].x;
        if (p[i].y > maxy) maxy=p[i].y;
        if (p[i].x < minx) minx=p[i].x;
        if (p[i].y < miny) miny=p[i].y;
    }
    return Rect2D(minx,miny,maxx,maxy);
}

Rect2D sum(const vector<Rect2D> &rects)
{
    if (!rects.size()) return Rect2D();
    Rect2D result = rects[0];
    int n=rects.size();
    for (int i = 1; i<n; ++i) result.united(rects[i]);
    return result;
}


inline bool Rect2D::overlap(const Rect2D& other) const
{
    return misc::overlap(interval(a.x,b.x),interval(other.a.x,other.b.x))
           && misc::overlap(interval(a.y,b.y),interval(other.a.y,other.b.y));
}



//linear cost algo
TreeNode* RTree::splitNode(TreeNode* node)
{
    TreeNode* newnode = new TreeNode(node->isLeaf,maxEnt);
    vector<Entry*> temp;
    temp.swap(node->entries);
    int i;
    int n = temp.size();
    real xhigh,xlow,yhigh,ylow;
    int rxhigh,rxlow,ryhigh,rylow; // the index of the respective rects
    Rect2D* rect = & (temp[0]->rect);
    rxhigh=rxlow=ryhigh=rylow=0;
    misc::putMinMax(rect->a.x,rect->b.x,xlow,xhigh);
    misc::putMinMax(rect->a.y,rect->b.y,ylow,yhigh);
    for (i=1; i< n; ++i)
    {
        rect = & (temp[i]->rect);
        misc::updateMinMax(rect->a.x,rect->b.x,xlow,xhigh,rxlow,rxhigh,i);
        misc::updateMinMax(rect->a.y,rect->b.y,ylow,yhigh,rylow,ryhigh,i);
    }
    Rect2D totalRect = root->totalArea();
    real dxNormal = (xlow-xhigh)/abs(totalRect.dx());
    real dyNormal = (ylow-yhigh)/abs(totalRect.dy());
    vector<Entry*>::const_iterator r1,r2;

    if (dxNormal < dyNormal)
    {
        r1 = temp.cbegin() + rxlow;
        r2 = temp.cbegin() + rxhigh;
    }
    else
    {
        r1 = temp.cbegin() + rylow;
        r2 = temp.cbegin() + ryhigh;
    }
    node->entries.push_back(*r1);
    if (*r2 != *r1)
    {
        newnode->entries.push_back(*r2);
    }
    auto g = [=](Entry * e) {return (e == *r1)  || (e== *r2) ;};
    auto end_valid = remove_if(temp.begin(),temp.end(),g);    
    temp.erase(end_valid,temp.end());
    while (true)
    {
        node->entries.push_back(temp.back()); temp.pop_back();
        if (!temp.size()) break;
        newnode->entries.push_back(temp.back()); temp.pop_back();
        if (!temp.size()) break;
    }

    int ns = node->entries.size();
    if (node->isLeaf)
    {
        for (int i =0; i< ns; ++i)
        {
            ((LeafEntry*)node->entries[i])->leaf = node;
        }
        ns=newnode->entries.size();
        for (int i =0; i< ns; ++i)
        {
            ((LeafEntry*)newnode->entries[i])->leaf = newnode;
        }
    }
    else
    {
        ns=newnode->entries.size();
        for (int i =0; i< ns; ++i)
        {
            ((NonLeafEntry*)newnode->entries[i])->childNode->parentNode = newnode;
        }
    }


    return newnode;
}




TreeNode::TreeNode(bool IsLeaf, int MaxEnt)
{
     parentNode = 0; parentEntry = 0; isLeaf = IsLeaf;
     entries.reserve(MaxEnt);
}

inline void TreeNode::adoptLeafEntry(LeafEntry *entry)
{
    entries.push_back(entry); entry->leaf = this;    
    if ( !isLeaf) cerr << "error adoptleafen" << endl;
}

void TreeNode::adoptNonLeafEntry(NonLeafEntry *entry)
{
   entries.push_back(entry);   
}

Rect2D TreeNode::totalArea() const
{
    if (!entries.size()) return Rect2D();
    Rect2D result=entries[0]->rect;
    int n = entries.size();
    for (int i = 1; i<n; ++i) result.united(entries[i]->rect);
    return result;

}
inline Rect2D TreeNode::quickTotalArea() const
{
    return (parentEntry) ? parentEntry->rect : totalArea();
}

deque<LeafEntry*> RTree::searchOverlap(const Rect2D& rect)
{
    deque<LeafEntry*> result;
    deque<TreeNode*> nodes;
    int n,i; TreeNode* node; bool isLeaf;
    nodes.push_back(root);
    while (nodes.size())
    {
        node = nodes.back();
        nodes.pop_back();
        n = node->entries.size();
        isLeaf = node->isLeaf;
        for (i=0; i<n; ++i)
        {
            if (rect.overlap(node->entries[i]->rect))
            {
                if (isLeaf)
                {
                    LeafEntry * en = (LeafEntry*) node->entries[i];
                    result.push_back(en);                    
                }
                else nodes.push_back(((NonLeafEntry*) node->entries[i])->childNode);
            }
        }
    }
    return result;
}

void RTree::insert(LeafEntry* entry)
{
    TreeNode* node = chooseLeaf(entry);
    TreeNode* newnode = 0;    
    node->adoptLeafEntry(entry);
    if (node->entries.size() > maxEnt)
    {
        newnode = splitNode(node);       
    }
    adjustTree(node,newnode);
}
TreeNode* RTree::chooseLeaf(LeafEntry* entry)
{
    if (root->isLeaf) return root;
    TreeNode* node = root;
    int i,imin,n;
    Rect2D* rect;
    real waste,wastemin;
    while (true)
    {
        if (node->isLeaf) return node;
        n = node->entries.size();
        if (!n) return 0;
        imin = 0;
        rect = &(node->entries[0]->rect);
        wastemin = entry->rect.unite(*rect).area()-rect->area();
        for (i=1; i<n; ++i)
        {
            rect = &(node->entries[i]->rect);
            waste = entry->rect.unite(*rect).area() -rect->area();
            if (waste<wastemin) { wastemin = waste; imin = i; }
        }
        node = ((NonLeafEntry*) node->entries[imin])->childNode;
    }
}

void RTree::adjustTree(TreeNode* node, TreeNode* newnode)
{

    if (newnode)
    {

        NonLeafEntry* entry2 = new NonLeafEntry(newnode->totalArea(),newnode);
        if (node == root)
        {
            root = new TreeNode(false,maxEnt);
            node->parentNode = root;
            TreeNode* parent = node->parentNode;
            newnode->parentNode = parent;
            NonLeafEntry* entry1 = new NonLeafEntry(node->totalArea(),node);

            parent->adoptNonLeafEntry(entry1);
            parent->adoptNonLeafEntry(entry2);
        }
        else
        {
            TreeNode* parent = node->parentNode;
            newnode->parentNode = parent;

            NonLeafEntry* entry1 = node->parentEntry;
            entry1->rect.united(node->totalArea());

            parent->adoptNonLeafEntry(entry2);

            if (parent->entries.size() > maxEnt)
                adjustTree(parent,splitNode(parent));
            else adjustTree(parent);
        }
    }
    else
    {
        if (node == root) return;
        node->parentEntry->rect.united(node->totalArea());
        adjustTree(node->parentNode);
    }
}
inline void NonLeafEntry::adoptChildNode(TreeNode* child)
{
    childNode = child;
    if (child) child->parentEntry = this;
}

RTree::~RTree()
{
    deallocNode(root);
}


void RTree::deallocNode(TreeNode* node)
{
    if (node->isLeaf)
    {
        int n = node->entries.size();
        for (int i =0; i<n; ++i)
        {
            LeafEntry* en = (LeafEntry*) node->entries[i];
            deallocLeafEntry(en);
        }
    }
    else
    {
        int n = node->entries.size();
        for (int i =0; i<n; ++i)
        {
            NonLeafEntry* en = (NonLeafEntry*) node->entries[i];
            deallocNode(en->childNode);
            delete en;
        }
    }
    delete node;
}

inline void RTree::deallocLeafEntry(LeafEntry* entry)
{
    dealloc(entry->data);
    delete entry;
}


void RTree::deleteEntry(LeafEntry* entry)
{
    TreeNode* leaf = entry->leaf;

    vector<Entry*>& ent = leaf->entries;
    auto end2 = remove(ent.begin(),ent.end(),entry);

    if (end2 == ent.end())
    {
        cout << "wat" << endl;
    }
    ent.erase(end2,ent.end());

    condenseTree(leaf);
    deallocLeafEntry(entry);
}

void RTree::condenseTree(TreeNode* leaf)
{
    TreeNode* node = leaf;
    deque<TreeNode*> hobo;

    vector<Entry*>* ent;
    NonLeafEntry* en;
    int n;
    while (node != root)
    {
        if (node->entries.size()  < minEnt)
        {
            hobo.push_back(node);
            ent = &(node->parentNode->entries);
            en = node->parentEntry;
            auto end2 = remove(ent->begin(),ent->end(),en);
            ent->erase(end2,ent->end());
            delete en;
        }
        else
        {
            node->parentEntry->rect.united(node->totalArea());
        }
        node = node->parentNode;
    }

    if (!root->entries.size()) root->isLeaf = true;

    while (hobo.size())
    {
        node = hobo.back();
        hobo.pop_back();
        n = node->entries.size();
        if (node->isLeaf)
        {
            for (int i = 0; i<n ; ++i) insert((LeafEntry*) node->entries[i]);
        }
        else
        {
            for (int i = 0; i<n ; ++i)
            {
                en = (NonLeafEntry*) node->entries[i];
                hobo.push_back(en->childNode);
                delete en;
            }
        }
        delete node;
    }
    if (!root->isLeaf && (root->entries.size() == 1))
    {
        en = (NonLeafEntry*) root->entries[0];
        TreeNode* newroot = en->childNode;
        newroot->parentNode = 0;
        newroot->parentEntry = 0;
        delete root;
        delete en;
        root = newroot;
    }
}


RTree::RTree(int MaxEntriesPerNode, void (*Dealloc)(void* data))
{
    maxEnt = MaxEntriesPerNode ;
    minEnt = maxEnt/2;
    dealloc = Dealloc;
    root = new TreeNode(true,maxEnt);
}
