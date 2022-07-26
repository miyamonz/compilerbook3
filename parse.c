#include "main.h"

// Scope for local variables, global variables or typedefs
typedef struct VarScope VarScope;
struct VarScope
{
    VarScope *next;
    char *name;
    Var *var;
    Type *type_def;
};

// Scope for struct tags
typedef struct TagScope TagScope;
struct TagScope
{
    TagScope *next;
    char *name;
    Type *ty;
};

VarList *locals;
VarList *globals;

VarScope *var_scope;
TagScope *tag_scope;

// find a variable or a typedef by name
VarScope *find_var(Token *tok)
{
    for (VarScope *sc = var_scope; sc; sc = sc->next)
    {
        if (strlen(sc->name) == tok->len && !memcmp(tok->str, sc->name, tok->len))
            return sc;
    }

    return NULL;
}

TagScope *find_tag(Token *tok)
{
    for (TagScope *sc = tag_scope; sc; sc = sc->next)
        if (strlen(sc->name) == tok->len && !memcmp(tok->str, sc->name, tok->len))
            return sc;
    return NULL;
}

Node *new_node(NodeKind kind, Token *tok)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->tok = tok;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
    Node *node = new_node(kind, tok);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_unary(NodeKind kind, Node *lhs, Token *tok)
{
    Node *node = new_node(kind, tok);
    node->lhs = lhs;
    return node;
}

Node *new_node_num(int val, Token *tok)
{
    Node *node = new_node(ND_NUM, tok);
    node->val = val;
    return node;
}

Node *new_var(Var *var, Token *tok)
{
    Node *node = new_node(ND_VAR, tok);
    node->var = var;
    return node;
}

VarScope *push_scope(char *name)
{
    VarScope *sc = calloc(1, sizeof(VarScope));
    sc->name = name;
    sc->next = var_scope;
    var_scope = sc;
    return sc;
}

Var *push_var(char *name, Type *ty, bool is_local)
{
    Var *var = calloc(1, sizeof(Var));
    var->name = name;
    var->ty = ty;
    var->is_local = is_local;

    VarList *vl = calloc(1, sizeof(Var));
    vl->var = var;

    if (is_local)
    {
        vl->next = locals;
        locals = vl;
    }
    else if (ty->kind != TY_FUNC)
    {
        vl->next = globals;
        globals = vl;
    }

    push_scope(name)->var = var;

    return var;
}

Type *find_typedef(Token *tok)
{
    if (tok->kind == TK_IDENT)
    {
        VarScope *sc = find_var(token);
        if (sc)
            return sc->type_def;
    }
    return NULL;
}

char *new_label()
{
    static int cnt = 0;
    char buf[20];
    sprintf(buf, ".L.data.%d", cnt++);
    return strndup(buf, 20);
}

Function *function();
Type *type_specifier();
Type *declarator();
Type *type_suffix();
Type *struct_decl();
Member *struct_member();
void global_var();
Node *declaration();
bool is_typename();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *postfix();
Node *primary();

bool is_function()
{
    Token *tok = token;

    Type *ty = type_specifier();
    char *name = NULL;
    declarator(ty, &name);
    bool isfunc = name && consume("(");

    token = tok;
    return isfunc;
}

// program = (global-var | function)*
Program *program()
{
    Function head;
    head.next = NULL;
    Function *cur = &head;
    globals = NULL;

    while (!at_eof())
    {
        if (is_function())
        {
            cur->next = function();
            cur = cur->next;
        }
        else
        {
            global_var();
        }
    }

    Program *prog = calloc(1, sizeof(Program));
    prog->globals = globals;
    prog->fns = head.next;
    return prog;
}

// type-specifier = builtin-type | struct-decl | typedef-name
// builtin-type = "char" | "short" | "long" | "int" | "void"
Type *type_specifier()
{
    if (!is_typename())
        error_tok(token, "token expected");

    if (consume("char"))
        return char_type();
    else if (consume("short"))
        return short_type();
    else if (consume("int"))
        return int_type();
    else if (consume("long"))
        return long_type();
    else if (consume("struct"))
        return struct_decl();
    else if (consume("void"))
        return void_type();
    else if (consume("_Bool"))
        return bool_type();
    else
        return find_var(consume_ident())->type_def;
}

