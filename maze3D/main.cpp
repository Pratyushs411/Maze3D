#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <math.h>
#include<GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;

// Timer variables
float gameStartTime = 0.0f;
float currentGameTime = 0.0f;
bool gameStarted = false;
char timeString[32];



enum GameState {
    TITLE_SCREEN,
    PLAYING,
    COMPLETED,
    RULES_SCREEN
};

enum GameState currentGameState = TITLE_SCREEN;
bool rulesDisplayed = false;

// Player properties
float playerX = 1.5f;
float playerY = 0.0f;
float playerZ = 1.5f;
float playerAngle = 0.0f;
float playerSpeed = 0.05f;
float rotationSpeed = 0.03f;
bool gameCompleted = false;
char congratsMessage[128];


int playerScore = 0;
char scoreString[32]; // For displaying the score
char finalscore[32];
char finaltime[32];
// Gold bar animation parameters
float goldBarYOffset = 0.0f;
float goldBarAnimSpeed = 1.5f;
float goldBarRotation = 0.0f;

// Maze dimensions and layout
const int MAZE_WIDTH = 10;
const int MAZE_HEIGHT = 10;
int maze[MAZE_WIDTH][MAZE_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 3, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 3, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 3, 0, 1, 0, 0, 1},
    {1, 3, 1, 1, 1, 0, 1, 0, 2, 1}, // 2 is still the goal (red wall)
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};
// Function prototypes
void setupCamera();
void renderMaze();
void drawCube(float size);
void handleKeyInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
bool checkCollision(float newX, float newZ);

