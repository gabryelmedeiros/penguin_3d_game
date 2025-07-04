// https://cs.lmu.edu/~ray/notes/openglexamples/
// g++ litsolids.cpp -lX11 -lGL -lGLU -lglut -g -Wall -O2 -o r.exe
//
// Este programa mostra três objetos ciano iluminados com uma única
// fonte de luz amarela. Ele ilustra vários dos parâmetros de iluminação.

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <ctime>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ===================================================================
// ESTRUTURAS DE DADOS & VARIÁVEIS GLOBAIS
// ===================================================================

// --- Constantes do Jogo ---
float GAME_DURATION = 300.0f;       // Duração total do jogo em segundos
float BABY_ENERGY_MAX = 60.0f;      // Energia/tempo máximo para o pinguim bebê
float PENGUIN_SPEED = 2.5f;         // Velocidade de movimento do pinguim mãe
float ROTATION_SPEED = 120.0f;       // Velocidade de rotação do pinguim mãe
float ICE_PLATFORM_SIZE = 10.0f;    // A largura e profundidade da plataforma de gelo
int MAX_FISH = 5;                   // Número máximo de peixes permitidos na tela
int MAX_HOLES = 8;                  // Número máximo de buracos permitidos na tela

// --- Variáveis de Estado do Jogo ---
int gameState;                      // 0: Jogando, 1: Vitória, 2: Derrota
int cameraSelected;                 // Qual visão de câmera está ativa
float gameTime;                     // Tempo de jogo decorrido
float babyEnergyTime;               // Tempo restante para a energia do bebê
float fishSpawnTimer;               // Timer para controlar quando novos peixes aparecem
float holeSpawnTimer;               // Timer para controlar quando novos buracos aparecem

// --- Structs para Objetos do Jogo ---
// Estrutura para representar uma posição 3D
struct Position {
    float x, y, z;
};

// Estrutura para armazenar o estado de um pinguim
struct Penguin {
    Position pos;
    float rotation;
    bool hasFish;
    bool isMoving;
    float wingAnimation;
    bool isBaby;
};

// Estrutura para armazenar o estado de um peixe
struct Fish {
    Position pos;
    bool active;
    float animationTime;
    float bobHeight;
};

// Estrutura para armazenar o estado de um buraco
struct Hole {
    Position pos;
    float radius;
    bool active;
    float animationTime;
};

// --- Objetos Globais do Jogo ---
Penguin motherPenguin;
Penguin babyPenguin;
std::vector<Fish> fishes;
std::vector<Hole> holes;

// ===================================================================
// PROTÓTIPOS DAS FUNÇÕES
// ===================================================================

void display();
void reshape(int w, int h);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void special(int key, int x, int y);
void init();
void resetGame();

// --- Funções de Desenho ---
void drawPenguin(const Penguin& p);
void drawFish(const Fish& fish);
void drawHole(const Hole& hole);
void drawIcePlatform();
void drawSkybox();
void drawUI();
void drawScene();

// --- Funções Auxiliares ---
void setMaterial(float r, float g, float b, float shininess);
void drawEllipsoid(float rx, float ry, float rz);
bool checkCollision(Position p1, float r1, Position p2, float r2);

// ===================================================================
// FUNÇÃO PRINCIPAL
// ===================================================================

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Missão de Resgate do Pinguim");

    init();
    resetGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);

    glutMainLoop();
    return 0;
}

// ===================================================================
// INICIALIZAÇÃO E ESTADO DO JOGO
// ===================================================================

// init: Realiza a inicialização específica da aplicação.
void init() {
    srand(time(NULL));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);

    // Configura a fonte de luz primária (sol)
    GLfloat lightPos0[] = {5.0f, 10.0f, 5.0f, 1.0f};
    GLfloat lightAmbient0[] = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat lightDiffuse0[] = {1.0f, 1.0f, 0.9f, 1.0f};
    GLfloat lightSpecular0[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular0);

    // Configura a luz de preenchimento para melhor visibilidade
    GLfloat lightPos1[] = {-5.0f, 8.0f, -5.0f, 1.0f};
    GLfloat lightDiffuse1[] = {0.6f, 0.6f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse1);

    // Configura a névoa para dar profundidade à cena
    GLfloat fogColor[4] = {0.5, 0.8, 1.0, 1.0};
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 8.0f);
    glFogf(GL_FOG_END, 30.0f);
}

