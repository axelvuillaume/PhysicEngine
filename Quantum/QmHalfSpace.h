#include "stdafx.h"

#ifndef QMHALFSPACE_H
#define QMHALFSPACE_H

#include "QmParticle.h"
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include "QmBody.h"

namespace Quantum {

    class QmHalfSpace : public QmBody {
    public:
        glm::vec3 normal; 
        float offset;    


        QmHalfSpace(const glm::vec3& n, float off) : normal(n), offset(off) {
            normal = glm::normalize(normal);
        }

        virtual QmAABB getAABB() override {
            glm::vec3 extents(100.0f, 100.0f, 100.0f); // Exemple de taille des dimensions de l'AABB
            glm::vec3 minBound = offset * normal - extents; // Limite inférieure de l'AABB
            glm::vec3 maxBound = offset * normal + extents; // Limite supérieure de l'AABB

            return QmAABB(minBound, maxBound);
        }


    };

}

#endif