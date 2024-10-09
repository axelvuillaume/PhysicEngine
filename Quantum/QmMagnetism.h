#include "stdafx.h"

#ifndef QMMAGNETISM_H
#define QMMAGNETISM_H

#include "QmForceGenerator.h"
#include "QmParticle.h"
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

namespace Quantum {

    class QmMagnetism : public QmForceGenerator {
    public:
        // Constructeur avec le coefficient magn�tique 'k' et la r�f�rence � toutes les particules
        QmMagnetism(float k, const std::vector<QmParticle*>allParticles)
            : K(k), particles(allParticles) {}

        // M�thode pour mettre � jour la force magn�tique appliqu�e � une particule
        void update(QmParticle* particle) {
            //std::cout << "Nombre de particules: " << particles.size() << std::endl;
            //std::cout << "IN QM MAGNETISM " << std::endl;
            for (QmParticle* other : particles) {
                if (particle != other) {
                    glm::vec3 distanceVec = particle->getPos() - other->getPos();
                    float distance = glm::length(distanceVec);
                        
                    // Calcul de la force magn�tique entre les deux particules
                    float forceMagnitude = K * (particle->getCharge() * other->getCharge()) / (distance * distance + EPS);

                    // Ajoute la force � la particule (dans la direction oppos�e)
                    particle->addForce(glm::normalize(distanceVec) * forceMagnitude);
                }
            }


        }

    private:
        float K; // Coefficient magn�tique
        const std::vector<QmParticle*> particles; // R�f�rence aux autres particules
        static constexpr float EPS = 1e-6f; // Constante pour �viter les divisions par z�ro
    };

}

#endif
