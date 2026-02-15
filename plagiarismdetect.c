#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_FILE_SIZE 100000
#define MAX_TOKENS 10000
#define MAX_TOKEN_LEN 256
#define MAX_KEYWORDS 50

/* ==================== TOKEN STRUCTURE ==================== */
typedef enum {
    KEYWORD, IDENTIFIER, OPERATOR, LITERAL, 
    SEPARATOR, COMMENT, WHITESPACE, UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LEN];
} Token;

typedef struct {
    Token tokens[MAX_TOKENS];
    int count;
} TokenList;

/* ==================== KEYWORD CHECKER ==================== */
const char* keywords[] = {
    "auto", "break", "case", "char", "const", "continue", 
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "int", "long", "register",
    "return", "short", "signed", "sizeof", "static", "struct",
    "switch", "typedef", "union", "unsigned", "void", "volatile",
    "while", "inline", NULL
};

int isKeyword(const char* str) {
    int idx;
    for (idx = 0; keywords[idx] != NULL; idx++) {
        if (strcmp(str, keywords[idx]) == 0) {
            return 1;
        }
    }
    return 0;
}

int isOperatorChar(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '=' || c == '<' || c == '>' || c == '!' || c == '&' ||
            c == '|' || c == '^' || c == '~' || c == '.');
}

int isSeparator(char c) {
    return (c == '(' || c == ')' || c == '{' || c == '}' || 
            c == '[' || c == ']' || c == ';' || c == ',' || c == ':');
}

/* ==================== TOKENIZER ==================== */
void addToken(TokenList* list, TokenType type, const char* value) {
    if (list->count >= MAX_TOKENS) return;
    list->tokens[list->count].type = type;
    strncpy(list->tokens[list->count].value, value, MAX_TOKEN_LEN - 1);
    list->tokens[list->count].value[MAX_TOKEN_LEN - 1] = '\0';
    list->count++;
}

TokenList* tokenize(const char* code) {
    TokenList* list;
    int len, idx, j;
    char id[MAX_TOKEN_LEN];
    char op[3];
    char sep[2];
    char quote;
    
    list = (TokenList*)malloc(sizeof(TokenList));
    list->count = 0;
    
    len = strlen(code);
    idx = 0;
    
    while (idx < len) {
        /* Skip whitespace */
        if (isspace(code[idx])) {
            idx++;
            continue;
        }
        
        /* Single-line comment */
        if (idx + 1 < len && code[idx] == '/' && code[idx+1] == '/') {
            idx += 2;
            while (idx < len && code[idx] != '\n') idx++;
            continue;
        }
        
        /* Multi-line comment */
        if (idx + 1 < len && code[idx] == '/' && code[idx+1] == '*') {
            idx += 2;
            while (idx + 1 < len && !(code[idx] == '*' && code[idx+1] == '/')) idx++;
            idx += 2;
            continue;
        }
        
        /* String literal */
        if (code[idx] == '"' || code[idx] == '\'') {
            quote = code[idx];
            idx++;
            while (idx < len && code[idx] != quote) {
                if (code[idx] == '\\' && idx + 1 < len) idx++;
                idx++;
            }
            if (idx < len) idx++;
            addToken(list, LITERAL, "STR");
            continue;
        }
        
        /* Number literal */
        if (isdigit(code[idx])) {
            while (idx < len && (isdigit(code[idx]) || code[idx] == '.')) idx++;
            addToken(list, LITERAL, "NUM");
            continue;
        }
        
        /* Identifier or keyword */
        if (isalpha(code[idx]) || code[idx] == '_') {
            j = 0;
            while (idx < len && (isalnum(code[idx]) || code[idx] == '_')) {
                id[j++] = code[idx++];
            }
            id[j] = '\0';
            
            if (isKeyword(id)) {
                addToken(list, KEYWORD, id);
            } else {
                // This is the corrected line
                addToken(list, IDENTIFIER, id); 
            }
            continue;
        }
        
        /* Operators */
        if (isOperatorChar(code[idx])) {
            op[0] = code[idx];
            op[1] = '\0';
            op[2] = '\0';
            if (idx + 1 < len && isOperatorChar(code[idx+1])) {
                op[1] = code[idx+1];
                idx += 2;
            } else {
                idx++;
            }
            addToken(list, OPERATOR, op);
            continue;
        }
        
        /* Separators */
        if (isSeparator(code[idx])) {
            sep[0] = code[idx];
            sep[1] = '\0';
            addToken(list, SEPARATOR, sep);
            idx++;
            continue;
        }
        
        idx++;
    }
    
    return list;
}

