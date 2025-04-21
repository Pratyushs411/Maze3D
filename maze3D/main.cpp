#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <math.h>
#include<GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>

// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;

// Timer variables
float gameStartTime = 0.0f;
float currentGameTime = 0.0f;
bool gameStarted = false;
char timeString[32];

// Player properties
float playerX = 1.5f;
float playerY = 0.0f;
float playerZ = 1.5f;
float playerAngle = 0.0f;
float playerSpeed = 0.05f;
float rotationSpeed = 0.03f;

// Maze dimensions and layout
const int MAZE_WIDTH = 10;
const int MAZE_HEIGHT = 10;
int maze[MAZE_WIDTH][MAZE_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 2, 1}, // 2 represents the goal (red wall)
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
    
    // Return to 3D rendering
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    
    // Re-enable lighting
    glEnable(GL_LIGHTING);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
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
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);
        
        // Update timer if game has started
        if (gameStarted) {
            currentGameTime = glfwGetTime() - gameStartTime;
            sprintf(timeString, "Time: %.2f seconds", currentGameTime);
        } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            // Start timer when player first moves
            gameStarted = true;
            gameStartTime = glfwGetTime();
        }
        
        // Set background color (blue sky)
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Set up the camera view
        setupCamera();
        
        // Render the maze
        renderMaze();
        
        // Render timer display
        renderTimer();
        
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
                GLfloat green[] = { 0.0f, 0.8f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
                
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
        }
    }
}

// Process keyboard input for continuous movement
void processInput(GLFWwindow* window) {
    // Get the current time for smooth movement
    float currentTime = glfwGetTime();
    static float lastTime = 0.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Adjust speeds based on frame time
    float adjustedSpeed = playerSpeed * deltaTime * 60.0f;
    float adjustedRotSpeed = rotationSpeed * deltaTime * 60.0f;
    
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
        if (maze[cellZ][cellX] == 2) {
            printf("Congratulations! You reached the goal!\n");
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }
    
    // ESC key to exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
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

