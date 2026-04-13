#include "raylib.h"
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <cstring>
#include <filesystem>

// --- STRUCTS --- //
/// @struct Metric - Represents a code metric with name, value, and visual color
struct Metric {
    std::string name;   // Name of the metric (e.g., "Lines of Code")
    int value;          // Numerical value of the metric
    Color color;        // Color used for rendering the metric
};

// --- CONSTANTS --- //
/// Window dimensions
const int screenWidth = 600;
const int screenHeight = 800;

// --- DEFINITIONS --- //
/// Maximum number of metrics that can be displayed
#define MAX_METRICS 30
/// Maximum characters allowed in input box
#define MAX_INPUT_CHARS 256

// --- VARIABLES --- //
/// Background color for the application (dark gray with transparency)
Color BG_COLOR = {15, 15, 15, 155};



int main() {
    // Initialize raylib window
    InitWindow(screenWidth, screenHeight, "JP Code Analysis");

    // Set target frame rate to 60 FPS
    SetTargetFPS(60);

    // Load title font (larger, decorative font)
    Font t = LoadFont("src/title.otf");
    if (t.texture.id == 0) {std::cout << "Failed to load font" << std::endl;} else {std::cout << "Font loaded successfully" << std::endl;}

    // Load default body font (standard size font)
    Font d = LoadFont("src/default.ttf");
    if (d.texture.id == 0) {std::cout << "Failed to load font" << std::endl;} else {std::cout << "Font loaded successfully" << std::endl;}

    // --- APPLICATION STATE VARIABLES --- //
    /// Available programming language options
    const char* langs[] = {"C++", "Java"};

    /// Currently selected language index (-1 = none selected)
    int selectedLang = -1;
    /// Whether the language dropdown is currently open
    bool dropdownOpen = false;

    /// Main dropdown button position and dimensions
    Rectangle dropdownBtn = {20, 130, 200, 30};
    /// Dropdown option positions and dimensions (one for each language)
    Rectangle dropdownOptions[2] = {{20, 160, 200, 30}, {20, 190, 200, 30}};

    /// Buffer to store the file path entered by user
    char filePath[512] = {0};
    /// Whether the input box is currently active for typing
    bool typing = false;

    /// Whether the mouse is hovering over the input box
    bool mouseOnText = false;

    /// File path input box position and dimensions
    Rectangle inputBox = {20, 260, 560, 30};

    while (!WindowShouldClose()) {
        // --- UPDATE LOGIC --- //

        /// Update whether mouse is hovering over the input box
        if (CheckCollisionPointRec(GetMousePosition(), inputBox)) mouseOnText = true;
        else mouseOnText = false;

        /// Update cursor style based on whether mouse is over text input
        if (mouseOnText)
        {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);  // I-beam cursor for text editing
        } else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);  // Default arrow cursor
        }

        Vector2 mousePos = GetMousePosition();

        // --- INPUT HANDLING: MOUSE CLICKS --- //
        /// Handle dropdown and input box interactions on mouse click
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Toggle dropdown open/closed when clicking the dropdown button
            if (CheckCollisionPointRec(mousePos, dropdownBtn)) {
                dropdownOpen = !dropdownOpen;
                typing = false;
            } else if (dropdownOpen) {
                /// Check if user clicked on a language option
                bool optionClicked = false;
                for (int i = 0; i < 2; i++) {
                    if (CheckCollisionPointRec(mousePos, dropdownOptions[i])) {
                        selectedLang = i;           // Store selected language
                        dropdownOpen = false;       // Close dropdown after selection
                        optionClicked = true;
                        typing = false;
                        break;
                    }
                }
                /// Close dropdown if user clicked but didn't select an option
                if (!optionClicked) {
                    dropdownOpen = false;
                }
            }
            // Set focus to input box if not interacting with dropdown
            if (!dropdownOpen) {typing = CheckCollisionPointRec(mousePos, inputBox);}
        }

        // --- INPUT HANDLING: KEYBOARD (TEXT INPUT) --- //
        /// Process character input and editing commands while typing in the input box
        if (typing) {
            // Capture and append typed characters to file path
            int key = GetCharPressed();
            while (key > 0) {
                int len = strlen(filePath);
                /// Only add character if buffer hasn't reached max size
                if (len < 511) {filePath[len] = (char)key; filePath[len + 1] = '\0';}
                key = GetCharPressed();
            }
            
            /// Handle backspace key to delete last character
            if (IsKeyPressed(KEY_BACKSPACE) && strlen(filePath) > 0) {filePath[strlen(filePath) - 1] = '\0';}
            
            /// Handle Ctrl+V paste from clipboard
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
                const char* clip = GetClipboardText();
                if (clip) {
                    int len = strlen(filePath);
                    int clipLen = strlen(clip);
                    /// Only paste if it fits in the buffer
                    if (len + clipLen < 511) {strncat(filePath, clip, 511 - len);}
                }
            }
        }

        BeginDrawing();
        ClearBackground(BG_COLOR);

        // --- RENDERING: HEADER SECTION --- //
        /// Draw main title using title font
        DrawTextEx(t, "JP Code Analysis", {20, 20}, 24, 2, LIGHTGRAY);
        /// Draw subtitle with description
        DrawTextEx(d, "Developed by Jupyter, use this to generate code analysis reports.", {20, 60}, 16, 1, LIGHTGRAY);

        /// Draw horizontal divider line below header
        DrawLineEx({20, 95}, {580, 95}, 1, {60, 60, 60, 255});

        // --- RENDERING: LANGUAGE DROPDOWN --- //
        /// Draw label for language selection
        DrawTextEx(d, "Select programming language:", {20, 110}, 16, 1, LIGHTGRAY);
        /// Change button color based on mouse hover
        Color btnColor = CheckCollisionPointRec(mousePos, dropdownBtn) ? Color{50, 50, 50, 255} : Color{30, 30, 30, 255};

        DrawRectangleRec(dropdownBtn, btnColor);
        /// Draw button border (brighter when dropdown is open)
        DrawRectangleLinesEx(dropdownBtn, 1, dropdownOpen ? WHITE : GRAY);

        /// Draw selected language or "Select..." placeholder text
        DrawTextEx(d, selectedLang >= 0 ? langs[selectedLang] : "Select...", {dropdownBtn.x + 10, dropdownBtn.y + 7}, 16, 1, selectedLang >= 0 ? WHITE : GRAY);
        /// Draw dropdown arrow (^ when open, v when closed)
        DrawTextEx(d, dropdownOpen ? "^" : "v", {dropdownBtn.x + dropdownBtn.width - 22, dropdownBtn.y + 7}, 16, 1, GRAY);

        /// Draw dropdown options if dropdown is open
        if (dropdownOpen) {
            for (int i = 0; i < 2; i++) {
                /// Highlight option on hover
                Color optColor = CheckCollisionPointRec(mousePos, dropdownOptions[i]) ? Color{60, 100, 160, 255} : Color{35, 50, 90, 255};
                DrawRectangleRec(dropdownOptions[i], optColor);
                DrawRectangleLinesEx(dropdownOptions[i], 1, GRAY);
                DrawTextEx(d, langs[i], {dropdownOptions[i].x + 10, dropdownOptions[i].y + 7}, 16, 1, WHITE);
            }
        }

        // --- FILE PATH INPUT BOX --- //
        /// Draw label for file path input
        DrawTextEx(d, "Enter file path to start analysis:", {20, 240}, 16, 1, LIGHTGRAY);
        /// Change input box color based on mouse hover
        Color inputColor = CheckCollisionPointRec(mousePos, inputBox) ? Color{50, 50, 50, 255} : Color{30, 30, 30, 255};
        DrawRectangleRec(inputBox, inputColor);
        /// Draw input box border (bright when typing, dim when idle)
        DrawRectangleLinesEx(inputBox, 1, typing ? LIGHTGRAY : GRAY);
        /// Draw the entered file path text
        DrawTextEx(d, filePath, {inputBox.x + 10, inputBox.y + 7}, 16, 1, WHITE);

        EndDrawing();
    }

    // Clean up raylib resources
    CloseWindow();
    return 0;
}

// WRITTEN BY JUPYTER, 2026