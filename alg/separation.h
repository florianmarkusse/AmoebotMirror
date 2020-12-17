#ifndef AMOEBOTSIM_ALG_SEPARATION_H_
#define AMOEBOTSIM_ALG_SEPARATION_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

enum Team {
    Red,
    Blue
};

class SeparationParticle : public AmoebotParticle {
    friend class SeparationSystem;

public:
    // Constructs a new particle with a node position for its head, a global
    // compass direction from its head to its tail (-1 if contracted), an offset
    // for its local compass, a system which it belongs to, and a bias parameter.
    SeparationParticle(const Node head, const int globalTailDir,
        const int orientation, AmoebotSystem& system,
        const double lambda, const double kappa, const Team team);

    // Executes one particle activation.
    virtual void activate();

    // Returns the string to be displayed when this particle is inspected; used
    // to snapshot the current values of this particle's memory at runtime.
    virtual QString inspectionText() const;

    int headMarkColor() const override;
    int tailMarkColor() const override;

protected:
    // Particle memory.
    const double lambda;
    const double kappa;
    const Team team;
    double q;
    int numNbrsBefore;
    bool flag;
    Node nodeBefore;

private:
    // Gets a reference to the neighboring particle incident to the specified port
    // label. Crashes if no such particle exists at this label; consider using
    // hasNbrAtLabel() first if unsure.
    SeparationParticle& nbrAtLabel(int label) const;

    // hasExpNbr() checks whether this particle has an expanded neighbor, while
    // hasExpHeadAtLabel() checks whether the head of an expanded neighbor is at
    // the position at the specified label.
    bool hasExpNbr() const;
    bool hasExpHeadAtLabel(const int label) const;

    // Counts the number of neighbors in the labeled positions. Note: this
    // implicitly assumes all neighbors are unique, as none are expanded.
    int nbrCount(std::vector<int> labels) const;

    // Functions for checking Properties 1 and 2 of the compression algorithm.
    bool checkProp1(std::vector<int> S) const;
    bool checkProp2(std::vector<int> S) const;
};

class SeparationSystem : public AmoebotSystem {

public:
    SeparationSystem(int numParticles = 100, double lambda = 4.0, double kappa = 4.0);

    // Because this algorithm never terminates, this simply returns false.
    virtual bool hasTerminated() const;
};

#endif // AMOEBOTSIM_ALG_SEPARATION_H_