// declarator = "*"* ("(" declarator ")" | ident) type-suffix
Type *declarator(Type *ty, char **name)
{
    while (consume("*"))
        ty = pointer_to(ty);

    if (consume("("))
    {
        Type *placeholder = calloc(1, sizeof(Type));
        Type *new_ty = declarator(placeholder, name);
        expect(")");
        *placeholder = *type_suffix(ty);
        return new_ty;
    }

    *name = expect_ident();
    return type_suffix(ty);
}

// type-suffix = ("[" num "]" type-suffix)?
Type *type_suffix(Type *base)
{
    if (!consume("["))
        return base;
    int sz = expect_number();
    expect("]");
    base = type_suffix(base);
    return array_of(base, sz);
}

void push_tag_scope(Token *tok, Type *ty)
{
    TagScope *sc = calloc(1, sizeof(TagScope));
    sc->next = tag_scope;
    sc->name = strndup(tok->str, tok->len);
    sc->ty = ty;
    tag_scope = sc;
}

// struct-decl = "struct" ident
//             | "struct" ident? "{" struct-member "}"
Type *struct_decl()
{
    // Read a struct tag.
    Token *tag = consume_ident();
    if (tag && !peek("{"))
    {
        TagScope *sc = find_tag(tag);
        if (!sc)
            error_tok(tag, "unknown struct type");
        return sc->ty;
    }
    expect("{");

    Member head;
    head.next = NULL;
    Member *cur = &head;

    while (!consume("}"))
    {
        cur->next = struct_member();
        cur = cur->next;
    }

    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_STRUCT;
    ty->members = head.next;

    // Assign offsets within the struct to members.
    int offset = 0;
    for (Member *mem = ty->members; mem; mem = mem->next)
    {
        offset = align_to(offset, mem->ty->align);
        mem->offset = offset;
        offset += size_of(mem->ty);

        // structのalignは、memberのalignの最大値とする
        if (ty->align < mem->ty->align)
            ty->align = mem->ty->align;
    }

    // Register the struct type if a name was given.
    if (tag)
        push_tag_scope(tag, ty);

    return ty;
}

// struct-member = type-specifier declarator type-suffix ";"
Member *struct_member()
{
    Type *ty = type_specifier();
    char *name = NULL;
    ty = declarator(ty, &name);
    ty = type_suffix(ty);

    Member *mem = calloc(1, sizeof(Member));
    mem->name = name;
    mem->ty = ty;
    expect(";");
    return mem;
}

VarList *read_func_param()
{
    Type *ty = type_specifier();
    char *name = NULL;
    ty = declarator(ty, &name);
    ty = type_suffix(ty);

    VarList *vl = calloc(1, sizeof(VarList));
    vl->var = push_var(name, ty, true);
    return vl;
}

VarList *read_func_params()
{
    if (consume(")"))
        return NULL;

    VarList *head = read_func_param();
    VarList *cur = head;

    while (!consume(")"))
    {
        expect(",");
        cur->next = read_func_param();
        cur = cur->next;
    }

    return head;
}

// function = type-specifier declarator "(" params? ")" "{" stmt* "}"
// params   = param ("," param)*
// param    = type-specifier declarator
Function *function()
{
    locals = NULL;
    Type *ty = type_specifier();
    char *name = NULL;
    ty = declarator(ty, &name);

    // add a function type to the scope
    push_var(name, func_type(ty), false);

    // construct a function object
    Function *fn = calloc(1, sizeof(Function));
    fn->name = name;

    expect("(");
    fn->params = read_func_params();
    expect("{");

    // body
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!consume("}"))
    {
        cur->next = stmt();
        cur = cur->next;
    }

    fn->node = head.next;
    fn->locals = locals;

    // Assign offsets to local variables
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next)
    {
        Var *var = vl->var;
        offset = align_to(offset, var->ty->align);
        offset += size_of(var->ty);
        vl->var->offset = offset;
    }
    fn->stack_size = align_to(offset, 8);

    return fn;
}

// global-var = type-specifier declarator type-suffix ";"
void global_var()
{
    Type *ty = type_specifier();
    char *name = NULL;
    ty = declarator(ty, &name);
    ty = type_suffix(ty);
    expect(";");
    push_var(name, ty, false);
}

