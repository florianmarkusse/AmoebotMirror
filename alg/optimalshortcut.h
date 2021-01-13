#ifndef OPTIMALSHORTCUT_H
#define OPTIMALSHORTCUT_H

#include <Qstring>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class OptimalShortcut : public AmoebotSystem {
    friend class OptimalShortcutPerimeterMeasure;
    friend class OptimalShortcutGapPerimeterMeasure;
    friend class OptimalWeightedPerimeterMeasure;

public:
    OptimalShortcut(int numParticles = 100, double lambda = 4.0, double c = 3 / 2);

    virtual bool hasTerminated() const;

    const double c;

private:
    void drawZ(int numParticles, double lambda, double c);

    void drawHexagon(int numParticles, double lambda, double c);
    void drawHexagonIsland(int numParticles);

    void drawVGeneric(int numParticles, double lambda, double c, bool smallIslands, bool bigIslands, bool obstacle);
    void drawV(int numParticles, double lambda, double c);
    void drawVObstacles(int numParticles, double lambda, double c);
    void drawVSmallIslands(int numParticles, double lambda, double c);
    void drawVBigIslands(int numParticles, double lambda, double c);

    void drawTest(int numParticles, double lambda, double c);
};

class OptimalShortcutPerimeterMeasure : public Measure {
public:
    // Constructs a PerimeterMeasure by using the parent constructor and adding a
    // reference to the CompressionSystem being measured.
    OptimalShortcutPerimeterMeasure(const QString name, const unsigned int freq,
        OptimalShortcut& system);

    // Calculates the perimeter of the system, i.e., the number of edges on the
    // walk around the unique external boundary of the system. Uses the fact
    // that perimeter = (3 * #particles) - (#nearest neighbor pairs) - 3.
    double calculate() const final;

protected:
    OptimalShortcut& _system;
};

class OptimalShortcutGapPerimeterMeasure : public Measure {
public:
    // Constructs a GapPerimeterMeasure by using the parent constructor and adding a
    // reference to the CompressionSystem being measured.
    OptimalShortcutGapPerimeterMeasure(const QString name, const unsigned int freq,
        OptimalShortcut& system);

    double calculate() const final;

protected:
    OptimalShortcut& _system;
};

class OptimalWeightedPerimeterMeasure : public Measure {
public:
    OptimalWeightedPerimeterMeasure(const QString name, const unsigned int freq,
        OptimalShortcut& system);

    double calculate() const final;

protected:
    OptimalShortcut& _system;
};

#endif // OPTIMALSHORTCUT_H
