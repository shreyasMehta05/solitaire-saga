#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <ctime>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "file_utils.h"
#include "math_utils.h"

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define GL_SILENCE_DEPRECATION

using namespace std;


/* ################################################################# */
// Variables //
const int BOARD_SIZE = 7;
char theProgramTitle[] = "Marble Solitaire";
int theWindowWidth = 1000, theWindowHeight = 1000;
int theWindowPositionX = 40, theWindowPositionY = 40;
bool isFullScreen = false;
bool isAnimating = true;
float rotation = 0.0f;
GLuint boardVBO, boardVAO, marbleVBO, marbleVAO;
GLuint gWorldLocation, gColorLocation, gSelectedLocation;
GLuint gPulseLocation;
const int MAX_UNDO_MOVES = 3;  // Maximum number of undo/redo moves
bool showUndoLimitMsg = false; // Flags for undo/redo limit notifications
bool showRedoLimitMsg = false;
float msgDisplayTime = 0.0f;
const float MSG_DISPLAY_DURATION = 2.0f; // Time for notification display in seconds
/* ################################################################# */



/* ################################################################# */
// Game state //
enum CellState { EMPTY = 0, FILLED = 1, INVALID = 2 };
CellState boardState[BOARD_SIZE][BOARD_SIZE];
bool isMarbleSelected = false;
int selectedRow = -1, selectedCol = -1;
int hoverRow = -1, hoverCol = -1;
vector<pair<pair<int, int>, pair<int, int>>> moveHistory; // (from, to)
vector<pair<pair<int, int>, pair<int, int>>> redoStack;
int remainingMarbles = 0;
time_t gameStartTime;
bool gameWon = false;
bool gameLost = false;
/* ################################################################# */


/* ################################################################# */
/* Constants */
const int ANIMATION_DELAY = 20; /* milliseconds between rendering */
const char *pVSFileName = "shaders/shader.vs";
const char *pFSFileName = "shaders/shader.fs";
const char *pMarbleVSFileName = "shaders/marble_shader.vs";
const char *pMarbleFSFileName = "shaders/marble_shader.fs";
/* ################################################################# */

/* ################################################################# */
// Shader programs
GLuint boardShaderProgram;
GLuint marbleShaderProgram;
/* ################################################################# */

/* ################################################################# */
/* Utility functions */


// Initialize the board state
void initializeBoard() {
    // Reset counters
    remainingMarbles = 0;
    moveHistory.clear();
    redoStack.clear();
    gameWon = false;
    gameLost = false;
    gameStartTime = time(NULL);
    
    printf("Initializing board of size %d x %d\n", BOARD_SIZE, BOARD_SIZE);
    
    // Set up board with invalid regions (corners) and valid cells
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if ((i < 2 || i >= BOARD_SIZE - 2) && (j < 2 || j >= BOARD_SIZE - 2)) {
                boardState[i][j] = INVALID; // Invalid corners
            } else {
                boardState[i][j] = FILLED; // All other spaces have marbles
                remainingMarbles++;
            }
        }
    }
    
    // Create the central empty space
    boardState[BOARD_SIZE/2][BOARD_SIZE/2] = EMPTY;
    remainingMarbles--;
    
    printf("Board initialized with %d marbles\n", remainingMarbles);
}

// Check if the game is over (no more valid moves)
bool checkGameOver() {
    // Go through every position on the board
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (boardState[i][j] == FILLED) {
                // Check all four directions for a valid move
                // Check up
                if (i >= 2 && boardState[i-1][j] == FILLED && boardState[i-2][j] == EMPTY)
                    return false;
                // Check down
                if (i < BOARD_SIZE-2 && boardState[i+1][j] == FILLED && boardState[i+2][j] == EMPTY)
                    return false;
                // Check left
                if (j >= 2 && boardState[i][j-1] == FILLED && boardState[i][j-2] == EMPTY)
                    return false;
                // Check right
                if (j < BOARD_SIZE-2 && boardState[i][j+1] == FILLED && boardState[i][j+2] == EMPTY)
                    return false;
            }
        }
    }
    
    // If we got here, no valid moves were found
    if (remainingMarbles == 1) {
        gameWon = true;
    } else {
        gameLost = true;
    }
    return true;
}

