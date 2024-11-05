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
	resolve(narrowphase(broadphase()));

    
    ticktime += t; // Met � jour le temps du dernier tick
    return time - ticktime; // Retourne le temps restant
}

bool sphereHalfSpaceIntersect(QmParticle* particle, QmHalfSpace* halfSpace) {
	glm::vec3 particlePos = particle->getPos();
	float radius = particle->getRadius();

	// Distance du centre de la sph�re au plan
	float distanceToPlane = glm::dot(halfSpace->normal, particlePos) - halfSpace->offset;

	// V�rifiez si la sph�re touche le demi-espace
	return distanceToPlane < radius;
}


bool intersect(QmAABB a, QmAABB b) {
	return
		(a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z);
}



std::vector<QmContact> QmWorld::broadphase() {
	std::vector<QmContact> potentialContacts;

	// Separate dynamic and static bodies
	std::vector<QmBody*> staticBodies;
	std::vector<QmBody*> dynamicBodies;

	for (QmBody* body : bodies) {
		if (dynamic_cast<QmHalfSpace*>(body)) {
			staticBodies.push_back(body);
		}
		else {
			dynamicBodies.push_back(body);
		}
	}

	// Check collisions between dynamic bodies
	for (size_t i = 0; i < dynamicBodies.size(); ++i) {
		for (size_t j = i + 1; j < dynamicBodies.size(); ++j) {
			QmParticle* particle1 = dynamic_cast<QmParticle*>(dynamicBodies[i]);
			QmParticle* particle2 = dynamic_cast<QmParticle*>(dynamicBodies[j]);

			if (particle1 && particle2) {
				QmAABB aabb1 = particle1->getAABB();
				QmAABB aabb2 = particle2->getAABB();

				if (intersect(aabb1, aabb2)) {
					glm::vec3 contactNormal = glm::normalize(particle2->getPos() - particle1->getPos());
					float penetrationDepth = 0.0f; // Calculate based on actual intersection depth
					potentialContacts.push_back(QmContact(particle1, particle2, contactNormal, penetrationDepth));
				}
			}
		}
	}

	// Check collisions between dynamic bodies and static bodies (half spaces)
	for (QmBody* dynamicBody : dynamicBodies) {
		QmParticle* particle = dynamic_cast<QmParticle*>(dynamicBody);
		for (QmBody* staticBody : staticBodies) {
			QmHalfSpace* halfSpace = dynamic_cast<QmHalfSpace*>(staticBody);

			if (particle && halfSpace) {
				float distanceFromPlane = glm::dot(particle->getPos(), halfSpace->normal) - halfSpace->offset;

				// Check if the particle is below the plane
				if (sphereHalfSpaceIntersect(particle, halfSpace)) {
					glm::vec3 contactNormal = halfSpace->normal;
					float penetrationDepth = particle->getRadius() - distanceFromPlane; // Calculate penetration
					potentialContacts.push_back(QmContact(particle, halfSpace, contactNormal, penetrationDepth));
				}
			}
		}
	}

	return potentialContacts;
}

std::vector<QmContact> QmWorld::narrowphase(std::vector<QmContact> contacts) {
	std::vector<QmContact> refinedContacts;

	// Parcourir tous les contacts d�tect�s dans la broadphase
	for (QmContact& contact : contacts) {
		QmParticle* particle1 = dynamic_cast<QmParticle*>(contact.body1);
		QmParticle* particle2 = dynamic_cast<QmParticle*>(contact.body2);
		QmHalfSpace* halfSpace = dynamic_cast<QmHalfSpace*>(contact.body2);

		if (particle1 && particle2) {
			// R�cup�rer les positions et les rayons des particules
			glm::vec3 pos1 = particle1->getPos();
			glm::vec3 pos2 = particle2->getPos();
			float radius1 = particle1->getRadius();
			float radius2 = particle2->getRadius();

			glm::vec3 delta = pos2 - pos1;
			float distSq = glm::dot(delta, delta); 

			float radiusSum = radius1 + radius2;
			float radiusSumSq = radiusSum * radiusSum;  

			// V�rifier si les particules se chevauchent r�ellement (distance < somme des rayons)
			if (distSq < radiusSumSq) {
				// Calculer la profondeur de p�n�tration et la normale
				float dist = glm::sqrt(distSq);
				glm::vec3 contactNormal = glm::normalize(delta);

				// Si la distance est proche de z�ro, �viter la division par z�ro
				if (dist < 0.0001f) {
					contactNormal = glm::vec3(1.0f, 0.0f, 0.0f); 
				}

				float penetrationDepth = radiusSum - dist;

				// Mettre � jour les informations de contact
				contact.normal = contactNormal;
				contact.depth = penetrationDepth;

				// Ajouter le contact raffin� � la liste des contacts
				refinedContacts.push_back(contact);
			}
		}

		if (particle1 && halfSpace) {
			// G�rer la collision sph�re-demi-espace
			glm::vec3 particlePos = particle1->getPos();
			float radius = particle1->getRadius();

			// Calculer la distance entre la sph�re et le plan du demi-espace
			float distanceFromPlane = glm::dot(particlePos, halfSpace->normal) - halfSpace->offset;

			// V�rifier si la sph�re p�n�tre dans le demi-espace
			if (distanceFromPlane < radius) {
				float penetrationDepth = radius - distanceFromPlane;

				// Mettre � jour les informations de contact
				contact.normal = halfSpace->normal;
				contact.depth = penetrationDepth;

				// Ajouter le contact raffin�
				refinedContacts.push_back(contact);
			}
		}
	}


	return refinedContacts;
	
}

