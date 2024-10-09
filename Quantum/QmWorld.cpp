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


// Ajout de la m�thode tick
float QmWorld::tick(float t)
{
	//resetBodies();
    if (gravityEnabled) {
        applyGravity(); // Appliquer la gravit� uniquement si elle est activ�e
    }
    updateForces(t);
    integrate(t);
    
    ticktime += t; // Met � jour le temps du dernier tick
    return time - ticktime; // Retourne le temps restant
}




// Ajout de la m�thode interpolate
void QmWorld::interpolate(float dt)
{
    for (QmBody* b : bodies) {
		if (QmParticle* p = dynamic_cast<QmParticle*>(b)) {
			if (p->updater) { // V�rifie si l'updater est valide
				p->updater->update(p->getPos() + dt * p->getVel());
			}
			else {
				std::cerr << "Updater is null for particle!" << std::endl;
			}
		}

    }
}


/*
void QmWorld::tickRK4(float t) {
	// �tape 1 : Calcul des acc�l�rations initiales
	computeAccelerations(0); // Calcul de a[0] avec v[0] et pos[0]

	// �tape 2 : Int�grer � t/2 en utilisant v[0] et a[0]
	integrate(t / 2, 1);

	// �tape 3 : Recalculer les acc�l�rations � l'instant 1
	computeAccelerations(1);

	// �tape 4 : Int�grer � t/2 en utilisant v[1] et a[1]
	integrate(t / 2, 2);

	// �tape 5 : Recalculer les acc�l�rations � l'instant 2
	computeAccelerations(2);

	// �tape 6 : Int�grer � t en utilisant v[2] et a[2]
	integrate(t, 3);

	// �tape 7 : Recalculer les acc�l�rations � l'instant 3
	computeAccelerations(3);

	// �tape 8 : Int�gration finale RK4
	integrateRK4(t);
}

*/

void QmWorld::computeAccelerations(int index) {

	
	// R�initialiser les forces
	for (QmForceRegistry* fr : forceRegistries) {
		delete fr;
	}
	forceRegistries.clear();

	

	// Appliquer les forces de gravit� si activ�es
	if (gravityEnabled) {
		applyGravity();
	}

	// Appliquer les forces enregistr�es
	for (QmForceRegistry* fr : forceRegistries) {
		fr->updateForces(index); // Supposons que cette m�thode prend un index
	}

	// Calculer les acc�l�rations
	for (QmBody* b : bodies) {
		if (QmParticle* p = dynamic_cast<QmParticle*>(b)) {
			// Assurez-vous que cette m�thode retourne la masse de la particule
			glm::vec3 acceleration = p->getNetForce() / p->getMass();
			p->setAcc(acceleration); // Assurez-vous que cette m�thode existe
		}
	}
}

void QmWorld::integrateRK4(float t) {
	for (QmBody* b : bodies) {
		if (QmParticle* p = dynamic_cast<QmParticle*>(b)) {
			glm::vec3 initialPosition = p->getPos();
			glm::vec3 initialVelocity = p->getVel();
			glm::vec3 initialAcceleration = p->getAcc();

			// Calcul des valeurs interm�diaires
			glm::vec3 k1_v = initialAcceleration;
			glm::vec3 k1_p = initialVelocity;

			// Int�gration � t/2
			p->setPos(initialPosition + 0.5f * t * k1_p);
			p->setVel(initialVelocity + 0.5f * t * k1_v);
			computeAccelerations(1); // Calculer les nouvelles acc�l�rations

			glm::vec3 k2_v = p->getAcc();
			glm::vec3 k2_p = p->getVel();

			// R�initialiser � la position initiale
			p->setPos(initialPosition + 0.5f * t * k2_p);
			p->setVel(initialVelocity + 0.5f * t * k2_v);
			computeAccelerations(2);

			glm::vec3 k3_v = p->getAcc();
			glm::vec3 k3_p = p->getVel();

			// R�initialiser � la position initiale
			p->setPos(initialPosition + t * k3_p);
			p->setVel(initialVelocity + t * k3_v);
			computeAccelerations(3);

			glm::vec3 k4_v = p->getAcc();
			glm::vec3 k4_p = p->getVel();

			// Calculer la moyenne des d�riv�es
			glm::vec3 newVelocity = initialVelocity + (t / 6.0f) * (k1_v + 2.0f * k2_v + 2.0f * k3_v + k4_v);
			glm::vec3 newPosition = initialPosition + (t / 6.0f) * (k1_p + 2.0f * k2_p + 2.0f * k3_p + k4_p);

			// Mettre � jour la position et la vitesse finales
			p->setPos(newPosition);
			p->setVel(newVelocity);
		}
	}
}


void QmWorld::integrate(float t)
{
	for (QmBody* b : bodies)

		b->integrate(t);
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

void QmWorld::addBody(QmBody* b)
{
	bodies.push_back(b);
	//std::cout << "Particule ajout�e au monde: " << b << std::endl;
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


