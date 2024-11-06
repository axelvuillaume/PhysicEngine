#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <ctime>
#include <string>

#include "Quantum.h"
#include "GxWorld.h"
#include "GxParticle.h"
#include "GxUpdater.h"
#include "QmDrag.h"
#include "QmMagnetism.h"
#include "QmFixedMagnetism.h"
#include "QmSpring.h"
#include <random>

using namespace std;
using namespace Quantum;


GxWorld gxWorld;
QmWorld pxWorld;
std::vector<QmParticle*> fireworksParticles;

glm::vec3* mousePointer;

int scene = 0;

// ********************** GLUT 
// Variables globales

int SCREEN_X = 800;
int SCREEN_Y = 800;
int VIEWPORT_X = 5; int VIEWPORT_Y = 5; int VIEWPORT_Z = 200;

GLuint DrawListSphere;

int buttons = 0;
float camRotate = 0.f;
float camDist = 30.f;
float camHeight = 0.f;
float mx = 0.f;
float my = 0.f;
float fixedCharge = 1.0f;
float particleSpawnTimer = 0.0f;
const float particleSpawnInterval = 0.001f;
QmParticle* cursorControlledParticle = nullptr;

std::vector<QmHalfSpace*> halfSpaces;

// Variables du calcul de framerate 
int timeold = 0;
int frame = 0;
float accTime = 0.0;
bool paused = false;

GLfloat light_pos[] = { 0.0, 6.0, 1.0, 1.0 };
QmFixedMagnetism* sharedFixedMagnetism = nullptr;

// ********************************************


glm::vec3 randomVector(float min, float max)
{
	float x = min + (max - min)*((rand() % 100) / 100.f);
	float y = min + (max - min)*((rand() % 100) / 100.f);
	float z = min + (max - min)*((rand() % 100) / 100.f);
	return glm::vec3(x, y, z);

}

float randomFloat(float min, float max) {
	float res = min + (max - min)*((rand() % 100) / 100.f); 
	return res;// Retourne un nombre aléatoire entre min et max
}

QmParticle* CreateMagnetismParticle(bool isPositive) {
	glm::vec3 pos = randomVector(-5, 5);
	float charge = isPositive ? 1.0f : -1.0f;
	float radius = 0.3f + 0.1f * ((rand() % 100) / 100.f);
	GxParticle* g = new GxParticle(randomVector(1, 0), radius, pos);
	QmParticle* p = new QmParticle(pos, randomVector(0, 0), randomVector(0, 0), 0.5f, charge, radius);
	p->setUpdater(new GxUpdater(g));

	// Récupération des particules existantes pour QmMagnetism
	std::vector<QmParticle*> particles;
	for (QmBody* body : pxWorld.getBodies()) {
		QmParticle* particle = static_cast<QmParticle*>(body);
		if (particle) {
			particles.push_back(particle);
		}
	}
	//std::cout << "Nombre de corps dans le monde: " << pxWorld.getBodies().size() << std::endl;

	// Ajout de la force magnétique à la nouvelle particule
	QmMagnetism* magnetismForce = new QmMagnetism(10.0f, particles);
	QmForceRegistry* registryScene = new QmForceRegistry();
	registryScene->addForceRegistry(p, magnetismForce);

	//std::cout << " p charge " << p->getCharge() << std::endl;

	// Ajout de la particule aux mondes
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	pxWorld.addForceRegistry(registryScene);
	return p;
}


QmParticle* createParticleRadius()
{
	glm::vec3 pos = glm::vec3(randomFloat(-5, 5), randomFloat(-5, 5), 0);
	float radius = 0.1f + 0.2f * ((rand() % 100) / 100.f);
	GxParticle* g = new GxParticle(randomVector(1, 0), radius, pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(randomFloat(-2, 2), randomFloat(-2, 2),0), glm::vec3(randomFloat(-2, 2), randomFloat(-2, 2), 0), 0.5f,-1 , radius);
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}

QmParticle* createParticle()
{
	glm::vec3 pos = randomVector(-5, 5);
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(0, 0, 0), randomVector(0, 0));
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}

QmParticle* createParticleRadius4(glm::vec3 pos, float radius, float mass)
{

	GxParticle* g = new GxParticle(randomVector(1, 0), radius, pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), mass, -1, radius);
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}