void QmWorld::resolve(std::vector<QmContact> contacts) {
	for (QmContact& contact : contacts) {
		// Extraire les deux corps impliqu�s dans la collision
		QmParticle* particle1 = dynamic_cast<QmParticle*>(contact.body1);
		QmParticle* particle2 = dynamic_cast<QmParticle*>(contact.body2);
		QmHalfSpace* halfSpace = dynamic_cast<QmHalfSpace*>(contact.body2);

		if (particle1 && particle2) {
			// 1. D�placer les objets hors de l'interp�n�tration
			float mass1 = particle1->getMass();
			float mass2 = particle2->getMass();
			float totalMass = mass1 + mass2;

			if (totalMass == 0) continue;

			// Normaliser la normale de contact
			glm::vec3 normal = glm::normalize(contact.normal);

			// Calculer le d�placement proportionnel � la masse
			glm::vec3 movePerMass1 = glm::vec3(contact.depth * mass2/ totalMass);
			glm::vec3 movePerMass2 = glm::vec3(contact.depth * mass1 / totalMass);

			// Mettre � jour les positions des particules
			particle1->setPos(particle1->getPos() + movePerMass1);
			particle2->setPos(particle2->getPos() - movePerMass2);




			// 2. Calculer les nouvelles vitesses apr�s la collision
			glm::vec3 velocity1 = particle1->getVel();
			glm::vec3 velocity2 = particle2->getVel();

			glm::vec3 relativeVelocity = velocity2 - velocity1;
			float velocityAlongNormal = glm::dot(relativeVelocity, normal);

			// Si les particules s'�loignent, ne rien faire
			if (velocityAlongNormal > 0) {
				continue;
			}

			float restitution = 1.0f;

			float impulseScalar = -(1 + restitution) * velocityAlongNormal / (1 / mass1 + 1 / mass2);
			glm::vec3 impulse = impulseScalar * normal;

			// Mettre � jour les vitesses des particules
			particle1->setVel(velocity1 - impulse / mass1);
			particle2->setVel(velocity2 + impulse / mass2);

	
		}

		if (particle1 && halfSpace) {
			// 1. R�solution des collisions entre une particule et un demi-espace

			float mass1 = particle1->getMass();
			if (mass1 == 0) continue; // Ignorer les particules sans masse

			glm::vec3 normal = glm::normalize(contact.normal);

			// D�placer la particule hors du demi-espace
			glm::vec3 move = normal * contact.depth;
			particle1->setPos(particle1->getPos() + move);

			// 2. Calcul des nouvelles vitesses apr�s la collision
			glm::vec3 velocity1 = particle1->getVel();
			float velocityAlongNormal = glm::dot(velocity1, normal);

			// V�rifie si la vitesse le long de la normale est n�gative (entr�e dans le demi-espace)
			if (velocityAlongNormal > 0) continue; // La particule se d�place vers l'ext�rieur

			// Coefficient de restitution pour le rebond
			float restitution = 1.0f;

			// Calcule l'impulsion
			float impulseScalar = -(1 + restitution) * velocityAlongNormal / (1 / mass1);
			glm::vec3 impulse = impulseScalar * normal;

			// Met � jour la vitesse de la particule
			particle1->setVel(velocity1 + impulse / mass1); // Ajoute l'impulsion pour inverser la vitesse
		}

	}

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


