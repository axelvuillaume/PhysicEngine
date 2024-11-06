#include "stdafx.h"
#include <iostream>
#include "QmParticle.h"
#include "QmUpdater.h"


using namespace Quantum;

QmParticle::QmParticle(bool isStatic, float radius)
	: position(0, 0, 0), velocity(0, 0, 0), acceleration(0, 0, 0),
	forceAccumulator(0, 0, 0), mass(5.0f), charge(0.0f),
	isAffectedByGravity(false), restitution(0.5f), isStatic(isStatic), radius(radius) // Initialisation de isStatic
{
}


QmParticle::QmParticle() : position(0, 0, 0), velocity(0, 0, 0), acceleration(0, 0, 0), forceAccumulator(0, 0, 0), mass(5.0f), charge(0.0f), isAffectedByGravity(true), restitution(0.5f)
{
}

QmParticle::QmParticle(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc, float mass, float charge, float radius, float restitution) : QmParticle()
{
	position = pos;
	velocity = vel;
	acceleration = acc;
	this->mass = mass;
	damping = 0.990f;
	type = TYPE_PARTICLE;
	this->charge = charge;
	this->radius = radius;
	this->restitution = restitution;
}


QmParticle::~QmParticle()
{
	//std::cout << "Particule détruite: " << this << std::endl;
	delete updater;
}

void QmParticle::integrate(float t)
{
	if (isStatic) return;
	glm::vec3 resultantAcc = acceleration + forceAccumulator / mass;

	velocity += t * resultantAcc;  
	velocity *= damping;
	position += t * (velocity + 0.5f * t * resultantAcc);
	if (updater != NULL)
	{
		updater->update(position);
	}
	forceAccumulator = glm::vec3(0.0f);
}


void QmParticle::addForce(const glm::vec3& force) {
	forceAccumulator += force;
}

glm::vec3 QmParticle::getAcc()
{
	return acceleration;
}

glm::vec3 QmParticle::getVel()	
{
	return velocity;
}

float QmParticle::getCharge()
{
	return charge;
}

glm::vec3 QmParticle::getPos()
{
	return position;
}

float QmParticle::getMass() {
	return mass;
}

float QmParticle::getRadius() {
	return radius;
}


glm::vec3 QmParticle::getNetForce() {
	return forceAccumulator; 
}

void QmParticle::setAcc(const glm::vec3& newAcc) {
	acceleration = newAcc;
}

void QmParticle::setVel(const glm::vec3& newVel) {
	velocity = newVel;
}

void QmParticle::setPos(const glm::vec3& newPos) {
	position = newPos;
}

void QmParticle::setUpdater(QmUpdater* updater)
{
	this->updater = updater;
}

void QmParticle::reset() {
	acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	forceAccumulator = glm::vec3(0.0f, 0.0f, 0.0f);
}


QmAABB QmParticle::getAABB() {
	glm::vec3 radiusVec(radius, radius,radius);
	return QmAABB(position - radiusVec, position + radiusVec);
}




