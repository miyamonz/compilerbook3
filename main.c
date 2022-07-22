#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tokenizer
typedef enum
{
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
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

Token *token;

// エラーを報告するための関数。printfと同じ引数を取る
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 入力プログラム
char *user_input;

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op)
{
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize()
{
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (strchr("+-*/()", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(token->str, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

// Parser
typedef enum
{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val; // only used when ND_NUM
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = add
Node *expr()
{
    return add();
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ("+" | "-")? unary
//         | primary
Node *unary()
{
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());
    return primary();
}

// primary = "(" expr ")" | num
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}

// Code generator

void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        // https://www.sigbus.info/compilerbook#%E6%9C%AC%E6%9B%B8%E3%81%AE%E8%A1%A8%E8%A8%98%E6%B3%95:~:text=idiv%E3%81%AF%E6%9A%97%E9%BB%99%E3%81%AE%E3%81%86%E3%81%A1%E3%81%ABRDX%E3%81%A8RAX%E3%82%92%E5%8F%96%E3%81%A3%E3%81%A6%E3%80%81%E3%81%9D%E3%82%8C%E3%82%92%E5%90%88%E3%82%8F%E3%81%9B%E3%81%9F%E3%82%82%E3%81%AE%E3%82%92128%E3%83%93%E3%83%83%E3%83%88%E6%95%B4%E6%95%B0%E3%81%A8%E3%81%BF%E3%81%AA%E3%81%97%E3%81%A6%E3%80%81%E3%81%9D%E3%82%8C%E3%82%92%E5%BC%95%E6%95%B0%E3%81%AE%E3%83%AC%E3%82%B8%E3%82%B9%E3%82%BF%E3%81%AE64%E3%83%93%E3%83%83%E3%83%88%E3%81%AE%E5%80%A4%E3%81%A7%E5%89%B2%E3%82%8A%E3%80%81%E5%95%86%E3%82%92RAX%E3%81%AB%E3%80%81%E4%BD%99%E3%82%8A%E3%82%92RDX%E3%81%AB%E3%82%BB%E3%83%83%E3%83%88%E3%81%99%E3%82%8B%E3%80%81%E3%81%A8%E3%81%84%E3%81%86%E4%BB%95%E6%A7%98%E3%81%AB%E3%81%AA%E3%81%A3%E3%81%A6%E3%81%84%E3%81%BE%E3%81%99%E3%80%82cqo%E5%91%BD%E4%BB%A4%E3%82%92%E4%BD%BF%E3%81%86%E3%81%A8%E3%80%81RAX%E3%81%AB%E5%85%A5%E3%81%A3%E3%81%A6%E3%81%84%E3%82%8B64%E3%83%93%E3%83%83%E3%83%88%E3%81%AE%E5%80%A4%E3%82%92128%E3%83%93%E3%83%83%E3%83%88%E3%81%AB%E4%BC%B8%E3%81%B0%E3%81%97%E3%81%A6RDX%E3%81%A8RAX%E3%81%AB%E3%82%BB%E3%83%83%E3%83%88%E3%81%99%E3%82%8B%E3%81%93%E3%81%A8%E3%81%8C%E3%81%A7%E3%81%8D%E3%82%8B%E3%81%AE%E3%81%A7%E3%80%81%E4%B8%8A%E8%A8%98%E3%81%AE%E3%82%B3%E3%83%BC%E3%83%89%E3%81%A7%E3%81%AFidiv%E3%82%92%E5%91%BC%E3%81%B6%E5%89%8D%E3%81%ABcqo%E3%82%92%E5%91%BC%E3%82%93%E3%81%A7%E3%81%84%E3%81%BE%E3%81%99%E3%80%82
        // idivは暗黙のうちにRDXとRAXを取って、それを合わせたものを128ビット整数とみなして、それを引数のレジスタの64ビットの値で割り、商をRAXに、余りをRDXにセットする、という仕様になっています。
        // cqo命令を使うと、RAXに入っている64ビットの値を128ビットに伸ばしてRDXとRAXにセットすることができるので、上記のコードではidivを呼ぶ前にcqoを呼んでいます。
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    }

    printf("  push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
        error("%s: 引数の個数が正しくありません\n", argv[0]);

    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);
    // 結果はstackの頭にあるので、raxにpopしてexit codeにする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}