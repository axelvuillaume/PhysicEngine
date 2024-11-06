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




bool sphereHalfSpaceIntersect(QmParticle* particle, QmHalfSpace* halfSpace) {
	glm::vec3 particlePos = particle->getPos();
	float radius = particle->getRadius();

	// Distance du centre de la sphère au plan
	float distanceToPlane = glm::dot(halfSpace->normal, particlePos) - halfSpace->offset;

	// Vérifiez si la sphère touche le demi-espace
	return distanceToPlane < radius;
}


bool intersect(QmAABB a, QmAABB b) {
	return
		(a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z);
}

std::vector<QmContact> QmWorld::broadphase() {

	if (!collisionsEnabled) {
		return {}; // Retourne une liste vide si les collisions sont désactivées
	}


	std::vector<QmContact> potentialContacts;

	for (size_t i = 0; i < bodies.size(); ++i) {
		for (size_t j = i + 1; j < bodies.size(); ++j) {
			QmBody* body1 = bodies[i];
			QmBody* body2 = bodies[j];

			// Vérifiez si les deux corps sont des QmParticles
			QmParticle* particle1 = dynamic_cast<QmParticle*>(body1);
			QmParticle* particle2 = dynamic_cast<QmParticle*>(body2);
			QmHalfSpace* halfSpace = dynamic_cast<QmHalfSpace*>(body2); // On peut aussi vérifier si body2 est un demi-espace

			// Si les deux particules sont statiques, on les ignore
			if (particle1 && particle2 && particle1->isStatic && particle2->isStatic) {
				continue;
			}

			// Collision entre deux sphères
			if (particle1 && particle2) {
				QmAABB aabb1 = particle1->getAABB();
				QmAABB aabb2 = particle2->getAABB();

				if (intersect(aabb1, aabb2)) {
					glm::vec3 contactNormal = glm::normalize(particle2->getPos() - particle1->getPos());
					float penetrationDepth = 0.0f;
					potentialContacts.push_back(QmContact(particle1, particle2, contactNormal, penetrationDepth));
				}
			}

			// Collision entre une sphère et un demi-espace
			if (particle1 && halfSpace) {
				// On peut simplifier la détection de collision pour un demi-espace.
				float distanceFromPlane = glm::dot(particle1->getPos(), halfSpace->normal) - halfSpace->offset;

				// Si la particule est en dessous du plan
				if (sphereHalfSpaceIntersect(particle1, halfSpace)) {
					glm::vec3 contactNormal = halfSpace->normal;
					float penetrationDepth = particle1->getRadius() - distanceFromPlane;
					potentialContacts.push_back(QmContact(particle1, halfSpace, contactNormal, penetrationDepth));
				}
			}
		}
	}

	return potentialContacts;
}

std::vector<QmContact> QmWorld::narrowphase(std::vector<QmContact> contacts) {

	if (!collisionsEnabled) {
		return {}; // Retourne une liste vide si les collisions sont désactivées
	}

	std::vector<QmContact> refinedContacts;

	// Parcourir tous les contacts détectés dans la broadphase
	for (QmContact& contact : contacts) {
		QmParticle* particle1 = dynamic_cast<QmParticle*>(contact.body1);
		QmParticle* particle2 = dynamic_cast<QmParticle*>(contact.body2);
		QmHalfSpace* halfSpace = dynamic_cast<QmHalfSpace*>(contact.body2);

		if (particle1 && particle2) {
			// Récupérer les positions et les rayons des particules
			glm::vec3 pos1 = particle1->getPos();
			glm::vec3 pos2 = particle2->getPos();
			float radius1 = particle1->getRadius();
			float radius2 = particle2->getRadius();

			glm::vec3 delta = pos2 - pos1;
			float distSq = glm::dot(delta, delta); 

			float radiusSum = radius1 + radius2;
			float radiusSumSq = radiusSum * radiusSum;  

			// Vérifier si les particules se chevauchent réellement (distance < somme des rayons)
			if (distSq < radiusSumSq) {
				// Calculer la profondeur de pénétration et la normale
				float dist = glm::sqrt(distSq);
				glm::vec3 contactNormal = glm::normalize(delta);

				// Si la distance est proche de zéro, éviter la division par zéro
				if (dist < 0.0001f) {
					contactNormal = glm::vec3(1.0f, 0.0f, 0.0f); 
				}

				float penetrationDepth = radiusSum - dist;

				// Mettre à jour les informations de contact
				contact.normal = contactNormal;
				contact.depth = penetrationDepth;

				// Ajouter le contact raffiné à la liste des contacts
				refinedContacts.push_back(contact);
			}
		}

		if (particle1 && halfSpace) {
			// Gérer la collision sphère-demi-espace
			glm::vec3 particlePos = particle1->getPos();
			float radius = particle1->getRadius();

			// Calculer la distance entre la sphère et le plan du demi-espace
			float distanceFromPlane = glm::dot(particlePos, halfSpace->normal) - halfSpace->offset;

			// Vérifier si la sphère pénètre dans le demi-espace
			if (distanceFromPlane < radius) {
				float penetrationDepth = radius - distanceFromPlane;

				// Mettre à jour les informations de contact
				contact.normal = halfSpace->normal;
				contact.depth = penetrationDepth;

				// Ajouter le contact raffiné
				refinedContacts.push_back(contact);
			}
		}
	}


	return refinedContacts;
	
}
void QmWorld::toggleStaticBodies() {
	if (staticBodiesEnabled) {
		// Désactivation des corps statiques - sauvegarde de l'état initial
		initialStaticBodies.clear();
		for (QmBody* body : bodies) {
			QmParticle* particle = dynamic_cast<QmParticle*>(body);
			if (particle && particle->isStatic) {
				initialStaticBodies.push_back(particle);
				particle->isStatic = false;  // Désactive `isStatic`
			}
		}
	}
	else {
		// Réactivation des corps statiques pour les corps initialement statiques
		for (QmParticle* particle : initialStaticBodies) {
			particle->isStatic = true;
		}
		initialStaticBodies.clear();  // Vide la liste après réactivation
	}

	// Bascule l'état global de `staticBodiesEnabled`
	staticBodiesEnabled = !staticBodiesEnabled;

	// Affiche le statut des corps statiques
	std::cout << "Static bodies toggled: "
		<< (staticBodiesEnabled ? "Enabled" : "Disabled") << std::endl;
}


