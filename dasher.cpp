#include "raylib.h"

struct AnimData {
    Rectangle rec;
    Vector2 pos;
    int frame;
    float updateTime;
    float runningTime;
};

int main() {
    // window dimensions
    const int windowWidth = 512;
    const int windowHeight = 380;
    // initialize window
    InitWindow(windowWidth, windowHeight, "Dapper Dasher!");
    
    // acceleration due to gravity (pixels/second^2)
    const int gravity = 1000;
    // check if the scarfy is in the air
    bool isInAir = false;
    // nebula velocity (pixels/second)
    const int nebulaVelocity = -200; 
    // jump velocity (pixels/second)
    const int jumpVelocity = -600;     
    // scarfy velocity(pixels/second)
    int velocityY = 0;

    // nebula variables
    Texture2D nebula = LoadTexture("textures/12_nebula_spritesheet.png");

    AnimData nebulaData{
        {0.0, 0.0, float(nebula.width/8), float(nebula.height/8)}, // Rectangle rec
        {windowWidth, windowHeight - float(nebula.height/8)}, // Vector 2 Position
        0, // Frame
        1.0/6.0, // Update Time
        0.0 // Running Time
    };

    AnimData nebulaData2{
        {0.0, 0.0, float(nebula.width/8), float(nebula.height/8)}, // Rectangle rec
        {windowWidth + 300, windowHeight - float(nebula.height/8)}, // Vector 2 Position
        0, // Frame
        1.0/24.0, // Update Time
        0.0 // Running Time
    };
    
    // scarfy variables
    Texture2D scarfy = LoadTexture("textures/scarfy.png");
    float scarfyData_rec_width = scarfy.width / 6;
    AnimData scarfyData{
        {0.0, 0.0, scarfyData_rec_width, float(scarfy.height)},
        {windowWidth / 2 - scarfyData_rec_width / 2, windowHeight - float(scarfy.height)},
        0,
        1.0/12.0,
        0.0
    };

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // start drawing
        BeginDrawing();
        ClearBackground(WHITE);

        // delta time: time since last frame
        float dT = GetFrameTime();

        isInAir = (scarfyData.pos.y < windowHeight - scarfyData.rec.height);
        // perform ground check
        if (!isInAir) {
            // rectangle is on the ground
            velocityY = 0;
        } else {
            // rectangle is in the air, apply gravity
            velocityY += gravity * dT;
        }

        // jump check
        if (IsKeyDown(KEY_SPACE) && !isInAir) {
            velocityY += jumpVelocity;
        }

        // update nebula position
        nebulaData.pos.x += nebulaVelocity * dT;
        nebulaData2.pos.x += nebulaVelocity * dT;
        // update scarfy position
        scarfyData.pos.y += velocityY * dT;

        // update scarfy animation frame
        if (!isInAir) {
            scarfyData.runningTime += dT;
            if (scarfyData.runningTime >= scarfyData.updateTime) {
                scarfyData.runningTime = 0.0;
                scarfyData.rec.x = scarfyData.frame * scarfyData.rec.width;
                scarfyData.frame++;
                if (scarfyData.frame > 5) {
                    scarfyData.frame = 0;
                }
            }
        }

        // update nebula animation frame
        nebulaData.runningTime += dT;
        if (nebulaData.runningTime >= nebulaData.updateTime) {
            nebulaData.runningTime = 0.0;
            nebulaData.rec.x = nebulaData.frame * nebulaData.rec.width;
            nebulaData.frame++;
            if (nebulaData.frame > 7) {
                nebulaData.frame = 0;
            }
        }

        // update nebula animation frame
        nebulaData2.runningTime += dT;
        if (nebulaData2.runningTime >= nebulaData2.updateTime) {
            nebulaData2.runningTime = 0.0;
            nebulaData2.rec.x = nebulaData2.frame * nebulaData2.rec.width;
            nebulaData2.frame++;
            if (nebulaData2.frame > 7) {
                nebulaData2.frame = 0;
            }
        }

        // draw nebula
        DrawTextureRec(nebula, nebulaData.rec, nebulaData.pos, WHITE);
        DrawTextureRec(nebula, nebulaData2.rec, nebulaData2.pos, RED);

        // draw scarfy
        DrawTextureRec(scarfy, scarfyData.rec, scarfyData.pos, WHITE);

        // end drawing
        EndDrawing();
    }
    UnloadTexture(scarfy);
    UnloadTexture(nebula);
    CloseWindow();
}