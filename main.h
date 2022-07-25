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
void error_tok(Token *tok, char *fmt, ...);
char *strndup(char *p, int len);
Token *consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
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
    char *name; // Variable name
    int offset; // offset from RBP
};

typedef struct VarList VarList;
struct VarList
{
    VarList *next;
    Var *var;
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
    ND_ADDR,      // unary &
    ND_DEREF,     // unary *
    ND_RETURN,    // return
    ND_IF,        // if
    ND_WHILE,     // while
    ND_FOR,       // for
    ND_BLOCK,     // {...}
    ND_FUNCALL,   // Function call
    ND_EXPR_STMT, // Expression statement
    ND_VAR,       // variable
    ND_NUM,       // 整数
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *next; // next statement node
    Token *tok; // Representative token

    Node *lhs;
    Node *rhs;

    // "if", "while" or "for" statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // Block
    Node *body;

    // Function call
    char *funcname;
    Node *args;

    Var *var; // Used if kind == ND_VAR
    int val;  // Used if kind == ND_NUM
};

typedef struct Function Function;
struct Function
{
    Function *next;
    char *name;
    VarList *params;

    Node *node;
    VarList *locals;
    int stack_size;
};

Function *program();

// codegen.c
void codegen(Function *prog);