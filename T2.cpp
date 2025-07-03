#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <algorithm>

// --- Constants ---
const float GAME_DURATION = 300.0f;
const float BABY_ENERGY_MAX = 60.0f;
const float PENGUIN_SPEED = 1.5f;
const float ROTATION_SPEED = 90.0f;
const float ICE_PLATFORM_SIZE = 10.0f;
const float PENGUIN_Y_OFFSET = 0.48f;
const int MAX_FISH = 5;
const int MAX_HOLES = 8;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Game State ---
enum GameState {
    PLAYING,
    WIN,
    LOSE
};

// --- Data Structures ---
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    float length() const { return sqrt(x*x + y*y + z*z); }
    Vec3 normalize() const { float len = length(); return len > 0 ? *this * (1.0f/len) : Vec3(); }
};

struct PenguinState {
    Vec3 position;
    float rotation;
    bool hasFish;
    bool isMoving;
    float wingAnimation;
    bool isBaby;
};

struct Camera {
    Vec3 eye, center, up;
    Camera(Vec3 e, Vec3 c, Vec3 u) : eye(e), center(c), up(u) {}
};

// --- Penguin Model Functions ---
void drawEllipsoid(float rx, float ry, float rz, int slices = 30, int stacks = 30) {
    glPushMatrix();
    glScalef(rx, ry, rz);
    glutSolidSphere(1.0, slices, stacks);
    glPopMatrix();
}

