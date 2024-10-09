#ifndef QMWORLD_H
#define QMWORLD_H

#include <list>
#include <vector>
#include "QmForceRegistry.h"

namespace Quantum {

	class QmBody;
	class QmForceRegistry;

	class QmWorld {
	public:
		QmWorld();
		~QmWorld();
		void simulate(float);
		void addBody(QmBody*);
		std::vector<QmBody*> getBodies();
		void addForceRegistry(QmForceRegistry* f);
		void updateForces(float deltaTime);
		void applyGravity();
		bool isGravityEnabled();
		bool setGravityEnabled(bool gravity);
		void interpolate(float dt);
		float tick(float t);
		void tickRK4(float t);
		void integrateRK4(float t);
		std::vector<QmForceRegistry*> getForceRegistries();
		void computeAccelerations(int index);
		

		void clear();
	private:
		float time;
		float ticktime;
		bool gravityEnabled;
		std::vector<QmBody*> bodies;
		std::vector<QmForceRegistry*> forceRegistries;
		glm::vec3 gravityForce;
		void integrate(float t);
	};

}

#endif