#include "kdtree.h"

KDNode::KDNode()
    : leftChild(nullptr), rightChild(nullptr), axis(0), minCorner(), maxCorner(), particles()
{}

KDNode::~KDNode()
{
    delete leftChild;
    delete rightChild;
}

KDTree::KDTree()
    : root(nullptr), isEmpty(true)
{}

KDTree::~KDTree()
{
    delete root;
}

// Comparator functions you can use with std::sort to sort vec3s along the cardinal axes
bool xSort(Photon a, Photon b) { return a.pos.x < b.pos.x; }
bool ySort(Photon a, Photon b) { return a.pos.y < b.pos.y; }
bool zSort(Photon a, Photon b) { return a.pos.z < b.pos.z; }

void KDTree::build(const std::vector<Photon> *points)
{
    if(points->size() == 0)
    {
        isEmpty = true;
        return;
    }
    else
    {
        isEmpty = false;
    }
    minCorner = getMinCorner(*points);
    maxCorner = getMaxCorner(*points);
    root = buildHelper(*points, 0);
    std::cout << "KDTree Build Finished!" << std::endl;
}

glm::vec3 KDTree::getMaxCorner(const std::vector<Photon> &points)
{
    std::vector<float> xarray;
    std::vector<float> yarray;
    std::vector<float> zarray;
    for (unsigned int i = 0; i < points.size(); i++)
    {
        xarray.push_back(points[i].pos.x);
        yarray.push_back(points[i].pos.y);
        zarray.push_back(points[i].pos.z);
    }
    auto xresult = std::max_element(xarray.begin(), xarray.end());
    auto yresult = std::max_element(yarray.begin(), yarray.end());
    auto zresult = std::max_element(zarray.begin(), zarray.end());
    return glm::vec3(*xresult, *yresult, *zresult);
}

glm::vec3 KDTree::getMinCorner(const std::vector<Photon> &points)
{
    std::vector<float> xarray;
    std::vector<float> yarray;
    std::vector<float> zarray;
    for (unsigned int i = 0; i < points.size(); i++)
    {
        xarray.push_back(points[i].pos.x);
        yarray.push_back(points[i].pos.y);
        zarray.push_back(points[i].pos.z);
    }
    auto xresult = std::min_element(xarray.begin(), xarray.end());
    auto yresult = std::min_element(yarray.begin(), yarray.end());
    auto zresult = std::min_element(zarray.begin(), zarray.end());
    return glm::vec3(*xresult, *yresult, *zresult);
}

KDNode* KDTree::buildHelper(const std::vector<Photon> &points, int depth)
{
    KDNode *mynode = new KDNode();
    std::vector<Photon> sortPoints = points;
    unsigned int axis = depth % 3;
    if (sortPoints.size() > 1)
    {
        if (axis == 0)
        {
            std::sort(sortPoints.begin(), sortPoints.end(), xSort);
        }
        else if (axis == 1)
        {
            std::sort(sortPoints.begin(), sortPoints.end(), ySort);
        }
        else
        {
            std::sort(sortPoints.begin(), sortPoints.end(), zSort);
        }
        int median = sortPoints.size() / 2;
        std::vector<Photon> leftPoints(sortPoints.begin(), sortPoints.begin()+median);
        std::vector<Photon> rightPoints(sortPoints.begin()+median, sortPoints.end());
        mynode->axis = axis;
        mynode->minCorner = getMinCorner(sortPoints);
        mynode->maxCorner = getMaxCorner(sortPoints);
        mynode->leftChild = buildHelper(leftPoints, depth+1);
        mynode->rightChild = buildHelper(rightPoints, depth+1);
    }
    else
    {
        mynode->axis = axis;
        mynode->particles = sortPoints;
        mynode->minCorner = getMinCorner(sortPoints);
        mynode->maxCorner = getMaxCorner(sortPoints);
    }
    return mynode;
}


std::vector<Photon> KDTree::particlesInSphere(glm::vec3 c, float r) const
{
    std::vector<Photon> list;
    rangeSearch(root, list, c, r);
    return list;
}

void KDTree::rangeSearch(KDNode *node, std::vector<Photon> &list, glm::vec3 c, float r) const
{
    if (node->particles.size() != 0 && (node->leftChild == nullptr && node->rightChild == nullptr))
    {
        glm::vec3 p = node->particles[0].pos;
        if (glm::distance(p, c) <= r)
        {
            list.push_back(node->particles[0]);
        }
    }
    else
    {
        if (intersect(node->leftChild, c, r))
        {
            rangeSearch(node->leftChild, list, c, r);
        }
        if (intersect(node->rightChild, c, r))
        {
            rangeSearch(node->rightChild, list, c, r);
        }
    }
}


bool KDTree::intersect(KDNode *node, glm::vec3 c, float r) const
{
    float sumDist = 0;
    float r2 = r*r;
    for (int i = 0; i < 3; i++)
    {
        if (c[i] < node->minCorner[i]) sumDist += glm::length2(c[i] - node->minCorner[i]);
        else if (c[i] > node->maxCorner[i]) sumDist += glm::length2(c[i] - node->maxCorner[i]);
    }
    return sumDist <= r2;
}

void KDTree::clear()
{
    delete root;
    root = nullptr;
}
