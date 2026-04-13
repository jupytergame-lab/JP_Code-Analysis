#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>

// --- STRUCTS --- //

/// @struct Metric - Represents a code metric with name, value, and visual color
struct Metric {
    std::string name;
    int value;
    Color color;
    std::string unit; // optional suffix (e.g. "%")
};

// --- CONSTANTS --- //

const int screenWidth  = 600;
const int screenHeight = 860;

#define MAX_METRICS 30

// --- COLORS --- //

Color BG_COLOR      = {15,  15,  15,  255};
Color PANEL_COLOR   = {22,  22,  22,  255};
Color CARD_COLOR    = {30,  30,  30,  255};
Color BORDER_DIM    = {55,  55,  55,  255};
Color BORDER_LIT    = {90,  90,  90,  255};
Color ACCENT_BLUE   = {50,  120, 190, 255};
Color ACCENT_HOVER  = {70,  150, 220, 255};
Color ACCENT_ORANGE = {220, 130, 30,  255};
Color ACCENT_GREEN  = {60,  180, 100, 255};
Color ACCENT_RED    = {200, 70,  60,  255};
Color TEXT_PRIMARY  = {230, 230, 230, 255};
Color TEXT_MUTED    = {130, 130, 130, 255};
Color TEXT_DIM      = {75,  75,  75,  255};

// --- STATE --- //

enum AnalyzeState { STATE_IDLE, STATE_ANALYZING, STATE_DONE, STATE_ERROR };
AnalyzeState analyzeState = STATE_IDLE;
std::string errorMessage;
std::vector<Metric> metrics;

// --- HELPERS --- //

/// Draw a rounded-rectangle card outline
static void DrawCard(Rectangle r, Color fill, Color border) {
    DrawRectangleRounded(r, 0.12f, 8, fill);
    DrawRectangleRoundedLinesEx(r, 0.12f, 8, 1.0f, border);
}

/// Draw a horizontal bar representing a fraction [0,1] inside a rect
static void DrawBar(Rectangle r, float fraction, Color fill, Color bg) {
    DrawRectangleRounded(r, 0.5f, 6, bg);
    Rectangle bar = r;
    float clamped = (fraction < 0.0f) ? 0.0f : ((fraction > 1.0f) ? 1.0f : fraction);
    bar.width = r.width * clamped;
    if (bar.width > 4) DrawRectangleRounded(bar, 0.5f, 6, fill);
}

// --- ANALYSIS --- //

/// @brief Full analysis: counts total, code, blank, comment lines, functions
void AnalyzeCode(const char* filePath, std::vector<Metric>& out, std::string& errMsg) {
    out.clear();
    errMsg.clear();

    std::ifstream file(filePath);
    if (!file.is_open()) {
        errMsg = "Could not open file: " + std::string(filePath);
        return;
    }

    int totalLines    = 0;
    int blankLines    = 0;
    int commentLines  = 0;
    int codeLines     = 0;
    int functionCount = 0;
    bool inBlockComment = false;

    std::string line;
    while (std::getline(file, line)) {
        totalLines++;

        // Trim leading whitespace
        size_t s = line.find_first_not_of(" \t\r\n");

        if (s == std::string::npos) {
            blankLines++;
            continue;
        }

        std::string trimmed = line.substr(s);

        // Handle block comment state
        if (inBlockComment) {
            commentLines++;
            if (trimmed.find("*/") != std::string::npos) inBlockComment = false;
            continue;
        }

        // Line comment
        if (trimmed.rfind("//", 0) == 0) {
            commentLines++;
            continue;
        }

        // Block comment start
        if (trimmed.rfind("/*", 0) == 0) {
            commentLines++;
            if (trimmed.find("*/") == std::string::npos) inBlockComment = true;
            continue;
        }

        // Code line
        codeLines++;

        // Heuristic: function definition — line contains '(' and ends with ')' or '{' (not just a call)
        // Detects patterns like:  ReturnType FuncName(...) {  or  ReturnType FuncName(...)
        if (trimmed.find('(') != std::string::npos) {
            // Exclude lines that are pure keyword statements (if/while/for/switch)
            bool isControl = (trimmed.rfind("if ", 0) == 0    ||
                              trimmed.rfind("if(", 0) == 0    ||
                              trimmed.rfind("for ", 0) == 0   ||
                              trimmed.rfind("for(", 0) == 0   ||
                              trimmed.rfind("while ", 0) == 0 ||
                              trimmed.rfind("while(", 0) == 0 ||
                              trimmed.rfind("switch ", 0) == 0||
                              trimmed.rfind("switch(", 0) == 0||
                              trimmed.rfind("return ", 0) == 0||
                              trimmed.rfind("//", 0) == 0);

            if (!isControl) {
                char last = trimmed.back();
                if (last == '{' || last == ')') {
                    functionCount++;
                }
            }
        }
    }

    file.close();

    // Comment ratio as a pseudo-metric (stored as integer percentage)
    int commentRatio = (totalLines > 0) ? (int)(100.0f * commentLines / totalLines) : 0;

    out.push_back({"Total Lines",    totalLines,    TEXT_PRIMARY, ""});
    out.push_back({"Code Lines",     codeLines,     ACCENT_BLUE,  ""});
    out.push_back({"Comment Lines",  commentLines,  ACCENT_GREEN, ""});
    out.push_back({"Blank Lines",    blankLines,    TEXT_MUTED,   ""});
    out.push_back({"Functions",      functionCount, ACCENT_ORANGE,""});
    out.push_back({"Comment Ratio",  commentRatio,  ACCENT_GREEN, "%"});
}


