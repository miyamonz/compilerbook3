#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;
typedef struct Member Member;
typedef struct Initializer Initializer;

// tokenize.c
typedef enum
{
    TK_RESERVED, // Keywords or punctuators
    TK_IDENT,    // Identifiers
    TK_STR,      // String literals
    TK_NUM,      // Interger Literals
    TK_EOF,      // End-of-file markers
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind;
    Token *next;
    Type *ty; // Type, e.g. int or pointer to int
    long val; // only used when TK_NUM
    char *str;
    int len;

    char *contents; // String literal contents including terminating '\0'
    int cont_len;   // String literal length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
char *strndup(char *p, int len);
Token *peek(char *s);
Token *consume(char *op);
Token *consume_ident();
void expect(char *op);
long expect_number();
char *expect_ident();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

extern char *filename;
extern char *user_input;
extern Token *token;

//
// parse.c
//

// variable
typedef struct Var Var;
struct Var
{
    char *name;    // Variable name
    Type *ty;      // Type
    Token *tok;    // for error message
    bool is_local; // local or global

    // Local variable
    int offset; // offset from RBP

    // Global variable
    Initializer *initializer;
};

// Global variable initializer. Global variables can be initialized
// either by a constant expression or a pointer to another global
// variable.
struct Initializer
{
    Initializer *next;

    // Constant expression
    int sz;
    long val;

    // Reference to another global variable
    char *label;
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
    ND_BITAND,    // &
    ND_BITOR,     // |
    ND_BITXOR,    // ^
    ND_SHL,       // <<
    ND_SHR,       // >>
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // =
    ND_TERNARY,   // ?:
    ND_PRE_INC,   // pre ++
    ND_PRE_DEC,   // pre --
    ND_POST_INC,  // post ++
    ND_POST_DEC,  // post --
    ND_A_ADD,     // +=
    ND_A_SUB,     // -=
    ND_A_MUL,     // *=
    ND_A_DIV,     // /=
    ND_A_SHL,     // <<=
    ND_A_SHR,     // >>=
    ND_COMMA,     //,
    ND_MEMBER,    // . (struct member access)
    ND_ADDR,      // unary &
    ND_DEREF,     // unary *
    ND_NOT,       // !
    ND_BITNOT,    // ~
    ND_LOGAND,    // &&
    ND_LOGOR,     // ||
    ND_RETURN,    // return
    ND_IF,        // if
    ND_WHILE,     // while
    ND_FOR,       // for
    ND_SWITCH,    // "switch"
    ND_CASE,      // "case"
    ND_SIZEOF,    // sizeof
    ND_BLOCK,     // {...}
    ND_BREAK,     // "break"
    ND_CONTINUE,  // "continue"
    ND_GOTO,      // "goto"
    ND_LABEL,     // Labeled statement
    ND_FUNCALL,   // Function call
    ND_EXPR_STMT, // Expression statement
    ND_STMT_EXPR, // Statement expression
    ND_VAR,       // variable
    ND_NUM,       // 整数
    ND_CAST,      // Type cast
    ND_NULL,      // Empty statement
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *next; // next statement node
    Type *ty;   // Type, e.g. int or pointer to int
    Token *tok; // Representative token

    Node *lhs;
    Node *rhs;

    // "if", "while" or "for" statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // Block or statement expression
    Node *body;

    // Struct member access
    char *member_name;
    Member *member;

    // Function call
    char *funcname;
    Node *args;

    // Goto or labeled statement
    char *label_name;

    // Switch-cases
    Node *case_next;
    Node *default_case;
    int case_label;
    int case_end_label;

    Var *var; // Used if kind == ND_VAR
    long val;
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

typedef struct
{
    VarList *globals;
    Function *fns;
} Program;

Program *program();

// typing.c
typedef enum
{
    TY_VOID,
    TY_BOOL,
    TY_CHAR,
    TY_SHORT,
    TY_INT,
    TY_LONG,
    TY_ENUM,
    TY_PTR,
    TY_ARRAY,
    TY_STRUCT,
    TY_FUNC,
} TypeKind;
struct Type
{
    TypeKind kind;
    bool is_typedef;    // typedef
    bool is_static;     // static
    bool is_incomplete; // incomplete array
    int align;          // alignment
    Type *base;         // pointer or array
    int array_size;     // array
    Member *members;    // struct
    Type *return_ty;    // function
};

// Struct member
struct Member
{
    Member *next;
    Type *ty;
    Token *tok;
    char *name;
    int offset;
};

int align_to(int n, int align);
Type *void_type();
Type *bool_type();
Type *char_type();
Type *short_type();
Type *int_type();
Type *long_type();
Type *enum_type();
Type *struct_type();
Type *func_type(Type *return_ty);
Type *pointer_to(Type *base);
Type *array_of(Type *base, int size);
int size_of(Type *ty, Token *tok);
void add_type(Program *prog);

// codegen.c
void codegen(Program *prog);