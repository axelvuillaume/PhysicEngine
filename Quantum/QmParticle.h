#include "stdafx.h"

#ifndef QMPARTICLE_H
#define QMPARTICLE_H

#include <glm/glm.hpp>
#include "QmBody.h"

namespace Quantum {
	class QmUpdater;

	class QmParticle : public QmBody {
	public:
		QmParticle();
		QmParticle(glm::vec3, glm::vec3, glm::vec3, float mass = 0.5f, float charge = 0.0f, float radius = 0.5f, float resitution = 0.5f);
		~QmParticle();
		virtual void integrate(float t);
		QmUpdater* updater;
		bool isAffectedByGravity;
		
		
		glm::vec3 getAcc();
		glm::vec3 getVel();
		glm::vec3 getPos();
		glm::vec3 getNetForce();
		float getMass();
		float getRadius();

		void setAcc(const glm::vec3& newAcc);
		void setVel(const glm::vec3& newVel);
		void setPos(const glm::vec3& newPos);
		float getCharge();
		virtual QmAABB getAABB();


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
		float radius;
		float restitution;

		float damping;

	};

}

#endif