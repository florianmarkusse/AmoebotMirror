

#include "alg/shortcutbridging.h"

ShortcutBridgingParticle::ShortcutBridgingParticle(const Node head,
    const int globalTailDir, const int orientation, AmoebotSystem& system, const double lambda, const double c)
    : AmoebotParticle(head, globalTailDir, orientation, system)
    , lambda(lambda)
    , q(0)
    , numNbrsBefore(0)
    , flag(false)
    , nodeBefore(head)
    , c(c)

{
}

void ShortcutBridgingParticle::activate()
{
    if (isContracted()) {
        int expandDir = randDir(); // Select a random neighboring location.
        q = randDouble(0, 1); // Select a random q in (0,1).

        if (canExpand(expandDir) && !hasExpNbr() && !hasAnchorObjectAtNode(head)) {
            // Count neighbors in original position and expand.
            numNbrsBefore = nbrCount(uniqueLabels());
            nodeBefore = head;
            expand(expandDir);
            flag = !hasExpNbr();
        }
    } else { // isExpanded().
        if (!flag || numNbrsBefore == 5) {
            contractHead();
        } else {
            // Count neighbors in new position and compute the set S.
            int numNbrsAfter = nbrCount(headLabels());
            std::vector<int> S;
            for (const int label : { headLabels()[4], tailLabels()[4] }) {
                if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
                    S.push_back(label);
                }
            }

            // If the conditions are satisfied, contract to the new position;
            // otherwise, contract back to the original one.
            if ((checkProp1(S) || checkProp2(S))) {

                int pDiff = numNbrsAfter - numNbrsBefore;
                int deltaP = 0;
                bool countEmpty = hasNbrAtLabel(tailLabels()[0]);
                for (int i = 1; i < tailLabels().size(); i++) {

                    if (hasNbrAtLabel(tailLabels()[i]) && !hasNbrAtLabel(tailLabels()[i - 1]) && !countEmpty) {
                        deltaP++;
                    }

                    if (!hasNbrAtLabel(tailLabels()[i]) && hasNbrAtLabel(tailLabels()[i - 1]) && countEmpty) {
                        deltaP++;
                    }
                }

                std::vector<int> R1;
                for (int label : tailLabels()) {
                    if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
                        R1.push_back(label);
                    }
                }

                std::vector<int> R2;
                for (const int label : headLabels()) {
                    if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
                        R2.push_back(label);
                    }
                }

                int sumR = 0;
                for (const int label : R1) {
                    if (std::find(S.begin(), S.end(), label) == S.end()) {
                        int G = hasTraversableObjectAtLabel(label);
                        int neighbors = 0;
                        if (label != 0 && std::find(R1.begin(), R1.end(), label - 1) != R1.end()) {
                            neighbors++;
                        } else if (label == 0 && std::find(R1.begin(), R1.end(), 9) != R1.end()) {
                            neighbors++;
                        }

                        if (label != 9 && std::find(R1.begin(), R1.end(), label + 1) != R1.end()) {
                            neighbors++;
                        } else if (label == 9 && std::find(R1.begin(), R1.end(), 0) != R1.end()) {
                            neighbors++;
                        }

                        if (neighbors == 2) {
                            sumR -= G;
                        } else if (neighbors == 0) {
                            sumR += G;
                        }
                    }
                }

                for (const int label : R2) {
                    if (std::find(S.begin(), S.end(), label) == S.end()) {
                        int G = hasTraversableObjectAtLabel(label);
                        int neighbors = 0;
                        if (label != 0 && std::find(R2.begin(), R2.end(), label - 1) != R2.end()) {
                            neighbors++;
                        } else if (label == 0 && std::find(R2.begin(), R2.end(), 9) != R2.end()) {
                            neighbors++;
                        }

                        if (label != 9 && std::find(R2.begin(), R2.end(), label + 1) != R2.end()) {
                            neighbors++;
                        } else if (label == 9 && std::find(R2.begin(), R2.end(), 0) != R2.end()) {
                            neighbors++;
                        }

                        if (neighbors == 2) {
                            sumR += G;
                        } else if (neighbors == 0) {
                            sumR -= G;
                        }
                    } else {
                        int G = hasTraversableObjectAtLabel(label);
                        int neighbors1 = 0;
                        int neighbors2 = 0;
                        if (label != 0 && std::find(R2.begin(), R2.end(), label - 1) != R2.end()) {
                            neighbors2++;
                        } else if (label == 0 && std::find(R2.begin(), R2.end(), 9) != R2.end()) {
                            neighbors2++;
                        }

                        if (label != 9 && std::find(R2.begin(), R2.end(), label + 1) != R2.end()) {
                            neighbors2++;
                        } else if (label == 9 && std::find(R2.begin(), R2.end(), 0) != R2.end()) {
                            neighbors2++;
                        }

                        if (label != 0 && std::find(R1.begin(), R1.end(), label - 1) != R1.end()) {
                            neighbors1++;
                        } else if (label == 0 && std::find(R1.begin(), R1.end(), 9) != R1.end()) {
                            neighbors1++;
                        }

                        if (label != 9 && std::find(R1.begin(), R1.end(), label + 1) != R1.end()) {
                            neighbors1++;
                        } else if (label == 9 && std::find(R1.begin(), R1.end(), 0) != R1.end()) {
                            neighbors1++;
                        }

                        if (neighbors1 == 1) {
                            sumR -= G;
                        } else if (neighbors2 == 1) {
                            sumR += G;
                        }
                    }
                }

                int gDiff = deltaP * (hasTraversableObjectAtNode(head) - hasTraversableObjectAtNode(nodeBefore)) + sumR;

                if (q < pow(lambda, pDiff) * pow(pow(lambda, c - 1), gDiff)) {
                    contractTail();
                } else {
                    contractHead();
                }

            } else {
                contractHead();
            }
        }
    }
}

