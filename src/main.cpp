#include "raylib.h"
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <cstring>

struct Metric {
    std::string name;
    int value;
    Color color;
};
#define MAX_METRICS 30

Color BG_COLOR = {15, 15, 15, 155};

const int screenWidth = 600;
const int screenHeight = 800;

int main() {
    InitWindow(screenWidth, screenHeight, "JP Code Analysis");

    SetTargetFPS(60);

    Font t = LoadFont("src/title.otf");
    if (t.texture.id == 0) {std::cout << "Failed to load font" << std::endl;} else {std::cout << "Font loaded successfully" << std::endl;}

    Font d = LoadFont("src/default.ttf");
    if (d.texture.id == 0) {std::cout << "Failed to load font" << std::endl;} else {std::cout << "Font loaded successfully" << std::endl;}

    // --- state ---
    const char* langs[] = {"C++", "Java"};

    int selectedLang = -1;
    bool dropdownOpen = false;

    Rectangle dropdownBtn = {20, 150, 200, 30};
    Rectangle dropdownOptions[2] = {{20, 183, 200, 30}, {20, 216, 200, 30}};

    char filePath[512] = {0};
    bool typing = false;

    Rectangle inputBox = {20, 290, 560, 30};

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        // dropdown
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, dropdownBtn)) {dropdownOpen = !dropdownOpen; typing = false;}
            else if (dropdownOpen) {
                for (int i = 0; i < 2; i++) {
                    if (CheckCollisionPointRec(mousePos, dropdownOptions[i])) {selectedLang = i; dropdownOpen = false;}
                }
                dropdownOpen = false;
            }
            // input box focus
            if (!dropdownOpen) {typing = CheckCollisionPointRec(mousePos, inputBox);}
        }

        // typing
        if (typing) {
            int key = GetCharPressed();
            while (key > 0) {
                int len = strlen(filePath);
                if (len < 511) {filePath[len] = (char)key; filePath[len + 1] = '\0';}
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && strlen(filePath) > 0) {filePath[strlen(filePath) - 1] = '\0';}
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
                const char* clip = GetClipboardText();
                if (clip) {
                    int len = strlen(filePath);
                    int clipLen = strlen(clip);
                    if (len + clipLen < 511) {strncat(filePath, clip, 511 - len);}
                }
            }
        }

        BeginDrawing();
        ClearBackground(BG_COLOR);

        // header
        DrawTextEx(t, "JP Code Analysis", {20, 20}, 24, 2, LIGHTGRAY);
        DrawTextEx(d, "Developed by Jupyter, use this to generate code analysis reports.", {20, 60}, 16, 1, LIGHTGRAY);

        // divider
        DrawLineEx({20, 95}, {580, 95}, 1, {60, 60, 60, 255});

        // language dropdown
        DrawTextEx(d, "Select programming language:", {20, 110}, 16, 1, LIGHTGRAY);
        Color btnColor = CheckCollisionPointRec(mousePos, dropdownBtn) ? Color{50, 50, 50, 255} : Color{30, 30, 30, 255};

        DrawRectangleRec(dropdownBtn, btnColor);
        DrawRectangleLinesEx(dropdownBtn, 1, dropdownOpen ? WHITE : GRAY);

        DrawTextEx(d, selectedLang >= 0 ? langs[selectedLang] : "Select...", {dropdownBtn.x + 10, dropdownBtn.y + 7}, 16, 1, selectedLang >= 0 ? WHITE : GRAY);
        DrawTextEx(d, dropdownOpen ? "^" : "v", {dropdownBtn.x + dropdownBtn.width - 22, dropdownBtn.y + 7}, 16, 1, GRAY);

        if (dropdownOpen) {
            for (int i = 0; i < 2; i++) {
                Color optColor = CheckCollisionPointRec(mousePos, dropdownOptions[i]) ? Color{60, 60, 60, 255} : Color{35, 35, 35, 255};

                DrawRectangleRec(dropdownOptions[i], optColor);

                DrawRectangleLinesEx(dropdownOptions[i], 1, GRAY);
                DrawTextEx(d, langs[i], {dropdownOptions[i].x + 10, dropdownOptions[i].y + 7}, 16, 1, WHITE);
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}