// Check if a move is valid
bool isValidMove(int startRow, int startCol, int endRow, int endCol) {
    // Check if start and end positions are valid
    if (startRow < 0 || startRow >= BOARD_SIZE || startCol < 0 || startCol >= BOARD_SIZE ||
        endRow < 0 || endRow >= BOARD_SIZE || endCol < 0 || endCol >= BOARD_SIZE)
        return false;
        
    // Start must have a marble, end must be empty
    if (boardState[startRow][startCol] != FILLED || boardState[endRow][endCol] != EMPTY)
        return false;
        
    // Move must be exactly two spaces horizontally or vertically
    if ((abs(startRow - endRow) == 2 && startCol == endCol) || 
        (abs(startCol - endCol) == 2 && startRow == endRow)) {
        
        // Check the marble to be jumped over
        int middleRow = (startRow + endRow) / 2;
        int middleCol = (startCol + endCol) / 2;
        
        return boardState[middleRow][middleCol] == FILLED;
    }
    
    return false;
}

// Make a move on the board with undo limit
void makeMove(int startRow, int startCol, int endRow, int endCol) {
    if (!isValidMove(startRow, startCol, endRow, endCol))
        return;
        
    int middleRow = (startRow + endRow) / 2;
    int middleCol = (startCol + endCol) / 2;
    
    // Update board state
    boardState[startRow][startCol] = EMPTY;
    boardState[middleRow][middleCol] = EMPTY;
    boardState[endRow][endCol] = FILLED;
    
    // Record move in history
    moveHistory.push_back(std::make_pair(std::make_pair(startRow, startCol), 
                                         std::make_pair(endRow, endCol)));
    
    // Limit the undo stack to MAX_UNDO_MOVES by removing the oldest move if necessary
    if (moveHistory.size() > MAX_UNDO_MOVES) {
        moveHistory.erase(moveHistory.begin()); // Remove the oldest move
    }
    
    // Clear redo stack when a new move is made (branching history)
    redoStack.clear();
    
    // Reset notification flags when a new move is made
    showUndoLimitMsg = false;
    showRedoLimitMsg = false;
    
    // Update marble count
    remainingMarbles--;
    
    // Check if game is over
    checkGameOver();
}

// Undo the last move, limited by MAX_UNDO_MOVES
void undoMove() {
    if (moveHistory.empty()) {
        showUndoLimitMsg = true;
        msgDisplayTime = glfwGetTime();
        return;
    }
    
    // Get the last move
    auto lastMove = moveHistory.back();
    moveHistory.pop_back();
    
    // Add to redo stack
    redoStack.push_back(lastMove);
    
    // Limit the redo stack to MAX_UNDO_MOVES by removing the oldest move if necessary
    if (redoStack.size() > MAX_UNDO_MOVES) {
        redoStack.erase(redoStack.begin()); // Remove the oldest move
    }
    
    // Restore positions
    int startRow = lastMove.first.first;
    int startCol = lastMove.first.second;
    int endRow = lastMove.second.first;
    int endCol = lastMove.second.second;
    int middleRow = (startRow + endRow) / 2;
    int middleCol = (startCol + endCol) / 2;
    
    // Restore board state
    boardState[startRow][startCol] = FILLED;
    boardState[middleRow][middleCol] = FILLED;
    boardState[endRow][endCol] = EMPTY;
    
    // Update marble count
    remainingMarbles++;
    
    // Reset game over flags
    gameWon = false;
    gameLost = false;
}

// Redo the last undone move
void redoMove() {
    if (redoStack.empty()) {
        return;
    }
    
    // Get the last undone move
    auto redoMove = redoStack.back();
    redoStack.pop_back();
    
    // Add back to history
    moveHistory.push_back(redoMove);
    
    // Limit the undo stack to MAX_UNDO_MOVES by removing the oldest move if necessary
    if (moveHistory.size() > MAX_UNDO_MOVES) {
        moveHistory.erase(moveHistory.begin()); // Remove the oldest move
    }
    
    // Apply the move
    int startRow = redoMove.first.first;
    int startCol = redoMove.first.second;
    int endRow = redoMove.second.first;
    int endCol = redoMove.second.second;
    int middleRow = (startRow + endRow) / 2;
    int middleCol = (startCol + endCol) / 2;
    
    // Update board state
    boardState[startRow][startCol] = EMPTY;
    boardState[middleRow][middleCol] = EMPTY;
    boardState[endRow][endCol] = FILLED;
    
    // Update marble count
    remainingMarbles--;
    
    // Check if game is over
    checkGameOver();
}

