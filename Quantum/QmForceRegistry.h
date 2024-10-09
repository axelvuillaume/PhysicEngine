#include "stdafx.h"

#ifndef QMFORCEREGISTRY_H
#define QMFORCEREGISTRY_H

#include <vector>
#include "QmParticle.h"
#include "QmForceGenerator.h"
#include <iostream>

namespace Quantum {

    class QmForceRegistry {
    public:

        void addForceRegistry(QmParticle* particle, QmForceGenerator* generator) {
            p.push_back(particle);
            fg.push_back(generator);
        }

        void updateForces(float deltaTime) {
            //std::cout << "Nombre de particules dans le registre: " << p.size() << std::endl;
            for (size_t i = 0; i < p.size(); ++i) {
                fg[i]->update(p[i]);
            }
        }

        size_t getParticleCount() const {
            return p.size(); // Retourne le nombre de particules
        }

        QmParticle* getParticle(size_t index) const {
            if (index < p.size()) {
                return p[index]; // Retourne la particule à l'index donné
            }
            return nullptr; // Renvoie nullptr si l'index est invalide
        }

        QmForceGenerator* getForceGen(size_t index) const {
            if (index < fg.size()) {
                return fg[index]; // Retourne le générateur de force à l'index donné
            }
            return nullptr; // Renvoie nullptr si l'index est invalide
        }




    private:
        std::vector<QmParticle*> p;      // Liste des particules
        std::vector<QmForceGenerator*> fg; // Liste des générateurs de force
    };

}

#endif
