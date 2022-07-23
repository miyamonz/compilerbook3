#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize.c
typedef enum
{
    TK_RESERVED, // Keywords or punctuators
    TK_IDENT,    // Identifiers
    TK_NUM,      // Interger Literals
    TK_EOF,      // End-of-file markers
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind;
    Token *next;
    int val; // only used when TK_NUM
    char *str;
    int len;
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
char *strndup(char *p, int len);
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

extern char *user_input;
extern Token *token;

//
// parse.c
//

// Local variable
typedef struct Var Var;
struct Var
{
    Var *next;
    char *name; // Variable name
    int offset; // offset from RBP
};

// AST node
typedef enum
{
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // =
    ND_RETURN,    // return
    ND_EXPR_STMT, // Expression statement
    ND_VAR,       // variable
    ND_NUM,       // 整数
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *next; // next statement node
    Node *lhs;
    Node *rhs;
    Var *var; // Used if kind == ND_VAR
    int val;  // Used if kind == ND_NUM
};

typedef struct
{
    Node *node;
    Var *locals;
    int stack_size;
} Program;

Program *program();

// codegen.c
void codegen(Program *prog);