QString ShortcutBridgingParticle::inspectionText() const
{
    QString text;
    text += "Global Info:\n";
    text += "  head: (" + QString::number(head.x) + ", "
        + QString::number(head.y) + ")\n";
    text += "  orientation: " + QString::number(orientation) + "\n";
    text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
    text += "Properties:\n";
    text += "  lambda = " + QString::number(lambda) + ",\n";
    text += "  q in (0,1) = " + QString::number(q) + ",\n";
    text += "  flag = " + QString::number(flag) + ".\n";

    if (isContracted()) {
        text += "Contracted properties:\n";
        text += "  #neighbors before = " + QString::number(numNbrsBefore) + ",\n";
    } else { // isExpanded().
        text += "Expanded properties:\n";
        text += "  #neighbors before = " + QString::number(numNbrsBefore) + ",\n";
        text += "  #neighbors after = " + QString::number(nbrCount(headLabels()))
            + ".\n";
    }

    return text;
}

ShortcutBridgingParticle& ShortcutBridgingParticle::nbrAtLabel(int label) const
{
    return AmoebotParticle::nbrAtLabel<ShortcutBridgingParticle>(label);
}

bool ShortcutBridgingParticle::hasExpNbr() const
{
    for (const int label : uniqueLabels()) {
        if (hasNbrAtLabel(label) && nbrAtLabel(label).isExpanded()) {
            return true;
        }
    }

    return false;
}

bool ShortcutBridgingParticle::hasExpHeadAtLabel(const int label) const
{
    return hasNbrAtLabel(label) && nbrAtLabel(label).isExpanded()
        && nbrAtLabel(label).pointsAtMyHead(*this, label);
}

int ShortcutBridgingParticle::nbrCount(std::vector<int> labels) const
{
    int numNbrs = 0;
    for (const int label : labels) {
        if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
            ++numNbrs;
        }
    }

    return numNbrs;
}

bool ShortcutBridgingParticle::checkProp1(std::vector<int> S) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(S.size() <= 2);
    Q_ASSERT(flag); // Not required, but equivalent/cleaner for implementation.

    if (S.size() == 0) {
        return false; // S has to be nonempty for Property 1.
    } else {
        const std::vector<int> labels = uniqueLabels();
        std::set<int> adjNbrs;

        // Starting from the particles in S, sweep out and mark connected neighbors.
        for (int s : S) {
            adjNbrs.insert(s);
            int i = distance(labels.begin(), find(labels.begin(), labels.end(), s));

            // First sweep counter-clockwise, stopping when an unoccupied position or
            // expanded head is encountered.
            for (uint offset = 1; offset < labels.size(); ++offset) {
                int label = labels[(i + offset) % labels.size()];
                if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
                    adjNbrs.insert(label);
                } else {
                    break;
                }
            }

            // Then sweep clockwise.
            for (uint offset = 1; offset < labels.size(); ++offset) {
                int label = labels[(i - offset + labels.size()) % labels.size()];
                if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
                    adjNbrs.insert(label);
                } else {
                    break;
                }
            }
        }

        // If all neighbors are connected to a particle in S by a path through the
        // neighborhood, then the number of labels in adjNbrs should equal the total
        // number of neighbors.
        return adjNbrs.size() == (uint)nbrCount(labels);
    }
}

