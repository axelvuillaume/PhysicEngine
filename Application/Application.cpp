// Physics-engine.cpp : définit le point d'entrée pour l'application console.
//

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
float camRotate = 90.f;
float camDist = 30.f;
float camHeight = 0.f;
float mx = 0.f;
float my = 0.f;
float fixedCharge = 1.0f;

// Variables du calcul de framerate 
int timeold = 0;
int frame = 0;
float accTime = 0.0;
bool paused = false;

GLfloat light_pos[] = { 0.0, 6.0, 1.0, 1.0 };

// ********************************************


glm::vec3 randomVector(float min, float max)
{
	float x = min + (max - min)*((rand() % 100) / 100.f);
	float y = min + (max - min)*((rand() % 100) / 100.f);
	float z = min + (max - min)*((rand() % 100) / 100.f);
	return glm::vec3(x, y, z);

}




QmParticle* createFountainParticle(glm::vec3 pos)
{
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, randomVector(-2, 2), randomVector(0, 0));
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


QmParticle* createParticle()
{
	glm::vec3 pos = randomVector(-5, 5);
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(0,0,0), randomVector(0, 0));
	p->setUpdater(new GxUpdater(g));
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
}

QmParticle* createParticleNoGravity()
{
	glm::vec3 pos = randomVector(-5, 5);
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, glm::vec3(0, 0, 0), randomVector(0, 0));
	p->setUpdater(new GxUpdater(g));
	p->isAffectedByGravity = false;
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	return p;
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

QmParticle* CreateMagnetismParticle(bool isPositive) {
	glm::vec3 pos = randomVector(-5, 5);
	float charge = isPositive ? 1.0f : -1.0f;
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, randomVector(0, 0), randomVector(0, 0),0.5f, charge);
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

QmParticle* CreateFixedMagnetismParticle(glm::vec3 fixedPos, float fixedCharge)
{
	glm::vec3 pos = randomVector(-5, 5);
	float charge = (std::rand() % 2 == 0) ? 1.0f : -1.0f; 
	GxParticle* g = new GxParticle(randomVector(1, 0), 0.1f + 0.2f * ((rand() % 100) / 100.f), pos);
	QmParticle* p = new QmParticle(pos, randomVector(0, 0), randomVector(0, 0), 0.5f, charge);
	p->setUpdater(new GxUpdater(g));

	// Création de la force magnétique fixe
	QmFixedMagnetism* fixedMagnetismForce = new QmFixedMagnetism(10.0f, fixedPos, fixedCharge);
	QmForceRegistry* registryScene = new QmForceRegistry();
	registryScene->addForceRegistry(p, fixedMagnetismForce);

	std::cout << " p charge " << p->getCharge() << std::endl;

	// Ajout de la particule aux mondes
	gxWorld.addParticle(g);
	pxWorld.addBody(p);
	pxWorld.addForceRegistry(registryScene);
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
	createFountainParticle(*mousePointer);
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
		QmFixedMagnetism fixedMagnetism(10.0f, *mousePointer, fixedCharge);

		for (QmBody* body : pxWorld.getBodies()) {
			QmParticle* particle = static_cast<QmParticle*>(body);
			if (particle) {
				fixedMagnetism.update(particle);
			}
		}

	}	

	calculateFPS(dt);
	if (!paused) pxWorld.simulate(dt);


	glutPostRedisplay();
}





// INIT SCENE

void initScene1()
{
	printf("Scene 1: Random particles.\n");
	printf("Type space to pause.\n");
	mousePointer = new glm::vec3(0, 4.5, 0);
	for (int i = 0; i < 100; i++)
		createParticle();
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
	printf("Scene 6. Qm magnestism Fixed\n");
	mousePointer = new glm::vec3(0, 4.5, 0);

	createFixedMagnScene(*mousePointer);
}

void initScene7()
{
	printf("Scene 7. Qm Spring\n");

	float restLength = 2.0f;
	float springConstant = 5.0f;

	// Créer le registre de forces
	QmForceRegistry* registry = new QmForceRegistry();

	// Créer la particule mère
	QmParticle* parentParticle = createParticleNoGravity();



	// Créer une liste de particules enfants
	std::vector<QmParticle*> childParticles;
	for (int i = 0; i < 4; ++i) {
		childParticles.push_back(createParticle());
	}

	QmSpring* springForce = new QmSpring(parentParticle, childParticles, restLength, springConstant);

	for (QmParticle* child : childParticles) {
		registry->addForceRegistry(child, springForce);
	}

	/*
	
	std::vector<QmParticle*> childParticles2;
	for (int i = 0; i < 2; ++i) {
		childParticles2.push_back(createParticle());
	}


	QmSpring* springForce2 = new QmSpring(childParticles[0], childParticles2, restLength, springConstant);
	for (QmParticle* child : childParticles2) {
		registry->addForceRegistry(child, springForce2);
	}

	*/

	pxWorld.addForceRegistry(registry);

}



void initScene8()
{
	printf("Scene 6. Qm magnestism Fixed\n");
	mousePointer = new glm::vec3(0, 4.5, 0);

	createFixedMagnScene(*mousePointer);
}


void initScene9()
{
	printf("Scene 6. Qm magnestism Fixed\n");
	mousePointer = new glm::vec3(0, 4.5, 0);

	createFixedMagnScene(*mousePointer);
}

void drawSprings()
{
	// Parcours de tous les registres de force dans le monde physique
	for (const auto& fr : pxWorld.getForceRegistries()) // Utilise la méthode ajoutée
	{
		// Itération sur chaque particule du registre
		for (size_t i = 0; i < fr->getParticleCount(); ++i)
		{
			QmParticle* particle = fr->getParticle(i); // Récupère la particule
			QmForceGenerator* forceGen = fr->getForceGen(i); // Récupère le générateur de force

			// Vérifie si le générateur de force est un ressort
			if (auto* spring = dynamic_cast<QmSpring*>(forceGen))
			{
				// Récupérer les positions des particules
				glm::vec3 pos1 = particle->getPos(); // Position de la première particule

				// Itération sur toutes les autres particules liées par le ressort
				for (QmParticle* otherParticle : spring->getAllParticles())
				{
					glm::vec3 pos2 = otherParticle->getPos();

					glBegin(GL_LINES);
					glColor3f(1.f, 1.f, 1.f); // Couleur de la ligne (blanc)
					glVertex3f(pos1.x, pos1.y, pos1.z); // Position de la première particule
					glVertex3f(pos2.x, pos2.y, pos2.z); // Position de la seconde particule
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