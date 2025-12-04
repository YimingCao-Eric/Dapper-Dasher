#include "raylib.h"

struct AnimData {
    Rectangle rec;
    Vector2 pos;
    int frame;
    float updateTime;
    float runningTime;
};

bool isOnGround(AnimData data, int windowHeight) {
    return data.pos.y >= windowHeight - data.rec.height;
}

AnimData updateAnimData(AnimData data, float deltaTime, int maxFrame) {
    data.runningTime += deltaTime;
    if (data.runningTime >= data.updateTime) {
        data.runningTime = 0.0;
        data.rec.x = data.frame * data.rec.width;
        data.frame++;
        if (data.frame > maxFrame) {
            data.frame = 0;     
        }
    }
    return data;
}

float drawBackground(Texture2D texture, float dT, float speed, float bgX, int windowWidth) {
    bgX -= speed * dT;
    if (bgX <= -texture.width * 2) {
        bgX = 0.0;
    }
    Vector2 bg1Pos = {bgX, 0.0};
    DrawTextureEx(texture, bg1Pos, 0.0, 2.0, WHITE);
    Vector2 bg2Pos = {bgX+float(texture.width * 2), 0.0};
    DrawTextureEx(texture, bg2Pos, 0.0, 2.0, WHITE);
    return bgX;
}

int main() {
    // window dimensions
    int windowDimensions[2];
    windowDimensions[0] = 512;
    windowDimensions[1] = 380;
    InitWindow(windowDimensions[0], windowDimensions[1], "Dapper Dasher!");
    
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
    // collision check
    bool collision{};

    // background variables
    Texture2D background = LoadTexture("textures/far-buildings.png");
    float bgX = 0.0;
    // midground variables
    Texture2D midground = LoadTexture("textures/back-buildings.png");
    float mgX = 0.0;
    // foreground variables
    Texture2D foreground = LoadTexture("textures/foreground.png");
    float fgX = 0.0;

    // nebula variables
    Texture2D nebula = LoadTexture("textures/12_nebula_spritesheet.png");
    const int numNebulae = 2;
    AnimData nebulae[numNebulae] = {};
    for (int i = 0; i < numNebulae; i++) {
        nebulae[i].rec.x = 0.0;
        nebulae[i].rec.y = 0.0;
        nebulae[i].rec.width = float(nebula.width/8);
        nebulae[i].rec.height = float(nebula.height/8);
        nebulae[i].pos.x = windowDimensions[0] + 300 * i;
        nebulae[i].pos.y = windowDimensions[1] - float(nebula.height/8);
        nebulae[i].frame = 0;
        nebulae[i].runningTime = 0.0;
        nebulae[i].updateTime = 1.0/12.0;
    }

    float finishLine = nebulae[numNebulae-1].pos.x;

    // scarfy variables
    Texture2D scarfy = LoadTexture("textures/scarfy.png");
    float scarfyData_rec_width = scarfy.width / 6;
    AnimData scarfyData{
        {0.0, 0.0, scarfyData_rec_width, float(scarfy.height)},
        {windowDimensions[0] / 2 - scarfyData_rec_width / 2, windowDimensions[1] - float(scarfy.height)},
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
        
        // draw background
        bgX = drawBackground(background, dT, 20, bgX, windowDimensions[0]);

        // draw midground
        mgX = drawBackground(midground, dT, 40, mgX, windowDimensions[0]);

        // draw foreground
        fgX = drawBackground(foreground, dT, 80, fgX, windowDimensions[0]);

        // perform ground check
        isInAir = !isOnGround(scarfyData, windowDimensions[1]);
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
        for (int i = 0; i < numNebulae; i++) {
            nebulae[i].pos.x += nebulaVelocity * dT;
        }
        finishLine += nebulaVelocity * dT;

        // update scarfy position
        scarfyData.pos.y += velocityY * dT;

        // update scarfy animation frame
        if (!isInAir) {
            scarfyData = updateAnimData(scarfyData, dT, 5);
        }

        // update nebula animation frame
        for (int i = 0; i < numNebulae; i++) {
            nebulae[i] = updateAnimData(nebulae[i], dT, 7);
        }

        for (AnimData nebula : nebulae) {
            float pad{40};
            Rectangle nebulaRec{
                nebula.pos.x + pad,
                nebula.pos.y + pad,
                nebula.rec.width - pad * 2,
                nebula.rec.height - pad * 2
            };
            Rectangle scarfyRec{
                scarfyData.pos.x,
                scarfyData.pos.y,
                scarfyData.rec.width,
                scarfyData.rec.height
            };
            if (CheckCollisionRecs(scarfyRec, nebulaRec)) {
                collision = true;
            }
        }
        if (collision) {
            // lose
            DrawText("Game Over", windowDimensions[0] / 4, windowDimensions[1] / 2, 40, RED);
        } else if (scarfyData.pos.x >= finishLine) {
            // win
            DrawText("You Win", windowDimensions[0] / 4, windowDimensions[1] / 2, 40, RED);
        } else {
            // draw nebula
            for (int i = 0; i < numNebulae; i++) {
                DrawTextureRec(nebula, nebulae[i].rec, nebulae[i].pos, WHITE);
            }
            // draw scarfy
            DrawTextureRec(scarfy, scarfyData.rec, scarfyData.pos, WHITE);
        }   
        
        // end drawing
        EndDrawing();
    }
    UnloadTexture(scarfy);
    UnloadTexture(nebula);
    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    CloseWindow();
}