QmParticle* createFountainParticle(glm::vec3 pos)
{
	float radius = 0.1f + 0.2f * ((rand() % 100) / 100.f);
	GxParticle* g = new GxParticle(randomVector(1, 0), radius, pos);
	QmParticle* p = new QmParticle(pos, randomVector(-10.0f, 10.0f), randomVector(-1, 1),radius = radius);
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}

QmParticle* createFireWorksParticle()
{
	glm::vec3 pos = *mousePointer;
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, randomVector(9, 10), randomVector(0, 0));
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}

QmParticle* createParticleForce(QmForceRegistry* registryScene)
{
	glm::vec3 pos = randomVector(-5, 5);
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(0, 0, 0), randomVector(0, 0));
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	pxWorld.addForceRegistry(registryScene);
	return p;
}

QmParticle* createParticleNoGravity()
{
	glm::vec3 pos = randomVector(-5, 5);
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(0, 0, 0), randomVector(0, 0), 0.5f, -1.0f, 0.0f);
	p->setUpdater(new GxUpdater(g));
	p->isAffectedByGravity = false;
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}

QmParticle* createParticleNoColisionGravity() {
	float radius = 0.1f + 0.2f * ((rand() % 100) / 100.f);
	GxParticle* g = new GxParticle(randomVector(1, 0), radius, glm::vec3(0, 0, 0));
	QmParticle* p = new QmParticle(false, radius);
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}

QmParticle* staticPlinko() {
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.09f, glm::vec3(0, 0, 0));
	QmParticle* p = new QmParticle(false, 0.09f);
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}

QmParticle* dynamiquePlinko(int levels) {
	float spacing = 0.5f; // Espace entre chaque particule
	float yOffset = -5.0f; // Décalage vers le bas
	float pyramidHeight = (levels - 1) * spacing + yOffset; // Hauteur maximale de la pyramide

	float xOffset = -5.0f; // Décalage sur l'axe X
	float zOffset = 10.0f; // Décalage sur l'axe Z

	// Générer une position aléatoire au-dessus de la pyramide
	glm::vec3 pos = glm::vec3(randomFloat(-7.0f, -3.0f),
		randomFloat(pyramidHeight + 0.1f, pyramidHeight + 5.0f),
		randomFloat(2.2f, 6.4f));
	float radius = 0.1f + 0.2f * ((rand() % 100) / 100.f);
	GxParticle* g = new GxParticle(randomVector(1, 0), radius, pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(randomFloat(-2, 2), randomFloat(-2, 2), 0), glm::vec3(randomFloat(-2, 2), randomFloat(-2, 2), 0), 0.5f, -1, radius);
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}


void createStaticPlinkoPyramid(int levels) {
	float spacing = 0.8f; // Espace entre chaque particule
	float yOffset = -5.0f; // Décalage vers le bas
	float zOffset = 5.0f; // Décalage vers la caméra
	float xOffset = -5.0f; // Décalage sur l'axe X

	for (int i = 0; i < levels; ++i) {
		int count = levels - i;
		float offset = -count * spacing * 0.5f;
		for (int x = 0; x < count; ++x) {
			for (int z = 0; z < count; ++z) {
				glm::vec3 pos = glm::vec3((x * spacing + offset) + xOffset,
					i * spacing + yOffset,
					(z * spacing + offset) + zOffset);
				QmParticle* particle = staticPlinko();
				particle->setPos(pos); // Initialiser la position de la particule
			}
		}
	}
}

QmParticle* CreateDragParticle2()
{
	glm::vec3 pos = glm::vec3(5, 10,0);
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(0, -3, 0), randomVector(0, 0));
	p->setUpdater(new GxUpdater(g));

	QmDrag* dragForce = new QmDrag(0.5f, 0.5f);
	QmForceRegistry* registryScene2 = new QmForceRegistry();
	registryScene2->addForceRegistry(p, dragForce);
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	pxWorld.addForceRegistry(registryScene2);
	return p;

}


