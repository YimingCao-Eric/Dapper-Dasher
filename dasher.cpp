// Dapper Dasher - A side-scrolling runner game built with raylib
// Player controls Scarfy character to avoid Nebula obstacles and reach the finish line

#include "raylib.h"

// Structure to hold animation data for sprites
struct AnimData {
    Rectangle rec;      // Rectangle defining the current frame in the spritesheet
    Vector2 pos;        // Position of the sprite on screen
    int frame;          // Current animation frame index
    float updateTime;   // Time interval between frame updates (seconds)
    float runningTime;  // Accumulated time since last frame update
};

// Checks if an animated sprite is touching the ground
// Returns true if the sprite's bottom edge is at or below the window bottom
bool isOnGround(AnimData data, int windowHeight) {
    return data.pos.y >= windowHeight - data.rec.height;
}

// Updates animation frame based on elapsed time
// Advances to next frame when updateTime has elapsed, loops back to frame 0 after maxFrame
AnimData updateAnimData(AnimData data, float deltaTime, int maxFrame) {
    data.runningTime += deltaTime;
    if (data.runningTime >= data.updateTime) {
        data.runningTime = 0.0;
        // Update the source rectangle to show the current frame
        data.rec.x = data.frame * data.rec.width;
        data.frame++;
        // Loop animation back to first frame
        if (data.frame > maxFrame) {
            data.frame = 0;     
        }
    }
    return data;
}

// Draws a scrolling background layer with seamless looping
// Moves background left at specified speed, resets position when it scrolls off screen
float drawBackground(Texture2D texture, float dT, float speed, float bgX, int windowWidth) {
    // Move background position left based on speed and delta time
    bgX -= speed * dT;
    // Reset position when first copy has scrolled completely off screen
    if (bgX <= -texture.width * 2) {
        bgX = 0.0;
    }
    // Draw first copy of background
    Vector2 bg1Pos = {bgX, 0.0};
    DrawTextureEx(texture, bg1Pos, 0.0, 2.0, WHITE);
    // Draw second copy immediately to the right for seamless scrolling
    Vector2 bg2Pos = {bgX+float(texture.width * 2), 0.0};
    DrawTextureEx(texture, bg2Pos, 0.0, 2.0, WHITE);
    return bgX;
}