void renderTitleScreen() {
    // Disable lighting for 2D rendering
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw a gradient background
    glBegin(GL_QUADS);
    // Deep blue at bottom to lighter blue at top
    glColor3f(0.1f, 0.1f, 0.3f);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glColor3f(0.4f, 0.4f, 0.8f);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();
    
    // Draw some decorative maze-like pattern in background
    glLineWidth(2.0f);
    glColor3f(0.5f, 0.5f, 0.9f);
    
    // Draw grid of maze-like lines
    int gridSize = 40;
    for (int x = 0; x < WIDTH; x += gridSize) {
        for (int y = 0; y < HEIGHT; y += gridSize) {
            // Randomly decide whether to draw horizontal or vertical line
            if (((x + y) / gridSize) % 3 == 0) {
                glBegin(GL_LINES);
                glVertex2f(x, y);
                glVertex2f(x + gridSize, y);
                glEnd();
            } else if (((x + y) / gridSize) % 3 == 1) {
                glBegin(GL_LINES);
                glVertex2f(x, y);
                glVertex2f(x, y + gridSize);
                glEnd();
            }
        }
    }
    
    // Calculate center position for text
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    
    // Draw title "MAZE3D" in big font
    glColor3f(1.0f, 0.8f, 0.2f); // Gold color
    char titleText[] = "MAZE3D";
    
    // Shadow effect (slight offset)
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(centerX - (strlen(titleText) * 24) + 4, centerY + 54);
    for (int i = 0; i < strlen(titleText); i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, titleText[i]);
    }
    
    // Main title - use the biggest font available and scale position accordingly
    glColor3f(1.0f, 0.8f, 0.0f); // Gold color
    glRasterPos2i(centerX - (strlen(titleText) * 24), centerY + 50);
    for (int i = 0; i < strlen(titleText); i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, titleText[i]);
    }
    
    // Draw instructions
    glColor3f(1.0f, 1.0f, 1.0f);
    char playText[] = "Press P to Play";
    glRasterPos2i(centerX - (strlen(playText) * 4), centerY - 20);
    for (int i = 0; i < strlen(playText); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, playText[i]);
    }
    
    char rulesText[] = "Press ? for Rules";
    glRasterPos2i(centerX - (strlen(rulesText) * 4), centerY - 50);
    for (int i = 0; i < strlen(rulesText); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, rulesText[i]);
    }
    
    // Draw animated effect (pulsing border)
    float pulseValue = (sin(glfwGetTime() * 2.0) + 1.0) * 0.5;
    glLineWidth(3.0f);
    glColor3f(0.5f + pulseValue * 0.5f, 0.5f + pulseValue * 0.3f, 0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(centerX - 200, centerY - 100);
    glVertex2f(centerX + 200, centerY - 100);
    glVertex2f(centerX + 200, centerY + 100);
    glVertex2f(centerX - 200, centerY + 100);
    glEnd();
    
    // Restore original state
    glLineWidth(1.0f);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

bool keyPressed[GLFW_KEY_LAST] = {false};

// Add this function to set up key callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Handle single key press events for menu navigation
    if (action == GLFW_PRESS) {
        // Save the key state
        keyPressed[key] = true;
        
        // Title screen controls
        if (currentGameState == TITLE_SCREEN) {
            if (key == GLFW_KEY_P) {
                currentGameState = PLAYING;
                gameStarted = false;
                gameCompleted = false;
                gameStartTime = glfwGetTime();
                sprintf(timeString, "Time: 0.00 seconds");
                
                // Reset player position
                playerX = 1.5f;
                playerY = 0.0f;
                playerZ = 1.5f;
                playerAngle = 0.0f;
            }
            
            // Check for '?' key (GLFW_KEY_SLASH with SHIFT)
            if (key == GLFW_KEY_SLASH && (mods & GLFW_MOD_SHIFT)) {
                currentGameState = RULES_SCREEN;
            }
        }
        
        // Rules screen controls
        else if (currentGameState == RULES_SCREEN) {
            if (key == GLFW_KEY_B) {
                currentGameState = TITLE_SCREEN;
            }
        }
        
        // Completed screen controls
        else if (currentGameState == COMPLETED) {
            if (key == GLFW_KEY_R) {
                // Reset player position
                playerX = 1.5f;
                playerY = 0.0f;
                playerZ = 1.5f;
                playerAngle = 0.0f;
                
                // Reset timer
                gameStarted = false;
                gameCompleted = false;
                currentGameState = PLAYING;
                gameStartTime = glfwGetTime();
                sprintf(timeString, "Time: 0.00 seconds");
            }
            
            if (key == GLFW_KEY_B) {
                currentGameState = TITLE_SCREEN;
            }
        }
    }
    else if (action == GLFW_RELEASE) {
        // Clear the key state
        keyPressed[key] = false;
    }
}
void renderRulesScreen() {
    // Disable lighting for 2D rendering
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw a light background
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.2f, 0.4f);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();
    
    // Draw title
    glColor3f(1.0f, 0.8f, 0.0f); // Gold color
    char titleText[] = "HOW TO PLAY";
    int centerX = WIDTH / 2;
    glRasterPos2i(centerX - (strlen(titleText) * 5), HEIGHT - 50);
    for (int i = 0; i < strlen(titleText); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, titleText[i]);
    }
    
    // Draw instructions
    glColor3f(1.0f, 1.0f, 1.0f);
    char rulesLines[][100] = {
        "CONTROLS:",
        "W - Move Forward",
        "S - Move Backward",
        "A - Turn Left",
        "D - Turn Right",
        "",
        "GOAL:",
        "Find the RED block in the maze as quickly as possible.",
        "Collect Gold Bars for Decreasing the time taken by 5 sec",
        "Your time will be displayed when you finish.",
        "Press B to return to the title screen"
    };
    
    for (int i = 0; i < 11; i++) {
        glRasterPos2i(WIDTH/4, HEIGHT - 100 - (i * 30));
        for (int j = 0; j < strlen(rulesLines[i]); j++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, rulesLines[i][j]);
        }
    }
    
    // Restore original state
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void renderTimer() {
    // Disable lighting for text rendering
    glDisable(GL_LIGHTING);
    
    // Switch to orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Set text color to white
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // Render the timer text in the top left corner
    glRasterPos2i(10, 20);
    for (int i = 0; timeString[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timeString[i]);
    }
    glRasterPos2i(10, 40); // Position below the timer
        for (int i = 0; scoreString[i] != '\0'; i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreString[i]);
        }
    // Return to 3D rendering
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    
    // Re-enable lighting
    glEnable(GL_LIGHTING);
}