int main() {

    InitWindow(screenWidth, screenHeight, "JP Code Analysis");
    SetTargetFPS(60);

    // Fonts
    Font fontTitle = LoadFont("src/title.otf");
    Font fontBody  = LoadFont("src/default.ttf");
    if (fontTitle.texture.id == 0) std::cout << "[WARN] title.otf not found, using default\n";
    if (fontBody.texture.id  == 0) std::cout << "[WARN] default.ttf not found, using default\n";

    // Use raylib default as fallback
    const Font& t = (fontTitle.texture.id != 0) ? fontTitle : GetFontDefault();
    const Font& d = (fontBody.texture.id  != 0) ? fontBody  : GetFontDefault();

    // --- LANGUAGE DROPDOWN --- //
    const char* langs[]  = {(const char*)"C++", (const char*)"Java", (const char*)"Python", (const char*)"C"};
    const int   langCount = 4;
    int  selectedLang  = -1;
    bool dropdownOpen  = false;

    Rectangle dropdownBtn = {20, 130, 200, 32};
    Rectangle dropdownOptions[4];
    for (int i = 0; i < langCount; i++)
        dropdownOptions[i] = {20, 162.0f + i * 30.0f, 200, 30};

    // --- FILE PATH INPUT --- //
    char filePath[512] = {0};
    bool typing        = false;
    Rectangle inputBox = {20, 260, 560, 34};

    // --- ANALYZE BUTTON --- //
    Rectangle analyzeBtn = {20, 314, 160, 40};

    // --- METRICS DISPLAY AREA --- //
    // Starts below a divider around y=380
    float metricsY = 390;

    while (!WindowShouldClose()) {

        Vector2 mouse = GetMousePosition();

        // Cursor style
        SetMouseCursor(CheckCollisionPointRec(mouse, inputBox)
            ? MOUSE_CURSOR_IBEAM : MOUSE_CURSOR_DEFAULT);

        // --- MOUSE CLICKS --- //
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

            // Dropdown toggle
            if (CheckCollisionPointRec(mouse, dropdownBtn)) {
                dropdownOpen = !dropdownOpen;
                typing = false;
            } else if (dropdownOpen) {
                bool hit = false;
                for (int i = 0; i < langCount; i++) {
                    if (CheckCollisionPointRec(mouse, dropdownOptions[i])) {
                        selectedLang = i;
                        dropdownOpen = false;
                        hit = true;
                        break;
                    }
                }
                if (!hit) dropdownOpen = false;
            }

            // Input box focus
            if (!dropdownOpen)
                typing = CheckCollisionPointRec(mouse, inputBox);

            // Analyze button
            if (CheckCollisionPointRec(mouse, analyzeBtn)) {
                bool ready = selectedLang >= 0 && strlen(filePath) > 0;
                if (ready && analyzeState != STATE_ANALYZING) {
                    analyzeState = STATE_ANALYZING;
                    metrics.clear();
                    errorMessage.clear();

                    AnalyzeCode(filePath, metrics, errorMessage);

                    analyzeState = errorMessage.empty() ? STATE_DONE : STATE_ERROR;
                }
            }
        }

        // --- KEYBOARD INPUT --- //
        if (typing) {
            int key = GetCharPressed();
            while (key > 0) {
                int len = strlen(filePath);
                if (len < 511) { filePath[len] = (char)key; filePath[len+1] = '\0'; }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && strlen(filePath) > 0)
                filePath[strlen(filePath) - 1] = '\0';

            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
                const char* clip = GetClipboardText();
                if (clip) {
                    int len = strlen(filePath), clipLen = strlen(clip);
                    if (len + clipLen < 511) strncat(filePath, clip, 511 - len);
                }
            }

            // Reset analysis if path changes
            if (analyzeState == STATE_DONE || analyzeState == STATE_ERROR)
                analyzeState = STATE_IDLE;
        }

        // ---- DRAW ---- //
        BeginDrawing();
        ClearBackground(BG_COLOR);

        // ── HEADER ──
        DrawTextEx(t, "JP Code Analysis", {20, 20}, 26, 2, TEXT_PRIMARY);
        DrawTextEx(d, "Analyze source files and generate code metrics.", {20, 58}, 15, 1, TEXT_MUTED);
        DrawLineEx({20, 94}, {580, 94}, 1, BORDER_DIM);

        // ── LANGUAGE DROPDOWN ──
        DrawTextEx(d, "Programming language", {20, 110}, 14, 1, TEXT_MUTED);
        Color btnFill  = CheckCollisionPointRec(mouse, dropdownBtn) ? Color{45,45,45,255} : CARD_COLOR;
        Color btnBord  = dropdownOpen ? BORDER_LIT : BORDER_DIM;
        DrawCard(dropdownBtn, btnFill, btnBord);
        DrawTextEx(d,
            selectedLang >= 0 ? langs[selectedLang] : "Select...",
            {dropdownBtn.x + 10, dropdownBtn.y + 8}, 15, 1,
            selectedLang >= 0 ? TEXT_PRIMARY : TEXT_DIM);
        DrawTextEx(d, dropdownOpen ? "^" : "v",
            {dropdownBtn.x + dropdownBtn.width - 20, dropdownBtn.y + 8}, 14, 1, TEXT_MUTED);

        if (dropdownOpen) {
            for (int i = 0; i < langCount; i++) {
                bool hov = CheckCollisionPointRec(mouse, dropdownOptions[i]);
                Color oc = hov ? Color{55, 100, 160, 255} : Color{32, 48, 85, 255};
                DrawCard(dropdownOptions[i], oc, BORDER_DIM);
                DrawTextEx(d, langs[i], {dropdownOptions[i].x + 10, dropdownOptions[i].y + 7}, 15, 1, TEXT_PRIMARY);
            }
        }

        // ── FILE PATH INPUT ──
        DrawTextEx(d, "File path", {20, 240}, 14, 1, TEXT_MUTED);
        Color inFill = CheckCollisionPointRec(mouse, inputBox) ? Color{40,40,40,255} : CARD_COLOR;
        Color inBord = typing ? ACCENT_BLUE : BORDER_DIM;
        DrawCard(inputBox, inFill, inBord);
        DrawTextEx(d, filePath, {inputBox.x + 10, inputBox.y + 9}, 15, 1, TEXT_PRIMARY);

        // Blinking cursor
        if (typing && ((int)(GetTime() * 2) % 2 == 0)) {
            float cx = inputBox.x + 10 + MeasureTextEx(d, filePath, 15, 1).x + 2;
            DrawLineEx({cx, inputBox.y + 7}, {cx, inputBox.y + 26}, 1.5f, TEXT_PRIMARY);
        }

        // Placeholder
        if (strlen(filePath) == 0 && !typing)
            DrawTextEx(d, "e.g. /home/user/main.cpp", {inputBox.x + 10, inputBox.y + 9}, 15, 1, TEXT_DIM);

        // ── ANALYZE BUTTON ──
        bool canAnalyze = selectedLang >= 0 && strlen(filePath) > 0;
        Color abFill, abBord;
        const char* abText;

        if (!canAnalyze) {
            abFill = Color{30,30,30,255}; abBord = BORDER_DIM; abText = "Analyze";
        } else if (analyzeState == STATE_ANALYZING) {
            abFill = ACCENT_ORANGE;       abBord = ACCENT_ORANGE; abText = "Analyzing...";
        } else if (analyzeState == STATE_DONE) {
            abFill = ACCENT_GREEN;        abBord = ACCENT_GREEN;  abText = "Re-analyze";
        } else if (analyzeState == STATE_ERROR) {
            abFill = ACCENT_RED;          abBord = ACCENT_RED;    abText = "Retry";
        } else {
            bool hov = CheckCollisionPointRec(mouse, analyzeBtn);
            abFill = hov ? ACCENT_HOVER : ACCENT_BLUE;
            abBord = Color{100, 180, 255, 255};
            abText = "Analyze";
        }
        DrawCard(analyzeBtn, abFill, abBord);
        {
            Vector2 ts = MeasureTextEx(d, abText, 16, 1);
            DrawTextEx(d, abText,
                {analyzeBtn.x + (analyzeBtn.width - ts.x) / 2,
                 analyzeBtn.y + (analyzeBtn.height - ts.y) / 2},
                16, 1, WHITE);
        }

        // ── DIVIDER ──
        DrawLineEx({20, 372}, {580, 372}, 1, BORDER_DIM);
        DrawTextEx(d, "Results", {20, 378}, 13, 1, TEXT_DIM);

        // ── ERROR MESSAGE ──
        if (analyzeState == STATE_ERROR && !errorMessage.empty()) {
            Rectangle errBox = {20, metricsY, 560, 36};
            DrawCard(errBox, Color{60, 20, 20, 255}, ACCENT_RED);
            DrawTextEx(d, errorMessage.c_str(), {errBox.x + 12, errBox.y + 10}, 14, 1, Color{255, 130, 120, 255});
        }

        // ── METRICS GRID ──
        if (analyzeState == STATE_DONE && !metrics.empty()) {

            // Find max value for bar scaling (use totalLines)
            int maxVal = metrics[0].value > 0 ? metrics[0].value : 1;

            float cardW = 270, cardH = 72;
            float gapX  = 20,  gapY  = 12;
            float startX = 20;

            for (int i = 0; i < (int)metrics.size(); i++) {
                int col = i % 2;
                int row = i / 2;
                float cx = startX + col * (cardW + gapX);
                float cy = metricsY + row * (cardH + gapY);

                Rectangle card = {cx, cy, cardW, cardH};
                DrawCard(card, CARD_COLOR, BORDER_DIM);

                // Metric name
                DrawTextEx(d, metrics[i].name.c_str(), {cx + 12, cy + 10}, 13, 1, TEXT_MUTED);

                // Value
                std::string valStr = std::to_string(metrics[i].value) + metrics[i].unit;
                DrawTextEx(d, valStr.c_str(), {cx + 12, cy + 28}, 22, 1, metrics[i].color);

                // Progress bar (skip ratio metric — already in %)
                if (metrics[i].unit.empty()) {
                    Rectangle barBg  = {cx + 12, cy + 56, cardW - 24, 6};
                    float frac = (float)metrics[i].value / (float)maxVal;
                    DrawBar(barBg, frac, metrics[i].color, Color{45, 45, 45, 255});
                } else {
                    // For percentage: show bar scaled 0–100
                    Rectangle barBg  = {cx + 12, cy + 56, cardW - 24, 6};
                    DrawBar(barBg, metrics[i].value / 100.0f, metrics[i].color, Color{45, 45, 45, 255});
                }
            }
        }

        // ── IDLE HINT ──
        if (analyzeState == STATE_IDLE) {
            DrawTextEx(d, "Select a language and enter a file path to begin.",
                {20, metricsY + 10}, 14, 1, TEXT_DIM);
        }

        EndDrawing();
    }

    UnloadFont(fontTitle);
    UnloadFont(fontBody);
    CloseWindow();
    return 0;
}

// WRITTEN BY JUPYTER, 2026 