void QmWorld::resolve(std::vector<QmContact> contacts) {

	if (!collisionsEnabled) {
		return; // Ne rien faire si les collisions sont désactivées


	}
	for (QmContact& contact : contacts) {
		// Extraire les deux corps impliqués dans la collision
		QmParticle* particle1 = dynamic_cast<QmParticle*>(contact.body1);
		QmParticle* particle2 = dynamic_cast<QmParticle*>(contact.body2);
		QmHalfSpace* halfSpace = dynamic_cast<QmHalfSpace*>(contact.body2);

		if (particle1 && particle2) {
			// 1. Déplacer les objets hors de l'interpénétration
			float mass1 = particle1->getMass();
			float mass2 = particle2->getMass();
			float totalMass = mass1 + mass2;

			if (totalMass == 0) continue;

			// Normaliser la normale de contact
			glm::vec3 normal = glm::normalize(contact.normal);

			// Calculer le déplacement proportionnel à la masse
			glm::vec3 movePerMass1 = glm::vec3(contact.depth * mass2/ totalMass);
			glm::vec3 movePerMass2 = glm::vec3(contact.depth * mass1 / totalMass);

			// Mettre à jour les positions des particules
			particle1->setPos(particle1->getPos() + movePerMass1);
			particle2->setPos(particle2->getPos() - movePerMass2);




			// 2. Calculer les nouvelles vitesses après la collision
			glm::vec3 velocity1 = particle1->getVel();
			glm::vec3 velocity2 = particle2->getVel();

			glm::vec3 relativeVelocity = velocity2 - velocity1;
			float velocityAlongNormal = glm::dot(relativeVelocity, normal);

			// Si les particules s'éloignent, ne rien faire
			if (velocityAlongNormal > 0) {
				continue;
			}

			float restitution = 1.0f;

			float impulseScalar = -(1 + restitution) * velocityAlongNormal / (1 / mass1 + 1 / mass2);
			glm::vec3 impulse = impulseScalar * normal;

			// Mettre à jour les vitesses des particules
			particle1->setVel(velocity1 - impulse / mass1);
			particle2->setVel(velocity2 + impulse / mass2);

	
		}

		if (particle1 && halfSpace) {
			// 1. Résolution des collisions entre une particule et un demi-espace

			float mass1 = particle1->getMass();
			if (mass1 == 0) continue; // Ignorer les particules sans masse

			glm::vec3 normal = glm::normalize(contact.normal);

			// Déplacer la particule hors du demi-espace
			glm::vec3 move = normal * contact.depth;
			particle1->setPos(particle1->getPos() + move);

			// 2. Calcul des nouvelles vitesses après la collision
			glm::vec3 velocity1 = particle1->getVel();
			float velocityAlongNormal = glm::dot(velocity1, normal);

			// Vérifie si la vitesse le long de la normale est négative (entrée dans le demi-espace)
			if (velocityAlongNormal > 0) continue; // La particule se déplace vers l'extérieur

			// Coefficient de restitution pour le rebond
			float restitution = 1.0f;

			// Calcule l'impulsion
			float impulseScalar = -(1 + restitution) * velocityAlongNormal / (1 / mass1);
			glm::vec3 impulse = impulseScalar * normal;

			// Met à jour la vitesse de la particule
			particle1->setVel(velocity1 + impulse / mass1); // Ajoute l'impulsion pour inverser la vitesse
		}

	}

}