/* ==================== STRUCTURE EXTRACTION ==================== */
typedef struct {
    char items[MAX_TOKENS][MAX_TOKEN_LEN];
    int count;
} StructureList;

StructureList* extractStructure(TokenList* tokens) {
    StructureList* structure;
    int idx;
    
    structure = (StructureList*)malloc(sizeof(StructureList));
    structure->count = 0;
    
    for (idx = 0; idx < tokens->count; idx++) {
        if (structure->count >= MAX_TOKENS) break;
        
        if (tokens->tokens[idx].type == KEYWORD) {
            strcpy(structure->items[structure->count++], tokens->tokens[idx].value);
        } else if (tokens->tokens[idx].type == SEPARATOR) {
            if (strcmp(tokens->tokens[idx].value, "{") == 0) {
                strcpy(structure->items[structure->count++], "BLOCK_START");
            } else if (strcmp(tokens->tokens[idx].value, "}") == 0) {
                strcpy(structure->items[structure->count++], "BLOCK_END");
            } else if (strcmp(tokens->tokens[idx].value, "(") == 0) {
                strcpy(structure->items[structure->count++], "PAREN_OPEN");
            } else if (strcmp(tokens->tokens[idx].value, ")") == 0) {
                strcpy(structure->items[structure->count++], "PAREN_CLOSE");
            }
        } else if (tokens->tokens[idx].type == OPERATOR) {
            strcpy(structure->items[structure->count++], "OP");
        }
    }
    
    return structure;
}

/* ==================== LCS SIMILARITY ==================== */
double lcsDistance(TokenList* t1, TokenList* t2) {
    int m, n, lcs;
    int** dp;
    int i1, j1;
    double result;
    
    m = t1->count;
    n = t2->count;
    if (m == 0 || n == 0) return 0.0;
    
    dp = (int**)malloc((m + 1) * sizeof(int*));
    for (i1 = 0; i1 <= m; i1++) {
        dp[i1] = (int*)calloc(n + 1, sizeof(int));
    }
    
    for (i1 = 1; i1 <= m; i1++) {
        for (j1 = 1; j1 <= n; j1++) {
            if (strcmp(t1->tokens[i1-1].value, t2->tokens[j1-1].value) == 0 &&
                t1->tokens[i1-1].type == t2->tokens[j1-1].type) {
                dp[i1][j1] = dp[i1-1][j1-1] + 1;
            } else {
                dp[i1][j1] = (dp[i1-1][j1] > dp[i1][j1-1]) ? dp[i1-1][j1] : dp[i1][j1-1];
            }
        }
    }
    
    lcs = dp[m][n];
    
    for (i1 = 0; i1 <= m; i1++) {
        free(dp[i1]);
    }
    free(dp);
    
    result = (2.0 * lcs) / (m + n);
    return result;
}

/* ==================== EDIT DISTANCE ==================== */
int min3(int a, int b, int c) {
    int min = a;
    if (b < min) min = b;
    if (c < min) min = c;
    return min;
}

