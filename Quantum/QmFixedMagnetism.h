#include "stdafx.h"
#ifndef QMFIXEDMAGNETISM_H
#define QMFIXEDMAGNETISM_H

#include "QmForceGenerator.h"
#include "QmParticle.h"
#include <glm/glm.hpp>

namespace Quantum {
    class QmFixedMagnetism : public QmForceGenerator {
    public:
        // Constructeur avec le coefficient magn�tique, la position initiale et la charge fixe
        QmFixedMagnetism(float k, const glm::vec3& initialPos, float fixedCharge)
            : K(k), position(initialPos), FIXEDCHARGE(fixedCharge) {}

        void update(QmParticle* particle) override {
            glm::vec3 d = particle->getPos() - position;
            float coeff = K * (particle->getCharge() * FIXEDCHARGE); // Utilisation de la charge fixe
            particle->addForce(glm::normalize(d) * coeff / (glm::length(d) * glm::length(d) + EPS));
        }

        // M�thode pour mettre � jour la position de la source magn�tique
        void setPosition(const glm::vec3& newPosition) {
            position = newPosition;
        }

    private:
        glm::vec3 position; // Position actuelle de la source de magn�tisme
        float K;
        float FIXEDCHARGE; // Charge fixe
        static constexpr float EPS = 1e-6f; // Pour �viter la division par z�ro
    };
}

#endif