/*
void QmWorld::tickRK4(float t) {
	// Étape 1 : Calcul des accélérations initiales
	computeAccelerations(0); // Calcul de a[0] avec v[0] et pos[0]

	// Étape 2 : Intégrer à t/2 en utilisant v[0] et a[0]
	integrate(t / 2, 1);

	// Étape 3 : Recalculer les accélérations à l'instant 1
	computeAccelerations(1);

	// Étape 4 : Intégrer à t/2 en utilisant v[1] et a[1]
	integrate(t / 2, 2);

	// Étape 5 : Recalculer les accélérations à l'instant 2
	computeAccelerations(2);

	// Étape 6 : Intégrer à t en utilisant v[2] et a[2]
	integrate(t, 3);

	// Étape 7 : Recalculer les accélérations à l'instant 3
	computeAccelerations(3);

	// Étape 8 : Intégration finale RK4
	integrateRK4(t);
}

*/

void QmWorld::computeAccelerations(int index) {

	
	// Réinitialiser les forces
	for (QmForceRegistry* fr : forceRegistries) {
		delete fr;
	}
	forceRegistries.clear();

	

	// Appliquer les forces de gravité si activées
	if (gravityEnabled) {
		applyGravity();
	}

	// Appliquer les forces enregistrées
	for (QmForceRegistry* fr : forceRegistries) {
		fr->updateForces(index); // Supposons que cette méthode prend un index
	}

	// Calculer les accélérations
	for (QmBody* b : bodies) {
		if (QmParticle* p = dynamic_cast<QmParticle*>(b)) {
			// Assurez-vous que cette méthode retourne la masse de la particule
			glm::vec3 acceleration = p->getNetForce() / p->getMass();
			p->setAcc(acceleration); // Assurez-vous que cette méthode existe
		}
	}
}

void QmWorld::integrateRK4(float t) {
	for (QmBody* b : bodies) {
		if (QmParticle* p = dynamic_cast<QmParticle*>(b)) {
			glm::vec3 initialPosition = p->getPos();
			glm::vec3 initialVelocity = p->getVel();
			glm::vec3 initialAcceleration = p->getAcc();

			// Calcul des valeurs intermédiaires
			glm::vec3 k1_v = initialAcceleration;
			glm::vec3 k1_p = initialVelocity;

			// Intégration à t/2
			p->setPos(initialPosition + 0.5f * t * k1_p);
			p->setVel(initialVelocity + 0.5f * t * k1_v);
			computeAccelerations(1); // Calculer les nouvelles accélérations

			glm::vec3 k2_v = p->getAcc();
			glm::vec3 k2_p = p->getVel();

			// Réinitialiser à la position initiale
			p->setPos(initialPosition + 0.5f * t * k2_p);
			p->setVel(initialVelocity + 0.5f * t * k2_v);
			computeAccelerations(2);

			glm::vec3 k3_v = p->getAcc();
			glm::vec3 k3_p = p->getVel();

			// Réinitialiser à la position initiale
			p->setPos(initialPosition + t * k3_p);
			p->setVel(initialVelocity + t * k3_v);
			computeAccelerations(3);

			glm::vec3 k4_v = p->getAcc();
			glm::vec3 k4_p = p->getVel();

			// Calculer la moyenne des dérivées
			glm::vec3 newVelocity = initialVelocity + (t / 6.0f) * (k1_v + 2.0f * k2_v + 2.0f * k3_v + k4_v);
			glm::vec3 newPosition = initialPosition + (t / 6.0f) * (k1_p + 2.0f * k2_p + 2.0f * k3_p + k4_p);

			// Mettre à jour la position et la vitesse finales
			p->setPos(newPosition);
			p->setVel(newVelocity);
		}
	}
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
	resolve(narrowphase(broadphase()));


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

    if (framerateIndependent) {
        while (dt >= DELTA) {
            dt -= DELTA;
            tick(DELTA);  
        }
        interpolate(dt); 
    } else {

        tick(dt); 
        interpolate(dt);  
    }
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