void setMaterial(float r, float g, float b, float shininess = 50.0) {
    GLfloat ambient[] = {r * 0.3f, g * 0.3f, b * 0.3f, 1.0f};
    GLfloat diffuse[] = {r, g, b, 1.0f};
    GLfloat specular[] = {0.8f, 0.8f, 0.8f, 1.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void drawWebbledFoot() {
    glPushMatrix();
    setMaterial(1.0f, 0.6f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.0f);
    drawEllipsoid(0.08f, 0.03f, 0.12f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.04f, 0.0f, 0.08f);
        drawEllipsoid(0.02f, 0.02f, 0.06f);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawFlipper(bool isBaby) {
    glPushMatrix();
    if (isBaby) {
        setMaterial(0.5f, 0.5f, 0.5f);
    } else {
        setMaterial(0.1f, 0.1f, 0.1f);
    }
    drawEllipsoid(0.05f, 0.25f, 0.12f);
    glPushMatrix();
    glTranslatef(0.02f, 0.0f, 0.0f);
    setMaterial(0.95f, 0.95f, 0.95f);
    drawEllipsoid(0.03f, 0.23f, 0.10f);
    glPopMatrix();
    glPopMatrix();
}

void drawHead(bool isBaby) {
    glPushMatrix();
    if (isBaby) {
        setMaterial(0.5f, 0.5f, 0.5f);
    } else {
        setMaterial(0.1f, 0.1f, 0.1f);
    }
    drawEllipsoid(0.15f, 0.18f, 0.16f);
    glPushMatrix();
    glTranslatef(0.0f, -0.02f, 0.08f);
    setMaterial(0.95f, 0.95f, 0.95f);
    drawEllipsoid(0.12f, 0.14f, 0.12f);
    glPopMatrix();
    setMaterial(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.08f, 0.05f, 0.12f);
    glutSolidSphere(0.02, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.08f, 0.05f, 0.12f);
    glutSolidSphere(0.02, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, -0.05f, 0.16f);
    setMaterial(1.0f, 0.6f, 0.0f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glutSolidCone(0.03, 0.08, 8, 4);
    glPopMatrix();
    glPopMatrix();
}

void drawBody(bool isBaby) {
    glPushMatrix();
    if (isBaby) {
        setMaterial(0.5f, 0.5f, 0.5f);
    } else {
        setMaterial(0.1f, 0.1f, 0.1f);
    }
    drawEllipsoid(0.20f, 0.35f, 0.25f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.05f);
    setMaterial(0.95f, 0.95f, 0.95f);
    drawEllipsoid(0.18f, 0.32f, 0.20f);
    glPopMatrix();
    glPopMatrix();
}

void drawPenguin(const PenguinState& p) {
    glPushMatrix();
    glTranslatef(p.position.x, p.position.y, p.position.z);
    glRotatef(p.rotation, 0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.0f);
    drawBody(p.isBaby);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    drawHead(p.isBaby);
    glPopMatrix();

    float wingOffset = p.isMoving ? sin(p.wingAnimation) * 25.0f : 0.0f;

    glPushMatrix();
    glTranslatef(-0.18f, 0.0f, 0.0f);
    glRotatef(-20.0, 0.0, 0.0, 1.0);
    glRotatef(-wingOffset, 1.0, 0.0, 0.0);
    drawFlipper(p.isBaby);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.18f, 0.0f, 0.0f);
    glRotatef(20.0, 0.0, 0.0, 1.0);
    glRotatef(-wingOffset, 1.0, 0.0, 0.0);
    drawFlipper(p.isBaby);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.08f, -0.45f, 0.0f);
    drawWebbledFoot();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.08f, -0.45f, 0.0f);
    drawWebbledFoot();
    glPopMatrix();
    
    if (p.hasFish) {
        glPushMatrix();
        glTranslatef(0.0f, 0.18f, 0.22f);
        setMaterial(0.75f, 0.75f, 0.8f, 30.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawEllipsoid(0.02f, 0.01f, 0.06f);
        glPopMatrix();
    }

    glPopMatrix();
}

// --- Enhanced Fish Class ---
class Fish {
public:
    Vec3 position;
    bool active;
    float animationTime;
    float bobHeight;

    Fish() : position(Vec3()), active(false), animationTime(0.0f), bobHeight(0.0f) {}

    void spawn() {
        float x = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.7f;
        float z = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.7f;
        position = Vec3(x, 0.3f, z); // Much higher position for visibility
        active = true;
        animationTime = 0.0f;
        bobHeight = 0.1f + (rand() / (float)RAND_MAX) * 0.2f; // Random bob height
    }

    void draw() const {
        if (!active) return;
        
        glPushMatrix();
        
        // Position with enhanced bobbing animation
        float currentY = position.y + sin(animationTime * 2.0f) * bobHeight;
        glTranslatef(position.x, currentY, position.z);
        
        // Rotation animation for extra visibility
        glRotatef(sin(animationTime * 0.5f) * 30.0f, 0.0f, 1.0f, 0.0f);
        
        // Draw bright outline first (black border)
        glDisable(GL_LIGHTING);
        glColor3f(0.0f, 0.0f, 0.0f);
        glPushMatrix();
        glScalef(0.18f, 0.08f, 0.10f); // Much larger outline
        glutSolidSphere(1.0, 20, 20);
        glPopMatrix();
        
        // Tail outline
        glPushMatrix();
        glTranslatef(-0.18f, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glutSolidCone(0.05f, 0.08f, 12, 8);
        glPopMatrix();
        
        glEnable(GL_LIGHTING);
        
        // Main fish body - bright orange-red for maximum visibility
        setMaterial(1.0f, 0.3f, 0.0f, 80.0f); // Bright orange-red
        glPushMatrix();
        glScalef(0.15f, 0.06f, 0.08f); // Much larger than original
        glutSolidSphere(1.0, 20, 20);
        glPopMatrix();
        
        // Bright tail
        setMaterial(0.9f, 0.2f, 0.0f, 80.0f);
        glPushMatrix();
        glTranslatef(-0.15f, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glutSolidCone(0.04f, 0.07f, 12, 8);
        glPopMatrix();
        
        // Top fin - bright yellow for contrast
        setMaterial(1.0f, 1.0f, 0.0f, 60.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.05f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(0.02f, 0.04f, 8, 6);
        glPopMatrix();
        
        // Side fins
        setMaterial(0.8f, 0.8f, 0.0f, 60.0f);
        glPushMatrix();
        glTranslatef(0.04f, 0.0f, 0.05f);
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        drawEllipsoid(0.03f, 0.015f, 0.06f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(0.04f, 0.0f, -0.05f);
        glRotatef(-45.0f, 0.0f, 1.0f, 0.0f);
        drawEllipsoid(0.03f, 0.015f, 0.06f);
        glPopMatrix();
        
        // Pulsing glow effect
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1.0f, 0.5f, 0.0f, 0.3f + 0.2f * sin(animationTime * 4.0f));
        glPushMatrix();
        glScalef(0.25f, 0.12f, 0.15f);
        glutSolidSphere(1.0, 15, 15);
        glPopMatrix();
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
    }

    void update(float deltaTime) {
        if (active) {
            animationTime += deltaTime * 3.0f;
        }
    }
};

// --- Enhanced Hole Struct ---
struct Hole {
    Vec3 position;
    float radius;
    bool active;
    float animationTime;

    Hole() : position(Vec3()), radius(0.4f), active(false), animationTime(0.0f) {}

    void spawn() {
        float x, z;
        do {
            x = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.7f;
            z = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.7f;
        } while (sqrt(x*x + z*z) < 1.5f); // Keep away from center
        
        position = Vec3(x, 0.0f, z);
        active = true;
        animationTime = 0.0f;
    }

    void draw() const {
        if (!active) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        
        // Draw warning ring around hole (bright red)
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.0f, 0.0f); // Bright red warning
        glLineWidth(4.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 32; i++) {
            float angle = i * 2.0f * M_PI / 32.0f;
            float warningRadius = radius + 0.15f + 0.05f * sin(animationTime * 6.0f);
            glVertex3f(cos(angle) * warningRadius, 0.05f, sin(angle) * warningRadius);
        }
        glEnd();
        glLineWidth(1.0f);
        
        // Draw black outline
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.01f, 0.0f);
        for (int i = 0; i <= 32; i++) {
            float angle = i * 2.0f * M_PI / 32.0f;
            glVertex3f(cos(angle) * (radius + 0.1f), 0.01f, sin(angle) * (radius + 0.1f));
        }
        glEnd();
        
        glEnable(GL_LIGHTING);
        
        // Main hole surface - bright cyan for visibility
        setMaterial(0.0f, 0.8f, 1.0f, 100.0f); // Bright cyan
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.02f, 0.0f);
        for (int i = 0; i <= 32; i++) {
            float angle = i * 2.0f * M_PI / 32.0f;
            glVertex3f(cos(angle) * radius, 0.02f, sin(angle) * radius);
        }
        glEnd();
        
        // Animated ripple effects
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        for (int ring = 1; ring <= 4; ring++) {
            float alpha = 0.4f * (1.0f - ring * 0.1f);
            float rippleRadius = radius + (ring * 0.08f) + sin(animationTime * 3.0f + ring) * 0.03f;
            glColor4f(0.0f, 0.6f, 1.0f, alpha);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < 24; i++) {
                float angle = i * 2.0f * M_PI / 24.0f;
                glVertex3f(cos(angle) * rippleRadius, 0.03f, sin(angle) * rippleRadius);
            }
            glEnd();
        }
        
        glLineWidth(1.0f);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
    }
    
    void update(float deltaTime) {
        if (active) {
            animationTime += deltaTime * 2.0f;
        }
    }
};

// --- Global Variables ---
GameState gameState = PLAYING;
float gameTime = 0.0f;
float babyEnergyTime = BABY_ENERGY_MAX;
float fishSpawnTimer = 0.0f;
float holeSpawnTimer = 0.0f;

PenguinState motherPenguin;
PenguinState babyPenguin;
std::vector<Fish> fishes(MAX_FISH);
std::vector<Hole> holes(MAX_HOLES);

bool keys[256] = {false};
bool specialKeys[256] = {false};

GLuint iceTexture;
GLuint skybox_textures[6];

// --- Function Prototypes ---
void display();
void reshape(int w, int h);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void special(int key, int x, int y);
void specialUp(int key, int x, int y);
void init();
void updateGame(float deltaTime);
void drawScene();
void drawIcePlatform();
void drawUI();
void drawSkybox();
void drawTexturedCube(float size);
bool checkCollision3D(const Vec3& pos1, float radius1, const Vec3& pos2, float radius2);
void spawnFish();
void spawnHoles();
GLuint loadBMP_custom(const char * imagepath);
void resetGame();

// --- Main Function ---
int main(int argc, char** argv) {
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("RIP: Pinguim aterrorizante (dourado)");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    resetGame();
    glutMainLoop();
    return 0;
}

// --- Initialization ---
void init() {
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);

    // Enhanced lighting setup
    GLfloat lightPos[] = {5.0f, 10.0f, 5.0f, 1.0f};
    GLfloat lightAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f}; // Brighter ambient
    GLfloat lightDiffuse[] = {1.0f, 1.0f, 0.9f, 1.0f}; // Brighter diffuse
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // Add second light for better visibility
    glEnable(GL_LIGHT1);
    GLfloat light1Pos[] = {-5.0f, 8.0f, -5.0f, 1.0f};
    GLfloat light1Diffuse[] = {0.6f, 0.6f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);

    glEnable(GL_FOG);
    GLfloat fogColor[4] = {0.5, 0.8, 1.0, 1.0};
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.1f);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 8.0f);
    glFogf(GL_FOG_END, 30.0f);

    // Load textures (create dummy textures if files don't exist)
    iceTexture = loadBMP_custom("ice.bmp");
    for (int i = 0; i < 6; i++) {
        skybox_textures[i] = loadBMP_custom("sky.bmp");
    }
}