// Get the pixel coordinates for a board position
void getBoardPixelCoordinates(int row, int col, float &x, float &y) {
    float cellSize = 2.0f / BOARD_SIZE;
    x = -1.0f + col * cellSize + cellSize / 2.0f;
    y = 1.0f - row * cellSize - cellSize / 2.0f;
    
    // printf("X :%f, Y:%f\n",x,y);
}

// Convert mouse coordinates to board coordinates
void getBoardCoordinates(double mouseX, double mouseY, int &row, int &col) {
    // Convert mouse coordinates to normalized device coordinates
    float x = (2.0f * mouseX / theWindowWidth) - 1.0f;
    float y = 1.0f - (2.0f * mouseY / theWindowHeight);
    
    // Calculate the board position
    float cellSize = 2.0f / BOARD_SIZE;
    col = (int)((x + 1.0f) / cellSize);
    row = (int)((1.0f - y) / cellSize);
    
    // Clamp to valid board coordinates
    if (row < 0) row = 0;
    if (row >= BOARD_SIZE) row = BOARD_SIZE - 1;
    if (col < 0) col = 0;
    if (col >= BOARD_SIZE) col = BOARD_SIZE - 1;
}

// Create the board vertex buffer
void CreateBoardVertexBuffer() {
    // Create squares for each cell on the board
    std::vector<float> vertices;
    float cellSize = 2.0f / BOARD_SIZE;
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (boardState[i][j] != INVALID) {
                float x = -1.0f + j * cellSize;
                float y = 1.0f - i * cellSize;
                
                // Add square (made of two triangles)
                // First triangle
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(0.0f);
                
                vertices.push_back(x + cellSize);
                vertices.push_back(y);
                vertices.push_back(0.0f);
                
                vertices.push_back(x);
                vertices.push_back(y - cellSize);
                vertices.push_back(0.0f);
                
                // Second triangle
                vertices.push_back(x + cellSize);
                vertices.push_back(y);
                vertices.push_back(0.0f);
                
                vertices.push_back(x + cellSize);
                vertices.push_back(y - cellSize);
                vertices.push_back(0.0f);
                
                vertices.push_back(x);
                vertices.push_back(y - cellSize);
                vertices.push_back(0.0f);
            }
        }
    }
    
    // Generate buffers and vertex array
    glGenVertexArrays(1, &boardVAO); // this helps in storing the vertex array
    glGenBuffers(1, &boardVBO); // this helps in storing board vertex array
    
    // Bind and fill the buffer
    glBindVertexArray(boardVAO); // Why we need to bind?
    glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Set attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Create marble vertex buffer (circles)
void CreateMarbleVertexBuffer() {
    std::vector<float> vertices;
    const int segments = 32;
    const float radius = 0.8f / BOARD_SIZE;  // Slightly smaller than cell
    const float zOffset = -0.1f;  // Add a small Z offset to ensure marbles appear in front
    
    printf("Creating marbles for board of size %d x %d\n", BOARD_SIZE, BOARD_SIZE);
    int validCells = 0;
    int marbleCount = 0;
    
    // Create vertices for marbles
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (boardState[i][j] != INVALID) {
                validCells++;
                float centerX, centerY;
                getBoardPixelCoordinates(i, j, centerX, centerY);
                
                if (boardState[i][j] == FILLED) {
                    marbleCount++;
                }
                
                // Center vertex (use 0,0,0 as the relative center for each marble)
                vertices.push_back(0.0f);  // Relative x (will be transformed in shader)
                vertices.push_back(0.0f);  // Relative y (will be transformed in shader)
                vertices.push_back(zOffset);  // Z offset
                
                // Circle vertices
                for (int k = 0; k <= segments; k++) {
                    float angle = 2.0f * M_PI * k / segments;
                    // Store positions relative to center, scaled by radius
                    vertices.push_back(radius * cos(angle));  // Relative x
                    vertices.push_back(radius * sin(angle));  // Relative y
                    vertices.push_back(zOffset);  // Z offset
                }
            }
        }
    }
    
    printf("Board has %d valid cells and %d marbles\n", validCells, marbleCount);
    printf("Created %lu vertices for marbles\n", vertices.size() / 3);
    
    // Generate buffers and vertex array
    glGenVertexArrays(1, &marbleVAO);
    glGenBuffers(1, &marbleVBO);
    
    // Bind and fill the buffer
    glBindVertexArray(marbleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, marbleVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Set attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AddShader(GLuint ShaderProgram, const char *pShaderText, GLenum ShaderType) {
    GLuint ShaderObj = glCreateShader(ShaderType);
    
    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(1);
    }
    
    const GLchar *p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    
    glAttachShader(ShaderProgram, ShaderObj);
}

