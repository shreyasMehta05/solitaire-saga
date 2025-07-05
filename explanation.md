# Code Explanation: Marble Solitaire Implementation

This document explains the key components of the Marble Solitaire game implementation, focusing on the main.cpp file and the shader files that create the visual elements.

## Table of Contents
1. [Main Components of main.cpp](#main-components-of-maincpp)
2. [Detailed Sections of main.cpp](#detailed-sections-of-maincpp)
3. [Shader Implementation](#shader-implementation)
   - [Marble Vertex Shader](#marble-vertex-shader)
   - [Marble Fragment Shader](#marble-fragment-shader)
4. [Game Logic](#game-logic)
5. [UI and User Interaction](#ui-and-user-interaction)

## Main Components of main.cpp

### 1. Global Variables and Constants
```cpp
const int BOARD_SIZE = 7;
char theProgramTitle[] = "Marble Solitaire";
// Added game state variables (board state, selected marble, etc.)
// Added UI state variables
// Added undo/redo stack variables
```

These variables define the game configuration, track the game state, and manage OpenGL resources.

### 2. Vertex Buffer Creation
```cpp
void CreateBoardVertexBuffer() {
    // Creates a checkerboard pattern of squares
}

void CreateMarbleVertexBuffer() {
    // Creates circle geometry for each marble position
}
```

These functions create the geometric representation of the game board and marbles.

### 3. Shader Compilation
```cpp
static void CompileShaders() {
    // Compiles two shader programs:
    // 1. Board shaders for rendering the game board
    // 2. Marble shaders for rendering the marbles with lighting effects
}
```

The final implementation uses two separate shader programs to render different types of objects with different visual properties.

### 4. Rendering (onDisplay)
```cpp
static void onDisplay() {
    // Renders the board squares with checkerboard pattern
    // Renders each marble with proper position and highlighting
    // Uses depth testing and blending for better visual quality
}
```

The rendering was expanded to handle multiple types of objects, depth sorting, and transparency.

### 5. Event Handling
```cpp
void key_callback(...) {
    // Handles game controls (reset, undo/redo, selection cancellation)
}

void mouse_callback(...) {
    // Tracks mouse position for hover effects
}

void mouse_button_callback(...) {
    // Handles marble selection and move execution
}
```

The final implementation includes comprehensive input handling for all game interactions.

### 6. ImGui Integration
```cpp
void RenderImGui() {
    // Creates multiple UI windows:
    // - Game stats (time, marbles remaining)
    // - Control information
    // - Game instructions
    // - Undo/redo status
    // - Win/loss notifications
}
```

The ImGui implementation was greatly expanded to provide a rich user interface.

## Detailed Sections of main.cpp

The main.cpp file is organized into several distinct sections, each handling specific aspects of the game:

### 1. Includes and Definitions (Lines 1-30)
```cpp
#include <stdio.h>
#include <iostream>
// Additional includes...
#define GL_SILENCE_DEPRECATION
```
This section imports necessary libraries and defines constants:
- Standard C/C++ libraries for I/O and string handling
- OpenGL libraries (GLEW, GLFW) for graphics capabilities
- ImGui libraries for the user interface
- Math utilities for vector and matrix operations
- Custom utility files like file_utils.h for file operations

### 2. Global Variables and Constants (Lines 31-70)
```cpp
const int BOARD_SIZE = 7;
char theProgramTitle[] = "Marble Solitaire";
// Game state variables...
```
This section defines:
- Game constants (board size, window dimensions)
- OpenGL-related variables (VAOs, VBOs, shader program IDs)
- Game state variables (board state, selected marbles)
- Undo/redo system variables and limits

### 3. Game State Management (Lines 71-90)
```cpp
enum CellState { EMPTY = 0, FILLED = 1, INVALID = 2 };
CellState boardState[BOARD_SIZE][BOARD_SIZE];
// Additional state variables...
```
This section defines data structures for:
- The game board representation (a 2D array of cell states)
- Marble selection tracking
- Move history for undo/redo functionality
- Game progress tracking (remaining marbles, win/loss state)

### 4. Game Logic Functions (Lines 95-280)
```cpp
void initializeBoard() {
    // Reset counters and set up the initial board state
}

bool checkGameOver() {
    // Check if any valid moves remain
}

bool isValidMove(int startRow, int startCol, int endRow, int endCol) {
    // Validate move according to game rules
}

void makeMove(int startRow, int startCol, int endRow, int endCol) {
    // Execute a move and update game state
}

void undoMove() {
    // Revert the last move
}

void redoMove() {
    // Reapply a previously undone move
}
```
These functions implement the core game mechanics:
- Board initialization with the standard Marble Solitaire layout
- Move validation based on game rules
- Move execution including capturing marbles
- Game state updates and win/loss detection
- Undo/redo functionality with limits

### 5. Coordinate System Functions (Lines 281-320)
```cpp
void getBoardPixelCoordinates(int row, int col, float &x, float &y) {
    // Convert board coordinates to screen coordinates
}

void getBoardCoordinates(double mouseX, double mouseY, int &row, int &col) {
    // Convert mouse coordinates to board coordinates
}
```
These functions handle coordinate system conversions between:
- Board grid coordinates (row, column)
- OpenGL normalized device coordinates (-1 to 1)
- Screen pixel coordinates

### 6. OpenGL Rendering Functions (Lines 321-450)
```cpp
void CreateBoardVertexBuffer() {
    // Create geometry for the game board
}

void CreateMarbleVertexBuffer() {
    // Create geometry for the marbles
}

void AddShader(GLuint ShaderProgram, const char *pShaderText, GLenum ShaderType) {
    // Add and compile a shader to a program
}

static void CompileShaders() {
    // Compile shader programs for board and marbles
}
```
These functions handle the OpenGL rendering setup:
- Creating vertex buffers for board cells and marbles
- Creating and binding VAOs
- Compiling shaders for both the board and marbles
- Setting up OpenGL state for proper rendering

### 7. Callback Functions (Lines 451-600)
```cpp
void onInit(int argc, char *argv[]) {
    // Initialize OpenGL state and game components
}

static void onDisplay() {
    // Render the scene each frame
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // Handle mouse movement
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // Handle mouse clicks for marble selection and moves
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Handle keyboard input for game controls
}
```
These functions respond to user input and system events:
- Game initialization
- Scene rendering on each frame
- Mouse movement for hover effects
- Mouse clicks for marble selection and moves
- Keyboard controls for undo/redo, reset, and exit

### 8. ImGui Integration (Lines 601-800)
```cpp
void InitImGui(GLFWwindow *window) {
    // Initialize ImGui context and style
}

void RenderImGui() {
    // Render ImGui interface elements
}
```
These functions handle the user interface:
- Setting up ImGui for rendering
- Creating UI windows for game information
- Displaying game status (time, marbles, etc.)
- Showing notifications for undo/redo limits
- Presenting game controls and instructions

### 9. Main Function (Lines 801-885)
```cpp
int main(int argc, char *argv[]) {
    // Initialize GLFW and create window
    // Set up OpenGL context
    // Initialize game components
    // Main event loop
    // Cleanup and exit
}
```
The main function orchestrates the entire program:
- Initializes GLFW and creates the game window
- Sets up OpenGL context and capabilities
- Initializes game state and ImGui
- Registers callback functions
- Runs the main event loop
- Handles cleanup on exit

## Shader Implementation

The game uses two sets of shaders: one for the board and one for the marbles. The marble shaders are particularly important for creating visually appealing game pieces.

### Marble Vertex Shader

The vertex shader (`marble_shader.vs`) is responsible for transforming the marble vertices and preparing data for the fragment shader:

```glsl
#version 330

layout (location = 0) in vec3 Position;
uniform mat4 gWorld;
out vec3 FragPos;
out vec3 Normal;

void main() {
    // Transform the vertex position
    gl_Position = gWorld * vec4(Position, 1.0);
    
    // Just pass the local position (relative to marble center)
    // for lighting calculations in the fragment shader
    FragPos = Position;
    
    // Calculate normal for lighting (assuming sphere)
    // For a sphere, the normal is just the normalized position from center
    Normal = normalize(Position);
}
```

This shader:
1. **Takes vertex positions as input**: The `Position` attribute contains the 3D coordinates of each vertex relative to the marble's center.

2. **Applies transformations**: The `gWorld` uniform matrix contains the combined world transformation that positions each marble correctly on the board.

3. **Outputs position data**: The transformed position becomes the vertex's final screen position (`gl_Position`).

4. **Prepares lighting calculations**: The shader outputs the local position and normal vector for lighting calculations in the fragment shader.

5. **Calculates normals automatically**: Since marbles are approximated as spheres, the normal at each point is simply the normalized vector from the center to that point.

### Marble Fragment Shader

**Note**: I implemented lighting and shading as it was bieng taught in the class and out of interest I implemented this.

The fragment shader (`marble_shader.fs`) creates the final appearance of each marble with advanced lighting and visual effects:

```glsl
#version 330

uniform int gSelected;  // Selection state (0=normal, 1=selected, 2=hover)
uniform float gPulse;   // Pulse value for highlighting

in vec3 FragPos;        // Position from vertex shader
in vec3 Normal;         // Normal from vertex shader

out vec4 FragColor;     // Final output color

void main() {
    // Color palette based on suggestions
    vec3 marbleColor = vec3(0.12, 0.12, 0.12);         // Dark gray/black default marble
    vec3 selectedColor = vec3(0.0, 0.78, 0.33);        // Vibrant green for selected
    vec3 hoverColor = vec3(0.16, 0.71, 0.96);          // Light blue for hover
    vec3 validMoveColor = vec3(1.0, 0.84, 0.0);        // Bright yellow for valid move
    
    // Choose color based on selection state
    vec3 baseColor;
    if (gSelected == 1) {
        // Selected marble - add pulsing effect
        float pulseIntensity = sin(gPulse) * 0.25 + 0.1;
        baseColor = mix(selectedColor, vec3(1.0), pulseIntensity);
    } else if (gSelected == 2) {
        baseColor = hoverColor;
    } else if (gSelected == 3) {
        // Valid move target - add pulsing effect
        float pulseIntensity = sin(gPulse) * 0.25 + 0.1;
        baseColor = mix(validMoveColor, vec3(1.0), pulseIntensity);
    } else {
        baseColor = marbleColor;
    }
    
    // Calculate lighting
    // Distance from center for simple sphere mapping
    float distFromCenter = length(FragPos.xy);
    
    // Simple sphere normal calculation
    vec3 normal = normalize(Normal);
    
    // Lighting direction (from upper right)
    vec3 lightDir = normalize(vec3(0.5, 0.5, 0.7));
    
    // Calculate diffuse lighting with warmer tint
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 warmDiffuse = vec3(1.0, 0.95, 0.9); // Slightly warm diffuse light
    vec3 diffuse = diff * warmDiffuse;
    
    // Calculate specular highlights with golden tint
    vec3 viewDir = vec3(0.0, 0.0, 1.0);  // Viewing from straight ahead
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);  // Sharper highlight
    vec3 specular = spec * vec3(0.95, 0.77, 0.06) * 0.9;  // Golden specular
    
    // Soft ambient light
    vec3 ambient = vec3(0.38, 0.38, 0.38); // Neutral gray
    
    // Add radial gradient effect to marble
    float gradient = 0.85 + 0.15 * (0.5 - distFromCenter);
    baseColor *= gradient;
    
    // Combine lighting with base color
    vec3 finalColor = baseColor * (ambient + diffuse) + specular;
    
    // Add gentle rim lighting
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = smoothstep(0.5, 0.85, rim);
    
    // Enhance selected marbles with stronger effects
    float alpha = 1.0;
    if (gSelected > 0) {
        // Add extra glow and rim lighting for selected marbles
        finalColor += baseColor * 0.2;
        finalColor += rim * 0.4 * baseColor;
        
        // Add a gentle outline glow
        float edge = smoothstep(0.7, 0.95, distFromCenter);
        finalColor = mix(finalColor, vec3(1.0) * baseColor, edge * 0.45);
    } else {
        // Add subtle rim lighting for regular marbles
        finalColor += rim * 0.15 * baseColor;
    }
    
    // Add a soft shadow at the bottom for depth
    float shadowFactor = max(0.0, FragPos.y * 0.5 + 0.2);
    finalColor *= shadowFactor;
    
    // Output with full opacity
    FragColor = vec4(finalColor, alpha);
}
```

This fragment shader implements sophisticated visual effects:

1. **Dynamic Color Selection**: 
   - Uses different colors based on the marble state (normal, selected, hovered)
   - Implements pulsing effects for selected marbles using the `gPulse` uniform

2. **Advanced Lighting Model**:
   - **Ambient Lighting**: Provides a base level of illumination
   - **Diffuse Lighting**: Creates shading based on the angle to the light source
   - **Specular Highlights**: Adds realistic reflective highlights
   - **Rim Lighting**: Creates a glowing edge effect around the marble

3. **Visual Enhancements**:
   - **Radial Gradient**: Makes marbles appear more spherical
   - **Edge Highlighting**: Emphasizes the outline of selected marbles
   - **Bottom Shadow**: Creates a subtle shadow effect for depth
   - **Color Tinting**: Uses warm tones for diffuse lighting and gold for specular reflections

4. **Selection Feedback**:
   - Selected marbles glow green with a pulsing effect
   - Hovered marbles are highlighted in blue
   - Valid move targets are indicated in yellow

These advanced shader techniques transform simple circular geometry into realistic-looking marbles with proper lighting and visual feedback, significantly enhancing the game's visual appeal.

## Game Logic

### Board State Management

```cpp
void initializeBoard() {
    // Creates initial board layout with:
    // - Invalid regions in corners
    // - Filled cells with marbles
    // - Empty center cell
}
```

The board is represented as a 2D array with three possible states for each cell:
- `EMPTY`: No marble present
- `FILLED`: Marble present
- `INVALID`: Outside the playable area

### Move Validation and Execution

```cpp
bool isValidMove(int startRow, int startCol, int endRow, int endCol) {
    // Checks if a move is valid according to game rules
}

void makeMove(int startRow, int startCol, int endRow, int endCol) {
    // Updates board state
    // Records move in history
    // Manages undo stack
    // Updates marble count
    // Checks for game over
}
```

The move system implements the rules of Marble Solitaire:
- Marbles can jump over adjacent marbles
- The jumped-over marble is removed
- Moves must end in an empty space

### Undo/Redo System

```cpp
void undoMove() {
    // Reverts the last move
    // Updates the redo stack
    // Enforces the undo limit
}

void redoMove() {
    // Reapplies a previously undone move
    // Updates the undo stack
    // Enforces the redo limit
}
```

The undo/redo system:
- Maintains two stacks for move history
- Limits each stack to 3 moves
- Preserves the oldest moves when new ones are added
- Clears the redo stack when a new move is made

## UI and User Interaction

### Selection System

```cpp
void mouse_button_callback(...) {
    // First click: Select a marble
    // Second click: Attempt to move selected marble to target
}
```

The selection system:
- Allows picking up marbles with a click
- Shows visual feedback for selected marbles
- Validates and executes moves on the second click

### ImGui Windows

```cpp
void RenderImGui() {
    // Renders multiple UI windows
}
```

The UI displays:
- Game time
- Remaining marbles
- Undo/redo status
- Game controls
- Instructions
- Win/loss notifications

### Visual Feedback

The game provides rich visual feedback:
- Selected marbles are highlighted
- Hovered marbles are highlighted differently
- Notification messages appear when undo/redo limits are reached
- Win/loss status is prominently displayed

## Conclusion

The Marble Solitaire implementation demonstrates how modern OpenGL techniques can be used to create an engaging game experience. The combination of structured C++ code for game logic, advanced GLSL shaders for visuals, and ImGui for user interface creates a cohesive and polished application.

Key highlights of the implementation include:
1. Efficient geometry creation for board and marbles
2. Sophisticated shader effects for visual appeal
3. Comprehensive game logic with undo/redo support
4. Intuitive user interface with real-time feedback
5. Modular code organization that separates concerns

The shader implementation, in particular, shows how relatively simple GLSL code can create visually complex and appealing game elements through the careful application of modern graphics techniques.
