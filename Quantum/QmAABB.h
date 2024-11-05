#include "stdafx.h"

#ifndef QMAABB_H
#define QMAABB_H

#include <glm/glm.hpp>
#include <iostream>

namespace Quantum {

    class QmAABB {
    public:
        glm::vec3 min; // Coin inf�rieur gauche
        glm::vec3 max; // Coin sup�rieur droit

        // Constructeur par d�faut
        QmAABB() : min(glm::vec3(0)), max(glm::vec3(0)) {}

        // Constructeur avec des valeurs donn�es
        QmAABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
    };


}

#endif