// Check for shader loading and improve error reporting
static void CompileShaders() {
    // Create board shader program
    boardShaderProgram = glCreateProgram();
    
    if (boardShaderProgram == 0) {
        fprintf(stderr, "Error creating board shader program\n");
        exit(1);
    }
    
    std::string vs, fs;
    
    if (!ReadFile(pVSFileName, vs)) {
        fprintf(stderr, "Error: Could not read vertex shader file '%s'\n", pVSFileName);
        exit(1);
    }
    
    if (!ReadFile(pFSFileName, fs)) {
        fprintf(stderr, "Error: Could not read fragment shader file '%s'\n", pFSFileName);
        exit(1);
    }
    
    // Print shader file paths to debug
    printf("Loading board vertex shader from: %s\n", pVSFileName);
    printf("Loading board fragment shader from: %s\n", pFSFileName);
    
    AddShader(boardShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
    AddShader(boardShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);
    
    GLint success = 0;
    GLchar errorLog[1024] = {0};
    
    glLinkProgram(boardShaderProgram);
    glGetProgramiv(boardShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(boardShaderProgram, sizeof(errorLog), NULL, errorLog);
        fprintf(stderr, "Error linking board shader program: '%s'\n", errorLog);
        exit(1);
    }
    
    glValidateProgram(boardShaderProgram);
    glGetProgramiv(boardShaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(boardShaderProgram, sizeof(errorLog), NULL, errorLog);
        fprintf(stderr, "Invalid board shader program: '%s'\n", errorLog);
        exit(1);
    }
    
    // Get uniform locations for board shader
    gWorldLocation = glGetUniformLocation(boardShaderProgram, "gWorld");
    gColorLocation = glGetUniformLocation(boardShaderProgram, "gColor");
    
    
    // Create marble shader program
    marbleShaderProgram = glCreateProgram();
    
    if (marbleShaderProgram == 0) {
        fprintf(stderr, "Error creating marble shader program\n");
        exit(1);
    }
    
    std::string marbleVs, marbleFs;
    
    if (!ReadFile(pMarbleVSFileName, marbleVs)) {
        fprintf(stderr, "Error: Could not read marble vertex shader file '%s'\n", pMarbleVSFileName);
        exit(1);
    }
    
    if (!ReadFile(pMarbleFSFileName, marbleFs)) {
        fprintf(stderr, "Error: Could not read marble fragment shader file '%s'\n", pMarbleFSFileName);
        exit(1);
    }
    
    // Print shader file paths to debug
    printf("Loading marble vertex shader from: %s\n", pMarbleVSFileName);
    printf("Loading marble fragment shader from: %s\n", pMarbleFSFileName);
    
    AddShader(marbleShaderProgram, marbleVs.c_str(), GL_VERTEX_SHADER);
    AddShader(marbleShaderProgram, marbleFs.c_str(), GL_FRAGMENT_SHADER);
    
    glLinkProgram(marbleShaderProgram);
    glGetProgramiv(marbleShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(marbleShaderProgram, sizeof(errorLog), NULL, errorLog);
        fprintf(stderr, "Error linking marble shader program: '%s'\n", errorLog);
        exit(1);
    }
    
    glValidateProgram(marbleShaderProgram);
    glGetProgramiv(marbleShaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(marbleShaderProgram, sizeof(errorLog), NULL, errorLog);
        fprintf(stderr, "Invalid marble shader program: '%s'\n", errorLog);
        exit(1);
    }
    
    // Get uniform locations for marble shader
    gSelectedLocation = glGetUniformLocation(marbleShaderProgram, "gSelected");
}

/********************************************************************
 Callback Functions
 */

void onInit(int argc, char *argv[]) {
    // Initialize board
    initializeBoard();
    
    // Create vertex buffers
    CreateBoardVertexBuffer();
    CreateMarbleVertexBuffer();
    
    // Compile shaders
    CompileShaders();
    
    // Enable depth testing with proper function
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  // Use LEQUAL instead of LESS for better z-fighting handling
    
    // Disable face culling to make sure marbles are visible from both sides
    glDisable(GL_CULL_FACE);
    
    printf("OpenGL depth testing enabled with GL_LEQUAL function\n");
}

// Modify onDisplay to clear depth buffer properly and use blending
static void onDisplay() {
    // Set background color to a dark, modern color from the suggested palette
    glClearColor(0.11f, 0.15f, 0.20f, 1.0f);  // #1B2631 (Almost black modern theme)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Draw the board squares
    glUseProgram(boardShaderProgram);
    
    Matrix4f world;
    world.InitIdentity();
    // Add a 180-degree rotation around Y-axis to flip everything toward camera
    // world.InitRotateTransform(0.0f, 180.0f * M_PI / 180.0f, 0.0f);  // Use InitRotateTransform instead of Rotate
    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &world.m[0][0]);
    
    glBindVertexArray(boardVAO);
    
    // Draw each cell
    int cellCount = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (boardState[i][j] != INVALID) {
                // Use a dark wood theme for the checkerboard pattern
                if ((i + j) % 2 == 0) {
                    glUniform3f(gColorLocation, 0.24f, 0.15f, 0.14f); // #3E2723 Dark brown
                } else {
                    glUniform3f(gColorLocation, 0.36f, 0.25f, 0.22f); // #5D4037 Warm brown
                }
                
                // Draw the cell (2 triangles)
                glDrawArrays(GL_TRIANGLES, cellCount * 6, 6);
                cellCount++;
            }
        }
    }
    
    // Now draw the marbles
    glUseProgram(marbleShaderProgram);
    
    // Enable blending for better marble rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(marbleVAO);
    
    // Draw each marble
    int marbleOffset = 0;
    int marblesDrawn = 0;
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (boardState[i][j] != INVALID) {
                // Skip drawing if cell is empty
                if (boardState[i][j] == FILLED) {
                    // Calculate center position for this marble
                    float centerX, centerY;
                    getBoardPixelCoordinates(i, j, centerX, centerY);
                    
                    // Create model matrix for this marble
                    Matrix4f marbleModel;
                    marbleModel.InitTranslationTransform(centerX, centerY, 0.0f);
                    
                    // Apply the world transformation
                    Matrix4f marbleTransform = world * marbleModel;
                    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &marbleTransform.m[0][0]);
                    
                    // Highlight selected marble
                    if (isMarbleSelected && i == selectedRow && j == selectedCol) {
                        glUniform1i(gSelectedLocation, 1);
                    } else if (hoverRow == i && hoverCol == j) {
                        glUniform1i(gSelectedLocation, 2); // Hover state
                    } else {
                        glUniform1i(gSelectedLocation, 0);
                    }
                    
                    // Draw the marble as a triangle fan (center + segments)
                    glDrawArrays(GL_TRIANGLE_FAN, marbleOffset, 33);
                    marblesDrawn++;
                }
                marbleOffset += 33; // Move to next marble position
            }
        }
    }
    
    // Disable blending after drawing marbles
    glDisable(GL_BLEND);
    
    // Print debug info during the first frame
    static bool firstFrame = true;
    if (firstFrame) {
        printf("Drew %d marbles on first frame\n", marblesDrawn);
        firstFrame = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // Update the hover position
    getBoardCoordinates(xpos, ypos, hoverRow, hoverCol);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            
            int row, col;
            getBoardCoordinates(xpos, ypos, row, col);
            
            // Check if clicked on a valid board position
            if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
                if (!isMarbleSelected) {
                    // Select a marble
                    if (boardState[row][col] == FILLED) {
                        isMarbleSelected = true;
                        selectedRow = row;
                        selectedCol = col;
                    }
                } else {
                    // Try to move the selected marble
                    if (isValidMove(selectedRow, selectedCol, row, col)) {
                        makeMove(selectedRow, selectedCol, row, col);
                    }
                    
                    // Deselect the marble
                    isMarbleSelected = false;
                    selectedRow = -1;
                    selectedCol = -1;
                }
            }
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_R:
            // Reset the game
            initializeBoard();
            // Clear selection
            isMarbleSelected = false;
            selectedRow = -1;
            selectedCol = -1;
            // Reset notification flags
            showUndoLimitMsg = false;
            showRedoLimitMsg = false;
            // Refresh visuals
            CreateMarbleVertexBuffer();
            break;
        case GLFW_KEY_Z:
            // Undo move
            if (mods & GLFW_MOD_CONTROL) {
                undoMove();
            }
            break;
        case GLFW_KEY_Y:
            // Redo move
            if (mods & GLFW_MOD_CONTROL) {
                redoMove();
            }
            break;
        case GLFW_KEY_ESCAPE:
            // Cancel selection
            isMarbleSelected = false;
            selectedRow = -1;
            selectedCol = -1;
            break;
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, true);
            break;
        }
    }
}

