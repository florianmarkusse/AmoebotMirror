#include "alg/shortcutbridging.h"

int east = 0;
int northEast = 1;
int northWest = 2;
int west = 3;
int southWest = 4;
int southEast = 5;

bool nextStartNode(bool goEast, int topNodesLeft, Node& startNode);
void nextGapStarts(int i, int gapSize, Node& gapStarts);
bool nextFillerNode(Node& fillerNode, int yMax, int diagonalDirection, int xDirection, bool placeLeft);
bool thinNextStartNode(bool goEast, Node& startNode, int gapSizeInBetweenTopNodes, int topNodesLeft);

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
    //return;
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
                        int G = !hasTraversableObjectAtLabel(label);
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
                        int G = !hasTraversableObjectAtLabel(label);
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
                        int G = !hasTraversableObjectAtLabel(label);
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

                int gDiff = deltaP * (!hasTraversableObjectAtNode(nodeBefore) - !hasTraversableObjectAtNode(head)) + sumR;

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

void ShortcutBridgingSystem::getOptimalV(int numParticles, double lambda, double c)
{
    int lineSize = (numParticles - 3) / 4;

    int startingGap = 1;
    double bestResult = UINT_MAX;

    while (startingGap < lineSize) {
        drawV(numParticles, lambda, c);
        double result = thickBridge(lineSize, startingGap);
        if (result < bestResult) {
            bestResult = result;
        }
        removeParticles();
        startingGap++;
    }

    startingGap = 1;

    while (startingGap < lineSize) {
        drawV(numParticles, lambda, c);
        double result = thinBrigde(lineSize, startingGap);
        if (result < bestResult) {
            bestResult = result;
        }
        removeParticles();
        startingGap++;
    }

    optimalWeightedPerimeter = bestResult;
}

void ShortcutBridgingSystem::drawZTest(int numParticles, double lambda, double c)
{
    drawZ(numParticles, lambda, c);
    removeParticles(false);

    int lineSize = (numParticles - 13) / 6 + 2;
    int originalDir = 1; // NorthEast

    // Set a high value for bestResult. This value should be higher than any metric result.
    double bestResult = numParticles * numParticles * numParticles * c * c;
    int bestI = 0;
    for (int i = 0; i < lineSize + 1; i++) {
        Node startNode;
        Node endNode;
        std::vector<Node> candidates;
        for (int j = 0; j < i; j++) {
            insert(new ShortcutBridgingParticle(Node(-1,j), -1, randDir(), *this, lambda, c));
            insert(new ShortcutBridgingParticle(Node(0,j), -1, randDir(), *this, lambda, c));
            insert(new ShortcutBridgingParticle(Node(lineSize + 2, lineSize + 8 - j), -1, randDir(), *this, lambda, c));
            insert(new ShortcutBridgingParticle(Node(lineSize + 1, lineSize + 8 - j), -1, randDir(), *this, lambda, c));
        }
        startNode = Node(-1,i);
        endNode = Node(lineSize + 2, lineSize + 8 - i);
        insert(new ShortcutBridgingParticle(Node(startNode.x, startNode.y), -1, randDir(), *this, lambda, c));
        insert(new ShortcutBridgingParticle(Node(endNode.x, endNode.y), -1, randDir(), *this, lambda, c));

        int particlesRemaining = numParticles - 2 - 4*i;

        Node drawNode = startNode.nodeTowardsNode(endNode);
        Node endDrawNode = endNode.nodeTowardsNode(startNode);
        // Draw line from start to end
        while (drawNode.nodeTowardsNode(endDrawNode) != endDrawNode) {
            insert(new ShortcutBridgingParticle(Node(drawNode.x, drawNode.y), -1, randDir(), *this, lambda, c));
            --particlesRemaining;
            candidates.push_back(Node(drawNode.x+1, drawNode.y));
            candidates.push_back(Node(drawNode.x, drawNode.y+1));
            candidates.push_back(Node(drawNode.x-1, drawNode.y+1));
            candidates.push_back(Node(drawNode.x-1, drawNode.y));
            candidates.push_back(Node(drawNode.x, drawNode.y-1));
            candidates.push_back(Node(drawNode.x+1, drawNode.y-1));
            insert(new ShortcutBridgingParticle(Node(endDrawNode.x, endDrawNode.y), -1, randDir(), *this, lambda, c));
            --particlesRemaining;
            candidates.push_back(Node(endDrawNode.x+1, endDrawNode.y));
            candidates.push_back(Node(endDrawNode.x, endDrawNode.y+1));
            candidates.push_back(Node(endDrawNode.x-1, endDrawNode.y+1));
            candidates.push_back(Node(endDrawNode.x-1, endDrawNode.y));
            candidates.push_back(Node(endDrawNode.x, endDrawNode.y-1));
            candidates.push_back(Node(endDrawNode.x+1, endDrawNode.y-1));
            Node oldDrawNode = drawNode;
            drawNode = drawNode.nodeTowardsNode(endDrawNode);
            endDrawNode = endDrawNode.nodeTowardsNode(oldDrawNode);
        }
        insert(new ShortcutBridgingParticle(Node(drawNode.x, drawNode.y), -1, randDir(), *this, lambda, c));
        --particlesRemaining;
        if (drawNode != endDrawNode) {
            insert(new ShortcutBridgingParticle(Node(endDrawNode.x, endDrawNode.y), -1, randDir(), *this, lambda, c));
            --particlesRemaining;
        }

        while (particlesRemaining > 0) {
            Node candidate = candidates.front();
            if (particleMap.find(candidate) == particleMap.end()) {
                insert(new ShortcutBridgingParticle(candidate, -1, randDir(), *this, lambda, c));
                --particlesRemaining;
                candidates.push_back(Node(candidate.x+1, candidate.y));
                candidates.push_back(Node(candidate.x, candidate.y+1));
                candidates.push_back(Node(candidate.x-1, candidate.y+1));
                candidates.push_back(Node(candidate.x-1, candidate.y));
                candidates.push_back(Node(candidate.x, candidate.y-1));
                candidates.push_back(Node(candidate.x+1, candidate.y-1));
            }
            candidates.erase(candidates.begin());
        }

        double perim = getMeasure("Perimeter").calculate();
        double gapPerim = getMeasure("Gap Perimeter").calculate();

        double result = (perim + (c - 1) * gapPerim);

        if (result < bestResult) {
            bestResult = result;
            bestI = i;
            qDebug(std::to_string(bestResult).c_str());
            qDebug(std::to_string(bestI).c_str());
        }

        removeParticles(false);
    }

    // Todo use optimal value.
    optimalWeightedPerimeter = bestResult;
}