// --- Rendering ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    int hw = w / 2;
    int hh = h / 2;

    // Viewport 1: Top-Down
    glViewport(0, hh, hw, hh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)hw / hh, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(motherPenguin.position.x, 12, motherPenguin.position.z, 
              motherPenguin.position.x, motherPenguin.position.y, motherPenguin.position.z, 
              0, 0, -1);
    drawScene();

    // Viewport 2: Side View
    glViewport(hw, hh, hw, hh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)hw / hh, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(motherPenguin.position.x + 8, motherPenguin.position.y + 3, motherPenguin.position.z, 
              motherPenguin.position.x, motherPenguin.position.y, motherPenguin.position.z, 
              0, 1, 0);
    drawScene();

    // Viewport 3: Front View (Follow Cam)
    glViewport(0, 0, hw, hh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)hw / hh, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float radians = motherPenguin.rotation * M_PI / 180.0f;
    float look_dx = sin(radians);
    float look_dz = cos(radians);
    gluLookAt(motherPenguin.position.x - look_dx * 4, motherPenguin.position.y + 2, motherPenguin.position.z - look_dz * 4, 
              motherPenguin.position.x, motherPenguin.position.y + 0.5f, motherPenguin.position.z, 
              0, 1, 0);
    drawScene();

    // Viewport 4: Free View
    glViewport(hw, 0, hw, hh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)hw / hh, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(8, 6, 8, 0, 0, 0, 0, 1, 0);
    drawScene();

    // UI Overlay
    glViewport(0, 0, w, h);
    drawUI();

    glutSwapBuffers();
}

