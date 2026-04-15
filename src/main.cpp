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
    int displayValue;  // Current animated display value
    float animProgress;  // Animation progress (0.0 to 1.0)
};

/// @struct MetricAnimation - Tracks animation timing
struct MetricAnimation {
    bool active;
    float duration;  // Total animation duration in seconds
    float elapsed;   // Time elapsed in current animation
    
    MetricAnimation() : active(false), duration(0.6f), elapsed(0.0f) {}
    
    void Start() {
        active = true;
        elapsed = 0.0f;
    }
    
    void Update(float deltaTime) {
        if (active) {
            elapsed += deltaTime;
            if (elapsed >= duration) {
                active = false;
                elapsed = duration;
            }
        }
    }
    
    float GetProgress() const {
        if (duration <= 0) return 1.0f;
        return std::min(elapsed / duration, 1.0f);
    }
};

/// @struct FunctionInfo - Detailed information about a single function
struct FunctionInfo {
    std::string signature;  // Function name/signature
    int startLine;
    int endLine;
    int lineCount;
    int nestingDepth;
    int paramCount;
    bool isRecursive;
    int callCount;  // Approximate: how many times it appears to call other functions
    std::string returnType;
};

/// @struct CodeSmell - Detected code smells and quality issues
struct CodeSmell {
    std::string type;     // e.g., "Magic Number", "String Concatenation in Loop"
    int lineNumber;
    std::string description;
    int severity;         // 1-5 (5 = critical)
};

/// @struct TodoComment - Tracks TODO/FIXME comments
struct TodoComment {
    std::string type;     // "TODO" or "FIXME"
    int lineNumber;
    std::string text;
};

/// @struct CodePattern - Detected code patterns and potential issues
struct CodePattern {
    std::string name;
    std::string description;
    int occurrences;
    bool isAntiPattern;  // true if it's a bad pattern
};

/// @struct DetailedAnalysis - Complex multi-level code analysis
struct DetailedAnalysis {
    std::vector<FunctionInfo> functions;
    std::vector<CodePattern> patterns;
    std::vector<CodeSmell> codeSmells;
    std::vector<TodoComment> todos;
    
    // Structural elements
    int includeCount;
    int defineCount;
    int globalVarCount;
    int templateCount;
    int typedefCount;
    int namespaceCount;
    int classCount;
    int structCount;
    
    // Line analysis
    float avgLineLength;
    int maxLineLength;
    int longLineCount;  // Lines > 100 chars
    
    // Complexity metrics
    int nestedLoopCount;
    int tripleNestedCount;  // Loops nested 3+ levels
    int conditionalCount;
    int switchStatementCount;
    int tryBlockCount;      // try/catch blocks
    
    // Memory management
    int pointerUsageCount;
    int dynamicAllocCount;  // new/malloc
    int referenceCount;      // & references
    
    // Advanced patterns
    int recursionCount;     // Detected recursive calls
    int castCount;          // Type casts (potential issues)
    int magicNumberCount;   // Numeric literals
    int hardcodedStringCount; // String literals
    int ternaryOpCount;     // Ternary operators
    int bitwiseOpCount;     // Bitwise operations
    
    // Inheritance & OOP
    int inheritanceCount;   // Detected inheritance
    int virtualFuncCount;   // virtual functions
    int overloadCount;      // Function overloads
    int operatorOverloadCount; // operator overloads
    
    // Performance indicators
    int stringConcatInLoop; // += in loops (bad)
    int vectorPushInLoop;   // .push_back in loops
    int recursiveDepth;     // Max recursion depth detected
    int loopBreakCount;     // Early exits from loops
};

// --- CONSTANTS --- //

const int screenWidth  = 1000;
const int screenHeight = 860;

#define MAX_METRICS 40

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
std::string lastAnalyzedFile;
std::vector<Metric> metrics;
DetailedAnalysis detailedAnalysis;
MetricAnimation metricAnim;
bool exportMenuOpen = false;
Rectangle exportMenuBtns[2];

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