void renderCongratsScreen() {
    // Save current matrices and disable lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw gradient background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBegin(GL_QUADS);
    // Dark blue to light blue gradient
    glColor4f(0.0f, 0.0f, 0.3f, 0.9f);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glColor4f(0.2f, 0.2f, 0.6f, 0.9f);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();
    
    // Draw decorative border
    glLineWidth(5.0f);
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0f, 0.8f, 0.0f); // Gold border
    glVertex2f(WIDTH * 0.1f, HEIGHT * 0.2f);
    glVertex2f(WIDTH * 0.9f, HEIGHT * 0.2f);
    glVertex2f(WIDTH * 0.9f, HEIGHT * 0.8f);
    glVertex2f(WIDTH * 0.1f, HEIGHT * 0.8f);
    glEnd();
    glLineWidth(1.0f);
    
    // Calculate center of screen
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    
    // Draw title text - LARGER SIZE using GLUT_BITMAP_TIMES_ROMAN_24
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow text
    char titleText[] = "MAZE COMPLETED!";
    // Estimate width for TIMES_ROMAN_24 (wider than Helvetica)
    int titleWidth = strlen(titleText) * 20;
    glRasterPos2i(centerX - (titleWidth / 4), centerY + 100);
    
    for (int i = 0; titleText[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, titleText[i]);
    }

    glColor3f(1.0f, 1.0f, 1.0f); // White text
    int msgWidth = strlen(congratsMessage) * 20;
    glRasterPos2i(centerX/1.35, centerY);
    
    for (int i = 0; congratsMessage[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, congratsMessage[i]);
    }
    glColor3f(1.0f, 1.0f, 1.0f); // White text
    int msgWidth1 = strlen(finalscore) * 20; // Adjusted for larger font
    glRasterPos2i(centerX/1.2, centerY-20);
    
    for (int i = 0; finalscore[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, finalscore[i]);
    }
    glColor3f(1.0f, 1.0f, 1.0f); // White text
    int msgWidth2 = strlen(finaltime) * 20; // Adjusted for larger font
    glRasterPos2i(centerX/1.3, centerY-40);
    
    for (int i = 0; finaltime[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, finaltime[i]);
    }
    
    // Draw instruction text - increased size
    char instructText[] = "Press R to play again or ESC to quit";
    int instWidth = strlen(instructText) * 20; // Adjusted for larger font
    glRasterPos2i(centerX/1.3, centerY - 70);
    
    for (int i = 0; instructText[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, instructText[i]);
    }
    
    // Draw animated stars or particles
    static float starPositions[20][2]; // Store 20 star positions
    static bool initialized = false;
    
    if (!initialized) {
        // Initialize star positions randomly
        for (int i = 0; i < 20; i++) {
            starPositions[i][0] = (float)(rand() % WIDTH);
            starPositions[i][1] = (float)(rand() % HEIGHT);
        }
        initialized = true;
    }
    
    // Draw and update stars - bigger stars
    glPointSize(6.0f); // Increased star size
    glBegin(GL_POINTS);
    for (int i = 0; i < 20; i++) {
        // Use time-based oscillation for color
        float oscillation = (sin(glfwGetTime() * 3.0f + i) + 1.0f) * 0.5f;
        glColor3f(1.0f, oscillation, oscillation); // Pulsing white-red
        glVertex2f(starPositions[i][0], starPositions[i][1]);
        
        // Update position - circular movement
        starPositions[i][0] += sin(glfwGetTime() * 2.0f + i) * 2.0f;
        starPositions[i][1] += cos(glfwGetTime() * 2.0f + i) * 2.0f;
        
        // Wrap around edges
        if (starPositions[i][0] < 0) starPositions[i][0] = WIDTH;
        if (starPositions[i][0] > WIDTH) starPositions[i][0] = 0;
        if (starPositions[i][1] < 0) starPositions[i][1] = HEIGHT;
        if (starPositions[i][1] > HEIGHT) starPositions[i][1] = 0;
    }
    glEnd();
    glPointSize(1.0f);
    
    // Restore matrices and re-enable features
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    // Initialize game state
    currentGameState = TITLE_SCREEN;
    
    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "3D Maze Game", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    
    // Make the window's context current
    glfwMakeContextCurrent(window);
    
    // Set the framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Initialize OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Set up basic lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    // Set material properties
    GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat high_shininess[] = { 100.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    
    // Initialize timer values
    gameStartTime = glfwGetTime();
    sprintf(timeString, "Time: 0.00 seconds");
    sprintf(scoreString, "Gold Bars Collected: %d", playerScore);
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);
        
        // Clear screen with sky blue background
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render based on current game state
        switch (currentGameState) {
            case TITLE_SCREEN:
                renderTitleScreen();
                break;
                
            case RULES_SCREEN:
                renderRulesScreen();
                break;
                
            case PLAYING:
                // Update timer if game has started and not completed
                if (gameStarted && !gameCompleted) {
                    currentGameTime = glfwGetTime() - gameStartTime;
                    sprintf(timeString, "Time: %.2f seconds", currentGameTime);
                } else if (!gameStarted &&
                          (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                           glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
                           glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
                           glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)) {
                    // Start timer when player first moves
                    gameStarted = true;
                    gameStartTime = glfwGetTime();
                }
                
                // Set up the camera view
                setupCamera();
                
                // Render the maze
                renderMaze();
                
                // Render timer display
                renderTimer();
                break;
                
            case COMPLETED:
                renderCongratsScreen();
                break;
        }
        
        // Swap front and back buffers
        glfwSwapBuffers(window);
        
        // Poll for and process events
        glfwPollEvents();
    }
    
    // Clean up
    glfwTerminate();
    return 0;
}
// Set up the camera perspective and position
void setupCamera() {
    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1, 100.0);
    
    // Set up modelview matrix for camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Set camera position and orientation
    // The camera looks in the direction of the player's angle
    float lookX = sin(playerAngle);
    float lookZ = -cos(playerAngle);
    
    gluLookAt(
        playerX, playerY + 0.5f, playerZ,           // Eye position
        playerX + lookX, playerY + 0.5f, playerZ + lookZ,  // Look at position
        0.0f, 1.0f, 0.0f                          // Up vector
    );
}

