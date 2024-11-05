#include "stdafx.h"
#ifndef QMBODY_H
#define QMBODY_H
#include"QmAABB.h"

#include"QmUpdater.h"

namespace Quantum {

	const int TYPE_PARTICLE = 0;

	class QmBody {
	public:
		virtual void integrate(float t) {};
		virtual QmAABB getAABB() = 0;
		int getType() const { return type; }
		float getResti() const { return restitution; }

	protected:
		float restitution;
		int type;
	private:
	};
}

#endif