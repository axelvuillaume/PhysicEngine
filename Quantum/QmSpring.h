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
        // Constructeur pour un ressort unique
        QmSpring(QmParticle* parent, float restLength, float springConstant)
            : parentParticle(parent), restLength(restLength), springConstant(springConstant) {}

        // Constructeur pour un ressort avec des enfants
        QmSpring(QmParticle* parent, std::vector<QmParticle*> children, float restLength, float springConstant)
            : parentParticle(parent), childParticles(children), restLength(restLength), springConstant(springConstant) {}

        void addChild(QmParticle* child) {
            childParticles.push_back(child);
        }

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
                    child->addForce(glm::normalize(d) * coeff);
                }
            }
        }

        std::vector<QmParticle*> getAllParticles() {
            std::vector<QmParticle*> allParticles;
            allParticles.push_back(parentParticle); // Ajouter la particule mère
            allParticles.insert(allParticles.end(), childParticles.begin(), childParticles.end()); // Ajouter les enfants
            return allParticles;
        }
    };
}

#endif