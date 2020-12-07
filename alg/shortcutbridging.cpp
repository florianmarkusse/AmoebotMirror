

#include "alg/shortcutbridging.h"

ShortcutBridgingParticle::ShortcutBridgingParticle(const Node head,
    const int globalTailDir, const int orientation, AmoebotSystem& system, const double lambda)
    : AmoebotParticle(head, globalTailDir, orientation, system)
    , lambda(lambda)
    , q(0)
    , numNbrsBefore(0)
    , flag(false)
{
}

void ShortcutBridgingParticle::activate()
{
    if (isContracted()) {
        int expandDir = randDir(); // Select a random neighboring location.
        q = randDouble(0, 1); // Select a random q in (0,1).

        if (canExpand(expandDir) && !hasExpNbr()) {
            // Count neighbors in original position and expand.
            numNbrsBefore = nbrCount(uniqueLabels());
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
            if ((q < pow(lambda, numNbrsAfter - numNbrsBefore))
                && (checkProp1(S) || checkProp2(S))) {
                contractTail();
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

ShortcutBridgingSystem::ShortcutBridgingSystem(int numParticles, double lambda, double c)
{
    Q_ASSERT(lambda >= 0);

    // Draw v on its head.
    // 67 particles + 2 anchor points.

    int lineSize = (numParticles - 3) / 4;
    int originalDir = 1; // NorthEast
    for (int d = 0; d < 10; d++) {
        Node boundNode(-1*d, 0);
        int dir = originalDir;
        for (int i = 0; i < lineSize+d-1 ; ++i) {
            insert(new Object(boundNode, true));
            boundNode = boundNode.nodeInDir(dir);
        }
        dir = (dir + 5) % 6;
        for (int i = 0; i < d+1; ++i) {
            insert(new Object(boundNode, true));
            boundNode = boundNode.nodeInDir(dir);
        }
        dir = (dir + 5) % 6;
        for (int i = 0; i < lineSize+d; ++i) {
            insert(new Object(boundNode, true));
            boundNode = boundNode.nodeInDir(dir);
        }
    }
    /*
    // Initialize particle system.
    for (int i = 0; i < numParticles; ++i) {
        insert(new ShortcutBridgingParticle(Node(i, 0), -1, randDir(), *this, lambda));
    }
    */

    // Set up metrics.
    _measures.push_back(new ShortcutPerimeterMeasure("Perimeter", 1, *this));
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
