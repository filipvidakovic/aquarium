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

// Gold fish
float uX = 0.0f;
float uY = -0.5f;
float uS = 0.5f;
float uFlipX = -1.0f;
bool ufacingRight = false;

// Clown fish
float cX = 0.0f;
float cY = -0.5f;
float cS = 0.5f;
float cFlipX = 1.0f;
bool cfacingRight = true;

// Screen dimensions
int screenWidth = 800;
int screenHeight = 800;
unsigned goldfishTexture;
unsigned clownfishTexture;
unsigned bubblesTexture;

float initialJumpTime = -1.0f;
std::vector<Bubble> bubbles;

// Movement speeds
const float MOVEMENT_SPEED = 0.001f;
const float BUBBLE_COOLDOWN = 0.2f; // 200ms cooldown between bubbles
float lastBubbleTime = 0.0f;

void spawnBubble(float fishX, float fishY, int fishSource) {
    float currentTime = glfwGetTime();
    if (currentTime - lastBubbleTime < BUBBLE_COOLDOWN) {
        return; // Still in cooldown
    }

    // Find inactive bubble or create new one
    for (auto& bubble : bubbles) {
        if (!bubble.active) {
            bubble.x = fishX;
            bubble.y = fishY;
            bubble.speed = MOVEMENT_SPEED/4;
            bubble.size = 0.05f + (rand() % 50) / 1000.0f;
            bubble.lifetime = 3.0f + (rand() % 100) / 100.0f;
            bubble.active = true;
            bubble.fishSource = fishSource;
            lastBubbleTime = currentTime;
            return;
        }
    }

    // If no inactive bubbles and under max limit, create new one
    if (bubbles.size() < MAX_BUBBLES) {
        Bubble newBubble;
        newBubble.x = fishX;
        newBubble.y = fishY;
        newBubble.speed = MOVEMENT_SPEED/4;
        newBubble.size = 0.05f + (rand() % 50) / 1000.0f;
        newBubble.lifetime = 3.0f + (rand() % 100) / 100.0f;
        newBubble.active = true;
        newBubble.fishSource = fishSource;
        bubbles.push_back(newBubble);
        lastBubbleTime = currentTime;
    }
}

void updateBubbles(float deltaTime) {
    for (auto& bubble : bubbles) {
        if (bubble.active) {
            // Move bubble upward
            bubble.y += bubble.speed;

            // Generate angle
            bubble.x += (sin(glfwGetTime() * 2.0f + bubble.x * 10.0f) * 0.0001f);

            // Reduce lifetime
            bubble.lifetime -= deltaTime;

            // Deactivate if lifetime expired or off screen
            if (bubble.lifetime <= 0.0f || bubble.y > 1.2f) {
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

void drawRect(unsigned int rectShader, unsigned int VAOrect,
    float x, float y, float scale, float flip,
    unsigned int texture) {
    glUseProgram(rectShader);

    glUniform1f(glGetUniformLocation(rectShader, "uX"), x);
    glUniform1f(glGetUniformLocation(rectShader, "uY"), y);
    glUniform1f(glGetUniformLocation(rectShader, "uS"), scale);
    glUniform1f(glGetUniformLocation(rectShader, "uFlipX"), flip);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawBubble(unsigned int rectShader, unsigned int VAOrect, const Bubble& bubble) {
    glUseProgram(rectShader);

    glUniform1f(glGetUniformLocation(rectShader, "uX"), bubble.x);
    glUniform1f(glGetUniformLocation(rectShader, "uY"), bubble.y);
    glUniform1f(glGetUniformLocation(rectShader, "uS"), bubble.size);
    glUniform1f(glGetUniformLocation(rectShader, "uFlipX"), 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bubblesTexture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void squish_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Handle one-time press events only (not movement)
    if (action == GLFW_PRESS) {
        // Q key - squish goldfish
        if (key == GLFW_KEY_Q) {
            uS = (uS == 1.0f) ? 0.5f : 1.0f;
        }

        // Space key - goldfish jump
        if (key == GLFW_KEY_SPACE && initialJumpTime < 0) {
            initialJumpTime = glfwGetTime();
        }

        // Z key - goldfish bubble
        if (key == GLFW_KEY_Z) {
            spawnBubble(uX, uY, 0);
        }

        // K key - clownfish bubble
        if (key == GLFW_KEY_K) {
            spawnBubble(cX, cY, 1);
        }
    }
}

void center_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float xposNorm = (xpos / screenWidth) * 2 - 1;
        float yposNorm = -((ypos / screenHeight) * 2 - 1);

        if (xposNorm < 0.2 + uX && xposNorm > -0.2 + uX &&
            yposNorm < 0.2 * uS + uY && yposNorm > -0.2 * uS + uY) {
            uX = 0;
        }
    }
}

void processMovement(GLFWwindow* window) {
    // Goldfish movement - check key states directly
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        uX -= MOVEMENT_SPEED;
        if (!ufacingRight) {
            uFlipX = 1.0f;
            ufacingRight = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        uX += MOVEMENT_SPEED;
        if (ufacingRight) {
            uFlipX = -1.0f;
            ufacingRight = false;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        uY -= MOVEMENT_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        uY += MOVEMENT_SPEED;
    }

    // Clownfish movement
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cX -= MOVEMENT_SPEED;
        if (!cfacingRight) {
            cFlipX = 1.0f;
            cfacingRight = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cX += MOVEMENT_SPEED;
        if (cfacingRight) {
            cFlipX = -1.0f;
            cfacingRight = false;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cY -= MOVEMENT_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cY += MOVEMENT_SPEED;
    }
}

int main()
{
    // Initialize random seed
    srand(static_cast<unsigned>(time(0)));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Vezba 3", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    // Set up callbacks
    glfwSetKeyCallback(window, squish_callback);
    glfwSetMouseButtonCallback(window, center_callback);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load textures
    preprocessTexture(goldfishTexture, "res/4-gold-fish-png-image.png");
    preprocessTexture(clownfishTexture, "res/clown-fish.png");
    preprocessTexture(bubblesTexture, "res/bubbles.png");

    // Create shader
    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    glUseProgram(rectShader);
    glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

    // Vertex data
    float verticesRect[] = {
         -0.2f, 0.2f, 0.0f, 1.0f,
         -0.2f, -0.2f, 0.0f, 0.0f,
         0.2f, -0.2f, 1.0f, 0.0f,
         0.2f, 0.2f, 1.0f, 1.0f,
    };

    // Create VAOs
    unsigned int VAO_fish;
    unsigned int VAO_bubbles;
    formVAOs(verticesRect, sizeof(verticesRect), VAO_fish);
    formVAOs(verticesRect, sizeof(verticesRect), VAO_bubbles);

    glClearColor(0.5f, 0.6f, 1.0f, 1.0f);

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Process continuous movement - this is the key change!
        processMovement(window);

        // Update bubbles
        updateBubbles(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);

        // Draw fish
        drawRect(rectShader, VAO_fish, uX, uY, uS, uFlipX, goldfishTexture);
        drawRect(rectShader, VAO_fish, cX, cY, cS, cFlipX, clownfishTexture);

        // Draw bubbles
        for (const auto& bubble : bubbles) {
            if (bubble.active) {
                drawBubble(rectShader, VAO_bubbles, bubble);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(rectShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}