void drawScene() {
    drawSkybox();
    drawIcePlatform();
    drawPenguin(motherPenguin);
    drawPenguin(babyPenguin);
    
    for (auto& fish : fishes) {
        fish.draw();
    }
    
    for (auto& hole : holes) {
        hole.draw();
    }
}

void drawIcePlatform() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, iceTexture);
    GLfloat iceMaterial[] = {0.9f, 0.95f, 1.0f, 0.9f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, iceMaterial);
    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.0f);
    glScalef(ICE_PLATFORM_SIZE / 2.0f, 0.2f, ICE_PLATFORM_SIZE / 2.0f);
    drawTexturedCube(1.0f);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawSkybox() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDepthMask(GL_FALSE);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // Simple skybox without textures for now
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.6f, 0.9f, 1.0f);
    
    // Draw simple colored sky planes
    glBegin(GL_QUADS);
    // Front
    glVertex3f(-20, -10, -20);
    glVertex3f( 20, -10, -20);
    glVertex3f( 20,  30, -20);
    glVertex3f(-20,  30, -20);
    // Back
    glVertex3f( 20, -10, 20);
    glVertex3f(-20, -10, 20);
    glVertex3f(-20,  30, 20);
    glVertex3f( 20,  30, 20);
    // Left
    glVertex3f(-20, -10,  20);
    glVertex3f(-20, -10, -20);
    glVertex3f(-20,  30, -20);
    glVertex3f(-20,  30,  20);
    // Right
    glVertex3f(20, -10, -20);
    glVertex3f(20, -10,  20);
    glVertex3f(20,  30,  20);
    glVertex3f(20,  30, -20);
    // Top
    glColor3f(0.4f, 0.7f, 1.0f);
    glVertex3f(-20, 30, -20);
    glVertex3f( 20, 30, -20);
    glVertex3f( 20, 30,  20);
    glVertex3f(-20, 30,  20);
    glEnd();
    
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawUI() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 20);
    char gameInfo[256];
    sprintf(gameInfo, "Time: %.1f/%.0fs | Baby Energy: %.1f/%.0fs | State: %s | Fish: %d",
            gameTime, GAME_DURATION, babyEnergyTime, BABY_ENERGY_MAX,
            gameState == PLAYING ? "Playing" :
            gameState == WIN ? "YOU WIN!" : "GAME OVER",
            (int)std::count_if(fishes.begin(), fishes.end(), [](const Fish& f) { return f.active; }));
    for (char* c = gameInfo; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// --- Game Logic ---
void updateGame(float deltaTime) {
    if (gameState != PLAYING) return;
    
    gameTime += deltaTime;
    babyEnergyTime -= deltaTime;
    
    if (gameTime >= GAME_DURATION) {
        gameState = WIN; 
        return;
    }
    if (babyEnergyTime <= 0.0f) {
        gameState = LOSE; 
        return;
    }

    motherPenguin.isMoving = false;
    float radians = motherPenguin.rotation * M_PI / 180.0f;
    Vec3 forward(-sin(radians), 0, -cos(radians));
    
    if (specialKeys[GLUT_KEY_DOWN]) {
        motherPenguin.position = motherPenguin.position + forward * PENGUIN_SPEED * deltaTime;
        motherPenguin.isMoving = true;
    }
    if (specialKeys[GLUT_KEY_UP]) {
        motherPenguin.position = motherPenguin.position - forward * PENGUIN_SPEED * deltaTime;
        motherPenguin.isMoving = true;
    }
    if (specialKeys[GLUT_KEY_LEFT]) {
        motherPenguin.rotation += ROTATION_SPEED * deltaTime;
    }
    if (specialKeys[GLUT_KEY_RIGHT]) {
        motherPenguin.rotation -= ROTATION_SPEED * deltaTime;
    }

    float halfPlatform = ICE_PLATFORM_SIZE * 0.5f - 0.3f;
    if (motherPenguin.position.x > halfPlatform) motherPenguin.position.x = halfPlatform;
    if (motherPenguin.position.x < -halfPlatform) motherPenguin.position.x = -halfPlatform;
    if (motherPenguin.position.z > halfPlatform) motherPenguin.position.z = halfPlatform;
    if (motherPenguin.position.z < -halfPlatform) motherPenguin.position.z = -halfPlatform;
    
    if (motherPenguin.isMoving) {
        motherPenguin.wingAnimation += deltaTime * 8.0f;
    }

    // Update fish and holes
    for (auto& fish : fishes) {
        fish.update(deltaTime);
    }
    
    for (auto& hole : holes) {
        hole.update(deltaTime);
    }

    // Collision detection with enhanced radii
    for (const auto& hole : holes) {
        if (hole.active && checkCollision3D(motherPenguin.position, 0.3f, hole.position, hole.radius + 0.1f)) {
            gameState = LOSE; 
            return;
        }
    }

    if (!motherPenguin.hasFish) {
        for (auto& fish : fishes) {
            if (fish.active && checkCollision3D(motherPenguin.position, 0.4f, fish.position, 0.2f)) {
                fish.active = false;
                motherPenguin.hasFish = true;
                break;
            }
        }
    }

    if (motherPenguin.hasFish && checkCollision3D(motherPenguin.position, 0.5f, babyPenguin.position, 0.3f)) {
        motherPenguin.hasFish = false;
        babyEnergyTime = BABY_ENERGY_MAX;
    }

    fishSpawnTimer += deltaTime;
    if (fishSpawnTimer >= 5.0f) { // Spawn fish more frequently
        spawnFish();
        fishSpawnTimer = 0.0f;
    }

    holeSpawnTimer += deltaTime;
    if (holeSpawnTimer >= 12.0f) { // Spawn holes more frequently
        spawnHoles();
        holeSpawnTimer = 0.0f;
    }
}

void resetGame() {
    gameState = PLAYING;
    gameTime = 0.0f;
    babyEnergyTime = BABY_ENERGY_MAX;
    
    motherPenguin = {Vec3(0.0f, PENGUIN_Y_OFFSET, 2.0f), 0.0f, false, false, 0.0f, false};
    babyPenguin = {Vec3(0.0f, PENGUIN_Y_OFFSET, 0.0f), 0.0f, false, false, 0.0f, true};

    for (auto& fish : fishes) fish.active = false;
    for (auto& hole : holes) hole.active = false;
    
    spawnFish();
    spawnHoles();
}

bool checkCollision3D(const Vec3& pos1, float radius1, const Vec3& pos2, float radius2) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    float dz = pos1.z - pos2.z;
    float distance = sqrt(dx * dx + dy * dy + dz * dz);
    return distance < (radius1 + radius2);
}