int main() {
    // Initialize window dimensions [width, height]
    int windowDimensions[2];
    windowDimensions[0] = 512;
    windowDimensions[1] = 380;
    InitWindow(windowDimensions[0], windowDimensions[1], "Dapper Dasher!");
    
    // Physics constants
    const int gravity = 1000;        // Acceleration due to gravity (pixels/second^2)
    const int nebulaVelocity = -200; // Nebula obstacle movement speed (pixels/second, negative = left)
    const int jumpVelocity = -600;   // Initial upward velocity when jumping (pixels/second)
    
    // Game state variables
    bool isInAir = false;    // Tracks if Scarfy character is airborne
    int velocityY = 0;        // Scarfy's vertical velocity (pixels/second)
    bool collision{};         // Flag indicating collision with obstacle

    // Load background layer textures (parallax scrolling effect)
    Texture2D background = LoadTexture("textures/far-buildings.png");
    float bgX = 0.0;  // Background X position for scrolling
    Texture2D midground = LoadTexture("textures/back-buildings.png");
    float mgX = 0.0;  // Midground X position for scrolling
    Texture2D foreground = LoadTexture("textures/foreground.png");
    float fgX = 0.0;  // Foreground X position for scrolling

    // Load and initialize Nebula obstacle sprites
    // Nebula spritesheet contains 8 frames (8 columns)
    Texture2D nebula = LoadTexture("textures/12_nebula_spritesheet.png");
    const int numNebulae = 2;  // Number of nebula obstacles in the game
    AnimData nebulae[numNebulae] = {};
    
    // Initialize each nebula obstacle
    for (int i = 0; i < numNebulae; i++) {
        nebulae[i].rec.x = 0.0;
        nebulae[i].rec.y = 0.0;
        // Each frame is 1/8th of the spritesheet width
        nebulae[i].rec.width = float(nebula.width/8);
        nebulae[i].rec.height = float(nebula.height/8);
        // Position nebulae off-screen to the right, spaced 300 pixels apart
        nebulae[i].pos.x = windowDimensions[0] + 300 * i;
        // Position at ground level (bottom of window)
        nebulae[i].pos.y = windowDimensions[1] - float(nebula.height/8);
        nebulae[i].frame = 0;
        nebulae[i].runningTime = 0.0;
        nebulae[i].updateTime = 1.0/12.0;  // 12 frames per second animation
    }

    // Set finish line position at the rightmost nebula's position
    float finishLine = nebulae[numNebulae-1].pos.x;

    // Load and initialize Scarfy player character
    // Scarfy spritesheet contains 6 frames (6 columns)
    Texture2D scarfy = LoadTexture("textures/scarfy.png");
    float scarfyData_rec_width = scarfy.width / 6;  // Width of each animation frame
    AnimData scarfyData{
        {0.0, 0.0, scarfyData_rec_width, float(scarfy.height)},  // Source rectangle (first frame)
        {windowDimensions[0] / 2 - scarfyData_rec_width / 2, windowDimensions[1] - float(scarfy.height)},  // Center horizontally, ground level vertically
        0,           // Start at first frame
        1.0/12.0,   // 12 frames per second animation
        0.0         // No accumulated time yet
    };

    // Set target frame rate to 60 FPS
    SetTargetFPS(60);
    // Main game loop - runs until window is closed
    while (!WindowShouldClose()) {
        // Begin rendering frame
        BeginDrawing();
        ClearBackground(WHITE);
        
        // Get delta time (time elapsed since last frame) for frame-independent movement
        float dT = GetFrameTime();
        
        // Draw parallax scrolling background layers
        // Each layer moves at different speeds for depth effect (faster = closer)
        bgX = drawBackground(background, dT, 20, bgX, windowDimensions[0]);  // Slowest (farthest)
        mgX = drawBackground(midground, dT, 40, mgX, windowDimensions[0]);    // Medium speed
        fgX = drawBackground(foreground, dT, 80, fgX, windowDimensions[0]);   // Fastest (closest)

        // Physics: Ground check and gravity
        isInAir = !isOnGround(scarfyData, windowDimensions[1]);
        if (!isInAir) {
            // Character is on the ground - reset vertical velocity
            velocityY = 0;
        } else {
            // Character is in the air - apply gravity (increases downward velocity)
            velocityY += gravity * dT;
        }

        // Input: Jump check (only allow jumping when on ground)
        if (IsKeyDown(KEY_SPACE) && !isInAir) {
            velocityY += jumpVelocity;  // Apply upward jump velocity
        }

        // Update obstacle positions (move left across screen)
        for (int i = 0; i < numNebulae; i++) {
            nebulae[i].pos.x += nebulaVelocity * dT;
        }
        // Move finish line with obstacles
        finishLine += nebulaVelocity * dT;

        // Update player position based on velocity
        scarfyData.pos.y += velocityY * dT;

        // Animation: Update Scarfy's running animation (only when on ground)
        // Scarfy has 6 frames (0-5), so maxFrame is 5
        if (!isInAir) {
            scarfyData = updateAnimData(scarfyData, dT, 5);
        }

        // Animation: Update Nebula obstacle animations
        // Nebula has 8 frames (0-7), so maxFrame is 7
        for (int i = 0; i < numNebulae; i++) {
            nebulae[i] = updateAnimData(nebulae[i], dT, 7);
        }

        // Collision detection: Check if Scarfy collides with any Nebula obstacle
        // Uses padded collision rectangles for more forgiving hit detection
        for (AnimData nebula : nebulae) {
            float pad{40};  // Padding to make collision detection less strict
            // Create collision rectangle for nebula (smaller than visual size)
            Rectangle nebulaRec{
                nebula.pos.x + pad,
                nebula.pos.y + pad,
                nebula.rec.width - pad * 2,
                nebula.rec.height - pad * 2
            };
            // Create collision rectangle for Scarfy
            Rectangle scarfyRec{
                scarfyData.pos.x,
                scarfyData.pos.y,
                scarfyData.rec.width,
                scarfyData.rec.height
            };
            // Check if rectangles overlap
            if (CheckCollisionRecs(scarfyRec, nebulaRec)) {
                collision = true;
            }
        }
        
        // Game state: Display win/lose message or draw game sprites
        if (collision) {
            // Player hit an obstacle - game over
            DrawText("Game Over", windowDimensions[0] / 4, windowDimensions[1] / 2, 40, RED);
        } else if (scarfyData.pos.x >= finishLine) {
            // Player reached finish line - victory!
            DrawText("You Win", windowDimensions[0] / 4, windowDimensions[1] / 2, 40, RED);
        } else {
            // Normal gameplay - draw obstacles and player
            // Draw all nebula obstacles
            for (int i = 0; i < numNebulae; i++) {
                DrawTextureRec(nebula, nebulae[i].rec, nebulae[i].pos, WHITE);
            }
            // Draw Scarfy player character
            DrawTextureRec(scarfy, scarfyData.rec, scarfyData.pos, WHITE);
        }   
        
        // End rendering frame
        EndDrawing();
    }
    
    // Cleanup: Unload all textures and close window
    UnloadTexture(scarfy);
    UnloadTexture(nebula);
    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    CloseWindow();
    
    return 0;
}