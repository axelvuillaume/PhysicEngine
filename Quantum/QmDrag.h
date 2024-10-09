#include "stdafx.h"

#ifndef QMDRAG_H
#define QMDRAG_H

#include "QmForceGenerator.h"
#include "QmParticle.h"
#include <glm/glm.hpp>
#include <iostream>

namespace Quantum {

    class QmDrag : public QmForceGenerator {
    private:
        float K1, K2;

    public:
        QmDrag(float k1, float k2) : K1(k1), K2(k2) {}

        virtual void update(QmParticle* particle) override {
            //std::cout << "IN QMDRAG " << std::endl;
            glm::vec3 velocity = particle->getVel();
            float speed = glm::length(velocity);

            if (speed > 0) {
                float dragCoeff = -(K1 * speed + K2 * speed * speed);
                glm::vec3 dragForce = glm::normalize(velocity) * dragCoeff;

                particle->addForce(dragForce);
            }
        }
    };

}
#endif