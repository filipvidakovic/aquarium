#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include "Util.h"

#define NUM_SLICES 40
#define MAX_BUBBLES 50
#define MAX_FOOD 10  // Maximum number of food items on screen

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

// Bubble
struct Bubble {
    float x, y;
    float speed;
    float size;
    float lifetime;
    bool active;
    int fishSource; // 0 = goldfish, 1 = clownfish
};

// Food structure
struct Food {
    float x, y;
    float speed;
    float size;
    bool active;
    bool falling;
    bool onGround;
    float spawnTime;
};

// Gold fish
float uX = 0.0f;
float uY = -0.5f;
float uS = 1.0f;
float uFlipX = -1.0f;
bool ufacingRight = false;

// Clown fish
float cX = 0.0f;
float cY = -0.5f;
float cS = 1.0f;
float cFlipX = 1.0f;
bool cfacingRight = true;

// Screen dimensions
int screenWidth = 1920;
int screenHeight = 1280;
unsigned goldfishTexture;
unsigned clownfishTexture;
unsigned bubblesTexture;
unsigned burgerTexture;
unsigned sandTexture;
unsigned grassTexture;
unsigned anchorTexture;
float grassHeight = 0.15f;

// Mouse cursor position
float cursorX = 0.0f;
float cursorY = 0.0f;

float aquariumLeft = -1.0f;
float aquariumRight = 1.0f;
float aquariumBottom = -1.0f;
float aquariumTop = 0.2f;
float sandHeight = 0.2f;

unsigned signatureTexture;
unsigned chestOpenTexture;
unsigned chestClosedTexture;
unsigned chestCurrentTexture;
bool chestIsOpen = false; // false = closed, true = open
float chestX = 0.0f;      // X position of chest (right side)
float chestY = aquariumBottom + sandHeight / 2; // On sand
float initialJumpTime = -1.0f;
std::vector<Bubble> bubbles;
std::vector<Food> foods;

const float MOVEMENT_SPEED = 0.0005f;
const float BUBBLE_COOLDOWN = 0.2f;
const float FOOD_FALL_SPEED = 0.001f;
const float FOOD_SIZE = 0.15f;
float lastBubbleTime = 0.0f;

// Shader programs
unsigned int rectShader;
unsigned int colorShader;

// Function prototypes
void spawnFood();
void updateFood(float deltaTime);
void drawFood(unsigned int shader, unsigned int VAO);
bool checkFishFoodCollision(float fishX, float fishY, float fishSize, const Food& food);
bool checkFoodGroundCollision(const Food& food);
void drawAnchorCursor(unsigned int shader, unsigned int VAO);

// Mouse position callback
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    // Convert pixel coordinates to normalized OpenGL coordinates
    cursorX = (xpos / screenWidth) * 2.0f - 1.0f;
    cursorY = -((ypos / screenHeight) * 2.0f - 1.0f); // Flip Y axis
}

void spawnBubble(float fishX, float fishY, int fishSource) {
    float currentTime = glfwGetTime();
    if (currentTime - lastBubbleTime < BUBBLE_COOLDOWN) {
        return;
    }

    if (fishY < (aquariumBottom + sandHeight) || fishY > aquariumTop) {
        return;
    }

    for (auto& bubble : bubbles) {
        if (!bubble.active) {
            bubble.x = fishX;
            bubble.y = fishY;
            bubble.speed = MOVEMENT_SPEED / 4;
            bubble.size = 0.05f + (rand() % 50) / 1000.0f;
            bubble.lifetime = 3.0f + (rand() % 100) / 100.0f;
            bubble.active = true;
            bubble.fishSource = fishSource;
            lastBubbleTime = currentTime;
            return;
        }
    }

    if (bubbles.size() < MAX_BUBBLES) {
        Bubble newBubble;
        newBubble.x = fishX;
        newBubble.y = fishY;
        newBubble.speed = MOVEMENT_SPEED / 4;
        newBubble.size = 0.05f + (rand() % 50) / 1000.0f;
        newBubble.lifetime = 3.0f + (rand() % 100) / 100.0f;
        newBubble.active = true;
        newBubble.fishSource = fishSource;
        bubbles.push_back(newBubble);
        lastBubbleTime = currentTime;
    }
}

