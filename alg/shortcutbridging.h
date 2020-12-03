

#ifndef AMOEBOTSIM_ALG_SHORTCUTBRIDGING_H_
#define AMOEBOTSIM_ALG_SHORTCUTBRIDGING_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class ShortcutBridgingParticle : public AmoebotParticle {
public:
	// Constructs a new particle with a node position for its head, a global
	// compass direction from its head to its tail (-1 if contracted), an offset
	// for its local compass, a system that it belongs to.
	ShortcutBridgingParticle(const Node& head, const int globalTailDir,
		const int orientation, AmoebotSystem& system,
		const int counterMax);

	// Executes one particle activation.
	void activate() override;

	// Functions for altering the particle's color. headMarkColor() (resp.,
	// tailMarkColor()) returns the color to be used for the ring drawn around the
	// particle's head (resp., tail) node. In this demo, the tail color simply
	// matches the head color.
	int headMarkColor() const override;
	int tailMarkColor() const override;

	// Returns the string to be displayed when this particle is inspected; used to
	// snapshot the current values of this particle's memory at runtime.
	QString inspectionText() const override;
};

class ShortcutBridgingSystem : public AmoebotSystem {
public:
	// Constructs a system of the specified number of DiscoDemoParticles enclosed
	// by a hexagonal ring of objects.
	ShortcutBridgingSystem(unsigned int numParticles = 30, int counterMax = 5);
};


#endif  // AMOEBOTSIM_ALG_SHORTCUTBRIDGING_H_