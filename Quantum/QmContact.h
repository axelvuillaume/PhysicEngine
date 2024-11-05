#include "stdafx.h"

#ifndef QMCONTACT_H
#define QMCONTACT_H

#include "QmParticle.h"
#include "QmHalfSpace.h"
#include <glm/glm.hpp>
#include <vector>
#include <iostream>


namespace Quantum {

    class QmContact {
    public:
        float depth;
        glm::vec3 normal;
        QmBody* body1;
        QmBody* body2;

        QmContact() : depth(0), normal(glm::vec3(0)), body1(nullptr), body2(nullptr) {}

        QmContact(QmParticle* b1, QmParticle* b2, const glm::vec3& contactNormal, float penetrationDepth)
            : body1(b1), body2(b2), normal(contactNormal), depth(penetrationDepth) {}

        QmContact(QmParticle* b1, QmHalfSpace* b2, const glm::vec3& contactNormal, float penetrationDepth)
            : body1(b1), body2(b2), normal(contactNormal), depth(penetrationDepth) {}

 
    };


}

#endif