bool ShortcutBridgingParticle::checkProp2(std::vector<int> S) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(S.size() <= 2);
    Q_ASSERT(flag); // Not required, but equivalent/cleaner for implementation.

    if (S.size() != 0) {
        return false; // S has to be empty for Property 2.
    } else {
        const int numHeadNbrs = nbrCount(headLabels());
        const int numTailNbrs = nbrCount(tailLabels());

        // Check if the head's neighbors are connected.
        int numAdjHeadNbrs = 0;
        bool seenNbr = false;
        for (const int label : headLabels()) {
            if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
                seenNbr = true;
                ++numAdjHeadNbrs;
            } else if (seenNbr) {
                break;
            }
        }

        // Check if the tail's neighbors are connected.
        int numAdjTailNbrs = 0;
        seenNbr = false;
        for (const int label : tailLabels()) {
            if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
                seenNbr = true;
                ++numAdjTailNbrs;
            } else if (seenNbr) {
                break;
            }
        }

        // Property 2 is satisfied if both the head and tail have at least one
        // neighbor and all head (tail) neighbors are connected.
        return (numHeadNbrs > 0) && (numTailNbrs > 0) && (numHeadNbrs == numAdjHeadNbrs) && (numTailNbrs == numAdjTailNbrs);
    }
}

ShortcutBridgingSystem::ShortcutBridgingSystem(int numParticles, double lambda, double c, Shape shape)
    : c(c)
{
    Q_ASSERT(lambda >= 0);

    switch (shape) {
    case Shape::V:
        drawV(numParticles, lambda, c);
        break;
    case Shape::Z:
        drawZ(numParticles, lambda, c);
        break;
    case Shape::Circle:
        drawCircle(numParticles, lambda, c);
    }

    // Set up metrics.
    _measures.push_back(new ShortcutPerimeterMeasure("Perimeter", 1, *this));
    _measures.push_back(new ShortcutGapPerimeterMeasure("Gap Perimeter", 1, *this));
    _measures.push_back(new WeightedPerimeterMeasure("Weighted measure", 1, *this));
}

bool ShortcutBridgingSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if (!isConnected(particles)) {
        return true;
    }
#endif

    return false;
}

void ShortcutBridgingSystem::drawV(int numParticles, double lambda, double c)
{
    // Draw v on its head.
    int lineSize = (numParticles - 3) / 4;
    int originalDir = 1; // NorthEast
    for (int d = 0; d < 10; d++) {
        Node boundNode(-1 * d, 0);
        int dir = originalDir;
        for (int i = 0; i < lineSize + d - 1; ++i) {
            if (d == 1 && i == 0) {
                insert(new Object(boundNode, true, true));
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            } else {
                insert(new Object(boundNode, true));
                if (d < 2) {
                    insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
                }
            }
            boundNode = boundNode.nodeInDir(dir);
        }
        dir = (dir + 5) % 6;
        for (int i = 0; i < d + 1; ++i) {
            insert(new Object(boundNode, true));
            if (d < 2) {
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            }
            boundNode = boundNode.nodeInDir(dir);
        }
        dir = (dir + 5) % 6;
        for (int i = 0; i < lineSize + d; ++i) {
            if (d == 1 && i == lineSize + d - 1) {
                insert(new Object(boundNode, true, true));
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            } else {
                insert(new Object(boundNode, true));
                if (d < 2) {
                    insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
                }
            }
            boundNode = boundNode.nodeInDir(dir);
        }
    }
}