// Draw a cube with the specified size
void drawCube(float size) {
    float halfSize = size / 2.0f;
    
    // Define normals for each face
    GLfloat normals[][3] = {
        {0.0f, 0.0f, 1.0f},  // Front face
        {0.0f, 0.0f, -1.0f}, // Back face
        {1.0f, 0.0f, 0.0f},  // Right face
        {-1.0f, 0.0f, 0.0f}, // Left face
        {0.0f, 1.0f, 0.0f},  // Top face
        {0.0f, -1.0f, 0.0f}  // Bottom face
    };
    
    // Define vertices for each face
    GLfloat vertices[][3] = {
        // Front face
        {-halfSize, -halfSize, halfSize},
        {halfSize, -halfSize, halfSize},
        {halfSize, halfSize, halfSize},
        {-halfSize, halfSize, halfSize},
        
        // Back face
        {-halfSize, -halfSize, -halfSize},
        {-halfSize, halfSize, -halfSize},
        {halfSize, halfSize, -halfSize},
        {halfSize, -halfSize, -halfSize},
        
        // Right face
        {halfSize, -halfSize, halfSize},
        {halfSize, -halfSize, -halfSize},
        {halfSize, halfSize, -halfSize},
        {halfSize, halfSize, halfSize},
        
        // Left face
        {-halfSize, -halfSize, halfSize},
        {-halfSize, halfSize, halfSize},
        {-halfSize, halfSize, -halfSize},
        {-halfSize, -halfSize, -halfSize},
        
        // Top face
        {-halfSize, halfSize, halfSize},
        {halfSize, halfSize, halfSize},
        {halfSize, halfSize, -halfSize},
        {-halfSize, halfSize, -halfSize},
        
        // Bottom face
        {-halfSize, -halfSize, halfSize},
        {-halfSize, -halfSize, -halfSize},
        {halfSize, -halfSize, -halfSize},
        {halfSize, -halfSize, halfSize}
    };
    
    // Draw the cube
    glBegin(GL_QUADS);
    
    // Front face
    glNormal3fv(normals[0]);
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);
    
    // Back face
    glNormal3fv(normals[1]);
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[7]);
    
    // Right face
    glNormal3fv(normals[2]);
    glVertex3fv(vertices[8]);
    glVertex3fv(vertices[9]);
    glVertex3fv(vertices[10]);
    glVertex3fv(vertices[11]);
    
    // Left face
    glNormal3fv(normals[3]);
    glVertex3fv(vertices[12]);
    glVertex3fv(vertices[13]);
    glVertex3fv(vertices[14]);
    glVertex3fv(vertices[15]);
    
    // Top face
    glNormal3fv(normals[4]);
    glVertex3fv(vertices[16]);
    glVertex3fv(vertices[17]);
    glVertex3fv(vertices[18]);
    glVertex3fv(vertices[19]);
    
    // Bottom face
    glNormal3fv(normals[5]);
    glVertex3fv(vertices[20]);
    glVertex3fv(vertices[21]);
    glVertex3fv(vertices[22]);
    glVertex3fv(vertices[23]);
    
    glEnd();
}