void spawnFish() {
    for (auto& fish : fishes) {
        if (!fish.active) {
            Vec3 newPos;
            bool validPosition = false;
            int attempts = 0;
            
            while (!validPosition && attempts < 30) {
                float x = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.6f;
                float z = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.6f;
                newPos = Vec3(x, 0.3f, z);
                
                validPosition = true;
                // Check distance from holes and penguins
                for (const auto& hole : holes) {
                    if (hole.active && (newPos - hole.position).length() < 1.2f) {
                        validPosition = false;
                        break;
                    }
                }
                if (validPosition && (newPos - motherPenguin.position).length() < 1.0f) {
                    validPosition = false;
                }
                if (validPosition && (newPos - babyPenguin.position).length() < 1.0f) {
                    validPosition = false;
                }
                attempts++;
            }
            
            fish.position = newPos;
            fish.active = true;
            fish.animationTime = 0.0f;
            break;
        }
    }
}

void spawnHoles() {
    for (auto& hole : holes) {
        if (rand() % 100 < 50) { // 50% chance to spawn each hole
            Vec3 newPos;
            bool validPosition = false;
            int attempts = 0;
            
            while (!validPosition && attempts < 30) {
                float x = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.6f;
                float z = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.6f;
                newPos = Vec3(x, 0.0f, z);
                
                validPosition = true;
                // Keep away from center and penguins
                if (sqrt(x*x + z*z) < 2.0f) {
                    validPosition = false;
                }
                if (validPosition && (newPos - motherPenguin.position).length() < 1.5f) {
                    validPosition = false;
                }
                if (validPosition && (newPos - babyPenguin.position).length() < 1.5f) {
                    validPosition = false;
                }
                attempts++;
            }
            
            hole.position = newPos;
            hole.active = true;
            hole.animationTime = 0.0f;
        }
    }
}

