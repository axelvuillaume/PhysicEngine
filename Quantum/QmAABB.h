#include "stdafx.h"

#ifndef QMAABB_H
#define QMAABB_H

#include <glm/glm.hpp>
#include <iostream>

namespace Quantum {

    class QmAABB {
    public:
        glm::vec3 min; // Coin inférieur gauche
        glm::vec3 max; // Coin supérieur droit

        // Constructeur par défaut
        QmAABB() : min(glm::vec3(0)), max(glm::vec3(0)) {}

        // Constructeur avec des valeurs données
        QmAABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
    };


}

#endif