void spawnFood() {
    // Find inactive food or create new one
    for (auto& food : foods) {
        if (!food.active) {
            food.x = ((rand() % 100) / 100.0f - 0.5f) * 1.8f; // Random x position within aquarium
            food.y = 1.0f; // Start from top of screen
            food.speed = FOOD_FALL_SPEED;
            food.size = FOOD_SIZE;
            food.active = true;
            food.falling = true;
            food.onGround = false;
            food.spawnTime = glfwGetTime();
            return;
        }
    }

    // If no inactive food and under max limit, create new one
    if (foods.size() < MAX_FOOD) {
        Food newFood;
        newFood.x = ((rand() % 100) / 100.0f - 0.5f) * 1.8f;
        newFood.y = 1.0f;
        newFood.speed = FOOD_FALL_SPEED;
        newFood.size = FOOD_SIZE;
        newFood.active = true;
        newFood.falling = true;
        newFood.onGround = false;
        newFood.spawnTime = glfwGetTime();
        foods.push_back(newFood);
    }
}

const float GROWTH_PER_FOOD = 0.01f;

void updateFood(float deltaTime) {
    for (auto& food : foods) {
        if (food.active) {
            if (food.falling) {
                food.y -= food.speed;

                if (checkFoodGroundCollision(food)) {
                    food.falling = false;
                    food.onGround = true;
                    food.y = aquariumBottom + sandHeight - 0.07f;
                }

                if (checkFishFoodCollision(uX, uY, uS * 0.2f, food)) {
                    food.active = false;
                    uS += GROWTH_PER_FOOD;
                }
                else if (checkFishFoodCollision(cX, cY, cS * 0.2f, food)) {
                    food.active = false;
                    cS += GROWTH_PER_FOOD;
                }
            }

            if (food.onGround) {
                if (checkFishFoodCollision(uX, uY, uS * 0.2f, food)) {
                    food.active = false;
                    uS += GROWTH_PER_FOOD;
                }
                else if (checkFishFoodCollision(cX, cY, cS * 0.2f, food)) {
                    food.active = false;
                    cS += GROWTH_PER_FOOD;
                }

                if (glfwGetTime() - food.spawnTime > 5.0f) {
                    food.active = false;
                }
            }
        }
    }
}

bool checkFishFoodCollision(float fishX, float fishY, float fishSize, const Food& food) {
    float distanceX = fishX - food.x;
    float distanceY = fishY - food.y;
    float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

    return distance < (fishSize + food.size / 2);
}

bool checkFoodGroundCollision(const Food& food) {
    return food.y <= (aquariumBottom + sandHeight - 0.07f);
}