// Render text on screen (simple implementation)

// Render the maze walls, floor, and goal
void renderMaze() {
    // Floor
    glPushMatrix();
    GLfloat floor_ambient[] = { 0.45f, 0.25f, 0.08f, 1.0f };
        GLfloat floor_diffuse[] = { 0.65f, 0.35f, 0.15f, 1.0f };
        GLfloat floor_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat floor_shininess[] = { 5.0f };
        
        glMaterialfv(GL_FRONT, GL_AMBIENT, floor_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, floor_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, floor_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, floor_shininess);
    glBegin(GL_QUADS);
    GLfloat brown[] = { 0.65f, 0.32f, 0.10f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, brown);
    
    goldBarYOffset = sin(glfwGetTime() * goldBarAnimSpeed) * 0.1f;
        goldBarRotation += 1.0f; // Rotate gold bars
        if (goldBarRotation > 360.0f) goldBarRotation -= 360.0f;
    
    glNormal3f(0.0f, 1.0f, 0.0f); // Floor normal points up
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(MAZE_WIDTH, 0.0f, 0.0f);
    glVertex3f(MAZE_WIDTH, 0.0f, MAZE_HEIGHT);
    glVertex3f(0.0f, 0.0f, MAZE_HEIGHT);
    glEnd();
    glPopMatrix();
    
    // Maze walls
    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {
            if (maze[z][x] == 1) {
                // Regular wall (green)
                GLfloat grey[] = { 0.5f, 0.5f, 0.5f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grey);
                
                glPushMatrix();
                glTranslatef(x + 0.5f, 0.5f, z + 0.5f);
                drawCube(1.0f);
                glPopMatrix();
            } else if (maze[z][x] == 2) {
                // Goal (red wall)
                GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
                
                glPushMatrix();
                glTranslatef(x + 0.5f, 0.5f, z + 0.5f);
                drawCube(1.0f);
                glPopMatrix();
            }
            else if (maze[z][x] == 3) {
                // gold
                // Solid yellow-gold color without reflection changes during rotation
                GLfloat yellow[] = { 1.0f, 0.84f, 0.0f, 1.0f }; // Bright gold-yellow color
                GLfloat no_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // No specular highlight
                GLfloat no_shininess[] = { 0.0f }; // No shininess

                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
                glMaterialfv(GL_FRONT, GL_SPECULAR, no_specular); // Disable specular highlights
                glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess); // Disable shininess
                                           
                // Draw floating gold bar
                glPushMatrix();
                glTranslatef(x + 0.5f, 0.3f + goldBarYOffset, z + 0.5f);
                glRotatef(goldBarRotation, 0.0f, 1.0f, 0.0f); // Rotate around Y axis
                glScalef(0.3f, 0.15f, 0.6f); // Make it bar-shaped
                drawCube(1.0f);
                glPopMatrix();
            }
        }
    }
}