void ShortcutBridgingSystem::getOptimalHexagon(int numParticles, double lambda, double c)
{
    drawHexagon(numParticles, lambda, c);
    removeParticles(false);

    int sideLength = 2;
    int resultingParticleSize = 6 * sideLength + 6 * (sideLength - 1);
    while (resultingParticleSize < numParticles) {
        sideLength++;
        resultingParticleSize = 6 * sideLength + 6 * (sideLength - 1);
    }
    sideLength--;

    Node leftAnchorNode = { 0, 0 };
    Node rightAnchorNode = { sideLength, 0 };
    for (int i = 0; i < sideLength; i++) {
        leftAnchorNode = leftAnchorNode.nodeInDir(northWest);
        rightAnchorNode = rightAnchorNode.nodeInDir(northEast);
    }
    leftAnchorNode = leftAnchorNode.nodeInDir(west);
    rightAnchorNode = rightAnchorNode.nodeInDir(east);

    int totalParticles = sideLength * 6 + (sideLength + 1) * 6;

    double bestResult = UINT_MAX;

    double result = middleLineHexagon(totalParticles, leftAnchorNode, rightAnchorNode, lambda);
    removeParticles(false);
    if (result < bestResult) {
        bestResult = result;
    }

    for (int startSideLength = 1; startSideLength <= sideLength + 1; startSideLength++) {
        result = drawTopHexagon(totalParticles, leftAnchorNode, rightAnchorNode,
            startSideLength, lambda, sideLength);
        removeParticles(false);
        if (result < bestResult) {
            bestResult = result;
        }
    }

    optimalWeightedPerimeter = bestResult;

    removeParticles();
}

