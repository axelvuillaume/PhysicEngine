#ifndef QMPARTICLE_H
#define QMPARTICLE_H

#include <glm/glm.hpp>
#include "QmBody.h"


namespace Quantum {
	class QmUpdater;

	class QmParticle : public QmBody {
	public:
		QmParticle();
		QmParticle(glm::vec3, glm::vec3, glm::vec3, float mass = 0.5f, float charge = 0.0f);
		~QmParticle();
		virtual void integrate(float t);
		QmUpdater* updater;
		bool isAffectedByGravity;
		
		
		glm::vec3 getAcc();
		glm::vec3 getVel();
		glm::vec3 getPos();
		glm::vec3 getNetForce();
		float getMass();

		void setAcc(const glm::vec3& newAcc);
		void setVel(const glm::vec3& newVel);
		void setPos(const glm::vec3& newPos);
		float getCharge();


		void setUpdater(QmUpdater* updater);
		void addForce(const glm::vec3& force);
		void reset();

	private:
		
		glm::vec3 position;
		glm::vec3 velocity;
		glm::vec3 acceleration;
		glm::vec3 forceAccumulator;
		float mass;
		float charge;

		float damping;

	};

}

#endif