QmParticle* CreateFixedMagnetismParticle(glm::vec3 fixedPos, float fixedCharge)
{
	glm::vec3 pos = randomVector(-5, 5);
	float charge = (std::rand() % 2 == 0) ? 1.0f : -1.0f;
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, randomVector(0, 0), randomVector(0, 0), 0.5f, charge);
	p->setUpdater(new GxUpdater(g));

	// Ajoute la même force de magnétisme fixe pour chaque particule
	if (sharedFixedMagnetism) {
		QmForceRegistry* registryScene = new QmForceRegistry();
		registryScene->addForceRegistry(p, sharedFixedMagnetism);
		pxWorld.addForceRegistry(registryScene);
	}

	std::cout << " p charge " << p->getCharge() << std::endl;

	// Ajout de la particule aux mondes
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}



void explodeParticle(QmParticle* particle)
{
	glm::vec3 pos = particle->getPos();

	for (int i = 0; i < 20; ++i)
	{
		GxParticle* g = new GxParticle(randomVector(1, 0), 0.05f + 0.15f * ((rand() % 100) / 100.f), pos);
		QmParticle* p = new QmParticle(pos, randomVector(-3, 3), randomVector(2, -2));  
		p->setUpdater(new GxUpdater(g));
		gxWorld.addParticle(g);
		pxWorld.addBody(p);
	}
}

void updateFireWorksParticle(QmParticle* particle, float deltaTime)
{
	glm::vec3 velocity = particle->getVel();

	if (glm::length(velocity) < 0.1f || particle->getPos().y > 10.0f) 
	{
		explodeParticle(particle); 
		//particle->~QmParticle();
	}
}


void createFixedMagnScene(glm::vec3 pos)
{

	// Créer des particules avec un magnétisme fixe
	for (int i = 0; i < 10; ++i) {
		CreateFixedMagnetismParticle(pos, fixedCharge);
	}
}


// ***************************** GLUT methods

void resetView()
{
	camRotate = 90.f;
	camDist = 30.f;
	camHeight = 0.f;
	mx = 0.f;
	my = 0.f;
	if (mousePointer) {
		delete mousePointer;
		mousePointer = NULL;
	}
}

void createDisplayListSphere()
{
	DrawListSphere = glGenLists(1);
	glNewList(DrawListSphere, GL_COMPILE);
	GLUquadricObj* sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluSphere(sphere, 1, 12, 8);
	glEndList();
}

void initLight()
{
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHT0);

	GLfloat ambient_light[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat diffuse_light[] = { 0.6f, 0.6f, 0.6f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_LIGHTING);
}

void calculateFPS(float dt)
{
	accTime += dt;

	if (accTime > 0.5)
	{
		char t[200];
		sprintf_s(t, "Quantum Physics Engine: Scene %d, %.2f FPS", scene, frame / accTime);
		glutSetWindowTitle(t);
		accTime = 0.0;
		frame = 0;
	}
}

void updateScene2(float deltaTime)
{
	particleSpawnTimer += deltaTime;

	if (particleSpawnTimer >= particleSpawnInterval) {
		createFountainParticle(*mousePointer);
		particleSpawnTimer = 0.0f; 
	}
}

