#include "main.h"

int labelseq;
int brkseq;
int contseq;

// https://www.sigbus.info/compilerbook#%E6%95%B4%E6%95%B0%E3%83%AC%E3%82%B8%E3%82%B9%E3%82%BF%E3%81%AE%E4%B8%80%E8%A6%A7
char *argreg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *argreg2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
char *argreg4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char *argreg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

char *funcname;

void gen(Node *node);

void gen_addr(Node *node)
{
    switch (node->kind)
    {
    case ND_VAR:
    {
        Var *var = node->var;
        if (var->is_local)
        {
            printf("  lea rax, [rbp-%d]\n", var->offset);
            printf("  push rax\n");
        }
        else
        {
            printf("  push offset %s\n", var->name);
        }
        return;
    }
    case ND_DEREF:
        gen(node->lhs);
        return;
    case ND_MEMBER:
        gen_addr(node->lhs);
        printf("  pop rax\n");
        printf("  add rax, %d\n", node->member->offset);
        printf("  push rax\n");
        return;
    }

    error_tok(node->tok, "not an lvalue");
}
void gen_lval(Node *node)
{
    if (node->ty->kind == TY_ARRAY)
        error_tok(node->tok, "not an lvalue");
    gen_addr(node);
}

void load(Type *ty)
{
    printf("  pop rax\n");

    int sz = size_of(ty, NULL);
    if (sz == 1)
        printf("  movsx rax, byte ptr [rax]\n");
    else if (sz == 2)
    {
        printf("  movsx rax, word ptr [rax]\n");
    }
    else if (sz == 4)
    {
        printf("  movsx rax, dword ptr [rax]\n");
    }
    else
    {
        assert(sz == 8);
        printf("  mov rax, [rax]\n");
    }
    printf("  push rax\n");
}

void store(Type *ty)
{
    printf("  pop rdi\n"); // has value
    printf("  pop rax\n"); // has pointer location to save

    if (ty->kind == TY_BOOL)
    {
        printf("  cmp rdi, 0\n");
        printf("  setne dil\n");
        printf("  movzb rdi, dil\n");
    }

    int sz = size_of(ty, NULL);
    if (sz == 1)
        printf("  mov [rax], dil\n");
    else if (sz == 2)
    {
        printf("  mov [rax], di\n");
    }
    else if (sz == 4)
    {
        printf("  mov [rax], edi\n");
    }
    else
    {
        assert(sz == 8);
        printf("  mov [rax], rdi\n");
    }
    printf("  push rdi\n"); // returns the value
}

void truncate(Type *ty)
{
    printf("  pop rax\n");

    if (ty->kind == TY_BOOL)
    {
        // 0より多ければ1とする
        printf("  cmp rax, 0\n");
        printf("  setne al\n");
    }

    // https://www.sigbus.info/compilerbook#:~:text=%E3%82%B3%E3%83%A9%E3%83%A0%3A%208%E3%83%93%E3%83%83%E3%83%88%E3%83%AC%E3%82%B8%E3%82%B9%E3%82%BF%E3%81%A832%E3%83%93%E3%83%83%E3%83%88%E3%83%AC%E3%82%B8%E3%82%B9%E3%82%BF%E3%81%AE%E9%81%95%E3%81%84
    int sz = size_of(ty, NULL);
    if (sz == 1)
    {
        printf("  movsx rax, al\n");
    }
    else if (sz == 2)
    {
        printf("  movsx rax, ax\n");
    }
    else if (sz == 4)
    {
        printf("  movsxd rax, eax\n");
    }
    printf("  push rax\n");
}

void inc(Node *node)
{
    printf("  pop rax\n");
    printf("  add rax, %d\n", node->ty->base ? size_of(node->ty->base, node->tok) : 1);
    printf("  push rax\n");
}

void dec(Node *node)
{
    printf("  pop rax\n");
    printf("  sub rax, %d\n", node->ty->base ? size_of(node->ty->base, node->tok) : 1);
    printf("  push rax\n");
}