// --- Utility Functions ---
void timer(int value) {
    updateGame(16.0f / 1000.0f);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;
    if (key == 27) exit(0);
    if (key == 'r' || key == 'R') resetGame();
}

void keyboardUp(unsigned char key, int x, int y) { 
    keys[key] = false; 
}

void special(int key, int x, int y) { 
    specialKeys[key] = true; 
}

void specialUp(int key, int x, int y) { 
    specialKeys[key] = false; 
}

void drawTexturedCube(float size) {
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0.0, 0.0, 1.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( size, -size, size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( size,  size, size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size,  size, size);
    
    // Back face
    glNormal3f(0.0, 0.0, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, -size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-size,  size, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( size,  size, -size);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( size, -size, -size);
    
    // Top face
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, -size);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, size,  size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( size, size,  size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( size, size, -size);
    
    // Bottom face
    glNormal3f(0.0, -1.0, 0.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-size, -size, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( size, -size, -size);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( size, -size,  size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size,  size);
    
    // Right face
    glNormal3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, -size, -size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size,  size, -size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(size,  size,  size);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(size, -size,  size);
    
    // Left face
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, -size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size,  size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-size,  size,  size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size,  size, -size);
    glEnd();
}

GLuint loadBMP_custom(const char * imagepath) {
    // Create a simple white texture if file doesn't exist
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Create a simple 2x2 white texture
    unsigned char whiteTexture[12] = {
        255, 255, 255,  255, 255, 255,
        255, 255, 255,  255, 255, 255
    };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, whiteTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    return textureID;
}