void idleFunc()
{
	int timer = glutGet(GLUT_ELAPSED_TIME);
	float dt = (float)(timer - timeold) / 1000.f;
	timeold = timer;
	if (scene == 2) {
		updateScene2(dt);
	}
	
	if (scene == 3) {
		for (QmParticle* particle : fireworksParticles)
		{
			updateFireWorksParticle(particle, dt);

		}
	}

	if (scene == 6) {
		if (sharedFixedMagnetism) {
			// Met à jour la position de la source de magnétisme avec celle du curseur
			sharedFixedMagnetism->setPosition(*mousePointer);
		}

		// Mets à jour les forces dans QmForceRegistry
		pxWorld.updateForces(dt);
	}

	if (scene == 7) {
		if (cursorControlledParticle) {
				cursorControlledParticle->setPos(*mousePointer);
		}

		pxWorld.updateForces(dt);
	}

	calculateFPS(dt);
	if (!paused) pxWorld.simulate(dt);


	glutPostRedisplay();
}


    void drawHalfSpace(const QmHalfSpace* halfSpace) {
        if (!halfSpace) return; // Vérifier que l'objet n'est pas nul

        // Normaliser la normale
        glm::vec3 up = glm::normalize(halfSpace->normal);
        glm::vec3 right;
        glm::vec3 forward;

        // Déterminer les directions basées sur la normale
        if (up.y == 1.0f || up.y == -1.0f) { // Sol ou plafond
            right = glm::normalize(glm::cross(up, glm::vec3(1.0f, 0.0f, 0.0f)));
            forward = glm::normalize(glm::cross(up, right));
        } else { // Murs (normales horizontales)
            right = glm::normalize(glm::cross(up, glm::vec3(0.0f, 1.0f, 0.0f)));
            forward = glm::normalize(glm::cross(up, right));
        }

        // Déterminer les coins du plan
        glm::vec3 corner1 = halfSpace->offset * up - 50.0f * right - 50.0f * forward; // Coin inférieur gauche
        glm::vec3 corner2 = halfSpace->offset * up + 50.0f * right - 50.0f * forward; // Coin inférieur droit
        glm::vec3 corner3 = halfSpace->offset * up + 50.0f * right + 50.0f * forward; // Coin supérieur droit
        glm::vec3 corner4 = halfSpace->offset * up - 50.0f * right + 50.0f * forward; // Coin supérieur gauche

        // Couleur (facultatif)
		glColor3f(0.8f, 0.4f, 0.3f); // Couleur gris pour le sol

        // Définir les coins du quadrilatère
        glBegin(GL_QUADS);
        glVertex3f(corner1.x, corner1.y, corner1.z);
        glVertex3f(corner2.x, corner2.y, corner2.z);
        glVertex3f(corner3.x, corner3.y, corner3.z);
        glVertex3f(corner4.x, corner4.y, corner4.z);
        glEnd(); // Terminer le dessin
    }


void drawHalfSpaces(const std::vector<QmHalfSpace*>& halfSpaces) {
	for (const auto& halfSpace : halfSpaces) {
		drawHalfSpace(halfSpace); // Appeler la fonction pour chaque QmHalfSpace
	}
}


// INIT SCENE

void initScene1()
{
	printf("Scene 1: Random particles.\n");
	printf("Type space to pause.\n");


	for (int i = 0; i < 20; i++)
		createParticleRadius();



	halfSpaces.push_back(new QmHalfSpace(glm::vec3(0, 1, 0), -10.0f)); // Sol
	halfSpaces.push_back(new QmHalfSpace(glm::vec3(0, -1, 0), -10.0f)); // Plafond
	halfSpaces.push_back(new QmHalfSpace(glm::vec3(1, 0, 0), -10.0f));  // Mur droit
	halfSpaces.push_back(new QmHalfSpace(glm::vec3(-1, 0, 0), -10.0f)); // Mur gauche

	// Dessiner tous les halfSpaces
	drawHalfSpaces(halfSpaces);

	for (const auto& halfSpace : halfSpaces) {
		pxWorld.addBody(halfSpace);
	}
	


}


void initScene2()
{
	printf("Scene 2. Fountain\n");
	mousePointer = new glm::vec3(0, 4.5, 0);
	glutIdleFunc(idleFunc);
}

void initScene3()
{
	printf("Scene 3. Fireworks\n");
	mousePointer = new glm::vec3(0, 4.5, 0);
	QmParticle* firework = createFireWorksParticle();
	fireworksParticles.push_back(firework);
}

void initScene4()
{
	printf("Scene 4. Test Froce drag\n");
	createParticle();
	CreateDragParticle2();
}

void initScene5()
{
	printf("Scene 5. Magnetism\n");
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(false);
	CreateMagnetismParticle(false);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(false);
	CreateMagnetismParticle(false);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(false);
	CreateMagnetismParticle(false);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(true);
	CreateMagnetismParticle(false);

}



void initScene6()
{
	printf("Scene 6. Qm magnetism Fixed\n");
	mousePointer = new glm::vec3(0, 4.5, 0);

	sharedFixedMagnetism = new QmFixedMagnetism(1.0f, *mousePointer, fixedCharge);
	createFixedMagnScene(*mousePointer);
}