double editDistance(TokenList* t1, TokenList* t2) {
    int m, n, maxLen, cost;
    int** dp;
    int i2, j2;
    double result;
    
    m = t1->count;
    n = t2->count;
    if (m == 0) return n == 0 ? 1.0 : 0.0;
    if (n == 0) return 0.0;
    
    dp = (int**)malloc((m + 1) * sizeof(int*));
    for (i2 = 0; i2 <= m; i2++) {
        dp[i2] = (int*)malloc((n + 1) * sizeof(int));
    }
    
    for (i2 = 0; i2 <= m; i2++) dp[i2][0] = i2;
    for (j2 = 0; j2 <= n; j2++) dp[0][j2] = j2;
    
    for (i2 = 1; i2 <= m; i2++) {
        for (j2 = 1; j2 <= n; j2++) {
            cost = (strcmp(t1->tokens[i2-1].value, t2->tokens[j2-1].value) == 0 &&
                       t1->tokens[i2-1].type == t2->tokens[j2-1].type) ? 0 : 1;
            dp[i2][j2] = min3(dp[i2-1][j2] + 1, dp[i2][j2-1] + 1, dp[i2-1][j2-1] + cost);
        }
    }
    
    maxLen = (m > n) ? m : n;
    result = 1.0 - (double)dp[m][n] / maxLen;
    
    for (i2 = 0; i2 <= m; i2++) {
        free(dp[i2]);
    }
    free(dp);
    
    return result;
}

/* ==================== STRUCTURE SIMILARITY ==================== */
double structureSimilarity(StructureList* s1, StructureList* s2) {
    int m, n;
    int** dp;
    int i3, j3;
    double result;
    
    if (s1->count == 0 || s2->count == 0) return 0.0;
    
    m = s1->count;
    n = s2->count;
    dp = (int**)malloc((m + 1) * sizeof(int*));
    for (i3 = 0; i3 <= m; i3++) {
        dp[i3] = (int*)calloc(n + 1, sizeof(int));
    }
    
    for (i3 = 1; i3 <= m; i3++) {
        for (j3 = 1; j3 <= n; j3++) {
            if (strcmp(s1->items[i3-1], s2->items[j3-1]) == 0) {
                dp[i3][j3] = dp[i3-1][j3-1] + 1;
            } else {
                dp[i3][j3] = (dp[i3-1][j3] > dp[i3][j3-1]) ? dp[i3-1][j3] : dp[i3][j3-1];
            }
        }
    }
    
    result = (2.0 * dp[m][n]) / (m + n);
    
    for (i3 = 0; i3 <= m; i3++) {
        free(dp[i3]);
    }
    free(dp);
    
    return result;
}

/* ==================== N-GRAM SIMILARITY ==================== */
typedef struct {
    char ngrams[MAX_TOKENS][MAX_TOKEN_LEN * 4];
    int count;
} NgramSet;

void generateNgrams(TokenList* tokens, int n, NgramSet* set) {
    int i4, j4;
    char ngram[MAX_TOKEN_LEN * 4];
    
    set->count = 0;
    for (i4 = 0; i4 <= tokens->count - n; i4++) {
        if (set->count >= MAX_TOKENS) break;
        ngram[0] = '\0';
        for (j4 = 0; j4 < n; j4++) {
            strcat(ngram, tokens->tokens[i4+j4].value);
            strcat(ngram, "|");
        }
        strcpy(set->ngrams[set->count++], ngram);
    }
}

int ngramInSet(const char* ngram, NgramSet* set) {
    int i5;
    for (i5 = 0; i5 < set->count; i5++) {
        if (strcmp(ngram, set->ngrams[i5]) == 0) return 1;
    }
    return 0;
}

double ngramSimilarity(TokenList* t1, TokenList* t2, int n) {
    NgramSet* set1;
    NgramSet* set2;
    int intersection, totalUnique, i6;
    double result;
    
    if (t1->count < n || t2->count < n) return 0.0;
    
    set1 = (NgramSet*)malloc(sizeof(NgramSet));
    set2 = (NgramSet*)malloc(sizeof(NgramSet));
    
    generateNgrams(t1, n, set1);
    generateNgrams(t2, n, set2);
    
    intersection = 0;
    for (i6 = 0; i6 < set1->count; i6++) {
        if (ngramInSet(set1->ngrams[i6], set2)) intersection++;
    }
    
    totalUnique = set1->count + set2->count - intersection;
    result = (totalUnique > 0) ? (double)intersection / totalUnique : 0.0;
    
    free(set1);
    free(set2);
    
    return result;
}

