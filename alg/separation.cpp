#include "separation.h"

SeparationParticle::SeparationParticle(const Node head, const int globalTailDir, const int orientation, AmoebotSystem& system, const double lambda, const double kappa, Team team)
    : AmoebotParticle(head, globalTailDir, orientation, system)
    , lambda(lambda)
    , kappa(kappa)
    , team(team)
    , q(0)
    , numNbrsBefore(0)
    , numNbrsTeamBefore(0)
    , flag(false)
    , nodeBefore(head)
{
}

void SeparationParticle::activate()
{
    if (isContracted()) {
    int expandDir = randDir();  // Select a random neighboring location.
    q = randDouble(0, 1);        // Select a random q in (0,1).

    if (canExpand(expandDir) && !hasExpNbr()) {
      // Count neighbors in original position and expand.
      numNbrsBefore = nbrCount(uniqueLabels());
      numNbrsTeamBefore = nbrCountTeam(uniqueLabels(), team);
      expand(expandDir);
      flag = !hasExpNbr();
    } 
    //Swap operation
    else if (!hasExpNbr() && nbrAtLabel(expandDir).team != team) {
        int PnumNbr = nbrCountTeam(uniqueLabels(), team);
        Team otherTeam = Blue;
        if(team == Blue){
            otherTeam = Red;
        }
        int PnumNrbOther = nbrCountTeam(uniqueLabels(), otherTeam) -1;

        SeparationParticle neighbour = nbrAtLabel(expandDir);

        int QnumNrb = neighbour.nbrCountTeam(uniqueLabels(), neighbour.team);
        int QnumNrbOther = neighbour.nbrCountTeam(uniqueLabels(), team) -1;

        if (q < pow(kappa, QnumNrbOther - PnumNbr + PnumNrbOther - QnumNrb)){
            neighbour.team = team;
            team = otherTeam;            
        }


    }
  } else {  // isExpanded().
    if (!flag || numNbrsBefore == 5) {
      contractHead();
    } else {
      // Count neighbors in new position and compute the set S.
      int numNbrsAfter = nbrCount(headLabels());
      int numNbrsTeamAfter = nbrCountTeam(headLabels(), team);
      std::vector<int> S;
      for (const int label : {headLabels()[4], tailLabels()[4]}) {
        if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
          S.push_back(label);
        }
      }

      // If the conditions are satisfied, contract to the new position;
      // otherwise, contract back to the original one.
      if ((q < pow(lambda, numNbrsAfter - numNbrsBefore) * pow(kappa, numNbrsTeamAfter - numNbrsTeamBefore))
          && (checkProp1(S) || checkProp2(S))) {
        contractTail();
      } else {
        contractHead();
      }
    }
  }
}

QString SeparationParticle::inspectionText() const
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

int SeparationParticle::headMarkColor() const
{
    switch (team) {
    case Team::Red:
        return 0xff0000;
    case Team::Blue:
        return 0x0000ff;
    }

    return -1;
}

int SeparationParticle::tailMarkColor() const
{
    return headMarkColor();
}

SeparationParticle& SeparationParticle::nbrAtLabel(int label) const
{
    return AmoebotParticle::nbrAtLabel<SeparationParticle>(label);
}

bool SeparationParticle::hasExpNbr() const
{
    for (const int label : uniqueLabels()) {
        if (hasNbrAtLabel(label) && nbrAtLabel(label).isExpanded()) {
            return true;
        }
    }

    return false;
}


bool SeparationParticle::hasExpHeadAtLabel(const int label) const
{
    return hasNbrAtLabel(label) && nbrAtLabel(label).isExpanded()
        && nbrAtLabel(label).pointsAtMyHead(*this, label);
}

int SeparationParticle::nbrCount(std::vector<int> labels) const
{
    int numNbrs = 0;
    for (const int label : labels) {
        if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
            ++numNbrs;
        }
    }

    return numNbrs;
}

int SeparationParticle::nbrCountTeam(std::vector<int> labels, Team team) const
{
    int numNbrs = 0;
    for (const int label : labels) {
        if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label) && nbrAtLabel(label).team == team) {
            ++numNbrs;
        }
    }

    return numNbrs;
}

bool SeparationParticle::checkProp1(std::vector<int> S) const
{
    // Copy pasted from ShortcutBridging.
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

bool SeparationParticle::checkProp2(std::vector<int> S) const
{
    // Copy pasted from ShortcutBridging.
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

SeparationSystem::SeparationSystem(int numParticles, double lambda, double kappa)
{
    std::set<Node> occupied;
    insert(new SeparationParticle(Node(0, 0), -1, randDir(), *this,
        lambda, kappa, static_cast<Team>(rand() % 2)));
    occupied.insert(Node(0, 0));

    std::set<Node> candidates;
    for (int i = 0; i < 6; ++i) {
        candidates.insert(Node(0, 0).nodeInDir(i));
    }

    // Add all other particles.
    int particlesAdded = 1;
    while (particlesAdded < numParticles && !candidates.empty()) {
        // Pick a random candidate node.
        int randIndex = randInt(0, candidates.size());
        Node randCand;
        for (auto cand = candidates.begin(); cand != candidates.end(); ++cand) {
            if (randIndex == 0) {
                randCand = *cand;
                candidates.erase(cand);
                break;
            } else {
                randIndex--;
            }
        }

        // With probability 1 - holeProb, add a new particle at the candidate node.
        if (randBool(1.0 - 0.1)) {
            insert(new SeparationParticle(randCand, -1, randDir(), *this,
                lambda, kappa, static_cast<Team>(rand() % 2)));
            occupied.insert(randCand);
            particlesAdded++;

            // Add new candidates.
            for (int i = 0; i < 6; ++i) {
                if (occupied.find(randCand.nodeInDir(i)) == occupied.end()) {
                    candidates.insert(randCand.nodeInDir(i));
                }
            }
        }
    }
}

bool SeparationSystem::hasTerminated() const
{
    return false;
}
