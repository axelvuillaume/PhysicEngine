#include "stdafx.h"
#include <iostream>

#include "QmWorld.h"
#include "QmParticle.h"
#include "QmForceRegistry.h"

using namespace Quantum;

QmWorld::QmWorld() : gravityForce(0.0f, -9.81f, 0.0f), gravityEnabled(true), time(0.f), ticktime(0.f)
{
	std::cout << "Starting Quantum Physics engine." << std::endl;
}

QmWorld::~QmWorld()
{
	clear();
}


// Ajout de la méthode tick
float QmWorld::tick(float t)
{
	//resetBodies();
    if (gravityEnabled) {
        applyGravity(); // Appliquer la gravité uniquement si elle est activée
    }
    updateForces(t);
    integrate(t);
    
    ticktime += t; // Met à jour le temps du dernier tick
    return time - ticktime; // Retourne le temps restant
}

// Ajout de la méthode interpolate
void QmWorld::interpolate(float dt)
{
    for (QmBody* b : bodies) {
		if (QmParticle* p = dynamic_cast<QmParticle*>(b)) {
			if (p->updater) { // Vérifie si l'updater est valide
				p->updater->update(p->getPos() + dt * p->getVel());
			}
			else {
				std::cerr << "Updater is null for particle!" << std::endl;
			}
		}

    }
}



void QmWorld::simulate(float t)
{
    time += t;
    float dt = time - ticktime;

    const float DELTA = 0.016f;

    while (dt >= DELTA) {
        dt = tick(DELTA);
    }
    interpolate(dt); // Interpoler les positions des particules
}


bool QmWorld::isGravityEnabled() {
	return gravityEnabled;
}


bool QmWorld::setGravityEnabled(bool gravity) {
	return this->gravityEnabled = gravity;
}


void QmWorld::integrate(float t)
{
	for (QmBody* b : bodies)
		b->integrate(t);
}

void QmWorld::addBody(QmBody* b)
{
	bodies.push_back(b);
	//std::cout << "Particule ajoutée au monde: " << b << std::endl;
}

void QmWorld::addForceRegistry(QmForceRegistry* f)
{
	forceRegistries.push_back(f);
	//std::cout << " ADD FORCE REGISTRY ?????????????" << std::endl;

}

std::vector<QmForceRegistry*> QmWorld::getForceRegistries() {
	return forceRegistries; // Retourne les registres de force
}

std::vector<QmBody*> QmWorld::getBodies()
{
	return bodies;
}

void QmWorld::updateForces(float deltaTime)
{
	//std::cout << "UPDATE FORCE WORLD HAHAHAH " << std::endl;
	for (QmForceRegistry* fr : forceRegistries) {
		fr->updateForces(deltaTime);
	}
}


void QmWorld::applyGravity()
{
	for (QmBody* b : bodies) {
		if (QmParticle* p = dynamic_cast<QmParticle*>(b)) {

			if (p->isAffectedByGravity) {
				p->addForce(gravityForce);
			}
		}
	}
}


void QmWorld::clear()
{

	for (QmBody* b : bodies)
	{
		delete b;
	}
	bodies.clear();

	for (QmForceRegistry* fr : forceRegistries) {
		delete fr;
	}
	forceRegistries.clear();
}

