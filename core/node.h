/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a class representing nodes on the triangular lattice. The x-axis runs
// left-right and the y-axis runs northeast-southwest.
// The neighbors of a node are numbered ascendingy in clockwise order: 0=E, 1=NE, 2=NW, 3=W, 4=SW, 5=SE

#ifndef AMOEBOTSIM_CORE_NODE_H_
#define AMOEBOTSIM_CORE_NODE_H_

#include <array>

#include <QtGlobal>

class Node {
public:
    // Node constructors. The default constructor makes the origin node (0, 0),
    // the second constructs a node (x, y), and the third copies the input node.
    Node();
    Node(int x, int y);
    Node(const Node& other);

    // Equality operators; == returns true if the two nodes are equal and != does
    // the opposite.
    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

    // Returns the node adjacent to this one in the given global direction. For
    // more information on global directions, see localparticle.h.
    Node nodeInDir(int dir) const;

    // Returns the node adjacent to this one in the given other node's direction.
    Node nodeTowardsNode(Node other) const;

    int distanceBetween(Node other) const;

    int x, y;
};

// Comparator between two nodes. First compares the nodes' x-coordinates and, in
// case of a tie, compares their y-coordinates.
bool operator<(const Node& v1, const Node& v2);

inline Node::Node()
    : x(0)
    , y(0)
{
}

inline Node::Node(int x, int y)
    : x(x)
    , y(y)
{
}

inline Node::Node(const Node& other)
    : x(other.x)
    , y(other.y)
{
}

inline bool Node::operator==(const Node& other) const
{
    return (x == other.x) && (y == other.y);
}

inline bool Node::operator!=(const Node& other) const
{
    return !operator==(other);
}

inline Node Node::nodeInDir(int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);

    static constexpr std::array<int, 6> xOffset = { { 1, 0, -1, -1, 0, 1 } };
    static constexpr std::array<int, 6> yOffset = { { 0, 1, 1, 0, -1, -1 } };

    return Node(x + xOffset[dir], y + yOffset[dir]);
}

inline Node Node::nodeTowardsNode(Node other) const
{
    int ox = other.x;
    int oy = other.y;
    int xDiff = ox - this->x;
    int yDiff = oy - this->y;
    int zDiff = 0;
    if (xDiff > 0 && yDiff < 0) { // Possibly South-East
        zDiff = std::min(xDiff, -yDiff);
    } else if (xDiff < 0 && yDiff > 0) { // Possibly North-West
        zDiff = -std::min(-xDiff, yDiff);
    }
    if (std::abs(zDiff) > std::abs(xDiff) / 2 && std::abs(zDiff) > std::abs(yDiff) / 2) {
        if (zDiff > 0) {
            return Node(x + 1, y - 1);
        } else {
            return Node(x - 1, y + 1);
        }
    } else if (std::abs(xDiff) > std::abs(yDiff) && std::abs(xDiff) > std::abs(zDiff)) {
        if (xDiff > 0) {
            return Node(x + 1, y);
        } else {
            return Node(x - 1, y);
        }
    } else {
        if (yDiff > 0) {
            return Node(x, y + 1);
        } else {
            return Node(x, y - 1);
        }
    }
}

inline int Node::distanceBetween(Node other) const
{
    int distance = 0;
    while (*(this) != other) {
        other = other.nodeTowardsNode(*(this));
        distance++;
    }
    return distance;
}

inline bool operator<(const Node& v1, const Node& v2)
{
    return (v1.x < v2.x) || (v1.x == v2.x && v1.y < v2.y);
}

#endif // AMOEBOTSIM_CORE_NODE_H_
