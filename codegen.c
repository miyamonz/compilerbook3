#include "main.h"

int labelseq = 0;

void gen_addr(Node *node)
{
    if (node->kind == ND_VAR)
    {
        printf("  lea rax, [rbp-%d]\n", node->var->offset);
        printf("  push rax\n");
        return;
    }

    error("not an lvalue");
}

void load()
{
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
}

void store()
{
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
}

void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_EXPR_STMT:
        gen(node->lhs);
        printf("  add rsp, 8\n");
        return;
    case ND_VAR:
        gen_addr(node);
        load();
        return;
    case ND_ASSIGN:
        gen_addr(node->lhs);
        gen(node->rhs);
        store();
        return;
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
        printf(".Lbegin%d:\n", seq);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", seq);
        gen(node->then);
        printf("  jmp .Lbegin%d\n", seq);
        printf(".Lend%d:\n", seq);
        return;
    }
    case ND_FOR:
    {

        int seq = labelseq++;
        if (node->init)
            gen(node->init);
        printf(".Lbegin%d:\n", seq);
        if (node->cond)
        {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%d\n", seq);
        }
        gen(node->then);
        if (node->inc)
            gen(node->inc);
        printf("  jmp .Lbegin%d\n", seq);
        printf(".Lend%d:\n", seq);
        return;
    }
    case ND_BLOCK:
        for (Node *n = node->body; n; n = n->next)
            gen(n);
        return;
    case ND_FUNCALL:
        printf("  call %s\n", node->funcname);
        printf("  push rax\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  jmp .Lreturn\n");
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

void codegen(Program *prog)

{
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", prog->stack_size);

    for (Node *n = prog->node; n; n = n->next)
    {
        gen(n);
    }

    // Epilogue
    printf(".Lreturn:\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");

    printf("  ret\n");
}