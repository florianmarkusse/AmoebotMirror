#ifndef AMOEBOTSIM_ALG_SHORTCUTBRIDGING_H_
#define AMOEBOTSIM_ALG_SHORTCUTBRIDGING_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class ShortcutBridgingParticle : public AmoebotParticle {
	friend class ShortcutBridgingSystem;
	friend class ShortcutPerimeterMeasure;
	friend class ShortcutGapPerimeterMeasure;
	friend class WeightedPerimeterMeasure;

public:
	// Constructs a new particle with a node position for its head, a global
	// compass direction from its head to its tail (-1 if contracted), an offset
	// for its local compass, a system which it belongs to, and a bias parameter.
	ShortcutBridgingParticle(const Node head, const int globalTailDir,
		const int orientation, AmoebotSystem& system,
		const double lambda, const double c);

	// Executes one particle activation.
	virtual void activate();

	// Returns the string to be displayed when this particle is inspected; used
	// to snapshot the current values of this particle's memory at runtime.
	virtual QString inspectionText() const;

protected:
	// Particle memory.
	const double lambda;
	double q;
	int numNbrsBefore;
	bool flag;
	Node nodeBefore;
	const double c;

private:
	// Gets a reference to the neighboring particle incident to the specified port
	// label. Crashes if no such particle exists at this label; consider using
	// hasNbrAtLabel() first if unsure.
	ShortcutBridgingParticle& nbrAtLabel(int label) const;

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

class ShortcutBridgingSystem : public AmoebotSystem {
	friend class ShortcutPerimeterMeasure;
	friend class ShortcutGapPerimeterMeasure;
	friend class WeightedPerimeterMeasure;

public:
	enum Shape {
		V,
		Z,
		Hexagon,
		VBigIsland,
		VSmallIslands,
		VObstacle,
		HexagonIsland,
	};

	// Constructs a system of CompressionParticles connected to a randomly
	// generated surface (with no tunnels). Takes an optionally specified size
	// (#particles) and a bias parameter. A bias above 2 + sqrt(2) will provably
	// yield compression; a bias below 2.17 will provably yield expansion.
	ShortcutBridgingSystem(int numParticles = 100, double lambda = 4.0, double c = 3 / 2, Shape shape = Shape::V);

	// Because this algorithm never terminates, this simply returns false.
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

	void moveParticle(const Node& startNode, const Node& endNode);

	double thickBridge(int lineSize, int maxGapSize);
	double thinBrigde(int lineSize, int gapSize);
};

class ShortcutPerimeterMeasure : public Measure {
public:
	// Constructs a PerimeterMeasure by using the parent constructor and adding a
	// reference to the CompressionSystem being measured.
	ShortcutPerimeterMeasure(const QString name, const unsigned int freq,
		ShortcutBridgingSystem& system);

	// Calculates the perimeter of the system, i.e., the number of edges on the
	// walk around the unique external boundary of the system. Uses the fact
	// that perimeter = (3 * #particles) - (#nearest neighbor pairs) - 3.
	double calculate() const final;

protected:
	ShortcutBridgingSystem& _system;
};

class ShortcutGapPerimeterMeasure : public Measure {
public:
	// Constructs a GapPerimeterMeasure by using the parent constructor and adding a
	// reference to the CompressionSystem being measured.
	ShortcutGapPerimeterMeasure(const QString name, const unsigned int freq,
		ShortcutBridgingSystem& system);

	double calculate() const final;

protected:
	ShortcutBridgingSystem& _system;
};

class WeightedPerimeterMeasure : public Measure {
public:
	WeightedPerimeterMeasure(const QString name, const unsigned int freq,
		ShortcutBridgingSystem& system);

	double calculate() const final;

protected:
	ShortcutBridgingSystem& _system;
};

#endif // AMOEBOTSIM_ALG_SHORTCUTBRIDGING_H_
