#include "main.h"

// Scope for local variables, global variables or typedefs
typedef struct VarScope VarScope;
struct VarScope
{
    VarScope *next;
    char *name;
    int depth;
    Var *var;
    Type *type_def;
    Type *enum_ty;
    int enum_val;
};

// Scope for struct tags
typedef struct TagScope TagScope;
struct TagScope
{
    TagScope *next;
    char *name;
    int depth;
    Type *ty;
};

typedef struct
{
    VarScope *var_scope;
    TagScope *tag_scope;
} Scope;

VarList *locals;
VarList *globals;

VarScope *var_scope;
TagScope *tag_scope;
int scope_depth;

Node *current_switch;

Scope *enter_scope()
{
    Scope *sc = calloc(1, sizeof(Scope));
    sc->var_scope = var_scope;
    sc->tag_scope = tag_scope;
    ++scope_depth;
    return sc;
}

void leave_scope(Scope *sc)
{
    var_scope = sc->var_scope;
    tag_scope = sc->tag_scope;
    --scope_depth;
}

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

Node *new_node_num(long val, Token *tok)
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
    sc->depth = scope_depth;
    var_scope = sc;
    return sc;
}

Var *push_var(char *name, Type *ty, bool is_local, Token *tok)
{
    Var *var = calloc(1, sizeof(Var));
    var->name = name;
    var->ty = ty;
    var->tok = tok;
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

    return var;
}

Type *find_typedef(Token *tok)
{
    if (tok->kind == TK_IDENT)
    {
        VarScope *sc = find_var(tok);
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
Type *abstract_declarator(Type *ty);
Type *type_suffix();
Type *type_name();
Type *struct_decl();
Type *enum_specifier();
Member *struct_member();
void global_var();
Node *declaration();
bool is_typename();
Node *stmt();
Node *expr();
Node *assign();
Node *conditional();
Node *logor();
Node *logand();
Node *bitand();
Node * bitor ();
Node *bitxor();
Node *equality();
Node *relational();
Node *shift();
Node *add();
Node *mul();
Node *cast();
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
            Function *fn = function();
            if (!fn)
                continue;

            cur = cur->next = fn;
            continue;
        }
        global_var();
    }

    Program *prog = calloc(1, sizeof(Program));
    prog->globals = globals;
    prog->fns = head.next;
    return prog;
}