// resetGame: Reinicia todas as variáveis do jogo para seus estados iniciais.
void resetGame() {
    gameState = 0; // Jogando
    cameraSelected = 1; // Visão de câmera padrão
    gameTime = 0.0f;
    babyEnergyTime = BABY_ENERGY_MAX;
    fishSpawnTimer = 0.0f;
    holeSpawnTimer = 0.0f;

    motherPenguin.pos = {0.0f, 0.48f, 2.0f};
    motherPenguin.rotation = 0.0f;
    motherPenguin.hasFish = false;
    motherPenguin.isMoving = false;
    motherPenguin.wingAnimation = 0.0f;
    motherPenguin.isBaby = false;

    babyPenguin.pos = {0.0f, 0.48f, 0.0f};
    babyPenguin.rotation = 0.0f;
    babyPenguin.isBaby = true;

    fishes.assign(MAX_FISH, Fish());
    holes.assign(MAX_HOLES, Hole());
}

// ===================================================================
// FUNÇÕES DE RENDERIZAÇÃO
// ===================================================================

// display: Limpa a janela e desenha a cena a partir de uma única viewport que pode ser trocada.
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // --- Configura uma única viewport e projeção ---
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // --- Seleciona a visão da câmera com base em cameraSelected ---
    switch(cameraSelected) {
        case 1: // Câmera com vista de cima
            gluLookAt(motherPenguin.pos.x, 12, motherPenguin.pos.z,
                      motherPenguin.pos.x, motherPenguin.pos.y, motherPenguin.pos.z,
                      0, 0, -1);
            break;
        case 2: // Câmera ao lado da cena
            gluLookAt(motherPenguin.pos.x + 8, motherPenguin.pos.y + 3, motherPenguin.pos.z,
                      motherPenguin.pos.x, motherPenguin.pos.y, motherPenguin.pos.z,
                      0, 1, 0);
            break;
        case 3: // Câmera de frente pra cena (Follow Cam)
            { // Usa chaves para criar um novo escopo para variáveis locais
                float radians = motherPenguin.rotation * M_PI / 180.0f;
                float look_dx = sin(radians);
                float look_dz = cos(radians);
                gluLookAt(motherPenguin.pos.x - look_dx * 4, motherPenguin.pos.y + 2, motherPenguin.pos.z - look_dz * 4,
                          motherPenguin.pos.x, motherPenguin.pos.y + 0.5f, motherPenguin.pos.z,
                          0, 1, 0);
            }
            break;
        case 4: // Câmera com posição livre
            gluLookAt(8, 6, 8, 0, 0, 0, 0, 1, 0);
            break;
    }

    // --- Desenha a cena e a UI ---
    drawScene();
    drawUI();

    glutSwapBuffers();
}

// drawScene: Chama todas as funções necessárias para desenhar o mundo do jogo.
void drawScene() {
    drawSkybox();
    drawIcePlatform();
    drawPenguin(motherPenguin);
    drawPenguin(babyPenguin);

    for (int i = 0; i < fishes.size(); ++i) {
        drawFish(fishes[i]);
    }
    for (int i = 0; i < holes.size(); ++i) {
        drawHole(holes[i]);
    }
}

// drawIcePlatform: Desenha a plataforma de gelo principal.
void drawIcePlatform() {
    GLfloat iceMaterial[] = {0.9f, 0.95f, 1.0f, 0.9f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, iceMaterial);
    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.0f);
    glScalef(ICE_PLATFORM_SIZE, 0.2f, ICE_PLATFORM_SIZE);
    glutSolidCube(1.0);
    glPopMatrix();
}

// drawSkybox: Desenha um skybox colorido simples.
void drawSkybox() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glColor3f(0.6f, 0.9f, 1.0f);

    // Desenha planos coloridos simples para o céu
    float size = 20.0f;
    float height = 30.0f;
    float bottom = -10.0f;
    glBegin(GL_QUADS);
    // Frente
    glVertex3f(-size, bottom, -size); glVertex3f(size, bottom, -size); glVertex3f(size, height, -size); glVertex3f(-size, height, -size);
    // Trás
    glVertex3f(size, bottom, size); glVertex3f(-size, bottom, size); glVertex3f(-size, height, size); glVertex3f(size, height, size);
    // Esquerda
    glVertex3f(-size, bottom, size); glVertex3f(-size, bottom, -size); glVertex3f(-size, height, -size); glVertex3f(-size, height, size);
    // Direita
    glVertex3f(size, bottom, -size); glVertex3f(size, bottom, size); glVertex3f(size, height, size); glVertex3f(size, height, -size);
    // Topo
    glColor3f(0.4f, 0.7f, 1.0f);
    glVertex3f(-size, height, -size); glVertex3f(size, height, -size); glVertex3f(size, height, size); glVertex3f(-size, height, size);
    glEnd();

    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