ShortcutBridgingSystem::ShortcutBridgingSystem(int numParticles, double lambda, double c, Shape shape)
    : c(c)
{
    Q_ASSERT(lambda >= 0);

    // Set up metrics.
    _measures.push_back(new ShortcutPerimeterMeasure("Perimeter", 1, *this));
    _measures.push_back(new ShortcutGapPerimeterMeasure("Gap Perimeter", 1, *this));
    _measures.push_back(new WeightedPerimeterMeasure("Weighted measure", 1, *this));

    switch (shape) {
    case Shape::V:
        getOptimalV(numParticles, lambda, c);
        drawV(numParticles, lambda, c);
        break;
    case Shape::Z:
        drawZTest(numParticles, lambda, c);
        removeParticles();
        drawZ(numParticles, lambda, c);
        break;
    case Shape::Hexagon:
        getOptimalHexagon(numParticles, lambda, c);
        drawHexagon(numParticles, lambda, c);
        break;
    case Shape::HexagonIsland:
        terminateEveryXActivations = 2000000;
        drawHexagon(numParticles, lambda, c);
        drawHexagonIsland(numParticles);
        break;
    case Shape::VSmallIslands:
        terminateEveryXActivations = 2000000;
        drawVSmallIslands(numParticles, lambda, c);
        break;
    case Shape::VBigIsland:
        terminateEveryXActivations = 2000000;
        drawVBigIslands(numParticles, lambda, c);
        break;
    case Shape::VObstacle:
        terminateEveryXActivations = 2000000;
        drawVObstacles(numParticles, lambda, c);
        break;
    }
}

bool ShortcutBridgingSystem::hasTerminated() const
{
    if (terminateEveryXActivations > 0) {
        auto counts = getCount("# Activations")._value;
        if (counts > 0 && counts % terminateEveryXActivations == 0) {
            return true;
        }
    }

    size_t size = getMeasure("Weighted measure")._history.size();
    if (size < 1) {
        return false;
    }
    double measure = getMeasure("Weighted measure")._history[size - 1];

    // alpha values
    // for V: 1.08
    // for Z: ---
    // for Hexagon: 1.5
    if (measure <= optimalWeightedPerimeter * 1.5) {
        return true;
    }
    return false;
}

void ShortcutBridgingSystem::drawVGeneric(int numParticles, double lambda, double c, bool smallIslands, bool bigIslands, bool obstacle)
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

    // Draw obstacle
    if (obstacle) {
        insert(new Object(Node(2, lineSize - 4)));
        insert(new Object(Node(2, lineSize - 5)));
        insert(new Object(Node(3, lineSize - 5)));
    }

    // Draw big island
    if (bigIslands) {
        for (int i = 3; i < lineSize - 2; i++) {
            for (int h = lineSize - 3 - i; h >= 0 && h >= 0; h--) {
                insert(new Object(Node(i, h), true));
            }
        }
    }

    // Draw small islands
    if (smallIslands) {
        // Top island
        for (int i = 3; i < 6; i++) {
            for (int h = lineSize - 3 - i; h >= lineSize - 8 && h >= 0; h--) {
                insert(new Object(Node(i, h), true));
            }
        }

        // Left second layer island
        for (int i = 3; i < 6; i++) {
            for (int h = lineSize - 7 - i; h >= lineSize - 12 && h >= 0; h--) {
                insert(new Object(Node(i, h), true));
            }
        }

        // Right second layer island
        for (int i = 7; i < 10; i++) {
            for (int h = lineSize - 3 - i; h >= lineSize - 12 && h >= 0; h--) {
                insert(new Object(Node(i, h), true));
            }
        }
    }
}

void ShortcutBridgingSystem::drawV(int numParticles, double lambda, double c)
{
    drawVGeneric(numParticles, lambda, c, false, false, false);
}

void ShortcutBridgingSystem::drawVObstacles(int numParticles, double lambda, double c)
{
    drawVGeneric(numParticles, lambda, c, false, false, true);
}

void ShortcutBridgingSystem::drawVSmallIslands(int numParticles, double lambda, double c)
{
    drawVGeneric(numParticles, lambda, c, true, false, false);
}

void ShortcutBridgingSystem::drawVBigIslands(int numParticles, double lambda, double c)
{
    drawVGeneric(numParticles, lambda, c, false, true, false);
}

