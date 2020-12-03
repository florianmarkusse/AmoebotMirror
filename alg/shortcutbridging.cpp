

#include "alg/shortcutbridging.h"


ShortcutBridgingParticle::ShortcutBridgingParticle(const Node& head, const int globalTailDir, 
    const int orientation, AmoebotSystem& system, const int counterMax) 
    : AmoebotParticle(head, globalTailDir, orientation, system) {
}

void ShortcutBridgingParticle::activate()
{
}

int ShortcutBridgingParticle::headMarkColor() const
{
    return 0;
}

int ShortcutBridgingParticle::tailMarkColor() const
{
    return 0;
}

QString ShortcutBridgingParticle::inspectionText() const
{
    return QString();
}

ShortcutBridgingSystem::ShortcutBridgingSystem(unsigned int numParticles, int counterMax)
{
    // In order to enclose an area that's roughly 3.7x the # of particles using a
    // regular hexagon, the hexagon should have side length 1.4*sqrt(# particles).
    int sideLen = static_cast<int>(std::round(1.4 * std::sqrt(numParticles)));
    Node boundNode(0, 0);
    for (int dir = 0; dir < 6; ++dir) {
        for (int i = 0; i < sideLen; ++i) {
            insert(new Object(boundNode));
            boundNode = boundNode.nodeInDir(dir);
        }
    }

    // Let s be the bounding hexagon side length. When the hexagon is created as
    // above, the nodes (x,y) strictly within the hexagon have (i) -s < x < s,
    // (ii) 0 < y < 2s, and (iii) 0 < x+y < 2s. Choose interior nodes at random to
    // place particles, ensuring at most one particle is placed at each node.
    std::set<Node> occupied;
    while (occupied.size() < numParticles) {
        // First, choose an x and y position at random from the (i) and (ii) bounds.
        int x = randInt(-sideLen + 1, sideLen);
        int y = randInt(1, 2 * sideLen);
        Node node(x, y);

        // If the node satisfies (iii) and is unoccupied, place a particle there.
        if (0 < x + y && x + y < 2 * sideLen
            && occupied.find(node) == occupied.end()) {
            insert(new ShortcutBridgingParticle(node, -1, randDir(), *this, counterMax));
            occupied.insert(node);
        }
    }
}

