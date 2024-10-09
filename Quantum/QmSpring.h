#include "stdafx.h"

#ifndef QMSPRING_H
#define QMSPRING_H

#include "QmForceGenerator.h" 
#include "QmParticle.h"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace Quantum {

    class QmSpring : public QmForceGenerator {
    private:
        QmParticle* parentParticle; // Référence à la particule mère
        std::vector<QmParticle*> childParticles; // Liste de particules enfants
        float restLength; // Longueur de repos
        float springConstant; // Constante du ressort

    public:
        // Constructeur pour un ressort avec des enfants
        QmSpring(QmParticle* parent, std::vector<QmParticle*> children, float restLength, float springConstant)
            : parentParticle(parent), childParticles(children), restLength(restLength), springConstant(springConstant) {}

        virtual void update(QmParticle* p) override {
            if (!p || !parentParticle) {
                std::cerr << "Spring error: One of the particles is null!" << std::endl;
                return;
            }

            // Appliquer la force de la mère aux enfants
            for (QmParticle* child : childParticles) {
                if (child) {
                    glm::vec3 d = child->getPos() - parentParticle->getPos();
                    float length = glm::length(d);
                    float coeff = -((length - restLength) * springConstant);

                    glm::vec3 force = glm::normalize(d) * coeff;
              
                    child->addForce(force);
                }
            }
        }

        std::vector<QmParticle*> getOtherParticles() {
            return childParticles;
        }


        QmParticle* getParent() {
            return parentParticle;
        }
    };
}

#endif