// Initialize ImGui with input handling enabled
void InitImGui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    
    // Enable docking and configure ImGui style
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    
    // Enhance ImGui style
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 4.0f;
    style.WindowRounding = 6.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.16f, 0.21f, 0.8f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

// Render ImGui interface
void RenderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Set a larger font scale for the time and marble displays
    float originalFontScale = ImGui::GetFont()->Scale;
    float largeFontScale = 2.0f;
    
    // Display game stats directly on the game board in top corners
    // Create time display in top-left corner with improved styling
    time_t currentTime = time(NULL);
    int playTime = difftime(currentTime, gameStartTime);
    int minutes = playTime / 60;
    int seconds = playTime % 60;
    
    ImGui::GetFont()->Scale = largeFontScale;
    ImGui::PushFont(ImGui::GetFont());
    
    ImGui::SetNextWindowPos(ImVec2(40, 50));
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.7f); // Semi-transparent background
    ImGui::Begin("TimeDisplay", NULL, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    // Time display
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.65f, 0.0f, 1.0f));    // Orange (#FFA500)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));   // Semi-transparent Black
    ImGui::Text(" Time: %02d:%02d ", minutes, seconds);
    ImGui::PopStyleColor(2);
    ImGui::End();
    
    // Create marble count display in top-right corner with improved styling
    ImGui::SetNextWindowPos(ImVec2(theWindowWidth - 240, 50));
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.7f); // Semi-transparent background
    ImGui::Begin("MarbleDisplay", NULL, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    // Marble count display
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 1.0f, 0.0f, 1.0f));      // Lime Green (#BFFF00)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 0.7f));   // Semi-transparent Dark Gray
    ImGui::Text(" Marbles: %d ", remainingMarbles);
    ImGui::PopStyleColor(2);
    ImGui::End();
    
    ImGui::PopFont();
    ImGui::GetFont()->Scale = originalFontScale; // Restore original font scale
    
    // Game status (win/lose) displayed in center when applicable
    if (gameWon || gameLost ) {
        ImGui::SetNextWindowPos(ImVec2(theWindowWidth / 2 - 170, theWindowHeight / 2 - 50));
        ImGui::SetNextWindowSize(ImVec2(340, 100));
        ImGui::SetNextWindowBgAlpha(0.7f);
        ImGui::Begin("GameStatus", NULL, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        
        if (gameWon ) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "    Congratulations! \n\n        You won!");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "       Game over.\n\n Better luck next time!");
        }
        ImGui::End();
    }
    
    // Show text about remaining undos/redos in top-left of the screen
    ImGui::SetNextWindowPos(ImVec2(20, 120));
    ImGui::SetNextWindowSize(ImVec2(240, 65));
    ImGui::SetNextWindowBgAlpha(0.7f);
    ImGui::Begin("UndoRedoInfo", NULL, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoSavedSettings);
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    
    // Show text about undo/redo stack status
    ImGui::Text(" History Status\n   Undo: %d/%d  ",
        (int)moveHistory.size(), MAX_UNDO_MOVES
);
    
    ImGui::PopStyleColor();
    ImGui::End();
    
    // Show notification messages for undo/redo limits
    currentTime = glfwGetTime();
    
    if (showUndoLimitMsg && (currentTime - msgDisplayTime < MSG_DISPLAY_DURATION)) {
        ImGui::SetNextWindowPos(ImVec2(theWindowWidth / 2 - 170, 120));
        ImGui::SetNextWindowSize(ImVec2(340, 50));
        ImGui::SetNextWindowBgAlpha(0.7f);
        ImGui::Begin("UndoLimitMsg", NULL, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No more undos available!");
        ImGui::End();
    } else {
        showUndoLimitMsg = false;
    }
    
    if (showRedoLimitMsg && (currentTime - msgDisplayTime < MSG_DISPLAY_DURATION)) {
        ImGui::SetNextWindowPos(ImVec2(theWindowWidth / 2 - 170, 120));
        ImGui::SetNextWindowSize(ImVec2(340, 50));
        ImGui::SetNextWindowBgAlpha(0.7f);
        ImGui::Begin("RedoLimitMsg", NULL, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Maximum redo limit of %d moves reached!", MAX_UNDO_MOVES);
        ImGui::End();
    } else {
        showRedoLimitMsg = false;
    }
    
    // Keep keyboard controls in a separate window in bottom left
    ImGui::SetNextWindowPos(ImVec2(30, theWindowHeight - 230));
    ImGui::SetNextWindowSize(ImVec2(215, 160));
    ImGui::Begin("Controls", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    // Use smaller font for controls section
    float smallFontScale = 0.7f;
    ImGui::GetFont()->Scale = originalFontScale;
    ImGui::PushFont(ImGui::GetFont());
    
    // Keyboard shortcut information
    ImGui::Text("Keyboard Controls:");
    ImGui::Separator();
    ImGui::BulletText("R: Reset game");
    ImGui::BulletText("Ctrl+Z: Undo move (max %d)", MAX_UNDO_MOVES);
    ImGui::BulletText("Ctrl+Y: Redo move");
    ImGui::BulletText("ESC: Cancel selection");
    ImGui::BulletText("Q: Quit game");
    
    ImGui::PopFont();
    // Restore original font scale before ending the window
    ImGui::GetFont()->Scale = originalFontScale;
    ImGui::End();
    
    // Put game instructions in bottom right corner
    ImGui::SetNextWindowPos(ImVec2(theWindowWidth - 280, theWindowHeight - 210));
    ImGui::SetNextWindowSize(ImVec2(270, 100));
    ImGui::Begin("How to Play", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    // Use smaller font for instructions section
    ImGui::GetFont()->Scale = smallFontScale;
    
    ImGui::BulletText("Select: Click on marble");
    ImGui::BulletText("Move: Click empty cell");
    ImGui::BulletText("Win: Leave only one marble");

    // Restore original font scale before ending the window
    ImGui::GetFont()->Scale = originalFontScale;
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Define main function
int main(int argc, char *argv[]) {
    // Initialize GLFW
    glfwInit();
    
    // Define version and compatibility settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    // Create OpenGL window and context
    GLFWwindow *window = glfwCreateWindow(theWindowWidth, theWindowHeight, theProgramTitle, NULL, NULL);
    glfwMakeContextCurrent(window);
    
    // Check for window creation failure
    if (!window) {
        // Terminate GLFW
        glfwTerminate();
        return 0;
    }
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();
    printf("GL version: %s\n", glGetString(GL_VERSION));
    onInit(argc, argv);
    
    // Initialize ImGui
    InitImGui(window);
    
    // Set GLFW callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    // Check if shader files exist before running
    FILE *vs_file = fopen(pVSFileName, "r");
    if (!vs_file) {
        fprintf(stderr, "Error: Could not open vertex shader file '%s'\n", pVSFileName);
        fprintf(stderr, "Make sure the shaders directory exists and contains the shader files.\n");
        glfwTerminate();
        return 1;
    }
    fclose(vs_file);
    
    FILE *fs_file = fopen(pFSFileName, "r");
    if (!fs_file) {
        fprintf(stderr, "Error: Could not open fragment shader file '%s'\n", pFSFileName);
        fprintf(stderr, "Make sure the shaders directory exists and contains the shader files.\n");
        glfwTerminate();
        return 1;
    }
    fclose(fs_file);
    
    FILE *mvs_file = fopen(pMarbleVSFileName, "r");
    if (!mvs_file) {
        fprintf(stderr, "Error: Could not open marble vertex shader file '%s'\n", pMarbleVSFileName);
        fprintf(stderr, "Make sure the shaders directory exists and contains the shader files.\n");
        glfwTerminate();
        return 1;
    }
    fclose(mvs_file);
    
    FILE *mfs_file = fopen(pMarbleFSFileName, "r");
    if (!mfs_file) {
        fprintf(stderr, "Error: Could not open marble fragment shader file '%s'\n", pMarbleFSFileName);
        fprintf(stderr, "Make sure the shaders directory exists and contains the shader files.\n");
        glfwTerminate();
        return 1;
    }
    fclose(mfs_file);
    
    // Event loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        onDisplay();
        
        RenderImGui();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Terminate GLFW
    glfwTerminate();
    return 0;
}
