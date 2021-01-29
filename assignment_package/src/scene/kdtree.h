#pragma once
#include <la.h>
#include "photon.h"
#include <scene/geometry/mesh.h>

class Triangle;

class KDNode
{
public:
    KDNode();
    ~KDNode();

    KDNode* leftChild;
    KDNode* rightChild;
    unsigned int axis; // Which axis split this node represents
    glm::vec3 minCorner, maxCorner; // The world-space bounds of this node
    std::vector<Photon> particles; // A collection of pointers to the particles contained in this node.
};

class KDTree
{
public:
    KDTree();
    ~KDTree();

    void build(const std::vector<Photon> *points);

    KDNode* buildHelper(const std::vector<Photon> &points, int depth);

    glm::vec3 getMaxCorner(const std::vector<Photon> &points);
    glm::vec3 getMinCorner(const std::vector<Photon> &points);

    void clear();

    std::vector<Photon> particlesInSphere(glm::vec3 c, float r) const; // Returns all the points contained within a sphere with center c and radius r
    void rangeSearch(KDNode *node, std::vector<Photon> &list, glm::vec3 c, float r) const;
    bool intersect(KDNode *node, glm::vec3 c, float r) const;

    KDNode* root;
    glm::vec3 minCorner, maxCorner; // For visualization purposes
    bool isEmpty;
};