/* ==================== TOKEN FREQUENCY ==================== */
typedef struct {
    char token[MAX_TOKEN_LEN];
    int count;
} FreqItem;

typedef struct {
    FreqItem items[MAX_TOKENS];
    int size;
} FreqMap;

void addToFreqMap(FreqMap* map, const char* token) {
    int i7;
    for (i7 = 0; i7 < map->size; i7++) {
        if (strcmp(map->items[i7].token, token) == 0) {
            map->items[i7].count++;
            return;
        }
    }
    if (map->size < MAX_TOKENS) {
        strcpy(map->items[map->size].token, token);
        map->items[map->size].count = 1;
        map->size++;
    }
}

int getFrequency(FreqMap* map, const char* token) {
    int i8;
    for (i8 = 0; i8 < map->size; i8++) {
        if (strcmp(map->items[i8].token, token) == 0) {
            return map->items[i8].count;
        }
    }
    return 0;
}

double tokenFrequencySimilarity(TokenList* t1, TokenList* t2) {
    FreqMap* freq1;
    FreqMap* freq2;
    double dotProduct, mag1, mag2;
    int i9, f2;
    
    freq1 = (FreqMap*)malloc(sizeof(FreqMap));
    freq2 = (FreqMap*)malloc(sizeof(FreqMap));
    freq1->size = 0;
    freq2->size = 0;
    
    for (i9 = 0; i9 < t1->count; i9++) {
        if (t1->tokens[i9].type == KEYWORD || t1->tokens[i9].type == OPERATOR) {
            addToFreqMap(freq1, t1->tokens[i9].value);
        }
    }
    
    for (i9 = 0; i9 < t2->count; i9++) {
        if (t2->tokens[i9].type == KEYWORD || t2->tokens[i9].type == OPERATOR) {
            addToFreqMap(freq2, t2->tokens[i9].value);
        }
    }
    
    if (freq1->size == 0 || freq2->size == 0) {
        free(freq1);
        free(freq2);
        return 0.0;
    }
    
    dotProduct = 0.0;
    mag1 = 0.0;
    mag2 = 0.0;
    
    for (i9 = 0; i9 < freq1->size; i9++) {
        mag1 += freq1->items[i9].count * freq1->items[i9].count;
        f2 = getFrequency(freq2, freq1->items[i9].token);
        dotProduct += freq1->items[i9].count * f2;
    }
    
    for (i9 = 0; i9 < freq2->size; i9++) {
        mag2 += freq2->items[i9].count * freq2->items[i9].count;
    }
    
    free(freq1);
    free(freq2);
    
    if (mag1 == 0 || mag2 == 0) return 0.0;
    return dotProduct / (sqrt(mag1) * sqrt(mag2));
}

/* ==================== RESULT STRUCTURE ==================== */
typedef struct {
    double tokenSimilarity;
    double structureSimilarity;
    double ngramSimilarity;
    double frequencySimilarity;
    double editSimilarity;
    double overallScore;
} Result;

void displayResult(Result* result) {
    int i10;
    
    printf("\n");
    for (i10 = 0; i10 < 60; i10++) printf("=");
    printf("\n");
    printf("         PLAGIARISM DETECTION RESULTS\n");
    for (i10 = 0; i10 < 60; i10++) printf("=");
    printf("\n\n");
    
    printf("  Token Sequence Similarity (LCS):  %.2f%%\n", result->tokenSimilarity * 100);
    printf("  Structure Similarity:             %.2f%%\n", result->structureSimilarity * 100);
    printf("  N-gram Similarity (3-grams):      %.2f%%\n", result->ngramSimilarity * 100);
    printf("  Token Frequency Similarity:       %.2f%%\n", result->frequencySimilarity * 100);
    printf("  Edit Distance Similarity:         %.2f%%\n", result->editSimilarity * 100);
    printf("\n");
    for (i10 = 0; i10 < 60; i10++) printf("-");
    printf("\n");
    printf("  OVERALL PLAGIARISM SCORE:         %.2f%%\n", result->overallScore * 100);
    for (i10 = 0; i10 < 60; i10++) printf("=");
    printf("\n\n");
    
    if (result->overallScore >= 0.85) {
        printf("  WARNING: HIGH PLAGIARISM - Very likely copied\n");
    } else if (result->overallScore >= 0.70) {
        printf("  WARNING: MODERATE PLAGIARISM - Suspicious similarity\n");
    } else if (result->overallScore >= 0.50) {
        printf("  WARNING: LOW PLAGIARISM - Some similar patterns\n");
    } else {
        printf("  PASS: MINIMAL SIMILARITY - Likely original\n");
    }
    printf("\n");
    for (i10 = 0; i10 < 60; i10++) printf("=");
    printf("\n");
}