void ShortcutBridgingSystem::drawZ(int numParticles, double lambda, double c)
{
    // Draw v on its head.
    int lineSize = (numParticles - 3) / 4;
    int originalDir = 1; // NorthEast
    for (int d = 0; d < 10; d++) {
        Node boundNode(-1 * d, 0);
        int dir = originalDir;
        for (int i = 0; i < lineSize + d - 1; ++i) {
            if (d == 1 && i == 0) {
                insert(new Object(boundNode, true, true));
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            } else {
                insert(new Object(boundNode, true));
                if (d < 2) {
                    insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
                }
            }
            boundNode = boundNode.nodeInDir(dir);
        }
        dir = (dir + 5) % 6;
        for (int i = 0; i < d + 1; ++i) {
            insert(new Object(boundNode, true));
            if (d < 2) {
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            }
            boundNode = boundNode.nodeInDir(dir);
        }
        dir = (dir + 5) % 6;
        int middleLine = (lineSize - 8)/2 + d + 1;
        for (int i = 0; i < lineSize + d; ++i) {
            insert(new Object(boundNode, true));
            if (i < middleLine && d < 2) {
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            } else if (i > middleLine - 3 && i < middleLine && d >= 2) {
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            } else if (i >= middleLine && i < lineSize - 8 + d && d > 7) {
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            }
            boundNode = boundNode.nodeInDir(dir);
        }
    }
    // Draw last leg to form Z
    Node startNode(lineSize+10,0);
    for (int d = 0; d < 10; d++) {
        Node boundNode(startNode.x, startNode.y);
        for (int i = 0; i < lineSize - d + 9; i++) {
            if (d == 8 && i == lineSize - d + 8) {
                insert(new Object(boundNode, true, true));
                insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
            } else {
                insert(new Object(boundNode, true));
                if (d > 7) {
                    insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
                }
            }
            boundNode = boundNode.nodeInDir(1);
        }
        startNode = startNode.nodeInDir(2);
    }
}

void ShortcutBridgingSystem::drawCircle(int numParticles, double lambda, double c)
{
}

ShortcutPerimeterMeasure::ShortcutPerimeterMeasure(const QString name, const unsigned int freq, ShortcutBridgingSystem& system)
    : Measure(name, freq)
    , _system(system)
{
}

double ShortcutPerimeterMeasure::calculate() const
{
    int numEdges = 0;
    for (const auto& p : _system.particles) {
        auto comp_p = dynamic_cast<ShortcutBridgingParticle*>(p);
        auto tailLabels = comp_p->isContracted() ? comp_p->uniqueLabels()
                                                 : comp_p->tailLabels();
        for (const int label : tailLabels) {
            if (comp_p->hasNbrAtLabel(label) && !comp_p->hasExpHeadAtLabel(label)) {
                ++numEdges;
            }
        }
    }

    return (3 * _system.size()) - (numEdges / 2) - 3;
}

ShortcutGapPerimeterMeasure::ShortcutGapPerimeterMeasure(const QString name, const unsigned int freq, ShortcutBridgingSystem& system)
    : Measure(name, freq)
    , _system(system)
{
}

double ShortcutGapPerimeterMeasure::calculate() const
{
    int gapEdges = 0;

    for (const auto& p : _system.particles) {
        auto comp_p = dynamic_cast<ShortcutBridgingParticle*>(p);
        auto tailLabels = comp_p->isContracted() ? comp_p->uniqueLabels()
                                                 : comp_p->tailLabels();
        Node positionNode = comp_p->isContracted() ? comp_p->head
                                                   : comp_p->tail();
        auto particlePair = _system.objectMap.find(positionNode);

        for (const int label : tailLabels) {
            if (comp_p->hasNbrAtLabel(label) && !comp_p->hasExpHeadAtLabel(label)) {

                Node nodeNeighbour = comp_p->nbrAtLabel(label).isContracted()
                    ? comp_p->nbrAtLabel(label).head
                    : comp_p->nbrAtLabel(label).tail();
                auto neighbourPair = _system.objectMap.find(nodeNeighbour);
                // Land
                if (particlePair != _system.objectMap.end()) {
                    // Land-Land: do nothing
                    if (neighbourPair != _system.objectMap.end()) {
                    }
                    // Land-Gap: Add 1
                    else {
                        gapEdges++;
                    }
                }
                // Gap
                else {
                    // Gap-Land: Add 1
                    if (neighbourPair != _system.objectMap.end()) {
                        gapEdges++;
                    }
                    // Gap-Gap: add 2
                    else {
                        gapEdges += 2;
                    }
                }
            }
        }
    }
    auto res = gapEdges * _system.c / 4;
    return res;
}

WeightedPerimeterMeasure::WeightedPerimeterMeasure(const QString name, const unsigned int freq, ShortcutBridgingSystem& system)
    : Measure(name, freq)
    , _system(system)
{
}

double WeightedPerimeterMeasure::calculate() const
{
    size_t size = _system.getMeasure("Perimeter")._history.size();
    double perimeter = _system.getMeasure("Perimeter")._history[size - 1];

    size = _system.getMeasure("Gap Perimeter")._history.size();
    double gapPerimeter = _system.getMeasure("Gap Perimeter")._history[size - 1];

    return perimeter + (_system.c - 1) * gapPerimeter;
}
