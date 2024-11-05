#include "stdafx.h"
#ifndef QMFIXEDMAGNETISM_H
#define QMFIXEDMAGNETISM_H

#include "QmForceGenerator.h"
#include "QmParticle.h"
#include <glm/glm.hpp>

namespace Quantum {
    class QmFixedMagnetism : public QmForceGenerator {
    public:
        // Constructeur avec le coefficient magnétique, la position initiale et la charge fixe
        QmFixedMagnetism(float k, const glm::vec3& initialPos, float fixedCharge)
            : K(k), position(initialPos), FIXEDCHARGE(fixedCharge) {}

        void update(QmParticle* particle) override {
            glm::vec3 d = particle->getPos() - position;
            float coeff = K * (particle->getCharge() * FIXEDCHARGE); // Utilisation de la charge fixe
            particle->addForce(glm::normalize(d) * coeff / (glm::length(d) * glm::length(d) + EPS));
        }

        // Méthode pour mettre à jour la position de la source magnétique
        void setPosition(const glm::vec3& newPosition) {
            position = newPosition;
        }

    private:
        glm::vec3 position; // Position actuelle de la source de magnétisme
        float K;
        float FIXEDCHARGE; // Charge fixe
        static constexpr float EPS = 1e-6f; // Pour éviter la division par zéro
    };
}

#endif
