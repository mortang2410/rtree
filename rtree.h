#ifndef RTREE_H
#define RTREE_H
#include <vector>
#include <list>
#include <cmath>
#include <algorithm>
#include <deque>
#include <iostream>
#include <QRectF>
#include <QString>
using namespace std;
typedef double real;


typedef pair<real,real> interval;
namespace misc
{
bool sameSign(real x1,real x2);
bool overlap(const interval& i1, const interval& i2);
bool containedIn(const interval& i1, const interval& i2);
void putMinMax(real a, real b, real& min, real& max);
};

struct Point2D
{
    real x;
    real y;

    Point2D(real X=0, real Y=0) {x = X; y= Y;}

    Point2D operator+ (const Point2D& other) const {return Point2D(x+other.x,y+other.y);}
    Point2D operator- (const Point2D& other) const {return Point2D(x-other.x,y-other.y);}
    Point2D operator* (real k) const {return Point2D(x*k,y*k);}
    Point2D operator* (int k) const {return Point2D(x*k,y*k);}
    Point2D & operator+= (const Point2D& other) {x+= other.x; y+=other.y; return *this;}
    Point2D & operator-= (const Point2D& other) {x-= other.x; y-=other.y; return *this;}

    Point2D & operator*= (real k) {x*=k; y*=k;return *this;}

    Point2D & operator*= (int k) {x*=k; y*=k; return *this;}

//     Point2D & operator=(const Point2D & other) {x=other.x;y=other.y; return (*this);}

    real normalize() const  {return sqrt(x*x+y*y); }
};

struct Rect2D
{
    Point2D a;
    Point2D b;
    Rect2D(real ax,real ay, real bx, real by) {a=Point2D(ax,ay); b= Point2D(bx,by);}
    Rect2D(const Point2D& A = Point2D(), const Point2D& B = Point2D()) {a=A; b=B;}
    Rect2D  unite(const Rect2D& other) const;
    Rect2D  unite(const  vector<Rect2D> & others) const;
    Rect2D& united(const Rect2D& other);
    Rect2D&  united(const  vector<Rect2D> & others);
    friend Rect2D sum(const Rect2D & rect, const Rect2D & other);
    friend Rect2D sum(const  vector<Rect2D> & rects);
//     Rect2D & operator=(const Rect2D & other) {a=other.a;b=other.b; return (*this);}
    bool overlap(const Rect2D& other) const;
    real area() const {return abs((a.x-b.x) * (a.y-b.y)) ;}
    real dx() const {return a.x - b.x;}
    real dy() const {return a.y - b.y;}
};

Rect2D to2D(const QRectF & rect);
QRectF toRectF(const Rect2D & rect);
struct TreeNode;



struct Entry
{

    Rect2D rect;
};


struct LeafEntry : Entry
{

    void* data;
    TreeNode* leaf;
    LeafEntry(void* Data, const Rect2D& Rect) {data= Data; rect = Rect; leaf = 0;}

};
struct NonLeafEntry : Entry
{

    TreeNode* childNode;

    NonLeafEntry(const Rect2D& Rect, TreeNode * child = 0) { rect = Rect; adoptChildNode(child);   }
    void adoptChildNode(TreeNode * child);
};

struct TreeNode
{

    vector<Entry*> entries;
    bool isLeaf;
    NonLeafEntry* parentEntry;
    TreeNode* parentNode;

    TreeNode(bool IsLeaf = false, int MaxEnt=100);
    void adoptLeafEntry(LeafEntry* entry) ;
    void adoptNonLeafEntry(NonLeafEntry * entry);
    Rect2D totalArea() const;
    Rect2D quickTotalArea() const; // don't use when updating tree    
};



class RTree
{
public:
    RTree(int MaxEntriesPerNode, void (*Dealloc)(void* data));
    ~RTree();
    void insert(void * data, const Rect2D & rect) {insert( new LeafEntry(data, rect) ); }
    void insert(LeafEntry* entry);
    deque<LeafEntry*> searchOverlap(const Rect2D& rect);
    TreeNode * chooseLeaf(LeafEntry* entry);
    void adjustTree(TreeNode* node, TreeNode * newnode = 0);
    TreeNode* splitNode(TreeNode* node);
    void deleteEntry(LeafEntry* entry);
    void condenseTree(TreeNode* leaf);
    TreeNode * findLeaf(LeafEntry * entry) {return entry->leaf;}    


    TreeNode* root;
private:
    void deallocNode(TreeNode * node);
    void deallocLeafEntry(LeafEntry *entry);    
    void (*dealloc)(void* data);
    
    

    int maxEnt;
    int minEnt;
};
/*
notice that
TreeNode * findLeaf(LeafEntry * entry)
is really simple. The original version calculates from the root to derive the leaf.
And this gets called every time with deleteEntry?
I use a simple pointer to the Leaf for each entry instead.
It will cost more space (an extra pointer), but will give better performance 
than calculating all the time
(or at least I hope so)
*/
#endif // RTREE_H