void initScene7()
{
	printf("Scene 7. Qm Spring\n");
	mousePointer = new glm::vec3(0, 0, 0);

	float restLength = 2.0f;
	float springConstant = 3.0f;


	QmForceRegistry* registry = new QmForceRegistry();

	QmParticle* parentParticle = createParticleNoColisionGravity();

	cursorControlledParticle = parentParticle;

	std::vector<QmParticle*> childParticles;
	int numParticles = 12; 

	childParticles.push_back(parentParticle);
	for (int i = 0; i < numParticles; ++i) {

		childParticles.push_back(createParticle());
	}

	std::vector<std::pair<int, std::vector<int>>> connections = {
	{0, {1, 2, 3,4}},
	{1, {2, 3, 4}},    
	{2, {1, 3, 4}},   
	{3, {1, 2, 4}},  
	{4, {1,2,3,5}},
	{5, {4,8,7,6}},   
	{6, {5,8,7}}, 
	{7, {5,6,8}},       
	{8, {5,6,7,9}},      
	{9, {8,10,11,12}}, 
	{10, {9,11,12 }},
	{11, {9,10,12}},
	{12, {9,10,11}},
	};

	for (const auto& connection : connections) {
		int parentIndex = connection.first;
		std::vector<QmParticle*> connectedParticles;

	
		for (int childIndex : connection.second) {
			connectedParticles.push_back(childParticles[childIndex]);
		}

		
		QmSpring* springForce = new QmSpring(childParticles[parentIndex], connectedParticles, restLength, springConstant);

		
		for (QmParticle* child : connectedParticles) {
			registry->addForceRegistry(child, springForce);
		}
	}

	pxWorld.addForceRegistry(registry);

}



void initScene8()
{
	printf("Scene 6. Qm magnestism Fixed\n");

	dynamiquePlinko(10);
	dynamiquePlinko(10);
	dynamiquePlinko(10);
	dynamiquePlinko(10);
	dynamiquePlinko(10);
	dynamiquePlinko(10);
	createStaticPlinkoPyramid(10);
}


void initScene9()
{
	printf("Scene 9. ?????????\n");
	

	createParticleRadius4(glm::vec3(0, -5, -10), 5.0f, 5.0f);
	createParticleRadius4(glm::vec3(0, 1, -10), 1.0f, 1.0f);


	createParticleRadius4(glm::vec3(20, -3, -10), 5.0f, 5.0f);
	createParticleRadius4(glm::vec3(20, -9, -10), 1.0f, 1.0f);


	halfSpaces.push_back(new QmHalfSpace(glm::vec3(0, 1, 0), -10.0f)); // Sol

	drawHalfSpaces(halfSpaces);

	for (const auto& halfSpace : halfSpaces) {
		pxWorld.addBody(halfSpace);
	}

}

void drawSprings()
{
	// Parcours de tous les registres de force dans le monde physique
	for (const auto& fr : pxWorld.getForceRegistries())
	{
		// Itération sur chaque particule du registre
		for (size_t i = 0; i < fr->getParticleCount(); ++i)
		{
			QmParticle* particle = fr->getParticle(i); // Récupère la particule
			QmForceGenerator* forceGen = fr->getForceGen(i); // Récupère le générateur de force

			// Vérifie si le générateur de force est un ressort
			if (auto* spring = dynamic_cast<QmSpring*>(forceGen))
			{
				// Récupérer la particule parent
				QmParticle* parentParticle = spring->getParent();
				glm::vec3 parentPos = parentParticle->getPos(); // Position du parent

				// Itération sur toutes les particules enfants liées par le ressort
				for (QmParticle* childParticle : spring->getOtherParticles())
				{
					glm::vec3 childPos = childParticle->getPos(); // Position de l'enfant

					// Dessiner une ligne entre le parent et l'enfant
					glBegin(GL_LINES);
					glColor3f(1.f, 1.f, 1.f); // Couleur de la ligne (blanc)
					glVertex3f(parentPos.x, parentPos.y, parentPos.z); // Position du parent
					glVertex3f(childPos.x, childPos.y, childPos.z); // Position de l'enfant
					glEnd();
				}
			}
		}
	}
}

void drawFunc()
{
	frame++;
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camDist * cos(camRotate), camHeight, camDist * sin(camRotate), 0, camHeight, 0, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

	for (GxParticle* p : gxWorld.getParticles())
	{
		glPushMatrix();
		glm::vec3 color = p->getColor();
		glColor3f(color.x, color.y, color.z);
		glTranslatef(p->getPos().x, p->getPos().y, p->getPos().z);
		glScaled(p->getRadius(), p->getRadius(), p->getRadius());
		glCallList(DrawListSphere);
		glPopMatrix();
	}
	if (mousePointer) {
		glPushMatrix();
		glColor3f(1.f, 1.f, 1.f);
		glTranslatef(mousePointer->x, mousePointer->y, mousePointer->z);
		glScaled(0.1f, 0.1f, 0.1f);
		glCallList(DrawListSphere);
		glPopMatrix();
	}


	drawSprings();

	drawHalfSpaces(halfSpaces);

	glutSwapBuffers();
}