bool keyPressedLastFrame[GLFW_KEY_LAST] = {false};
// Process keyboard input for continuous movement
void processInput(GLFWwindow* window) {
    // Get the current time for smooth movement
    float currentTime = glfwGetTime();
    static float lastTime = 0.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Handle title screen input
    if (currentGameState == TITLE_SCREEN) {
        // Check for P key (Play game)
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_P]) {
            currentGameState = PLAYING;
            gameStarted = false;
            gameCompleted = false;
            gameStartTime = glfwGetTime();
            sprintf(timeString, "Time: 0.00 seconds");
            
            // Reset player position
            playerX = 1.5f;
            playerY = 0.0f;
            playerZ = 1.5f;
            playerAngle = 0.0f;
            
            keyPressedLastFrame[GLFW_KEY_P] = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_P] = false;
        }
       
        // Check for ? key (Rules)
        bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                           glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        
        if (glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS && shiftPressed &&
            !keyPressedLastFrame[GLFW_KEY_SLASH]) {
            currentGameState = RULES_SCREEN;
            keyPressedLastFrame[GLFW_KEY_SLASH] = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_SLASH] = false;
        }
        
        // ESC key to exit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        
        return; // Skip further processing
    }
    
    // Handle rules screen input
    if (currentGameState == RULES_SCREEN) {
        // B key to go back to title screen
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            currentGameState = TITLE_SCREEN;
            keyPressedLastFrame[GLFW_KEY_B] = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_B] = false;
        }
        
        // ESC key to exit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        
        return; // Skip further processing
    }
    
    // Handle game completed state
    if (currentGameState == COMPLETED || gameCompleted) {
        // R key to restart
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_R]) {
            // Reset player position
            playerX = 1.5f;
            playerY = 0.0f;
            playerZ = 1.5f;
            playerAngle = 0.0f;
            
            // Reset timer
            gameStarted = false;
            gameCompleted = false;
            currentGameState = PLAYING;
            gameStartTime = glfwGetTime();
            sprintf(timeString, "Time: 0.00 seconds");
            
            printf("Game restarted!\n");
            keyPressedLastFrame[GLFW_KEY_R] = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_R] = false;
        }
        
        // B key to go back to title
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            currentGameState = TITLE_SCREEN;
            keyPressedLastFrame[GLFW_KEY_B] = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_B] = false;
        }
        
        // ESC key to exit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        
        return; // Skip movement processing if game is completed
    }
    
    // From here on, we're in the PLAYING state
    // Adjust speeds based on frame time
    float adjustedSpeed = playerSpeed * deltaTime * 60.0f;
    float adjustedRotSpeed = rotationSpeed * deltaTime * 60.0f;
    
    // Start the game timer when first movement occurs
    if (!gameStarted && (
            glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)) {
        gameStarted = true;
        gameStartTime = glfwGetTime();
    }
    
    // Forward movement (W)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        float newX = playerX + sin(playerAngle) * adjustedSpeed;
        float newZ = playerZ - cos(playerAngle) * adjustedSpeed;
        
        // Try movement along individual axes if combined movement fails
        if (!checkCollision(newX, newZ)) {
            playerX = newX;
            playerZ = newZ;
        } else {
            // Try moving only along X-axis
            if (!checkCollision(newX, playerZ)) {
                playerX = newX;
            }
            // Try moving only along Z-axis
            else if (!checkCollision(playerX, newZ)) {
                playerZ = newZ;
            }
        }
    }
    
    // Backward movement (S)
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        float newX = playerX - sin(playerAngle) * adjustedSpeed;
        float newZ = playerZ + cos(playerAngle) * adjustedSpeed;
        
        // Try movement along individual axes if combined movement fails
        if (!checkCollision(newX, newZ)) {
            playerX = newX;
            playerZ = newZ;
        } else {
            // Try moving only along X-axis
            if (!checkCollision(newX, playerZ)) {
                playerX = newX;
            }
            // Try moving only along Z-axis
            else if (!checkCollision(playerX, newZ)) {
                playerZ = newZ;
            }
        }
    }
    
    // Rotate left (A)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        playerAngle -= adjustedRotSpeed;
    }
    
    // Rotate right (D)
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        playerAngle += adjustedRotSpeed;
    }
    
    // Check if player reached the goal
    int cellX = (int)playerX;
    int cellZ = (int)playerZ;
    
    if (cellX >= 0 && cellX < MAZE_WIDTH && cellZ >= 0 && cellZ < MAZE_HEIGHT) {
        if (maze[cellZ][cellX] == 2 && !gameCompleted) {
            // Mark game as completed
            gameCompleted = true;
            currentGameState = COMPLETED;
            // Set congratulation message with completion time
            sprintf(congratsMessage, "You completed the maze in %.2f seconds.\n", currentGameTime);
            sprintf(finalscore,"Gold Bars Collected: %d\n", playerScore);
            sprintf(finaltime,"Final Completion Time: %.2f seconds.\n", currentGameTime-(5* playerScore));
            printf("Congratulations! You reached the goal!\n");
        }
        if (maze[cellZ][cellX] == 3) {
                // Collect the gold bar
                maze[cellZ][cellX] = 0; // Remove gold bar from maze
                playerScore++;
                sprintf(scoreString, "Gold Bars Collected: %d", playerScore);
                printf("Gold bar collected! total: %d\n", playerScore);
            }
    }
    
    // ESC key to return to title screen (not exit the game)
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_ESCAPE]) {
        currentGameState = TITLE_SCREEN;
        keyPressedLastFrame[GLFW_KEY_ESCAPE] = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        keyPressedLastFrame[GLFW_KEY_ESCAPE] = false;
    }
}
// Check for collision with maze walls
bool checkCollision(float newX, float newZ) {
    // Use a smaller buffer for more precise collision detection
    float buffer = 0.15f;
    
    // Calculate the grid cell positions for the player's bounding circle
    int centerCellX = (int)newX;
    int centerCellZ = (int)newZ;
    
    // Check if the center position is valid
    if (centerCellX < 0 || centerCellX >= MAZE_WIDTH ||
        centerCellZ < 0 || centerCellZ >= MAZE_HEIGHT) {
        return true; // Out of bounds
    }
    
    // Check if the center cell is a wall
    if (maze[centerCellZ][centerCellX] == 1) {
        return true; // Collision with wall
    }
    
    // Check adjacent cells only if we're close to their boundaries
    float fracX = newX - centerCellX;
    float fracZ = newZ - centerCellZ;
    
    // Check cell to the right
    if (fracX > (1.0f - buffer) && centerCellX + 1 < MAZE_WIDTH) {
        if (maze[centerCellZ][centerCellX + 1] == 1) {
            return true;
        }
    }
    
    // Check cell to the left
    if (fracX < buffer && centerCellX - 1 >= 0) {
        if (maze[centerCellZ][centerCellX - 1] == 1) {
            return true;
        }
    }
    
    // Check cell below
    if (fracZ > (1.0f - buffer) && centerCellZ + 1 < MAZE_HEIGHT) {
        if (maze[centerCellZ + 1][centerCellX] == 1) {
            return true;
        }
    }
    
    // Check cell above
    if (fracZ < buffer && centerCellZ - 1 >= 0) {
        if (maze[centerCellZ - 1][centerCellX] == 1) {
            return true;
        }
    }
    
    // Check diagonal cells if we're close to corners
    if (fracX > (1.0f - buffer) && fracZ > (1.0f - buffer) &&
        centerCellX + 1 < MAZE_WIDTH && centerCellZ + 1 < MAZE_HEIGHT) {
        if (maze[centerCellZ + 1][centerCellX + 1] == 1) {
            return true;
        }
    }
    
    if (fracX < buffer && fracZ < buffer &&
        centerCellX - 1 >= 0 && centerCellZ - 1 >= 0) {
        if (maze[centerCellZ - 1][centerCellX - 1] == 1) {
            return true;
        }
    }
    
    if (fracX > (1.0f - buffer) && fracZ < buffer &&
        centerCellX + 1 < MAZE_WIDTH && centerCellZ - 1 >= 0) {
        if (maze[centerCellZ - 1][centerCellX + 1] == 1) {
            return true;
        }
    }
    
    if (fracX < buffer && fracZ > (1.0f - buffer) &&
        centerCellX - 1 >= 0 && centerCellZ + 1 < MAZE_HEIGHT) {
        if (maze[centerCellZ + 1][centerCellX - 1] == 1) {
            return true;
        }
    }
    
    return false; // No collision
}

// Handle window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)width / (GLfloat)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