/// @brief Full analysis with enhanced metrics
void AnalyzeCode(const char* filePath, std::vector<Metric>& out, std::string& errMsg) {
    out.clear();
    errMsg.clear();
    detailedAnalysis = DetailedAnalysis();

    std::ifstream file(filePath);
    if (!file.is_open()) {
        errMsg = "Could not open file: " + std::string(filePath);
        return;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    long fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    int fileSizeKB = (int)((fileSize + 512) / 1024);

    int totalLines    = 0;
    int blankLines    = 0;
    int commentLines  = 0;
    int codeLines     = 0;
    int functionCount = 0;
    int maxNestingDepth = 0;
    int currentNestDepth = 0;
    int totalBraces = 0;
    int bracketBalance = 0;
    bool inBlockComment = false;
    std::vector<int> functionLengths;
    int functionStartLine = 0;
    long totalLineLength = 0;

    std::string line;
    while (std::getline(file, line)) {
        totalLines++;

        // Track line length
        totalLineLength += line.length();
        if (line.length() > 100) detailedAnalysis.longLineCount++;
        if ((int)line.length() > detailedAnalysis.maxLineLength)
            detailedAnalysis.maxLineLength = (int)line.length();

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

        // Track TODO/FIXME comments
        if (trimmed.find("TODO") != std::string::npos) {
            TodoComment todo;
            todo.type = "TODO";
            todo.lineNumber = totalLines;
            todo.text = trimmed.substr(0, 70);
            detailedAnalysis.todos.push_back(todo);
        }
        if (trimmed.find("FIXME") != std::string::npos) {
            TodoComment todo;
            todo.type = "FIXME";
            todo.lineNumber = totalLines;
            todo.text = trimmed.substr(0, 70);
            detailedAnalysis.todos.push_back(todo);
        }

        // Count includes and defines
        if (trimmed.rfind("#include", 0) == 0) detailedAnalysis.includeCount++;
        if (trimmed.rfind("#define", 0) == 0) detailedAnalysis.defineCount++;
        if (trimmed.rfind("namespace", 0) == 0) detailedAnalysis.namespaceCount++;
        if (trimmed.rfind("class ", 0) == 0) detailedAnalysis.classCount++;
        if (trimmed.rfind("struct ", 0) == 0) detailedAnalysis.structCount++;
        if (trimmed.rfind("typedef", 0) == 0) detailedAnalysis.typedefCount++;
        if (trimmed.rfind("template", 0) == 0) detailedAnalysis.templateCount++;
        
        // Count OOP patterns
        if (trimmed.find("virtual ") != std::string::npos) detailedAnalysis.virtualFuncCount++;
        if (trimmed.find(": public") != std::string::npos || trimmed.find(": private") != std::string::npos) 
            detailedAnalysis.inheritanceCount++;
        if (trimmed.find("operator ") != std::string::npos && trimmed.find("operator") == trimmed.rfind("operator"))
            detailedAnalysis.operatorOverloadCount++;

        // Count control flow
        if (trimmed.find("if ") != std::string::npos || trimmed.find("if(") != std::string::npos)
            detailedAnalysis.conditionalCount++;
        if (trimmed.find("switch") != std::string::npos) detailedAnalysis.switchStatementCount++;
        if (trimmed.find("try {") != std::string::npos) detailedAnalysis.tryBlockCount++;
        if (trimmed.find("?") != std::string::npos && trimmed.find(":") != std::string::npos)
            detailedAnalysis.ternaryOpCount++;

        // Count nested loops (for/while inside for/while)
        int loopOccurrences = 0;
        if (trimmed.find("for ") != std::string::npos || trimmed.find("for(") != std::string::npos ||
            trimmed.find("while ") != std::string::npos || trimmed.find("while(") != std::string::npos) {
            loopOccurrences++;
        }
        if (loopOccurrences > 0) {
            if (currentNestDepth > 1) detailedAnalysis.nestedLoopCount++;
            if (currentNestDepth > 2) detailedAnalysis.tripleNestedCount++;
            
            // Performance issue: string concat or vector ops in loop
            if (trimmed.find("+=") != std::string::npos && trimmed.find("\"") != std::string::npos) {
                CodeSmell smell;
                smell.type = "String Concatenation in Loop";
                smell.lineNumber = totalLines;
                smell.description = "Using += for strings in loops is inefficient";
                smell.severity = 3;
                detailedAnalysis.codeSmells.push_back(smell);
                detailedAnalysis.stringConcatInLoop++;
            }
            if (trimmed.find("push_back") != std::string::npos) {
                detailedAnalysis.vectorPushInLoop++;
            }
        }

        // Count pointer and dynamic allocation usage
        if (trimmed.find("*") != std::string::npos) detailedAnalysis.pointerUsageCount++;
        if (trimmed.find("&") != std::string::npos) detailedAnalysis.referenceCount++;
        if (trimmed.find("new ") != std::string::npos || trimmed.find("malloc") != std::string::npos ||
            trimmed.find("delete ") != std::string::npos || trimmed.find("free(") != std::string::npos) {
            detailedAnalysis.dynamicAllocCount++;
        }

        // Detect bitwise operations and casts
        if (trimmed.find("<<") != std::string::npos || trimmed.find(">>") != std::string::npos ||
            trimmed.find("&") != std::string::npos || trimmed.find("|") != std::string::npos) {
            detailedAnalysis.bitwiseOpCount++;
        }
        if (trimmed.find("(") != std::string::npos && (trimmed.find("int)") != std::string::npos ||
            trimmed.find("float)") != std::string::npos || trimmed.find("double)") != std::string::npos ||
            trimmed.find("char)") != std::string::npos || trimmed.find("*") != std::string::npos)) {
            detailedAnalysis.castCount++;
        }

        // Count magic numbers (numeric literals)
        for (int i = 0; i < (int)trimmed.length(); i++) {
            if (std::isdigit(trimmed[i]) && (i == 0 || !std::isalpha(trimmed[i-1]))) {
                detailedAnalysis.magicNumberCount++;
                break;  // Count once per line
            }
        }

        // Count string literals
        if (trimmed.find("\"") != std::string::npos) {
            detailedAnalysis.hardcodedStringCount += std::count(trimmed.begin(), trimmed.end(), '"') / 2;
        }

        // Count braces for nesting depth
        for (char c : trimmed) {
            if (c == '{') { 
                currentNestDepth++; 
                totalBraces++;
                bracketBalance++;
                if (currentNestDepth > maxNestingDepth) 
                    maxNestingDepth = currentNestDepth;
            }
            else if (c == '}') { 
                currentNestDepth--; 
                if (currentNestDepth < 0) currentNestDepth = 0;
                bracketBalance--;
            }
        }

        // Heuristic: function definition
        if (trimmed.find('(') != std::string::npos) {
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
                    FunctionInfo func;
                    func.signature = trimmed.substr(0, 60);  // First 60 chars
                    func.startLine = totalLines;
                    func.nestingDepth = currentNestDepth;
                    // Count approximate params
                    size_t parenStart = trimmed.find('(');
                    size_t parenEnd = trimmed.find(')');
                    if (parenStart != std::string::npos && parenEnd != std::string::npos) {
                        std::string params = trimmed.substr(parenStart + 1, parenEnd - parenStart - 1);
                        func.paramCount = std::count(params.begin(), params.end(), ',') + (params.length() > 0 && params.find_first_not_of(" \t") != std::string::npos ? 1 : 0);
                    }
                    detailedAnalysis.functions.push_back(func);
                    functionCount++;
                    functionStartLine = totalLines;
                }
            }
        }

        // Track break statements and recursion
        if (trimmed.rfind("break", 0) == 0 || trimmed.find(";break;") != std::string::npos ||
            trimmed.find("; break;") != std::string::npos) {
            detailedAnalysis.loopBreakCount++;
        }
        
        // Detect direct recursion (function calling itself)
        if (functionStartLine > 0 && !detailedAnalysis.functions.empty()) {
            // Simple heuristic: look for function name pattern in line
            std::string& lastFuncSig = detailedAnalysis.functions.back().signature;
            if (!lastFuncSig.empty()) {
                size_t nameStart = lastFuncSig.find_last_of(" \t");
                if (nameStart != std::string::npos && nameStart + 1 < lastFuncSig.length()) {
                    std::string funcName = lastFuncSig.substr(nameStart + 1);
                    size_t parenPos = funcName.find('(');
                    if (parenPos != std::string::npos) {
                        funcName = funcName.substr(0, parenPos);
                        if (trimmed.find(funcName + "(") != std::string::npos && 
                            trimmed.find("//") == std::string::npos) {
                            detailedAnalysis.functions.back().isRecursive = true;
                            detailedAnalysis.recursionCount++;
                        }
                    }
                }
            }
        }

        // Track function end
        if (trimmed.find('}') != std::string::npos && functionStartLine > 0 && functionCount > 0) {
            functionLengths.push_back(totalLines - functionStartLine);
            if (!detailedAnalysis.functions.empty()) {
                detailedAnalysis.functions.back().endLine = totalLines;
                detailedAnalysis.functions.back().lineCount = totalLines - functionStartLine;
            }
            functionStartLine = 0;
        }
    }

    file.close();

    // Calculate derived metrics
    detailedAnalysis.avgLineLength = (totalLines > 0) ? (float)totalLineLength / totalLines : 0;
    detailedAnalysis.globalVarCount = (codeLines - commentLines - functionCount) / 4;  // Rough estimate
    
    // Detect recursion depth (rough estimate)
    if (!detailedAnalysis.functions.empty()) {
        for (const auto& f : detailedAnalysis.functions) {
            if (f.isRecursive && f.nestingDepth > detailedAnalysis.recursiveDepth)
                detailedAnalysis.recursiveDepth = f.nestingDepth;
        }
    }

    // Detect patterns
    if (detailedAnalysis.pointerUsageCount > codeLines * 0.3f) {
        CodePattern p;
        p.name = "Heavy Pointer Usage";
        p.description = "Frequent use of pointers may indicate C-style memory management";
        p.occurrences = detailedAnalysis.pointerUsageCount;
        p.isAntiPattern = true;
        detailedAnalysis.patterns.push_back(p);
    }
    if (detailedAnalysis.longLineCount > codeLines * 0.1f) {
        CodePattern p;
        p.name = "Long Lines";
        p.description = "Many lines exceed 100 characters (readability issue)";
        p.occurrences = detailedAnalysis.longLineCount;
        p.isAntiPattern = true;
        detailedAnalysis.patterns.push_back(p);
    }
    if (maxNestingDepth > 4) {
        CodePattern p;
        p.name = "Deep Nesting";
        p.description = "Deep nesting levels indicate complex control flow";
        p.occurrences = maxNestingDepth;
        p.isAntiPattern = true;
        detailedAnalysis.patterns.push_back(p);
    }
    if (detailedAnalysis.nestedLoopCount > functionCount * 0.2f) {
        CodePattern p;
        p.name = "Nested Loops";
        p.description = "Multiple nested loops may impact performance";
        p.occurrences = detailedAnalysis.nestedLoopCount;
        p.isAntiPattern = true;
        detailedAnalysis.patterns.push_back(p);
    }
    if (detailedAnalysis.includeCount > 15) {
        CodePattern p;
        p.name = "High Dependencies";
        p.description = "Many includes suggest high coupling";
        p.occurrences = detailedAnalysis.includeCount;
        p.isAntiPattern = true;
        detailedAnalysis.patterns.push_back(p);
    }

    // Comment ratio as a pseudo-metric (stored as integer percentage)
    int commentRatio = (totalLines > 0) ? (int)(100.0f * commentLines / totalLines) : 0;
    
    // Code density: lines of code per kilobyte
    int codeDensity = (fileSizeKB > 0) ? (int)(codeLines * 1.0f / fileSizeKB) : 0;
    int avgFunctionLength = functionCount > 0 ? (int)(codeLines / (float)functionCount) : 0;
    int codeToCommentRatio = commentLines > 0 ? (int)(codeLines / (float)commentLines) : (commentLines == 0 ? codeLines : 0);
    int cyclomaticEstimate = functionCount + maxNestingDepth;

    out.push_back({"Total Lines",       totalLines,              TEXT_PRIMARY, "", 0, 0.0f});
    out.push_back({"Code Lines",        codeLines,               ACCENT_BLUE,  "", 0, 0.0f});
    out.push_back({"Comment Lines",     commentLines,            ACCENT_GREEN, "", 0, 0.0f});
    out.push_back({"Blank Lines",       blankLines,              TEXT_MUTED,   "", 0, 0.0f});
    out.push_back({"Functions",         functionCount,           ACCENT_ORANGE,"", 0, 0.0f});
    out.push_back({"Max Nesting",       maxNestingDepth,         Color{180, 100, 200, 255}, "", 0, 0.0f});
    out.push_back({"Avg Func Length",   avgFunctionLength,       Color{150, 150, 180, 255}, "", 0, 0.0f});
    out.push_back({"Comment Ratio",     commentRatio,            ACCENT_GREEN, "%", 0, 0.0f});
    out.push_back({"Code:Comment",      codeToCommentRatio,      Color{200, 180, 100, 255}, "", 0, 0.0f});
    out.push_back({"File Size",         fileSizeKB,              ACCENT_BLUE,  "KB", 0, 0.0f});
    out.push_back({"Code Density",      codeDensity,             Color{200, 150, 100, 255}, "loc/KB", 0, 0.0f});
    out.push_back({"Bracket Balance",   bracketBalance == 0 ? 1 : 0, bracketBalance == 0 ? ACCENT_GREEN : ACCENT_RED, "", 0, 0.0f});
    out.push_back({"Complexity Est",    cyclomaticEstimate,      Color{220, 120, 100, 255}, "", 0, 0.0f});
}