void mouseFunc(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) buttons = 1;
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) buttons = 2;
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) buttons = 3;
	if (button == 3) camDist /= 1.07f;
	if (button == 4) camDist *= 1.07f;
	if (state == GLUT_UP) buttons = 0;

	mx = (float)x;
	my = (float)y;
}

void motionFunc(int x, int y)
{
	if (buttons == 1)
	{
		camRotate += (x - mx) / 100.f;
		camHeight += (y - my) * camDist / 1000.f;
	}
	if (buttons == 3)
	{
		if (mousePointer)
			*mousePointer += glm::vec3(x - mx, my - y, 0.f) / 15.f;
	}

	mx = (float)x;
	my = (float)y;
}

void clearWorld()
{
	gxWorld.clear();
	pxWorld.clear();
	halfSpaces.clear();

}

void toggleScene(int s)
{


	clearWorld();
	scene = s;
	resetView();

	
	switch (scene)
	{
	case 1: initScene1(); break;
	case 2: initScene2(); break;
	case 3: initScene3(); break;
	case 4: initScene4(); break;
	case 5: initScene5(); break;
	case 6: initScene6(); break;
	case 7: initScene7(); break;
	case 8: initScene8(); break;
	case 9: initScene9(); break;
	}
}


void keyFunc(unsigned char key, int x, int y)
{

	switch (key)
	{
	case 'q': case 'Q': case 27:
		clearWorld();
		glDeleteLists(DrawListSphere, 1);

		exit(0);
		break;
	case '1':
		toggleScene(1);
		break;
	case '2':
		toggleScene(2);
		break;
	case '3':
		toggleScene(3);
		break;

	case '4':
		toggleScene(4);
		break;

	case '5':
		toggleScene(5);
		break;

	case '6':
		toggleScene(6);
		break;
	case '7':
		toggleScene(7);
		break;
	case '8':
		toggleScene(8);
		break;
	case '9':
		toggleScene(9);
		break;

	case 'a':
		fixedCharge = (fixedCharge == 1.0f) ? -1.0f : 1.0f;
		std::cout << "Fixed charge is now: " << fixedCharge << std::endl;
		break;

	case 'g':
		pxWorld.setGravityEnabled(!pxWorld.isGravityEnabled()) ;
		std::cout << "Gravity toggled: "
			<< (pxWorld.isGravityEnabled() ? "Enabled" : "Disabled") << std::endl;
		break;

	case 'c':
		pxWorld.enableCollisions(!pxWorld.collisionsEnabled);
		std::cout << "Collisions toggled: "
			<< (pxWorld.collisionsEnabled ? "Enabled" : "Disabled") << std::endl;
		break;

	case 's':
		pxWorld.toggleStaticBodies();
		break;
	case 'f':  // Basculer entre framerate indépendant activé/désactivé
		pxWorld.framerateIndependent = !pxWorld.framerateIndependent;
		std::cout << "Framerate Independence "
			<< (pxWorld.framerateIndependent ? "Enabled" : "Disabled")
			<< std::endl;
		break;

	case ' ':
		paused = !paused;
		break;
	default:
		break;
	}
}

void initGraphics(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(SCREEN_X, SCREEN_Y);
	glutCreateWindow("Quantum Demo");

	glutIdleFunc(idleFunc);
	glutDisplayFunc(drawFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);
	glutKeyboardFunc(keyFunc);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_PROGRAM_POINT_SIZE);
	//glPointSize(5);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = SCREEN_X / (float)SCREEN_Y;
	gluPerspective(45.0, aspect, 1.0, VIEWPORT_Z);
	glViewport(0, 0, SCREEN_X, SCREEN_Y);
	createDisplayListSphere();
	initLight();
}

// ************************** end GLUT methods

int main(int argc, char** argv)
{
	srand((unsigned int)time(NULL));
	initGraphics(argc, argv);

	toggleScene(2);

	glutMainLoop();
	return 0;
}