void updateBubbles(float deltaTime) {
    for (auto& bubble : bubbles) {
        if (bubble.active) {
            bubble.y += bubble.speed;
            bubble.x += (sin(glfwGetTime() * 2.0f + bubble.x * 10.0f) * 0.0001f);
            bubble.lifetime -= deltaTime;

            if (bubble.lifetime <= 0.0f || bubble.y > aquariumTop) {
                bubble.active = false;
            }
        }
    }
}

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void formVAOs(float* verticesRect, size_t rectSize, unsigned int& VAOrect) {
    unsigned int VBOrect;
    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBOrect);

    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
    glBufferData(GL_ARRAY_BUFFER, rectSize, verticesRect, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void formSimpleRectVAO(float* vertices, size_t size, unsigned int& VAO) {
    unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void drawRect(unsigned int shader, unsigned int VAOrect,
    float x, float y, float scale, float flip,
    unsigned int texture, bool useTexture = true) {
    glUseProgram(shader);

    if (useTexture) {
        glUniform1f(glGetUniformLocation(shader, "uX"), x);
        glUniform1f(glGetUniformLocation(shader, "uY"), y);
        glUniform1f(glGetUniformLocation(shader, "uS"), scale);
        glUniform1f(glGetUniformLocation(shader, "uFlipX"), flip);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    else {
        glUniform1f(glGetUniformLocation(shader, "uX"), x);
        glUniform1f(glGetUniformLocation(shader, "uY"), y);
        glUniform1f(glGetUniformLocation(shader, "uS"), scale);
    }

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawFood(unsigned int shader, unsigned int VAO, const Food& food) {
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader, "uX"), food.x);
    glUniform1f(glGetUniformLocation(shader, "uY"), food.y);
    glUniform1f(glGetUniformLocation(shader, "uS"), 1.0f);
    glUniform1f(glGetUniformLocation(shader, "uFlipX"), 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, burgerTexture);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawSand(unsigned int shader, unsigned int VAOrect) {
    glUseProgram(shader);

    float sandY = aquariumBottom + sandHeight / 2;

    glUniform1f(glGetUniformLocation(shader, "uX"), 0.0f);
    glUniform1f(glGetUniformLocation(shader, "uY"), sandY);
    glUniform1f(glGetUniformLocation(shader, "uS"), 1.0f);
    glUniform1f(glGetUniformLocation(shader, "uFlipX"), 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sandTexture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawAquarium() {
    glUseProgram(colorShader);

    // Water area (from sand top to aquarium top)
    float waterBottom = aquariumBottom + sandHeight;
    glUniform4f(glGetUniformLocation(colorShader, "uColor"), 0.5f, 0.6f, 1.0f, 1.0f);

    float waterVertices[] = {
        aquariumLeft, aquariumTop,
        aquariumLeft, waterBottom,
        aquariumRight, waterBottom,
        aquariumRight, aquariumTop
    };

    unsigned int VAO_water;
    formSimpleRectVAO(waterVertices, sizeof(waterVertices), VAO_water);

    float waterCenterY = (aquariumTop + waterBottom) / 2.0f;
    glUniform1f(glGetUniformLocation(colorShader, "uX"), 0.0f);
    glUniform1f(glGetUniformLocation(colorShader, "uY"), waterCenterY);
    glUniform1f(glGetUniformLocation(colorShader, "uS"), 1.0f);

    glBindVertexArray(VAO_water);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteVertexArrays(1, &VAO_water);

    // Transparent glass overlay
    glUniform4f(glGetUniformLocation(colorShader, "uColor"), 1.0f, 1.0f, 1.0f, 0.2f);

    float glassVertices[] = {
        aquariumLeft, aquariumTop,
        aquariumLeft, waterBottom,
        aquariumRight, waterBottom,
        aquariumRight, aquariumTop
    };

    unsigned int VAO_glass;
    formSimpleRectVAO(glassVertices, sizeof(glassVertices), VAO_glass);

    glUniform1f(glGetUniformLocation(colorShader, "uX"), 0.0f);
    glUniform1f(glGetUniformLocation(colorShader, "uY"), waterCenterY);
    glUniform1f(glGetUniformLocation(colorShader, "uS"), 1.0f);

    glBindVertexArray(VAO_glass);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteVertexArrays(1, &VAO_glass);

    // Black borders
    glUniform4f(glGetUniformLocation(colorShader, "uColor"), 0.0f, 0.0f, 0.0f, 1.0f);
    float borderThickness = 10.0f / screenHeight * 2.0f;

    // Bottom border
    float bottomBorderVertices[] = {
        aquariumLeft, aquariumBottom + borderThickness,
        aquariumLeft, aquariumBottom,
        aquariumRight, aquariumBottom,
        aquariumRight, aquariumBottom + borderThickness
    };

    unsigned int VAO_bottom;
    formSimpleRectVAO(bottomBorderVertices, sizeof(bottomBorderVertices), VAO_bottom);

    glUniform1f(glGetUniformLocation(colorShader, "uX"), 0.0f);
    glUniform1f(glGetUniformLocation(colorShader, "uY"), aquariumBottom + borderThickness / 2);
    glUniform1f(glGetUniformLocation(colorShader, "uS"), 1.0f);

    glBindVertexArray(VAO_bottom);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteVertexArrays(1, &VAO_bottom);

    // Left border
    float leftBorderVertices[] = {
        aquariumLeft + borderThickness, aquariumBottom,
        aquariumLeft, aquariumBottom,
        aquariumLeft, aquariumTop,
        aquariumLeft + borderThickness, aquariumTop
    };

    unsigned int VAO_left;
    formSimpleRectVAO(leftBorderVertices, sizeof(leftBorderVertices), VAO_left);

    glUniform1f(glGetUniformLocation(colorShader, "uX"), aquariumLeft + borderThickness / 2);
    glUniform1f(glGetUniformLocation(colorShader, "uY"), (aquariumTop + aquariumBottom) / 2);
    glUniform1f(glGetUniformLocation(colorShader, "uS"), 1.0f);

    glBindVertexArray(VAO_left);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteVertexArrays(1, &VAO_left);

    // Right border
    float rightBorderVertices[] = {
        aquariumRight, aquariumBottom,
        aquariumRight - borderThickness, aquariumBottom,
        aquariumRight - borderThickness, aquariumTop,
        aquariumRight, aquariumTop
    };

    unsigned int VAO_right;
    formSimpleRectVAO(rightBorderVertices, sizeof(rightBorderVertices), VAO_right);

    glUniform1f(glGetUniformLocation(colorShader, "uX"), aquariumRight - borderThickness / 2);
    glUniform1f(glGetUniformLocation(colorShader, "uY"), (aquariumTop + aquariumBottom) / 2);
    glUniform1f(glGetUniformLocation(colorShader, "uS"), 1.0f);

    glBindVertexArray(VAO_right);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteVertexArrays(1, &VAO_right);
}

void drawGrass(unsigned int shader, unsigned int VAO_grass) {
    glUseProgram(shader);

    float grass1X = -0.6f;
    float grass1Y = aquariumBottom + sandHeight / 2 + 0.1f;

    glUniform1f(glGetUniformLocation(shader, "uX"), grass1X);
    glUniform1f(glGetUniformLocation(shader, "uY"), grass1Y);
    glUniform1f(glGetUniformLocation(shader, "uS"), 1.0f);
    glUniform1f(glGetUniformLocation(shader, "uFlipX"), 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    glBindVertexArray(VAO_grass);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    float grass2X = 0.6f;
    float grass2Y = aquariumBottom + sandHeight / 2 + 0.1f;

    glUniform1f(glGetUniformLocation(shader, "uX"), grass2X);
    glUniform1f(glGetUniformLocation(shader, "uY"), grass2Y);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawBubble(unsigned int rectShader, unsigned int VAOrect, const Bubble& bubble) {
    glUseProgram(rectShader);

    glUniform1f(glGetUniformLocation(rectShader, "uX"), bubble.x);
    glUniform1f(glGetUniformLocation(rectShader, "uY"), bubble.y);
    glUniform1f(glGetUniformLocation(rectShader, "uS"), 1.0f);
    glUniform1f(glGetUniformLocation(rectShader, "uFlipX"), 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bubblesTexture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawChest(unsigned int shader, unsigned int VAOrect) {
    glUseProgram(shader);

    glUniform1f(glGetUniformLocation(shader, "uX"), chestX);
    glUniform1f(glGetUniformLocation(shader, "uY"), chestY);
    glUniform1f(glGetUniformLocation(shader, "uS"), 1.0f);
    glUniform1f(glGetUniformLocation(shader, "uFlipX"), 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, chestCurrentTexture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawSignatureRectangle(unsigned int colorShader, unsigned int VAOrect) {
    glUseProgram(colorShader);

    float nameX = -0.9f;
    float nameY = 0.8f;
    float nameWidth = 0.3f;
    float nameHeight = 0.1f;

    // Light blue rectangle with text
    glUniform4f(glGetUniformLocation(colorShader, "uColor"), 0.7f, 0.8f, 1.0f, 0.8f);
    glUniform1f(glGetUniformLocation(colorShader, "uX"), nameX);
    glUniform1f(glGetUniformLocation(colorShader, "uY"), nameY);
    glUniform1f(glGetUniformLocation(colorShader, "uScaleX"), nameWidth);
    glUniform1f(glGetUniformLocation(colorShader, "uScaleY"), nameHeight);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, signatureTexture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


void squish_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE && initialJumpTime < 0) {
            initialJumpTime = glfwGetTime();
        }

        if (key == GLFW_KEY_Z) {
            spawnBubble(uX, uY, 0);
        }

        if (key == GLFW_KEY_K) {
            spawnBubble(cX, cY, 1);
        }

        if (key == GLFW_KEY_ENTER) {
            spawnFood();
        }
        if (key == GLFW_KEY_C) {
            chestIsOpen = !chestIsOpen;
            chestCurrentTexture = chestIsOpen ? chestOpenTexture : chestClosedTexture;
        }
        if (key == GLFW_KEY_ESCAPE) {
            endProgram("Exited on Escape");
            exit(0);
        }
    }
}

void processMovement(GLFWwindow* window) {
    // Goldfish movement
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        uX -= MOVEMENT_SPEED;
        if (!ufacingRight) {
            uFlipX = 1.0f;
            ufacingRight = true;
        }
        uX = std::max(uX, aquariumLeft + 0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        uX += MOVEMENT_SPEED;
        if (ufacingRight) {
            uFlipX = -1.0f;
            ufacingRight = false;
        }
        uX = std::min(uX, aquariumRight - 0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        uY -= MOVEMENT_SPEED;
        uY = std::max(uY, aquariumBottom + sandHeight - 0.05f);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        uY += MOVEMENT_SPEED;
        uY = std::min(uY, aquariumTop - 0.1f);
    }

    // Clownfish movement
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cX -= MOVEMENT_SPEED;
        if (!cfacingRight) {
            cFlipX = 1.0f;
            cfacingRight = true;
        }
        cX = std::max(cX, aquariumLeft + 0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cX += MOVEMENT_SPEED;
        if (cfacingRight) {
            cFlipX = -1.0f;
            cfacingRight = false;
        }
        cX = std::min(cX, aquariumRight - 0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cY -= MOVEMENT_SPEED;
        cY = std::max(cY, aquariumBottom + sandHeight - 0.05f);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cY += MOVEMENT_SPEED;
        cY = std::min(cY, aquariumTop - 0.1f);
    }
}

GLFWcursor* cursor;

int main()
{
    srand(static_cast<unsigned>(time(0)));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Aquarium with Food", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, squish_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    cursor = loadImageToCursor("res/anchor.png");
    glfwSetCursor(window, cursor);

    // Hide default cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load all textures
    preprocessTexture(goldfishTexture, "res/4-gold-fish-png-image.png");
    preprocessTexture(clownfishTexture, "res/clown-fish.png");
    preprocessTexture(bubblesTexture, "res/bubbles.png");
    preprocessTexture(burgerTexture, "res/burger.png");
    preprocessTexture(sandTexture, "res/sand.png");
    preprocessTexture(grassTexture, "res/grass.png");
    preprocessTexture(chestOpenTexture, "res/chest-open.png");
    preprocessTexture(chestClosedTexture, "res/chest-closed.png");
    preprocessTexture(signatureTexture, "res/signature-cyrillic.png");
    chestCurrentTexture = chestClosedTexture;

    // Create shaders
    rectShader = createShader("rect.vert", "rect.frag");
    colorShader = createShader("color.vert", "color.frag");

    glUseProgram(rectShader);
    glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

    glUseProgram(colorShader);

    // Vertex data for fish and food (same shape)
    float verticesRect[] = {
         -0.1f, 0.1f, 0.0f, 1.0f,
         -0.1f, -0.1f, 0.0f, 0.0f,
         0.1f, -0.1f, 1.0f, 0.0f,
         0.1f, 0.1f, 1.0f, 1.0f,
    };
    float verticesSand[] = {
     -1.0f, sandHeight, 0.0f, 1.0f,
     -1.0f, -sandHeight, 0.0f, 0.0f,
      1.0f, -sandHeight, 1.0f, 0.0f,
      1.0f, sandHeight, 1.0f, 1.0f
    };
    float verticesGrass[] = {
     -0.05f, 0.15f, 0.0f, 1.0f,
     -0.05f, -0.15f, 0.0f, 0.0f,
      0.05f, -0.15f, 1.0f, 0.0f,
      0.05f, 0.15f, 1.0f, 1.0f
    };

    // Create VAOs
    unsigned int VAO_fish;
    unsigned int VAO_grass;
    unsigned int VAO_bubbles;
    unsigned int VAO_food;
    unsigned int VAO_sand;
    formVAOs(verticesRect, sizeof(verticesRect), VAO_fish);
    formVAOs(verticesRect, sizeof(verticesRect), VAO_bubbles);
    formVAOs(verticesRect, sizeof(verticesRect), VAO_food);
    formVAOs(verticesSand, sizeof(verticesSand), VAO_sand);
    formVAOs(verticesGrass, sizeof(verticesGrass), VAO_grass);

    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processMovement(window);
        updateBubbles(deltaTime);
        updateFood(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);

        drawAquarium();

        drawSand(rectShader, VAO_sand);

        drawGrass(rectShader, VAO_grass);

        drawChest(rectShader, VAO_fish);

        drawSignatureRectangle(rectShader, VAO_fish);

        for (const auto& food : foods) {
            if (food.active) {
                drawFood(rectShader, VAO_food, food);
            }
        }

        drawRect(rectShader, VAO_fish, uX, uY, uS, uFlipX, goldfishTexture);
        drawRect(rectShader, VAO_fish, cX, cY, cS, cFlipX, clownfishTexture);

        for (const auto& bubble : bubbles) {
            if (bubble.active) {
                drawBubble(rectShader, VAO_bubbles, bubble);
            }
        }

        // Draw anchor cursor (LAST so it's on top of everything)

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}