/// @brief Export results to JSON file with detailed analysis
void ExportToJSON(const char* filePath, const std::vector<Metric>& metrics) {
    std::ofstream out(filePath);
    if (!out.is_open()) return;

    out << "{\n  \"summary_metrics\": [\n";
    for (int i = 0; i < (int)metrics.size(); i++) {
        out << "    {\n";
        out << "      \"name\": \"" << metrics[i].name << "\",\n";
        out << "      \"value\": " << metrics[i].value << ",\n";
        out << "      \"unit\": \"" << metrics[i].unit << "\"\n";
        out << "    }";
        if (i < (int)metrics.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    out << "  \"detailed_analysis\": {\n";
    out << "    \"includes\": " << detailedAnalysis.includeCount << ",\n";
    out << "    \"defines\": " << detailedAnalysis.defineCount << ",\n";
    out << "    \"namespaces\": " << detailedAnalysis.namespaceCount << ",\n";
    out << "    \"classes\": " << detailedAnalysis.classCount << ",\n";
    out << "    \"structs\": " << detailedAnalysis.structCount << ",\n";
    out << "    \"typedefs\": " << detailedAnalysis.typedefCount << ",\n";
    out << "    \"templates\": " << detailedAnalysis.templateCount << ",\n";
    out << "    \"avg_line_length\": " << (int)detailedAnalysis.avgLineLength << ",\n";
    out << "    \"max_line_length\": " << detailedAnalysis.maxLineLength << ",\n";
    out << "    \"long_lines\": " << detailedAnalysis.longLineCount << ",\n";
    out << "    \"conditionals\": " << detailedAnalysis.conditionalCount << ",\n";
    out << "    \"switch_statements\": " << detailedAnalysis.switchStatementCount << ",\n";
    out << "    \"try_blocks\": " << detailedAnalysis.tryBlockCount << ",\n";
    out << "    \"nested_loops\": " << detailedAnalysis.nestedLoopCount << ",\n";
    out << "    \"triple_nested_loops\": " << detailedAnalysis.tripleNestedCount << ",\n";
    out << "    \"pointer_usage\": " << detailedAnalysis.pointerUsageCount << ",\n";
    out << "    \"references\": " << detailedAnalysis.referenceCount << ",\n";
    out << "    \"dynamic_allocations\": " << detailedAnalysis.dynamicAllocCount << ",\n";
    out << "    \"recursion_count\": " << detailedAnalysis.recursionCount << ",\n";
    out << "    \"recursive_depth\": " << detailedAnalysis.recursiveDepth << ",\n";
    out << "    \"type_casts\": " << detailedAnalysis.castCount << ",\n";
    out << "    \"magic_numbers\": " << detailedAnalysis.magicNumberCount << ",\n";
    out << "    \"hardcoded_strings\": " << detailedAnalysis.hardcodedStringCount << ",\n";
    out << "    \"ternary_operators\": " << detailedAnalysis.ternaryOpCount << ",\n";
    out << "    \"bitwise_operations\": " << detailedAnalysis.bitwiseOpCount << ",\n";
    out << "    \"virtual_functions\": " << detailedAnalysis.virtualFuncCount << ",\n";
    out << "    \"inheritance_count\": " << detailedAnalysis.inheritanceCount << ",\n";
    out << "    \"operator_overloads\": " << detailedAnalysis.operatorOverloadCount << ",\n";
    out << "    \"string_concat_in_loop\": " << detailedAnalysis.stringConcatInLoop << ",\n";
    out << "    \"vector_push_in_loop\": " << detailedAnalysis.vectorPushInLoop << ",\n";
    out << "    \"loop_breaks\": " << detailedAnalysis.loopBreakCount << "\n";
    out << "  },\n";

    out << "  \"functions\": [\n";
    for (int i = 0; i < (int)detailedAnalysis.functions.size(); i++) {
        const auto& f = detailedAnalysis.functions[i];
        out << "    {\n";
        out << "      \"signature\": \"" << f.signature << "\",\n";
        out << "      \"start_line\": " << f.startLine << ",\n";
        out << "      \"end_line\": " << f.endLine << ",\n";
        out << "      \"length\": " << f.lineCount << ",\n";
        out << "      \"nesting_depth\": " << f.nestingDepth << ",\n";
        out << "      \"param_count\": " << f.paramCount << ",\n";
        out << "      \"is_recursive\": " << (f.isRecursive ? "true" : "false") << "\n";
        out << "    }";
        if (i < (int)detailedAnalysis.functions.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    out << "  \"patterns_detected\": [\n";
    for (int i = 0; i < (int)detailedAnalysis.patterns.size(); i++) {
        const auto& p = detailedAnalysis.patterns[i];
        out << "    {\n";
        out << "      \"name\": \"" << p.name << "\",\n";
        out << "      \"description\": \"" << p.description << "\",\n";
        out << "      \"occurrences\": " << p.occurrences << ",\n";
        out << "      \"is_antipattern\": " << (p.isAntiPattern ? "true" : "false") << "\n";
        out << "    }";
        if (i < (int)detailedAnalysis.patterns.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    out << "  \"code_smells\": [\n";
    for (int i = 0; i < (int)detailedAnalysis.codeSmells.size(); i++) {
        const auto& s = detailedAnalysis.codeSmells[i];
        out << "    {\n";
        out << "      \"type\": \"" << s.type << "\",\n";
        out << "      \"line\": " << s.lineNumber << ",\n";
        out << "      \"description\": \"" << s.description << "\",\n";
        out << "      \"severity\": " << s.severity << "\n";
        out << "    }";
        if (i < (int)detailedAnalysis.codeSmells.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    out << "  \"todos\": [\n";
    for (int i = 0; i < (int)detailedAnalysis.todos.size(); i++) {
        const auto& t = detailedAnalysis.todos[i];
        out << "    {\n";
        out << "      \"type\": \"" << t.type << "\",\n";
        out << "      \"line\": " << t.lineNumber << ",\n";
        out << "      \"text\": \"" << t.text << "\"\n";
        out << "    }";
        if (i < (int)detailedAnalysis.todos.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    out.close();
}

/// @brief Export results to CSV file
void ExportToCSV(const char* filePath, const std::vector<Metric>& metrics) {
    std::ofstream out(filePath);
    if (!out.is_open()) return;

    out << "Metric,Value,Unit\n";
    for (const auto& m : metrics) {
        out << m.name << "," << m.value << "," << m.unit << "\n";
    }
    
    out << "\n--- DETAILED ANALYSIS ---\n";
    out << "Includes," << detailedAnalysis.includeCount << ",\n";
    out << "Defines," << detailedAnalysis.defineCount << ",\n";
    out << "Namespaces," << detailedAnalysis.namespaceCount << ",\n";
    out << "Classes," << detailedAnalysis.classCount << ",\n";
    out << "Structs," << detailedAnalysis.structCount << ",\n";
    out << "Typedefs," << detailedAnalysis.typedefCount << ",\n";
    out << "Templates," << detailedAnalysis.templateCount << ",\n";
    out << "Avg Line Length," << (int)detailedAnalysis.avgLineLength << ",\n";
    out << "Max Line Length," << detailedAnalysis.maxLineLength << ",\n";
    out << "Long Lines," << detailedAnalysis.longLineCount << ",\n";
    out << "Conditionals," << detailedAnalysis.conditionalCount << ",\n";
    out << "Switch Statements," << detailedAnalysis.switchStatementCount << ",\n";
    out << "Try Blocks," << detailedAnalysis.tryBlockCount << ",\n";
    out << "Nested Loops," << detailedAnalysis.nestedLoopCount << ",\n";
    out << "Triple Nested Loops," << detailedAnalysis.tripleNestedCount << ",\n";
    out << "Pointer Usage," << detailedAnalysis.pointerUsageCount << ",\n";
    out << "References," << detailedAnalysis.referenceCount << ",\n";
    out << "Dynamic Allocations," << detailedAnalysis.dynamicAllocCount << ",\n";
    out << "Recursion Count," << detailedAnalysis.recursionCount << ",\n";
    out << "Recursive Depth," << detailedAnalysis.recursiveDepth << ",\n";
    out << "Type Casts," << detailedAnalysis.castCount << ",\n";
    out << "Magic Numbers," << detailedAnalysis.magicNumberCount << ",\n";
    out << "Hardcoded Strings," << detailedAnalysis.hardcodedStringCount << ",\n";
    out << "Ternary Operators," << detailedAnalysis.ternaryOpCount << ",\n";
    out << "Bitwise Operations," << detailedAnalysis.bitwiseOpCount << ",\n";
    out << "Virtual Functions," << detailedAnalysis.virtualFuncCount << ",\n";
    out << "Inheritance Count," << detailedAnalysis.inheritanceCount << ",\n";
    out << "Operator Overloads," << detailedAnalysis.operatorOverloadCount << ",\n";
    out << "String Concat in Loop," << detailedAnalysis.stringConcatInLoop << ",\n";
    out << "Vector Push in Loop," << detailedAnalysis.vectorPushInLoop << ",\n";
    out << "Loop Breaks," << detailedAnalysis.loopBreakCount << ",\n";
    
    out << "\n--- FUNCTIONS ---\n";
    out << "Signature,Start,End,Length,Nesting,Params,Recursive\n";
    for (const auto& f : detailedAnalysis.functions) {
        out << "\"" << f.signature << "\"," << f.startLine << "," << f.endLine << "," 
            << f.lineCount << "," << f.nestingDepth << "," << f.paramCount << ","
            << (f.isRecursive ? "Yes" : "No") << "\n";
    }
    
    out << "\n--- PATTERNS DETECTED ---\n";
    out << "Pattern,Occurrences,Is_Antipattern,Description\n";
    for (const auto& p : detailedAnalysis.patterns) {
        out << "\"" << p.name << "\"," << p.occurrences << "," 
            << (p.isAntiPattern ? "Yes" : "No") << ",\"" << p.description << "\"\n";
    }
    
    out << "\n--- CODE SMELLS ---\n";
    out << "Type,Line,Severity,Description\n";
    for (const auto& s : detailedAnalysis.codeSmells) {
        out << "\"" << s.type << "\"," << s.lineNumber << "," << s.severity << ",\"" << s.description << "\"\n";
    }
    
    out << "\n--- TODO/FIXME COMMENTS ---\n";
    out << "Type,Line,Text\n";
    for (const auto& t : detailedAnalysis.todos) {
        out << t.type << "," << t.lineNumber << ",\"" << t.text << "\"\n";
    }
    out.close();
}


int main() {

    InitWindow(screenWidth, screenHeight, "JCAT");
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
    const char* langs[]  = {
        (const char*)"C++",
        (const char*)"Java"
    };

    const int   langCount = 2;
    int  selectedLang  = -1;
    bool dropdownOpen  = false;

    Rectangle dropdownBtn = {20, 130, 200, 32};
    Rectangle dropdownOptions[2];
    for (int i = 0; i < langCount; i++)
        dropdownOptions[i] = {20, 162.0f + i * 30.0f, 200, 30};

    // --- FILE PATH INPUT --- //
    char filePath[512] = {0};
    bool typing        = false;
    Rectangle inputBox = {20, 260, 560, 34};

    // --- ANALYZE BUTTON --- //
    Rectangle analyzeBtn = {20, 314, 160, 40};
    
    // --- EXPORT BUTTONS --- //
    Rectangle exportBtn = {190, 314, 100, 40};
    Rectangle jsonExportBtn = {190, 354, 100, 30};
    Rectangle csvExportBtn = {295, 354, 100, 30};

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
                    lastAnalyzedFile = filePath;

                    analyzeState = errorMessage.empty() ? STATE_DONE : STATE_ERROR;
                    if (analyzeState == STATE_DONE) {
                        metricAnim.Start();  // Start animation when analysis completes
                    }
                }
            }

            // Export button toggle
            if (CheckCollisionPointRec(mouse, exportBtn) && analyzeState == STATE_DONE) {
                exportMenuOpen = !exportMenuOpen;
            }

            // JSON export
            if (exportMenuOpen && CheckCollisionPointRec(mouse, jsonExportBtn)) {
                std::string jsonPath = lastAnalyzedFile + ".metrics.json";
                ExportToJSON(jsonPath.c_str(), metrics);
                exportMenuOpen = false;
            }

            // CSV export
            if (exportMenuOpen && CheckCollisionPointRec(mouse, csvExportBtn)) {
                std::string csvPath = lastAnalyzedFile + ".metrics.csv";
                ExportToCSV(csvPath.c_str(), metrics);
                exportMenuOpen = false;
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

        // --- UPDATE ANIMATIONS --- //
        metricAnim.Update(GetFrameTime());
        if (analyzeState == STATE_DONE) {
            float progress = metricAnim.GetProgress();
            for (int i = 0; i < (int)metrics.size(); i++) {
                int targetVal = metrics[i].value;
                metrics[i].displayValue = (int)(targetVal * progress);
                metrics[i].animProgress = progress;
            }
        }

        // ---- DRAW ---- //
        BeginDrawing();
        ClearBackground(BG_COLOR);

        // ── HEADER ──
        DrawTextEx(t, "JCAT", {20, 20}, 26, 2, TEXT_PRIMARY);
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

        // ── EXPORT BUTTON ──
        if (analyzeState == STATE_DONE) {
            bool expHov = CheckCollisionPointRec(mouse, exportBtn);
            Color expFill = expHov ? ACCENT_HOVER : Color{50, 120, 180, 255};
            DrawCard(exportBtn, expFill, BORDER_LIT);
            DrawTextEx(d, "Export", {exportBtn.x + 15, exportBtn.y + 12}, 14, 1, WHITE);
            DrawTextEx(d, exportMenuOpen ? "^" : "v", {exportBtn.x + 75, exportBtn.y + 12}, 12, 1, TEXT_MUTED);

            // Export menu
            if (exportMenuOpen) {
                DrawCard(jsonExportBtn, Color{45, 75, 120, 255}, BORDER_DIM);
                DrawTextEx(d, "JSON", {jsonExportBtn.x + 25, jsonExportBtn.y + 8}, 13, 1, TEXT_PRIMARY);

                DrawCard(csvExportBtn, Color{45, 75, 120, 255}, BORDER_DIM);
                DrawTextEx(d, "CSV", {csvExportBtn.x + 30, csvExportBtn.y + 8}, 13, 1, TEXT_PRIMARY);
            }
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

            float cardW = 300, cardH = 72;
            float gapX  = 20,  gapY  = 12;
            float startX = 20;

            for (int i = 0; i < (int)metrics.size(); i++) {
                int col = i % 3;
                int row = i / 3;
                float cx = startX + col * (cardW + gapX);
                float cy = metricsY + row * (cardH + gapY);

                Rectangle card = {cx, cy, cardW, cardH};
                DrawCard(card, CARD_COLOR, BORDER_DIM);

                // Metric name
                DrawTextEx(d, metrics[i].name.c_str(), {cx + 12, cy + 10}, 13, 1, TEXT_MUTED);

                // Value (animated)
                std::string valStr = std::to_string(metrics[i].displayValue) + metrics[i].unit;
                DrawTextEx(d, valStr.c_str(), {cx + 12, cy + 28}, 22, 1, metrics[i].color);

                // Progress bar (skip ratio metric — already in %)
                if (metrics[i].unit.empty()) {
                    Rectangle barBg  = {cx + 12, cy + 56, cardW - 24, 6};
                    float frac = (float)metrics[i].displayValue / (float)maxVal;
                    DrawBar(barBg, frac, metrics[i].color, Color{45, 45, 45, 255});
                } else {
                    // For percentage: show bar scaled 0–100
                    Rectangle barBg  = {cx + 12, cy + 56, cardW - 24, 6};
                    DrawBar(barBg, metrics[i].displayValue / 100.0f, metrics[i].color, Color{45, 45, 45, 255});
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
// Test 1

// JCAT - Jupyter's Code Analysis Tool, 2026 (V0.6)