#pragma once
#ifndef QMBODY_H
#define QMBODY_H

#include <glm/glm.hpp>
#include "QmUpdater.h"

namespace Quantum {

	const int TYPE_PARTICLE = 0;
	
	class QmBody {
	public:
		virtual void integrate(float) {};
		int getType() const { return type; }

	protected:
		int type;
	private:
	};
}

#endif