// drawUI: Desenha o texto da interface do usuário na tela.
void drawUI() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(10, glutGet(GLUT_WINDOW_HEIGHT) - 20);
    char gameInfo[256];
    const char* stateStr = (gameState == 0) ? "Jogando" : (gameState == 1) ? "VOCE VENCEU!" : "FIM DE JOGO";
    sprintf(gameInfo, "Tempo: %.1f | Energia do Bebe: %.1f | Estado: %s", gameTime, babyEnergyTime, stateStr);
    for (char* c = gameInfo; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    if (gameState != 0) {
        char endMessage[256];
        sprintf(endMessage, "Pressione 'r' para reiniciar.");
        glRasterPos2f(glutGet(GLUT_WINDOW_WIDTH) / 2 - 80, glutGet(GLUT_WINDOW_HEIGHT) / 2);
         for (char* c = endMessage; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ===================================================================
// LÓGICA DO JOGO E ATUALIZAÇÕES
// ===================================================================

// checkCollision: Verifica a colisão entre duas esferas.
bool checkCollision(Position p1, float r1, Position p2, float r2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float dz = p1.z - p2.z;
    return (dx * dx + dy * dy + dz * dz) < (r1 + r2) * (r1 + r2);
}

// spawnFish: Encontra um peixe inativo e o gera em um local aleatório.
void spawnFish() {
    for (int i = 0; i < fishes.size(); ++i) {
        if (!fishes[i].active) {
            float x = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.8f;
            float z = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.8f;
            fishes[i].pos = {x, 0.3f, z};
            fishes[i].active = true;
            fishes[i].animationTime = 0.0f;
            fishes[i].bobHeight = 0.1f + (rand() / (float)RAND_MAX) * 0.2f;
            return; // Gera um peixe de cada vez
        }
    }
}

// spawnHole: Encontra um buraco inativo e o gera em um local aleatório.
void spawnHole() {
    for (int i = 0; i < holes.size(); ++i) {
        if (!holes[i].active) {
             float x, z;
            do {
                x = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.8f;
                z = (rand() / (float)RAND_MAX - 0.5f) * ICE_PLATFORM_SIZE * 0.8f;
            } while (sqrt(x*x + z*z) < 2.0f); // Mantém longe do centro

            holes[i].pos = {x, 0.0f, z};
            holes[i].active = true;
            holes[i].animationTime = 0.0f;
            holes[i].radius = 0.4f;
            return; // Gera um buraco de cada vez
        }
    }
}

// timer: O loop principal do jogo, chamado periodicamente para atualizar o estado do jogo.
void timer(int value) {
    float deltaTime = 16.0f / 1000.0f;

    if (gameState == 0) { // Se o jogo estiver em andamento
        gameTime += deltaTime;
        babyEnergyTime -= deltaTime;

        if (gameTime >= GAME_DURATION) gameState = 1; // Vitória
        if (babyEnergyTime <= 0.0f) gameState = 2; // Derrota

        // Atualiza as animações
        if (motherPenguin.isMoving) {
            motherPenguin.wingAnimation += deltaTime * 8.0f;
        }
        for (int i = 0; i < fishes.size(); ++i) {
            if(fishes[i].active) fishes[i].animationTime += deltaTime * 3.0f;
        }
        for (int i = 0; i < holes.size(); ++i) {
            if(holes[i].active) holes[i].animationTime += deltaTime * 2.0f;
        }

        // Verifica colisões
        for (int i = 0; i < holes.size(); ++i) {
            if (holes[i].active && checkCollision(motherPenguin.pos, 0.3f, holes[i].pos, holes[i].radius)) {
                gameState = 2; // Derrota
            }
        }
        if (!motherPenguin.hasFish) {
            for (int i = 0; i < fishes.size(); ++i) {
                if (fishes[i].active && checkCollision(motherPenguin.pos, 0.4f, fishes[i].pos, 0.2f)) {
                    fishes[i].active = false;
                    motherPenguin.hasFish = true;
                    break;
                }
            }
        }
        if (motherPenguin.hasFish && checkCollision(motherPenguin.pos, 0.5f, babyPenguin.pos, 0.3f)) {
            motherPenguin.hasFish = false;
            babyEnergyTime = BABY_ENERGY_MAX;
        }

        // Controla a geração de objetos
        fishSpawnTimer += deltaTime;
        if (fishSpawnTimer >= 5.0f) {
            spawnFish();
            fishSpawnTimer = 0.0f;
        }
        holeSpawnTimer += deltaTime;
        if (holeSpawnTimer >= 8.0f) {
            spawnHole();
            holeSpawnTimer = 0.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ===================================================================
// TRATAMENTO DE ENTRADA
// ===================================================================

// reshape: Chamada quando a janela é redimensionada.
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

// keyboard: Trata os pressionamentos de teclas normais.
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0); // ESC para sair
    if (key == 'r' || key == 'R') resetGame();

    // Adiciona o switch para trocar a câmera
    switch(key) {
        case '1':
            cameraSelected = 1;
            break;
        case '2':
            cameraSelected = 2;
            break;
        case '3':
            cameraSelected = 3;
            break;
        case '4':
            cameraSelected = 4;
            break;
    }
}

// special: Trata os pressionamentos de teclas especiais (ex: setas).
void special(int key, int x, int y) {
    if (gameState != 0) return;

    float deltaTime = 16.0f / 1000.0f; // Delta time aproximado para input
    float radians = motherPenguin.rotation * M_PI / 180.0f;
    float dx = -sin(radians);
    float dz = -cos(radians);

    motherPenguin.isMoving = true;

    switch (key) {
        case GLUT_KEY_DOWN:
            motherPenguin.pos.x += dx * PENGUIN_SPEED * deltaTime;
            motherPenguin.pos.z += dz * PENGUIN_SPEED * deltaTime;
            break;
        case GLUT_KEY_UP:
            motherPenguin.pos.x -= dx * PENGUIN_SPEED * deltaTime;
            motherPenguin.pos.z -= dz * PENGUIN_SPEED * deltaTime;
            break;
        case GLUT_KEY_LEFT:
            motherPenguin.rotation += ROTATION_SPEED * deltaTime;
            break;
        case GLUT_KEY_RIGHT:
            motherPenguin.rotation -= ROTATION_SPEED * deltaTime;
            break;
        default:
             motherPenguin.isMoving = false;
             break;
    }

    // Limita a posição à plataforma
    float halfPlatform = ICE_PLATFORM_SIZE / 2.0f - 0.3f;
    if (motherPenguin.pos.x > halfPlatform) motherPenguin.pos.x = halfPlatform;
    if (motherPenguin.pos.x < -halfPlatform) motherPenguin.pos.x = -halfPlatform;
    if (motherPenguin.pos.z > halfPlatform) motherPenguin.pos.z = halfPlatform;
    if (motherPenguin.pos.z < -halfPlatform) motherPenguin.pos.z = -halfPlatform;
}

// ===================================================================
// FUNÇÕES AUXILIARES DE DESENHO
// ===================================================================

// setMaterial: Define as propriedades do material para um objeto.
void setMaterial(float r, float g, float b, float shininess = 50.0) {
    GLfloat ambient[] = {r * 0.3f, g * 0.3f, b * 0.3f, 1.0f};
    GLfloat diffuse[] = {r, g, b, 1.0f};
    GLfloat specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

// drawEllipsoid: Desenha um elipsoide sólido.
void drawEllipsoid(float rx, float ry, float rz) {
    glPushMatrix();
    glScalef(rx, ry, rz);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
}

// drawPenguin: Desenha um modelo completo de pinguim.
void drawPenguin(const Penguin& p) {
    glPushMatrix();
    glTranslatef(p.pos.x, p.pos.y, p.pos.z);
    glRotatef(p.rotation, 0.0f, 1.0f, 0.0f);
    glScalef(0.8, 0.8, 0.8); // Reduz a escala do modelo inteiro

    // Corpo
    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.0f);
    if(p.isBaby) setMaterial(0.5f, 0.5f, 0.5f); else setMaterial(0.1f, 0.1f, 0.1f);
    drawEllipsoid(0.20f, 0.35f, 0.25f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.05f);
    setMaterial(0.95f, 0.95f, 0.95f);
    drawEllipsoid(0.18f, 0.32f, 0.20f);
    glPopMatrix();
    glPopMatrix();

    // Cabeça
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    if(p.isBaby) setMaterial(0.5f, 0.5f, 0.5f); else setMaterial(0.1f, 0.1f, 0.1f);
    drawEllipsoid(0.15f, 0.18f, 0.16f);
    glPushMatrix();
    glTranslatef(0.0f, -0.02f, 0.08f);
    setMaterial(0.95f, 0.95f, 0.95f);
    drawEllipsoid(0.12f, 0.14f, 0.12f);
    glPopMatrix();
    setMaterial(0.0f, 0.0f, 0.0f); // Olhos
    glPushMatrix(); glTranslatef(0.08f, 0.05f, 0.12f); glutSolidSphere(0.02, 10, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.08f, 0.05f, 0.12f); glutSolidSphere(0.02, 10, 10); glPopMatrix();
    glPushMatrix(); // Bico
    glTranslatef(0.0f, -0.05f, 0.16f);
    setMaterial(1.0f, 0.6f, 0.0f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glutSolidCone(0.03, 0.08, 8, 4);
    glPopMatrix();
    glPopMatrix();

    // Asas
    float wingOffset = p.isMoving ? sin(p.wingAnimation) * 25.0f : 0.0f;
    glPushMatrix();
    glTranslatef(-0.18f, 0.0f, 0.0f);
    glRotatef(-20.0, 0.0, 0.0, 1.0);
    glRotatef(-wingOffset, 1.0, 0.0, 0.0);
    if(p.isBaby) setMaterial(0.5f, 0.5f, 0.5f); else setMaterial(0.1f, 0.1f, 0.1f);
    drawEllipsoid(0.05f, 0.25f, 0.12f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.18f, 0.0f, 0.0f);
    glRotatef(20.0, 0.0, 0.0, 1.0);
    glRotatef(-wingOffset, 1.0, 0.0, 0.0);
    if(p.isBaby) setMaterial(0.5f, 0.5f, 0.5f); else setMaterial(0.1f, 0.1f, 0.1f);
    drawEllipsoid(0.05f, 0.25f, 0.12f);
    glPopMatrix();

    // Pés
    setMaterial(1.0f, 0.6f, 0.0f);
    glPushMatrix(); glTranslatef(-0.08f, -0.45f, 0.0f); drawEllipsoid(0.08f, 0.03f, 0.12f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.08f, -0.45f, 0.0f); drawEllipsoid(0.08f, 0.03f, 0.12f); glPopMatrix();

    // Peixe na boca
    if (p.hasFish) {
        glPushMatrix();
        glTranslatef(0.0f, 0.18f, 0.22f);
        setMaterial(0.75f, 0.75f, 0.8f, 30.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawEllipsoid(0.06f, 0.02f, 0.04f);
        glPopMatrix();
    }
    glPopMatrix();
}

// drawFish: Desenha um único modelo de peixe.
void drawFish(const Fish& fish) {
    if (!fish.active) return;
    glPushMatrix();
    float currentY = fish.pos.y + sin(fish.animationTime * 2.0f) * fish.bobHeight;
    glTranslatef(fish.pos.x, currentY, fish.pos.z);
    glRotatef(sin(fish.animationTime * 0.5f) * 30.0f, 0.0f, 1.0f, 0.0f);

    setMaterial(1.0f, 0.3f, 0.0f, 80.0f);
    glPushMatrix();
    glScalef(1.5, 1.5, 1.5); // Torna o peixe maior
    drawEllipsoid(0.1f, 0.04f, 0.05f); // Corpo
    glPushMatrix();
    glTranslatef(-0.1f, 0.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCone(0.04f, 0.07f, 12, 8); // Cauda
    glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}

// drawHole: Desenha um único buraco no gelo.
void drawHole(const Hole& hole) {
    if (!hole.active) return;
    glPushMatrix();
    glTranslatef(hole.pos.x, hole.pos.y, hole.pos.z);

    // Parte de água escura
    setMaterial(0.0f, 0.2f, 0.4f, 10.0f);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.02f, 0.0f);
    for (int i = 0; i <= 32; i++) {
        float angle = i * 2.0f * M_PI / 32.0f;
        glVertex3f(cos(angle) * hole.radius, 0.02f, sin(angle) * hole.radius);
    }
    glEnd();
    glPopMatrix();
}