void ShortcutBridgingSystem::drawZ(int numParticles, double lambda, double c)
{
	// Draw v on its head.
    int lineSize = (numParticles - 13) / 6 + 2;
	int originalDir = 1; // NorthEast
	for (int d = 0; d < 10; d++) {
		Node boundNode(-1 * d, 0);
		int dir = originalDir;
		for (int i = 0; i < lineSize + d - 1; ++i) {
			if (d == 1 && i == 0) {
				insert(new Object(boundNode, true, true));
				insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
			}
			else {
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
		int middleLine = (lineSize - 8) / 2 + d + 1;
		for (int i = 0; i < lineSize + d; ++i) {
			insert(new Object(boundNode, true));
			if (i < middleLine && d < 2) {
				insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
			}
			else if (i > middleLine - 3 && i < middleLine && d >= 2) {
				insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
			}
			else if (i >= middleLine && i < lineSize - 8 + d && d > 7) {
				insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
			}
			boundNode = boundNode.nodeInDir(dir);
		}
	}
	// Draw last leg to form Z
	Node startNode(lineSize + 10, 0);
	for (int d = 0; d < 10; d++) {
		Node boundNode(startNode.x, startNode.y);
		for (int i = 0; i < lineSize - d + 9; i++) {
			if (d == 8 && i == lineSize - d + 8) {
				insert(new Object(boundNode, true, true));
				insert(new ShortcutBridgingParticle(Node(boundNode.x, boundNode.y), -1, randDir(), *this, lambda, c));
			}
			else {
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

void ShortcutBridgingSystem::drawHexagon(int numParticles, double lambda, double c)
{
    int x = 2;
    int result = 6 * x + 6 * (x - 1);
    while (result < numParticles) {
        x++;
        result = 6 * x + 6 * (x - 1);
    }

    for (int i = 0; i < 6; i++) {
        Node node(0, 0 - i);
        for (int j = 0; j < 6; j++) {
            for (int z = 0; z < x - 1 + i; z++) {
                if (i == 1 && (j == 2 || j == 5) && z == 0) {
                    insert(new Object(node, true, true));
                } else {
                    insert(new Object(node, true));
                }
                if (i < 2) {
                    insert(new ShortcutBridgingParticle(Node(node.x, node.y), -1, randDir(), *this, lambda, c));
                }
                node = node.nodeInDir(j);
            }
        }
    }
}

void ShortcutBridgingSystem::drawHexagonIsland(int numParticles)
{
    int x = 2;
    int result = 6 * x + 6 * (x - 1);
    while (result < numParticles) {
        x++;
        result = 6 * x + 6 * (x - 1);
    }

    for (int i = 0; i < x - 4; i++) {
        Node node(0, 3 + i);
        for (int j = 0; j < 6; j++) {
            for (int z = 0; z < x - 4 - i; z++) {
                insert(new Object(node, true));
                node = node.nodeInDir(j);
            }
        }
    }

    insert(new Object(Node(0, 3 + x - 4), true));
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

void ShortcutBridgingSystem::moveParticle(const Node& startNode, const Node& endNode)
{
    auto p = getParticleAt(startNode);
    Node startLocation = Node(p->head.x, p->head.y);
    p->head.x = endNode.x;
    p->head.y = endNode.y;
    changeLocation(startLocation);
}

double ShortcutBridgingSystem::thickBridge(int lineSize, int maxGapSize)
{
    int topNodes = 3;
    int topNodesLeft = 3;

    bool goEast = true;

    int gapSize = 1;

    Node startNode = Node(-1, lineSize);
    Node gapStarts = Node(1, lineSize - 2 - (gapSize - 1));

    while (gapSize < maxGapSize) {
        for (int i = gapSize; i > 0; i--) {
            moveParticle(startNode, gapStarts);
            topNodesLeft--;

            if (topNodesLeft == 0) {
                startNode = Node(-1, lineSize - (topNodes - 2));

                goEast = true;

                topNodes++;
                topNodesLeft = topNodes;
            } else {
                goEast = nextStartNode(goEast, topNodesLeft, startNode);
            }
            nextGapStarts(i, gapSize, gapStarts);
        }
        gapSize++;
    }

    bool placeLeft = false;
    int yMax = startNode.y - 1;
    Node fillerNodeLeft = { -2, 0 };
    Node fillerNodeRight = { fillerNodeLeft.x + 4 + lineSize, 0 };

    while (topNodesLeft > 0) {
        if (placeLeft) {
            moveParticle(startNode, fillerNodeLeft);
        } else {
            moveParticle(startNode, fillerNodeRight);
        }
        topNodesLeft--;

        goEast = nextStartNode(goEast, topNodesLeft, startNode);

        if (placeLeft) {
            placeLeft = nextFillerNode(fillerNodeLeft, yMax, northEast, -1, placeLeft);
        } else {
            placeLeft = nextFillerNode(fillerNodeRight, yMax, northWest, 1, placeLeft);
        }
    }

    double perim = getMeasure("Perimeter").calculate();
    double gapPerim = getMeasure("Gap Perimeter").calculate();

    return (perim + (c - 1) * gapPerim);
}

double ShortcutBridgingSystem::thinBrigde(int lineSize, int gapSize)
{
    int topNodes = 3;
    int topNodesLeft = 3;

    int gapSizeInBetweenTopNodes = -1;
    bool goEast = true;

    Node startNode = Node(-1, lineSize);
    Node gapStarts = Node(1, lineSize - 2 - (gapSize - 1));

    // Create bridge.
    for (int i = gapSize; i > 0; i--) {
        moveParticle(startNode, gapStarts);
        topNodesLeft--;

        if (topNodesLeft == 0) {
            startNode = Node(-1, lineSize - (topNodes - 2));
            gapSizeInBetweenTopNodes++;

            goEast = true;

            topNodes++;
            topNodesLeft = 4;
        } else {
            if (gapSizeInBetweenTopNodes > 0) {
                goEast = thinNextStartNode(goEast, startNode, gapSizeInBetweenTopNodes, topNodesLeft);
            } else {
                goEast = nextStartNode(goEast, topNodesLeft, startNode);
            }
        }
        gapStarts = gapStarts.nodeInDir(0);
    }

    bool placeLeft = false;
    int yMax = gapStarts.y;
    Node fillerNodeLeft = { -2, 0 };
    Node fillerNodeRight = { fillerNodeLeft.x + 4 + lineSize, 0 };

    // Create filler.
    while (startNode.y > gapStarts.y) {
        if (placeLeft) {
            moveParticle(startNode, fillerNodeLeft);
            placeLeft = nextFillerNode(fillerNodeLeft, yMax, northEast, -1, placeLeft);
        } else {
            moveParticle(startNode, fillerNodeRight);
            placeLeft = nextFillerNode(fillerNodeRight, yMax, northWest, 1, placeLeft);
        }
        topNodesLeft--;

        if (topNodesLeft == 0) {
            startNode = Node(-1, lineSize - (topNodes - 2));
            gapSizeInBetweenTopNodes++;

            goEast = true;

            topNodes++;
            topNodesLeft = 4;
        } else {
            if (gapSizeInBetweenTopNodes > 0) {
                goEast = thinNextStartNode(goEast, startNode, gapSizeInBetweenTopNodes, topNodesLeft);
            } else {
                goEast = nextStartNode(goEast, topNodesLeft, startNode);
            }
        }
    }

    double perim = getMeasure("Perimeter").calculate();
    double gapPerim = getMeasure("Gap Perimeter").calculate();

    return (perim + (c - 1) * gapPerim);
}

bool thinNextStartNode(bool goEast, Node& startNode, int gapSizeInBetweenTopNodes, int topNodesLeft)
{

    if (goEast) {
        for (int j = 0; j < gapSizeInBetweenTopNodes; j++) {
            startNode = startNode.nodeInDir(east);
        }
        if (topNodesLeft == 3) {
            startNode = startNode.nodeInDir(east);
            startNode = startNode.nodeInDir(east);
        }
        startNode = startNode.nodeInDir(east);
        return false;
    } else {
        for (int j = 0; j < gapSizeInBetweenTopNodes; j++) {
            startNode = startNode.nodeInDir(west);
        }
        startNode = startNode.nodeInDir(west);
        startNode = startNode.nodeInDir(west);
        return true;
    }
}

bool nextStartNode(bool goEast, int topNodesLeft, Node& startNode)
{
    if (goEast) {
        for (int j = 0; j < topNodesLeft; j++) {
            startNode = startNode.nodeInDir(east);
        }
    } else {
        for (int j = 0; j < topNodesLeft; j++) {
            startNode = startNode.nodeInDir(west);
        }
    }

    return !goEast;
}

void nextGapStarts(int i, int gapSize, Node& gapStarts)
{

    if (i == 1) {
        if (gapSize % 2 == 1) {
            gapStarts = gapStarts.nodeInDir(southWest);
        } else {
            gapStarts = gapStarts.nodeInDir(southEast);
        }
    } else {
        if (gapSize % 2 == 1) {
            gapStarts = gapStarts.nodeInDir(west);
        } else {
            gapStarts = gapStarts.nodeInDir(east);
        }
    }
}

bool nextFillerNode(Node& fillerNode, int yMax, int diagonalDirection, int xDirection, bool placeLeft)
{
    if (fillerNode.y < yMax) {
        fillerNode = fillerNode.nodeInDir(diagonalDirection);
        return placeLeft;
    } else {
        fillerNode.x += xDirection;
        while (fillerNode.y > 0) {
            fillerNode = fillerNode.nodeInDir(diagonalDirection + 3);
        }
        return !placeLeft;
    }
}

int ShortcutBridgingSystem::createLineOfParticles(const Node& startNode, const Node& endNode, double lambda)
{
    int numParticlesInserted = 0;

    Node node = Node(startNode);
    while (node != endNode) {
        insert(new ShortcutBridgingParticle(node, -1, randDir(), *this, lambda, c));
        numParticlesInserted++;
        node = node.nodeTowardsNode(endNode);
    }
    insert(new ShortcutBridgingParticle(node, -1, randDir(), *this, lambda, c));
    numParticlesInserted++;

    return numParticlesInserted;
}

double ShortcutBridgingSystem::middleLineHexagon(
    int totalParticles, const Node& leftAnchorNode, const Node& rightAnchorNode,
    double lambda)
{
    // Widest line
    int iterations = 0;

    while (totalParticles > 0) {
        Node leftUpperNode = Node(leftAnchorNode);
        Node rightUpperNode = Node(rightAnchorNode);
        Node leftLowerNode = Node(leftAnchorNode);
        Node rightLowerNode = Node(rightAnchorNode);

        if (iterations == 0) {
            for (int i = 0; i < iterations; i++) {
                leftUpperNode = leftUpperNode.nodeInDir(northEast);
                rightUpperNode = rightUpperNode.nodeInDir(northWest);
            }
            totalParticles -= createLineOfParticles(leftUpperNode, rightUpperNode, lambda);
        } else {
            for (int i = 0; i < iterations; i++) {
                leftUpperNode = leftUpperNode.nodeInDir(northEast);
                rightUpperNode = rightUpperNode.nodeInDir(northWest);
                leftLowerNode = leftLowerNode.nodeInDir(southEast);
                rightLowerNode = rightLowerNode.nodeInDir(southWest);
            }
            int particlesOnLine = leftUpperNode.distanceBetween(rightUpperNode) + 1;

            // upper part.
            if (particlesOnLine > totalParticles) {
                int steps = particlesOnLine - totalParticles;
                while (steps > 0) {
                    rightUpperNode = rightUpperNode.nodeInDir(west);
                    steps--;
                }
            }
            totalParticles -= createLineOfParticles(leftUpperNode, rightUpperNode, lambda);

            // lower part.
            if (totalParticles > 0) {
                if (particlesOnLine > totalParticles) {
                    int steps = particlesOnLine - totalParticles;
                    while (steps > 0) {
                        rightUpperNode = rightLowerNode.nodeInDir(west);
                        steps--;
                    }
                }
                totalParticles -= createLineOfParticles(leftLowerNode, rightLowerNode, lambda);
            }
        }

        iterations++;
    }

    double perim = getMeasure("Perimeter").calculate();
    double gapPerim = getMeasure("Gap Perimeter").calculate();

    return (perim + (c - 1) * gapPerim);
}

double ShortcutBridgingSystem::drawTopHexagon(
    int totalParticles, const Node& leftAnchorNode, const Node& rightAnchorNode,
    int startSideLength, double lambda, int sideLengthHexa)
{
    while (totalParticles > 0) {
        Node node(leftAnchorNode);
        node = node.nodeInDir(east);

        for (int i = 0; i < sideLengthHexa; i++) {
            node = node.nodeInDir(east);
        }

        for (int i = 1; i < startSideLength; i++) {
            node = node.nodeInDir(west);
        }

        // draw hexa
        if (startSideLength > 1) {
            for (int j = 0; j < 3; j++) {
                for (int z = 0; z < (j < 2 ? startSideLength - 1 : startSideLength) /*+ i*/; z++) {
                    if (totalParticles > 0) {
                        insert(new ShortcutBridgingParticle(Node(node.x, node.y), -1, randDir(), *this, lambda, c));
                        totalParticles--;
                    }
                    switch (j) {
                    case 0:
                        node = node.nodeInDir(northEast);
                        break;
                    case 1:
                        node = node.nodeInDir(east);
                        break;
                    case 2:
                        node = node.nodeInDir(southEast);
                        break;
                    }
                }
            }
        } else {
            if (totalParticles > 0) {
                insert(new ShortcutBridgingParticle(Node(node.x, node.y), -1, randDir(), *this, lambda, c));
                totalParticles--;
            }
        }

        startSideLength++;
    }

    double perim = getMeasure("Perimeter").calculate();
    double gapPerim = getMeasure("Gap Perimeter").calculate();

    return (perim + (c - 1) * gapPerim);
}