// declaration = type-specifier declarator type-suffix ("=" expr) ";"
//             | type-specifier ";"
Node *declaration()
{
    Token *tok = token;
    Type *ty = type_specifier();
    if (consume(";"))
        return new_node(ND_NULL, tok);

    char *name = NULL;
    ty = declarator(ty, &name);
    ty = type_suffix(ty);
    Var *var = push_var(name, ty, true);

    if (consume(";"))
        return new_node(ND_NULL, tok);

    expect("=");
    Node *lhs = new_var(var, tok);
    Node *rhs = expr();
    expect(";");
    Node *node = new_binary(ND_ASSIGN, lhs, rhs, tok);
    return new_unary(ND_EXPR_STMT, node, tok);
}

Node *read_expr_stmt()
{
    Token *tok = token;
    return new_unary(ND_EXPR_STMT, expr(), tok);
}

bool is_typename()
{
    return peek("char") || peek("short") || peek("int") || peek("long") ||
           peek("struct") || peek("void") || peek("_Bool") || find_typedef(token);
}

// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "while" "(" expr ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | "{" stmt* "}"
//        | "typedef" type-specifier declarator type-suffix ";"
//        | declaration
//        | expr ";"
Node *stmt()
{
    Token *tok;
    if (tok = consume("return"))
    {
        Node *node = new_unary(ND_RETURN, expr(), tok);
        expect(";");
        return node;
    }

    if (tok = consume("if"))
    {
        Node *node = new_node(ND_IF, tok);

        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else"))
            node->els = stmt();
        return node;
    }

    if (tok = consume("while"))
    {
        Node *node = new_node(ND_WHILE, tok);

        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    }

    if (tok = consume("for"))
    {
        Node *node = new_node(ND_FOR, tok);

        // codegenの際にstackに保持したいものをexpr,いらないものをexpr_stmtとして作れば良い
        expect("(");
        if (!consume(";"))
        {
            node->init = read_expr_stmt();
            expect(";");
        }
        if (!consume(";"))
        {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")"))
        {
            node->inc = read_expr_stmt();
            expect(")");
        }
        node->then = stmt();
        return node;
    }

    if (tok = consume("{"))
    {
        Node head;
        head.next = NULL;
        Node *cur = &head;

        VarScope *sc1 = var_scope;
        TagScope *sc2 = tag_scope;
        while (!consume("}"))
        {
            cur->next = stmt();
            cur = cur->next;
        }
        var_scope = sc1;
        tag_scope = sc2;

        Node *node = new_node(ND_BLOCK, tok);
        node->body = head.next;
        return node;
    }

    if (tok = consume("typedef"))
    {
        Type *ty = type_specifier();
        char *name = NULL;
        ty = declarator(ty, &name);
        ty = type_suffix(ty);
        expect(";");
        push_scope(name)->type_def = ty;
        return new_node(ND_NULL, tok);
    }

    if (is_typename())
        return declaration();

    Node *node = read_expr_stmt();
    expect(";");
    return node;
}

// expr = assign
Node *expr()
{
    return assign();
}