// genは、Nodeに応じてcodeを生成している。なので、genの結果、スタックが増減するかしないかは、Nodeの種類に依存する
void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NULL:
        return;
    case ND_NUM:
        if (node->val == (int)node->val)
        {
            printf("  push %ld\n", node->val);
        }
        else
        {
            printf("  movabs rax, %ld\n", node->val);
            printf("  push rax\n");
        }
        return;
    case ND_EXPR_STMT:
        gen(node->lhs);           // EXPRをgenして
        printf("  add rsp, 8\n"); // 直ちにstackを減らして、↑を捨ててる
        return;
    case ND_VAR:
    case ND_MEMBER:
        gen_addr(node);
        if (node->ty->kind != TY_ARRAY)
            load(node->ty);
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs); // a pointer is pushed into the stack
        gen(node->rhs);      // value is pushed to the stack
        store(node->ty);
        return;
    case ND_PRE_INC:
        gen_lval(node->lhs);      // 変数のポインタをスタックにpushする。後にstoreするためにスタックに乗せる。
        printf("  push [rsp]\n"); // [rsp]はスタックの頭なので、これは同じ値をpushし直すことになる。
        load(node->ty);           // ポインタを、ポインタが指す値に置き換える
        inc(node);                // 値を増やす
        store(node->ty);          // gen_lvalのときにスタックに乗せたポインタに、上の値をstoreする
        return;
    case ND_PRE_DEC:
        gen_lval(node->lhs);
        printf("  push [rsp]\n");
        load(node->ty);
        dec(node);
        store(node->ty);
        return;
    case ND_POST_INC:
        gen_lval(node->lhs);
        printf("  push [rsp]\n");
        load(node->ty);
        inc(node);       // 先に+1しておく
        store(node->ty); // 変数のポインタが指す値は、増えた値がstoreされる。変数のポインタの値は、1増えたものになる。
        dec(node);       // storeの結果、値がstackに残る。これを-1しておくことで、スタック上の値は、+-で元の値になる。
        return;
    case ND_POST_DEC:
        gen_lval(node->lhs);
        printf("  push [rsp]\n");
        load(node->ty);
        dec(node);
        store(node->ty);
        inc(node);
        return;
    case ND_A_ADD:
    case ND_A_SUB:
    case ND_A_MUL:
    case ND_A_DIV:
    {
        gen_lval(node->lhs);      // 後のstoreのためにポインタをpush
        printf("  push [rsp]\n"); // 同じポインタをpush
        load(node->lhs->ty);      // 値を読む
        gen(node->rhs);           // スタックに右辺をpush
        printf("  pop rdi\n");    // 右辺の値
        printf("  pop rax\n");    // 左辺の変数の値

        switch (node->kind)
        {
        case ND_A_ADD:
            if (node->ty->base)
                printf("  imul rdi, %d\n", size_of(node->ty->base, node->tok));
            printf("  add rax, rdi\n");
            break;
        case ND_A_SUB:
            if (node->ty->base)
                printf("  imul rdi, %d\n", size_of(node->ty->base, node->tok));
            printf("  sub rax, rdi\n");
            break;
        case ND_A_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_A_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        }

        printf("  push rax\n");
        store(node->ty);
        return;
    }
    case ND_COMMA:
        gen(node->lhs); // ND_EXPR_STMT
        gen(node->rhs); // last assign
        return;
    case ND_ADDR:
        gen_addr(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
        if (node->ty->kind != TY_ARRAY)
            load(node->ty);
        return;
    case ND_NOT:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
        return;
    case ND_BITNOT:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  not rax\n");
        printf("  push rax\n");
        return;
    case ND_LOGAND:
    {
        int seq = labelseq++;
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lfalse%d\n", seq);
        gen(node->rhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lfalse%d\n", seq);
        printf("  push 1\n");
        printf("  jmp .Lend%d\n", seq);
        printf(".Lfalse%d:\n", seq);
        printf("  push 0\n");
        printf(".Lend%d:\n", seq);
        return;
    }
    case ND_LOGOR:
    {
        int seq = labelseq++;
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .Ltrue%d\n", seq);
        gen(node->rhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .Ltrue%d\n", seq);
        printf("  push 0\n");
        printf("  jmp .Lend%d\n", seq);
        printf(".Ltrue%d:\n", seq);
        printf("  push 1\n");
        printf(".Lend%d:\n", seq);
        return;
    }
    case ND_IF:
    {
        int seq = labelseq++;
        if (node->els)
        {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lelse%d\n", seq);
            gen(node->then);
            printf("  jmp .Lend%d\n", seq);
            printf(".Lelse%d:\n", seq);
            gen(node->els);
            printf(".Lend%d:\n", seq);
        }
        else
        {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%d\n", seq);
            gen(node->then);
            printf(".Lend%d:\n", seq);
        }
        return;
    }
    case ND_WHILE:
    {

        int seq = labelseq++;
        int brk = brkseq;
        int cont = contseq;
        brkseq = contseq = seq;

        printf(".L.continue.%d:\n", seq);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .L.break.%d\n", seq);
        gen(node->then);
        printf("  jmp  .L.continue.%d\n", seq);
        printf(".L.break.%d:\n", seq);

        brkseq = brk;
        contseq = cont;
        return;
    }
    case ND_FOR:
    {

        int seq = labelseq++;
        int brk = brkseq;
        int cont = contseq;
        brkseq = contseq = seq;

        if (node->init)
            gen(node->init);
        printf(".Lbegin%d:\n", seq);
        if (node->cond)
        {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .L.break.%d\n", seq);
        }
        gen(node->then);
        printf(".L.continue.%d:\n", seq);
        if (node->inc)
            gen(node->inc);
        printf("  jmp .Lbegin%d\n", seq);
        printf(".L.break.%d:\n", seq);

        brkseq = brk;
        contseq = cont;
        return;
    }
    case ND_BLOCK:
    case ND_STMT_EXPR:
        for (Node *n = node->body; n; n = n->next)
            gen(n);
        return;
    case ND_BREAK:
        if (brkseq == 0)
            error_tok(node->tok, "stray break");
        printf("  jmp .L.break.%d\n", brkseq);
        return;
    case ND_CONTINUE:
        if (contseq == 0)
            error_tok(node->tok, "stray continue");
        printf("  jmp .L.continue.%d\n", contseq);
        return;
    case ND_FUNCALL:
    {
        int nargs = 0;
        for (Node *arg = node->args; arg; arg = arg->next)
        {
            gen(arg);
            nargs++;
        }
        for (int i = nargs - 1; i >= 0; i--)
        {
            printf("  pop %s\n", argreg8[i]);
        }
        // We need to align RSP to a 16 byte boundary before
        // calling a function because it is an ABI requirement.
        // RAX is set to 0 for variadic function.

        // pushやpopは8バイト単位で変更するので、RSPは16バイト単位で0とは限らない
        int seq = labelseq++;
        printf("  mov rax, rsp\n");
        printf("  and rax, 15\n"); // ここで16の倍数であるかを確認する
        printf("  jnz .Lcall%d\n", seq);
        printf("  mov rax, 0\n");
        printf("  call %s\n", node->funcname);
        printf("  jmp .Lend%d\n", seq);
        printf(".Lcall%d:\n", seq);
        printf("  sub rsp, 8\n"); // 8引いて16の倍数にしてうr
        printf("  mov rax, 0\n");
        printf("  call %s\n", node->funcname);
        printf("  add rsp, 8\n"); // ずらした分を戻しておく
        printf(".Lend%d:\n", seq);
        printf("  push rax\n");

        truncate(node->ty);
        return;
    }
    case ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  jmp .Lreturn.%s\n", funcname);
        return;
    case ND_CAST:
        gen(node->lhs);
        truncate(node->ty);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        if (node->ty->base)
            printf("  imul rdi, %d\n", size_of(node->ty->base, node->tok));
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        if (node->ty->base)
            printf("  imul rdi, %d\n", size_of(node->ty->base, node->tok));
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
    case ND_BITAND:
        printf("  and rax, rdi\n");
        break;
    case ND_BITOR:
        printf("  or rax, rdi\n");
        break;
    case ND_BITXOR:
        printf("  xor rax, rdi\n");
        break;
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    }

    printf("  push rax\n");
}

void emit_data(Program *prog)
{
    printf(".data\n");

    for (VarList *vl = prog->globals; vl; vl = vl->next)
    {
        Var *var = vl->var;
        printf("%s:\n", var->name);

        if (!var->contents)
        {
            printf("  .zero %d\n", size_of(var->ty, var->tok));
            continue;
        }
        for (int i = 0; i < var->cont_len; i++)
            printf("  .byte %d\n", var->contents[i]);
    }
}

void load_arg(Var *var, int idx)
{
    int sz = size_of(var->ty, var->tok);
    if (sz == 1)
    {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg1[idx]);
    }
    else if (sz == 2)
    {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg2[idx]);
    }
    else if (sz == 4)
    {
        printf("  mov [rbp-%d], %s\n", var->offset, argreg4[idx]);
    }
    else
    {
        assert(sz == 8);
        printf("  mov [rbp-%d], %s\n", var->offset, argreg8[idx]);
    }
}

void emit_text(Program *prog)
{
    printf(".text\n");

    for (Function *fn = prog->fns; fn; fn = fn->next)
    {
        printf(".global %s\n", fn->name);
        printf("%s:\n", fn->name);
        funcname = fn->name;

        // Prologue
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", fn->stack_size);

        int i = 0;
        for (VarList *vl = fn->params; vl; vl = vl->next)
        {
            load_arg(vl->var, i++);
        }

        for (Node *n = fn->node; n; n = n->next)
        {
            gen(n);
        }

        // Epilogue
        printf(".Lreturn.%s:\n", funcname);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");

        printf("  ret\n");
    }
}

void codegen(Program *prog)
{
    printf(".intel_syntax noprefix\n");
    emit_data(prog);
    emit_text(prog);
}