// type-specifier = builtin-type | struct-decl | typedef-name | enum-specifier
// builtin-type   = "void"
//                | "_Bool"
//                | "char"
//                | "short" | "short" "int" | "int" "short"
//                | "long" | "long" "int" | "int" "long"
//                | "int"
// Note that "typedef" and "static" can appear anywhere in a type-specifier.
Type *type_specifier()
{
    if (!is_typename())
        error_tok(token, "token expected");

    bool is_typedef = false;
    bool is_static = false;
    Type *ty = NULL;
    enum
    {
        VOID = 1 << 1,
        BOOL = 1 << 3,
        CHAR = 1 << 5,
        SHORT = 1 << 7,
        INT = 1 << 9,
        LONG = 1 << 11,
    };
    int base_type = 0;
    Type *user_type = NULL;

    for (;;)
    {
        Token *tok = token;

        if (consume("typedef"))
            is_typedef = true;
        else if (consume("static"))
            is_static = true;
        else if (consume("void"))
            base_type += VOID;
        else if (consume("_Bool"))
            base_type += BOOL;
        else if (consume("char"))
            base_type += CHAR;
        else if (consume("short"))
            base_type += SHORT;
        else if (consume("long"))
            base_type += LONG;
        else if (consume("int"))
            base_type += INT;
        else if (peek("struct"))
        {
            if (base_type || user_type)
                break;
            user_type = struct_decl();
        }
        else if (peek("enum"))
        {
            if (base_type || user_type)
                break;
            user_type = enum_specifier();
        }
        else
        {
            if (base_type || user_type)
                break;
            Type *found = find_typedef(token);
            if (!found)
                break;
            token = token->next;
            user_type = found;
        }

        switch (base_type)
        {
        case VOID:
            ty = void_type();
            break;
        case BOOL:
            ty = bool_type();
            break;
        case CHAR:
            ty = char_type();
            break;
        case SHORT:
        case SHORT + INT:
            ty = short_type();
            break;
        case INT:
            ty = int_type();
            break;
        case LONG:
        case LONG + INT:
            ty = long_type();
            break;
        case 0:
            // If there's no type specifier, it becomes int.
            // For example, `typedef x` defines x as an alias for int.
            ty = user_type ? user_type : int_type();
            break;
        default:
            error_tok(tok, "invalid type");
        }
    }

    ty->is_typedef = is_typedef;
    ty->is_static = is_static;
    return ty;
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

// abstract-declarator = "*"* ("(" abstract-declarator ")")? type-suffix
Type *abstract_declarator(Type *ty)
{
    while (consume("*"))
        ty = pointer_to(ty);

    if (consume("("))
    {
        Type *placeholder = calloc(1, sizeof(Type));
        Type *new_ty = abstract_declarator(placeholder);
        expect(")");
        *placeholder = *type_suffix(ty);
        return new_ty;
    }
    return type_suffix(ty);
}

// type-suffix = ("[" num? "]" type-suffix)?
Type *type_suffix(Type *ty)
{
    if (!consume("["))
        return ty;

    int sz = 0;
    bool is_incomplete = true;
    if (!consume("]"))
    {
        sz = expect_number();
        is_incomplete = false;
        expect("]");
    }
    ty = type_suffix(ty);
    ty = array_of(ty, sz);
    ty->is_incomplete = is_incomplete;
    return ty;
}

// type-name = type-specifier abstract-declarator type-suffix
// 独自にtype-suffixを削ってみる
// type-name = type-specifier abstract-declarator
Type *type_name()
{
    Type *ty = type_specifier();
    ty = abstract_declarator(ty);
    // abstract_declaratorがtype_suffixをやってるからいらないのでは？
    // return type_suffix(ty);
    return ty;
}

void push_tag_scope(Token *tok, Type *ty)
{
    TagScope *sc = calloc(1, sizeof(TagScope));
    sc->next = tag_scope;
    sc->name = strndup(tok->str, tok->len);
    sc->depth = scope_depth;
    sc->ty = ty;
    tag_scope = sc;
}

// struct-decl = "struct" ident? ("{" struct-member "}")?
Type *struct_decl()
{
    expect("struct");
    // Read a struct tag.
    Token *tag = consume_ident();
    if (tag && !peek("{"))
    {
        TagScope *sc = find_tag(tag);
        if (!sc)
        {
            Type *ty = struct_type();
            push_tag_scope(tag, ty);
            return ty;
        }
        if (sc->ty->kind != TY_STRUCT)
            error_tok(tag, "not a struct tag");
        return sc->ty;
    }
    // Although it looks weird, "struct *foo" is legal C that defines
    // foo as a pointer to an unnamed incomplete struct type.
    if (!consume("{"))
        return struct_type();

    TagScope *sc = find_tag(tag);
    Type *ty;

    if (sc && sc->depth == scope_depth)
    {
        // If there's an existing struct type having the same tag name in
        // the same block scope, this is a redefinition.
        if (sc->ty->kind != TY_STRUCT)
            error_tok(tag, "not a struct tag");
        ty = sc->ty;
    }
    else
    {
        // Register a struct type as an incomplete type early, so that you
        // can write recursive structs such as
        // "struct T { struct T *next; }".
        ty = struct_type();
        if (tag)
            push_tag_scope(tag, ty);
    }

    Member head;
    head.next = NULL;
    Member *cur = &head;

    while (!consume("}"))
    {
        cur->next = struct_member();
        cur = cur->next;
    }

    ty->members = head.next;

    // Assign offsets within the struct to members.
    int offset = 0;
    for (Member *mem = ty->members; mem; mem = mem->next)
    {
        offset = align_to(offset, mem->ty->align);
        mem->offset = offset;
        offset += size_of(mem->ty, mem->tok);

        // structのalignは、memberのalignの最大値とする
        if (ty->align < mem->ty->align)
            ty->align = mem->ty->align;
    }

    // Register the struct type if a name was given.
    if (tag)
        push_tag_scope(tag, ty);

    ty->is_incomplete = false;

    return ty;
}

// enum-specifier = "enum" ident
//                | "enum" ident? "{" enum-list? "}"
//
// enum-list = ident ("=" num)? ("," ident ("=" num)?)* ","?
Type *enum_specifier()
{
    expect("enum");
    Type *ty = enum_type();

    // Read an enum tag.
    Token *tag = consume_ident();
    if (tag && !peek("{"))
    {
        TagScope *sc = find_tag(tag);
        if (!sc)
            error_tok(tag, "unknown enum type");
        if (sc->ty->kind != TY_ENUM)
            error_tok(tag, "not an enum tag");
        return sc->ty;
    }

    expect("{");

    // Read enum-list.
    int cnt = 0;
    for (;;)
    {
        char *name = expect_ident();
        if (consume("="))
            cnt = expect_number();

        VarScope *sc = push_scope(name);
        sc->enum_ty = ty;
        sc->enum_val = cnt++;

        if (consume(","))
        {
            if (consume("}"))
                break;
            continue;
        }
        expect("}");
        break;
    }

    if (tag)
        push_tag_scope(tag, ty);
    return ty;
}

// struct-member = type-specifier declarator type-suffix ";"
Member *struct_member()
{
    Type *ty = type_specifier();
    Token *tok = token;
    char *name = NULL;
    ty = declarator(ty, &name);
    ty = type_suffix(ty);

    Member *mem = calloc(1, sizeof(Member));
    mem->name = name;
    mem->ty = ty;
    mem->tok = tok;
    expect(";");
    return mem;
}

VarList *read_func_param()
{
    Type *ty = type_specifier();
    char *name = NULL;
    Token *tok = token;
    ty = declarator(ty, &name);
    ty = type_suffix(ty);

    // "array of T" is converted to "pointer to T" only in the parameter
    // context. For example, *argv[] is converted to **argv by this.
    if (ty->kind == TY_ARRAY)
        ty = pointer_to(ty->base);

    Var *var = push_var(name, ty, true, tok);
    push_scope(name)->var = var;

    VarList *vl = calloc(1, sizeof(VarList));
    vl->var = var;
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

// function = type-specifier declarator "(" params? ")" ( "{" stmt* "}" | ";" )
// params   = param ("," param)*
// param    = type-specifier declarator
Function *function()
{
    locals = NULL;
    Type *ty = type_specifier();
    char *name = NULL;
    Token *tok = token;
    ty = declarator(ty, &name);

    // add a function type to the scope
    Var *var = push_var(name, func_type(ty), false, tok);
    push_scope(name)->var = var;

    // construct a function object
    Function *fn = calloc(1, sizeof(Function));
    fn->name = name;

    expect("(");
    fn->params = read_func_params();

    if (consume(";"))
        return NULL;

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
        offset += size_of(var->ty, var->tok);
        vl->var->offset = offset;
    }
    fn->stack_size = align_to(offset, 8);

    return fn;
}

// Initializer list can end either with "}" or "," followed by "}"
// to allow a trailing comma. This function returns true if it looks
// like we are at the end of an initializer list.
bool peek_end()
{
    Token *tok = token;
    bool ret = consume("}") || (consume(",") && consume("}"));
    token = tok;
    return ret;
}

void expect_end()
{
    Token *tok = token;
    if (consume(",") && consume("}"))
        return;
    token = tok;
    expect("}");
}

// global-var = type-specifier declarator type-suffix ";"
Initializer *new_init_val(Initializer *cur, int sz, int val)
{
    Initializer *init = calloc(1, sizeof(Initializer));
    init->sz = sz;
    init->val = val;
    cur->next = init;
    return init;
}

Initializer *new_init_label(Initializer *cur, char *label)
{
    Initializer *init = calloc(1, sizeof(Initializer));
    init->label = label;
    cur->next = init;
    return init;
}

Initializer *new_init_zero(Initializer *cur, int nbytes)
{
    for (int i = 0; i < nbytes; i++)
        cur = new_init_val(cur, 1, 0);
    return cur;
}

Initializer *gvar_init_string(char *p, int len)
{
    Initializer head;
    head.next = NULL;
    Initializer *cur = &head;
    for (int i = 0; i < len; i++)
        cur = new_init_val(cur, 1, p[i]);
    return head.next;
}

Initializer *emit_struct_padding(Initializer *cur, Type *parent, Member *mem)
{
    int end = mem->offset + size_of(mem->ty, token);

    int padding;
    if (mem->next)
        padding = mem->next->offset - end;
    else
        padding = size_of(parent, token) - end;

    if (padding)
        cur = new_init_zero(cur, padding);
    return cur;
}

Initializer *gvar_initializer(Initializer *cur, Type *ty)
{
    Token *tok = token;

    if (consume("{"))
    {
        if (ty->kind == TY_ARRAY)
        {
            int i = 0;

            do
            {
                cur = gvar_initializer(cur, ty->base);
                i++;
            } while (!peek_end() && consume(","));

            expect_end();

            // Set excess array elements to zero.
            if (i < ty->array_size)
                cur = new_init_zero(cur, size_of(ty->base, tok) * (ty->array_size - i));

            if (ty->is_incomplete)
            {
                ty->array_size = i;
                ty->is_incomplete = false;
            }

            return cur;
        }

        if (ty->kind == TY_STRUCT)
        {
            Member *mem = ty->members;

            do
            {
                cur = gvar_initializer(cur, mem->ty);
                cur = emit_struct_padding(cur, ty, mem);
                mem = mem->next;
            } while (!peek_end() && consume(","));

            expect_end();

            // Set excess struct elements to zero.
            if (mem)
            {
                int sz = size_of(ty, tok) - mem->offset;
                if (sz)
                    cur = new_init_zero(cur, sz);
            }
            return cur;
        }

        error_tok(tok, "invalid initializer");
    }

    Node *expr = conditional();
    // int init_val = eval(expr);

    // if (expr->kind == ND_ADDR)
    // {
    //     if (expr->lhs->kind != ND_VAR)
    //         error_tok(tok, "invalid initializer");
    //     return new_init_label(cur, expr->lhs->var->name);
    // }

    if (expr->kind == ND_VAR && expr->var->ty->kind == TY_ARRAY)
        return new_init_label(cur, expr->var->name);

    if (expr->kind == ND_NUM)
    {
        int init_val = expr->val;
        return new_init_val(cur, size_of(ty, token), init_val);
    }

    error_tok(tok, "initializer is stray");
}

void global_var()
{
    Type *ty = type_specifier();
    char *name = NULL;
    Token *tok = token;
    ty = declarator(ty, &name);
    ty = type_suffix(ty);

    Var *var = push_var(name, ty, false, tok);
    push_scope(name)->var = var;

    if (consume("="))
    {
        Initializer head;
        head.next = NULL;
        gvar_initializer(&head, ty);
        var->initializer = head.next;
    }
    expect(";");
}

typedef struct Designator Designator; // 指定子
struct Designator
{
    Designator *next;
    int idx;     // array
    Member *mem; // struct
};
// Creates a node for an array access. For example, if var represents
// a variable x and desg represents indices 3 and 4, this function
// returns a node representing x[3][4].
Node *new_desg_node2(Var *var, Designator *desg)
{
    Token *tok = var->tok;
    if (!desg)
        return new_var(var, tok);
    // x = {{ , , },{ , ,here}}
    //  この場合、desgは{idx=2, next={idx=1}}
    Node *node = new_desg_node2(var, desg->next);
    // 帰りがけ走査なので、x[1][2] こうなる

    if (desg->mem)
    {
        node = new_unary(ND_MEMBER, node, desg->mem->tok);
        node->member_name = desg->mem->name;
        return node;
    }

    node = new_binary(ND_ADD, node, new_node_num(desg->idx, tok), tok);
    return new_unary(ND_DEREF, node, tok);
}

Node *new_desg_node(Var *var, Designator *desg, Node *rhs)
{
    Node *lhs = new_desg_node2(var, desg);
    Node *node = new_binary(ND_ASSIGN, lhs, rhs, rhs->tok);
    return new_unary(ND_EXPR_STMT, node, rhs->tok);
}

Node *lvar_init_zero(Node *cur, Var *var, Type *ty, Designator *desg)
{
    if (ty->kind == TY_ARRAY)
    {
        for (int i = 0; i < ty->array_size; i++)
        {
            Designator desg2 = {desg, i++, NULL};
            cur = lvar_init_zero(cur, var, ty->base, &desg2);
        }
        return cur;
    }

    cur->next = new_desg_node(var, desg, new_node_num(0, token));
    return cur->next;
}

// lvar-initializer = assign
//                  | "{" lvar-initializer ("," lvar-initializer)* ","? "}"
//
// An initializer for a local variable is expanded to multiple
// assignments. For example, this function creates the following
// nodes for x[2][3]={{1,2,3},{4,5,6}}.
//
//   x[0][0]=1;
//   x[0][1]=2;
//   x[0][2]=3;
//   x[1][0]=4;
//   x[1][1]=5;
//   x[1][2]=6;
//
// Struct members are initialized in declaration order. For example,
// `struct { int a; int b; } x = {1, 2}` sets x.a to 1 and x.b to 2.
//
// There are a few special rules for ambiguous initializers and
// shorthand notations:
//
// - If an initializer list is shorter than an array, excess array
//   elements are initialized with 0.
//
// - A char array can be initialized by a string literal. For example,
//   `char x[4] = "foo"` is equivalent to `char x[4] = {'f', 'o', 'o',
//   '\0'}`.
//
// - If a rhs is an incomplete array, its size is set by counting the
//   number of items on the rhs. For example, `x` in `int x[]={1,2,3}`
//   has type `int[3]`.
Node *lvar_initializer(Node *cur, Var *var, Type *ty, Designator *desg)
{
    if (ty->kind == TY_ARRAY && ty->base->kind == TY_CHAR &&
        token->kind == TK_STR)
    {
        // Initialize a char array with a string literal.
        Token *tok = token;
        token = token->next;

        if (ty->is_incomplete)
        {
            ty->array_size = tok->cont_len;
            ty->is_incomplete = false;
        }

        int len = (ty->array_size < tok->cont_len)
                      ? ty->array_size
                      : tok->cont_len;
        int i;

        for (i = 0; i < len; i++)
        {
            Designator desg2 = {desg, i, NULL};
            Node *rhs = new_node_num(tok->contents[i], tok);
            cur->next = new_desg_node(var, &desg2, rhs);
            cur = cur->next;
        }

        for (; i < ty->array_size; i++)
        {
            Designator desg2 = {desg, i, NULL};
            cur = lvar_init_zero(cur, var, ty->base, &desg2);
        }
        return cur;
    }

    // cur でND_BLOCKを構築する。なので、cur->nextで連結リストを作る
    Token *tok = consume("{");
    if (!tok)
    {
        // { }で囲まれない場合、右辺はassignになる。
        // 一番外側ではdesg=NULLになり、単なる`var "=" assign`になる
        cur->next = new_desg_node(var, desg, assign());
        return cur->next;
    }

    // x[2][3]={{1,2,3},{4,5,6}}
    // このとき、2 length array of (3 length array of (int)) であり、Typeもこうなってる
    if (ty->kind == TY_ARRAY)
    {
        int i = 0;

        do
        {
            // Designatorは、array typeを外側から巡回しつつ、Designator自身をnextに入れる
            // なので、上の例だと 3len -> 2len -> null
            Designator desg2 = {desg, i++, NULL};
            cur = lvar_initializer(cur, var, ty->base, &desg2);
        } while (!peek_end() && consume(","));

        expect_end();

        // Set excess array elements to zero.
        while (i < ty->array_size)
        {
            Designator desg2 = {desg, i++, NULL};
            cur = lvar_init_zero(cur, var, ty->base, &desg2);
        }

        if (ty->is_incomplete)
        {
            ty->array_size = i;
            ty->is_incomplete = false;
        }

        return cur;
    }

    if (ty->kind == TY_STRUCT)
    {
        Member *mem = ty->members;

        do
        {
            Designator desg2 = {desg, 0, mem};
            cur = lvar_initializer(cur, var, mem->ty, &desg2);
            mem = mem->next;
        } while (!peek_end() && consume(","));

        expect_end();

        // Set excess struct elements to zero.
        for (; mem; mem = mem->next)
        {
            Designator desg2 = {desg, 0, mem};
            cur = lvar_init_zero(cur, var, mem->ty, &desg2);
        }
        return cur;
    }

    error_tok(tok, "invalid array initializer");
}

// declaration = type-specifier declarator type-suffix ("=" lvar-initializer)? ";"
//             | type-specifier ";"
Node *declaration()
{
    Token *tok;
    Type *ty = type_specifier();
    if (consume(";"))
        return new_node(ND_NULL, tok);

    tok = token;
    char *name = NULL;
    ty = declarator(ty, &name);
    ty = type_suffix(ty);

    if (ty->is_typedef)
    {
        expect(";");
        ty->is_typedef = false;
        push_scope(name)->type_def = ty;
        return new_node(ND_NULL, tok);
    }

    Var *var;
    if (ty->is_static)
        var = push_var(new_label(), ty, false, tok);
    else
        var = push_var(name, ty, true, tok);
    push_scope(name)->var = var;

    if (consume(";"))
        return new_node(ND_NULL, tok);

    expect("=");

    Node head;
    head.next = NULL;
    lvar_initializer(&head, var, var->ty, NULL);
    expect(";");

    Node *node = new_node(ND_BLOCK, tok);
    node->body = head.next;
    return node;
}

Node *read_expr_stmt()
{
    Token *tok = token;
    return new_unary(ND_EXPR_STMT, expr(), tok);
}

bool is_typename()
{
    return peek("void") || peek("_Bool") || peek("char") || peek("short") || peek("int") || peek("long") ||
           peek("struct") || peek("enum") ||
           peek("typedef") || peek("static") || find_typedef(token);
}

// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "switch" "(" expr ")" stmt
//        | "case" num ":" stmt
//        | "default" ":" stmt
//        | "while" "(" expr ")" stmt
//        | "for" "(" (expr? ";" | declaration) expr? ";" expr? ")" stmt
//        | "{" stmt* "}"
//        | "break" ";"
//        | "continue" ";"
//        | "goto" ident ";"
//        | ident ":" stmt
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

    if (tok = consume("switch"))
    {
        Node *node = new_node(ND_SWITCH, tok);
        expect("(");
        node->cond = expr();
        expect(")");

        Node *sw = current_switch;
        current_switch = node;
        node->then = stmt();
        current_switch = sw;
        return node;
    }

    if (tok = consume("case"))
    {
        if (!current_switch)
            error_tok(tok, "stray case");
        int val = expect_number(); // currently supports number only
        expect(":");

        Node *node = new_unary(ND_CASE, stmt(), tok);
        node->val = val;
        node->case_next = current_switch->case_next;
        current_switch->case_next = node;
        return node;
    }

    if (tok = consume("default"))
    {
        if (!current_switch)
            error_tok(tok, "stray default");
        expect(":");

        Node *node = new_unary(ND_CASE, stmt(), tok);
        current_switch->default_case = node;
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
        Scope *sc = enter_scope();

        if (!consume(";"))
        {
            if (is_typename())
            {
                node->init = declaration();
            }
            else
            {
                node->init = read_expr_stmt();
                expect(";");
            }
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

        leave_scope(sc);
        return node;
    }

    if (tok = consume("{"))
    {
        Node head;
        head.next = NULL;
        Node *cur = &head;

        Scope *sc = enter_scope();
        while (!consume("}"))
        {
            cur->next = stmt();
            cur = cur->next;
        }
        leave_scope(sc);

        Node *node = new_node(ND_BLOCK, tok);
        node->body = head.next;
        return node;
    }

    if (tok = consume("break"))
    {
        expect(";");
        return new_node(ND_BREAK, tok);
    }

    if (tok = consume("continue"))
    {
        expect(";");
        return new_node(ND_CONTINUE, tok);
    }

    if (tok = consume("goto"))
    {
        Node *node = new_node(ND_GOTO, tok);
        node->label_name = expect_ident();
        expect(";");
        return node;
    }

    if (tok = consume_ident())
    {
        if (consume(":"))
        {
            Node *node = new_unary(ND_LABEL, stmt(), tok);
            node->label_name = strndup(tok->str, tok->len);
            return node;
        }
        token = tok;
    }

    if (is_typename())
        return declaration();

    Node *node = read_expr_stmt();
    expect(";");
    return node;
}

// expr = assign ("," assign)*
Node *expr()
{
    Node *node = assign();
    Token *tok;
    while (tok = consume(","))
    {
        node = new_unary(ND_EXPR_STMT, node, node->tok);
        node = new_binary(ND_COMMA, node, assign(), tok);
    }
    return node;
}

// assign    = conditional (assign-op assign)?
// assign-op = "=" | "+=" | "-=" | "*=" | "/=" | "<<=" | ">>="
Node *assign()
{
    Node *node = conditional();
    Token *tok;
    if (tok = consume("="))
        node = new_binary(ND_ASSIGN, node, assign(), tok);
    if (tok = consume("+="))
        node = new_binary(ND_A_ADD, node, assign(), tok);
    if (tok = consume("-="))
        node = new_binary(ND_A_SUB, node, assign(), tok);
    if (tok = consume("*="))
        node = new_binary(ND_A_MUL, node, assign(), tok);
    if (tok = consume("/="))
        node = new_binary(ND_A_DIV, node, assign(), tok);
    if (tok = consume("<<="))
        node = new_binary(ND_A_SHL, node, assign(), tok);
    if (tok = consume(">>="))
        node = new_binary(ND_A_SHR, node, assign(), tok);
    return node;
}

// conditional = logor ("?" expr ":" conditional)?
Node *conditional()
{
    Node *node = logor();

    Token *tok = consume("?");
    if (!tok)
        return node;

    Node *ternary = new_node(ND_TERNARY, tok);
    ternary->cond = node;
    ternary->then = expr();
    expect(":");
    // // なんでexprじゃないんだろ
    ternary->els = conditional();
    return ternary;
}

// logor = logand ("||" logand)*
Node *logor()
{
    Node *node = logand();
    Token *tok;
    while (tok = consume("||"))
        node = new_binary(ND_LOGOR, node, logand(), tok);
    return node;
}

// logand = bitor ("&&" bitor)*
Node *logand()
{
    Node *node = bitor ();
    Token *tok;
    while (tok = consume("&&"))
        node = new_binary(ND_LOGAND, node, bitor (), tok);
    return node;
}

// bitor = bitxor ("|" bitxor)*
Node * bitor ()
{
    Node *node = bitxor();
    Token *tok;
    while (tok = consume("|"))
        node = new_binary(ND_BITOR, node, bitxor(), tok);
    return node;
}

// bitxor = bitand ("^" bitand)*
Node *bitxor()
{
    Node *node = bitand();
    Token *tok;
    while (tok = consume("^"))
        node = new_binary(ND_BITXOR, node, bitand(), tok);
    return node;
}

// bitand = equality ("&" equality)*
Node *bitand()
{
    Node *node = equality();
    Token *tok;
    while (tok = consume("&"))
        node = new_binary(ND_BITAND, node, equality(), tok);
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

// relational = shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
Node *relational()
{
    Node *node = shift();
    Token *tok;

    for (;;)
    {
        if (tok = consume("<"))
            node = new_binary(ND_LT, node, shift(), tok);
        if (tok = consume("<="))
            node = new_binary(ND_LE, node, shift(), tok);
        if (tok = consume(">"))
            node = new_binary(ND_LT, shift(), node, tok);
        if (tok = consume(">="))
            node = new_binary(ND_LE, shift(), node, tok);
        else
            return node;
    }
}

// shift = add ("<<" add | ">>" add)*
Node *shift()
{
    Node *node = add();
    Token *tok;

    for (;;)
    {
        if (tok = consume("<<"))
            node = new_binary(ND_SHL, node, add(), tok);
        else if (tok = consume(">>"))
            node = new_binary(ND_SHR, node, add(), tok);
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

// mul = cast("*" cast | "/" cast)*
Node *mul()
{
    Node *node = cast();
    Token *tok;

    for (;;)
    {
        if (tok = consume("*"))
            node = new_binary(ND_MUL, node, cast(), tok);
        if (tok = consume("/"))
            node = new_binary(ND_DIV, node, cast(), tok);
        else
            return node;
    }
}

// cast = "(" type-name ")" cast | unary
Node *cast()
{
    Token *tok = token;

    if (consume("("))
    {
        if (is_typename())
        {
            Type *ty = type_name();
            expect(")");
            Node *node = new_unary(ND_CAST, cast(), tok);
            node->ty = ty;
            return node;
        }
        token = tok;
    }

    return unary();
}

// unary = ("+" | "-" | "*" | "&" | "!" | "~")? cast
//       | ("++" | "--") unary
//       | postfix
Node *unary()
{
    Token *tok;
    if (tok = consume("+"))
        return cast();
    if (tok = consume("-"))
        return new_binary(ND_SUB, new_node_num(0, tok), cast(), tok);

    if (tok = consume("*"))
        return new_unary(ND_DEREF, cast(), tok);
    if (tok = consume("&"))
        return new_unary(ND_ADDR, cast(), tok);

    if (tok = consume("!"))
        return new_unary(ND_NOT, cast(), tok);

    if (tok = consume("~"))
        return new_unary(ND_BITNOT, cast(), tok);

    if (tok = consume("++"))
        return new_unary(ND_PRE_INC, unary(), tok);
    if (tok = consume("--"))
        return new_unary(ND_PRE_DEC, unary(), tok);
    return postfix();
}

// postfix = primary ("[" expr "]" | "." ident | "->" ident | "++" | "--")*
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
        if (tok = consume("++"))
        {
            node = new_unary(ND_POST_INC, node, tok);
            continue;
        }
        if (tok = consume("--"))
        {
            node = new_unary(ND_POST_DEC, node, tok);
            continue;
        }
        return node;
    }
}

// stmt-expr = "(" "{" stmt stmt* "}" ")"
// Statement expression is a GNU C extension
Node *stmt_expr(Token *tok)
{
    Scope *sc = enter_scope();
    Node *node = new_node(ND_STMT_EXPR, tok);
    node->body = stmt();
    Node *cur = node->body;

    while (!consume("}"))
    {
        cur->next = stmt();
        cur = cur->next;
    }
    expect(")");
    leave_scope(sc);

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
//         | "sizeof" "(" type-name ")"
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
    {
        if (consume("("))
        {
            if (is_typename())
            {
                Type *ty = type_name();
                expect(")");
                return new_node_num(size_of(ty, tok), tok);
            }
            token = tok->next;
        }
        return new_unary(ND_SIZEOF, unary(), tok);
    }

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
        if (sc)
        {
            if (sc->var)
                return new_var(sc->var, tok);
            if (sc->enum_ty)
                return new_node_num(sc->enum_val, tok);
        }
        error_tok(tok, "undefined variable");
    }

    tok = token;

    if (tok->kind == TK_STR)
    {
        token = token->next;
        // 文字列リテラルはcharの配列であり、その場でグローバル変数を定義して、初期化内容をVarに保持しつつ、codegen時に生成して実現されている。
        Type *ty = array_of(char_type(), tok->cont_len);
        Var *var = push_var(new_label(), ty, false, tok);

        var->initializer = gvar_init_string(tok->contents, tok->cont_len);
        return new_var(var, tok);
    }

    if (tok->kind != TK_NUM)
        error_tok(tok, "expected expression");

    return new_node_num(expect_number(), tok);
}