// assign = equality ("=" assign)?
Node *assign()
{
    Node *node = equality();
    Token *tok;
    if (tok = consume("="))
        node = new_binary(ND_ASSIGN, node, assign(), tok);
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();
    Token *tok;

    for (;;)
    {
        if (tok = consume("=="))
            node = new_binary(ND_EQ, node, relational(), tok);
        if (tok = consume("!="))
            node = new_binary(ND_NE, node, relational(), tok);
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
    Node *node = add();
    Token *tok;

    for (;;)
    {
        if (tok = consume("<"))
            node = new_binary(ND_LT, node, add(), tok);
        if (tok = consume("<="))
            node = new_binary(ND_LE, node, add(), tok);
        if (tok = consume(">"))
            node = new_binary(ND_LT, add(), node, tok);
        if (tok = consume(">="))
            node = new_binary(ND_LE, add(), node, tok);
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();
    Token *tok;

    for (;;)
    {
        if (tok = consume("+"))
            node = new_binary(ND_ADD, node, mul(), tok);
        else if (tok = consume("-"))
            node = new_binary(ND_SUB, node, mul(), tok);
        else
            return node;
    }
}

// mul = unary("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();
    Token *tok;

    for (;;)
    {
        if (tok = consume("*"))
            node = new_binary(ND_MUL, node, unary(), tok);
        if (tok = consume("/"))
            node = new_binary(ND_DIV, node, unary(), tok);
        else
            return node;
    }
}

// unary = ("+" | "-")? unary
//       | "*" unary
//       | "&" unary
//       | postfix
Node *unary()
{
    Token *tok;
    if (tok = consume("+"))
        return unary();
    if (tok = consume("-"))
        return new_binary(ND_SUB, new_node_num(0, tok), unary(), tok);

    if (tok = consume("*"))
        return new_unary(ND_DEREF, unary(), tok);
    if (tok = consume("&"))
        return new_unary(ND_ADDR, unary(), tok);

    return postfix();
}

// postfix = primary ("[" expr "]" | "." ident | "->" ident)*
Node *postfix()
{
    Node *node = primary();
    Token *tok;

    for (;;)
    {
        // x[y] is short for *(x+y)
        if (tok = consume("["))
        {
            Node *exp = new_binary(ND_ADD, node, expr(), tok);
            expect("]");
            node = new_unary(ND_DEREF, exp, tok);
            continue;
        }
        if (tok = consume("."))
        {
            node = new_unary(ND_MEMBER, node, tok);
            node->member_name = expect_ident();
            continue;
        }
        if (tok = consume("->"))
        {
            // x->y is short for (&x).y
            node = new_unary(ND_DEREF, node, tok);
            node = new_unary(ND_MEMBER, node, tok);
            node->member_name = expect_ident();
            continue;
        }
        return node;
    }
}

// stmt-expr = "(" "{" stmt stmt* "}" ")"
// Statement expression is a GNU C extension
Node *stmt_expr(Token *tok)
{
    VarScope *sc1 = var_scope;
    TagScope *sc2 = tag_scope;

    Node *node = new_node(ND_STMT_EXPR, tok);
    node->body = stmt();
    Node *cur = node->body;

    while (!consume("}"))
    {
        cur->next = stmt();
        cur = cur->next;
    }
    expect(")");

    var_scope = sc1;
    tag_scope = sc2;

    if (cur->kind != ND_EXPR_STMT)
        error_tok(cur->tok, "stmt expr returning void is not supported");
    *cur = *cur->lhs;
    return node;
}

// func-args = "(" (assign ("," assign)*)? ")"
Node *func_args()
{
    if (consume(")"))
        return NULL;

    Node *head = assign();
    Node *cur = head;
    while (consume(","))
    {
        cur->next = assign();
        cur = cur->next;
    }
    expect(")");
    return head;
}

// primary = "(" "{" stmt-exprtail
//         | "(" expr ")"
//         | "sizeof" unary
//         | ident func-args?
//         | str
//         | num
Node *primary()
{
    Token *tok;

    if (tok = consume("("))
    {
        if (consume("{"))
            return stmt_expr(tok);

        Node *node = expr();
        expect(")");
        return node;
    }

    if (tok = consume("sizeof"))
        return new_unary(ND_SIZEOF, unary(), tok);

    if (tok = consume_ident())
    {
        // function call
        if (consume("("))
        {
            Node *node = new_node(ND_FUNCALL, tok);
            node->funcname = strndup(tok->str, tok->len);
            node->args = func_args();

            VarScope *sc = find_var(tok);
            if (sc)
            {
                if (!sc->var || sc->var->ty->kind != TY_FUNC)
                    error_tok(tok, "not a function");
                node->ty = sc->var->ty->return_ty;
            }
            else
            {
                node->ty = int_type();
            }
            return node;
        }

        VarScope *sc = find_var(tok);
        if (sc && sc->var)
            return new_var(sc->var, tok);
        error_tok(tok, "undefined variable");
    }

    tok = token;

    if (tok->kind == TK_STR)
    {
        token = token->next;
        Type *ty = array_of(char_type(), tok->cont_len);
        Var *var = push_var(new_label(), ty, false);
        var->contents = tok->contents;
        var->cont_len = tok->cont_len;
        return new_var(var, tok);
    }

    if (tok->kind != TK_NUM)
        error_tok(tok, "expected expression");

    return new_node_num(expect_number(), tok);
}