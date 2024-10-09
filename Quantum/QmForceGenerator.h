#include "stdafx.h"
#ifndef QMFORCEGENERATOR_H
#define QMFORCEGENERATOR_H

#include "QmParticle.h"

namespace Quantum {

    class QmForceGenerator {
    public:
        virtual ~QmForceGenerator() {}
        virtual void update(QmParticle* particle) = 0; // M�thode abstraite pour mettre � jour la force
    };

}

#endif