/* ==================== FILE READING ==================== */
char* readFile(const char* filename) {
    FILE* file;
    char* buffer;
    size_t size;
    
    file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    buffer = (char*)malloc(MAX_FILE_SIZE);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size = fread(buffer, 1, MAX_FILE_SIZE - 1, file);
    buffer[size] = '\0';
    fclose(file);
    
    return buffer;
}

/* ==================== MAIN COMPARISON ==================== */
Result compare(const char* code1, const char* code2) {
    Result result;
    TokenList* tokens1;
    TokenList* tokens2;
    StructureList* structure1;
    StructureList* structure2;
    
    result.tokenSimilarity = 0;
    result.structureSimilarity = 0;
    result.ngramSimilarity = 0;
    result.frequencySimilarity = 0;
    result.editSimilarity = 0;
    result.overallScore = 0;
    
    tokens1 = tokenize(code1);
    tokens2 = tokenize(code2);
    
    structure1 = extractStructure(tokens1);
    structure2 = extractStructure(tokens2);
    
    result.tokenSimilarity = lcsDistance(tokens1, tokens2);
    result.structureSimilarity = structureSimilarity(structure1, structure2);
    result.ngramSimilarity = ngramSimilarity(tokens1, tokens2, 3);
    result.frequencySimilarity = tokenFrequencySimilarity(tokens1, tokens2);
    result.editSimilarity = editDistance(tokens1, tokens2);
    
    result.overallScore = 
        0.30 * result.tokenSimilarity +
        0.25 * result.structureSimilarity +
        0.20 * result.ngramSimilarity +
        0.15 * result.frequencySimilarity +
        0.10 * result.editSimilarity;
    
    free(tokens1);
    free(tokens2);
    free(structure1);
    free(structure2);
    
    return result;
}

/* ==================== MAIN FUNCTION ==================== */
int main() {
    char file1[256], file2[256];
    char* code1;
    char* code2;
    Result result;
    int i11;
    
    printf("\n");
    printf("  ============================================================\n");
    printf("  ||     CODE PLAGIARISM DETECTOR v1.0                      ||\n");
    printf("  ||     Advanced Multi-Algorithm Detection System          ||\n");
    printf("  ============================================================\n\n");
    
    printf("  Enter path to first code file:  ");
    fgets(file1, sizeof(file1), stdin);
    file1[strcspn(file1, "\n")] = '\0';
    
    printf("  Enter path to second code file: ");
    fgets(file2, sizeof(file2), stdin);
    file2[strcspn(file2, "\n")] = '\0';
    
    printf("\n  Analyzing files...\n");
    printf("  ");
    for (i11 = 0; i11 < 50; i11++) printf("=");
    printf("\n");
    
    code1 = readFile(file1);
    code2 = readFile(file2);
    
    if (!code1 || !code2) {
        printf("\nError: Could not read one or both files.\n");
        if (code1) free(code1);
        if (code2) free(code2);
        return 1;
    }
    
    result = compare(code1, code2);
    displayResult(&result);
    
    free(code1);
    free(code2);
    
    printf("\n  Press Enter to exit...");
    getchar();
    
    return 0;
}
