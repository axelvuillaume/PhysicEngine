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
        // Constructeur avec le coefficient magnétique 'k' et la référence à toutes les particules
        QmMagnetism(float k, const std::vector<QmParticle*>allParticles)
            : K(k), particles(allParticles) {}

        // Méthode pour mettre à jour la force magnétique appliquée à une particule
        void update(QmParticle* particle) {
            //std::cout << "Nombre de particules: " << particles.size() << std::endl;
            //std::cout << "IN QM MAGNETISM " << std::endl;
            for (QmParticle* other : particles) {
                if (particle != other) {
                    glm::vec3 distanceVec = particle->getPos() - other->getPos();
                    float distance = glm::length(distanceVec);
                        
                    // Calcul de la force magnétique entre les deux particules
                    float forceMagnitude = K * (particle->getCharge() * other->getCharge()) / (distance * distance + EPS);

                    // Ajoute la force à la particule (dans la direction opposée)
                    particle->addForce(glm::normalize(distanceVec) * forceMagnitude);
                }
            }


        }

    private:
        float K; // Coefficient magnétique
        const std::vector<QmParticle*> particles; // Référence aux autres particules
        static constexpr float EPS = 1e-6f; // Constante pour éviter les divisions par zéro
    };

}

#endif
