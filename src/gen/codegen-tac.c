#include "chibicc.h"

FILE *OUT;

static int nnext_tmp;

static const char *ty_kind_map[] = {
    "void",    "bool", "char", "short", "ind",   "long", "float",  "double",
    "ldouble", "enum", "ptr",  "func",  "array", "vla",  "struct", "union",
};

__attribute__((format(printf, 1, 2))) static void println(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  // vfprintf(OUT, fmt, ap);
  va_end(ap);
  printf("\n");
  // fprintf(OUT, "\n");
}

static const char *next_tmp(int ty_kind) {
  char *_buf = calloc(32, 1);
  sprintf(_buf, "$t%d", nnext_tmp++);
  return _buf;
}

static const char *number(Node *node) {
  const char *tmp = next_tmp(node->ty->kind);
  switch (node->ty->kind) {
  case TY_FLOAT:
  case TY_DOUBLE:
  case TY_LDOUBLE:
    println("%s = %Lf", tmp, node->fval);
    break;
  default:
    println("%s = %ld", tmp, node->val);
    break;
  }

  return tmp;
}

// Function return variable name
static const char *gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    return number(node);
  }

  error_tok(node->tok, "invalid expression");
}

static void gen_stmt(Node *node) {
  // println("  .loc %d %d", node->tok->file->file_no, node->tok->line_no);
  printf("%d\n", node->kind);
  switch (node->kind) {
  // case ND_IF: {
  //   int c = count();
  //   gen_expr(node->cond);
  //   cmp_zero(node->cond->ty);
  //   println("  je  .L.else.%d", c);
  //   gen_stmt(node->then);
  //   println("  jmp .L.end.%d", c);
  //   println(".L.else.%d:", c);
  //   if (node->els)
  //     gen_stmt(node->els);
  //   println(".L.end.%d:", c);
  //   return;
  // }
  // case ND_FOR: {
  //   int c = count();
  //   if (node->init)
  //     gen_stmt(node->init);
  //   println(".L.begin.%d:", c);
  //   if (node->cond) {
  //     gen_expr(node->cond);
  //     cmp_zero(node->cond->ty);
  //     println("  je %s", node->brk_label);
  //   }
  //   gen_stmt(node->then);
  //   println("%s:", node->cont_label);
  //   if (node->inc)
  //     gen_expr(node->inc);
  //   println("  jmp .L.begin.%d", c);
  //   println("%s:", node->brk_label);
  //   return;
  // }
  // case ND_DO: {
  //   int c = count();
  //   println(".L.begin.%d:", c);
  //   gen_stmt(node->then);
  //   println("%s:", node->cont_label);
  //   gen_expr(node->cond);
  //   cmp_zero(node->cond->ty);
  //   println("  jne .L.begin.%d", c);
  //   println("%s:", node->brk_label);
  //   return;
  // }
  // case ND_SWITCH:
  //   gen_expr(node->cond);

  //   for (Node *n = node->case_next; n; n = n->case_next) {
  //     char *ax = (node->cond->ty->size == 8) ? "%rax" : "%eax";
  //     char *di = (node->cond->ty->size == 8) ? "%rdi" : "%edi";

  //     if (n->begin == n->end) {
  //       println("  cmp $%ld, %s", n->begin, ax);
  //       println("  je %s", n->label);
  //       continue;
  //     }

  //     // [GNU] Case ranges
  //     println("  mov %s, %s", ax, di);
  //     println("  sub $%ld, %s", n->begin, di);
  //     println("  cmp $%ld, %s", n->end - n->begin, di);
  //     println("  jbe %s", n->label);
  //   }

  //   if (node->default_case)
  //     println("  jmp %s", node->default_case->label);

  //   println("  jmp %s", node->brk_label);
  //   gen_stmt(node->then);
  //   println("%s:", node->brk_label);
  //   return;
  // case ND_CASE:
  //   println("%s:", node->label);
  //   gen_stmt(node->lhs);
  //   return;
  case ND_BLOCK:
    for (Node *n = node->body; n; n = n->next)
      gen_stmt(n);
    return;
  // case ND_GOTO:
  //   println("  jmp %s", node->unique_label);
  //   return;
  // case ND_GOTO_EXPR:
  //   gen_expr(node->lhs);
  //   println("  jmp *%%rax");
  //   return;
  // case ND_LABEL:
  //   println("%s:", node->unique_label);
  //   gen_stmt(node->lhs);
  //   return;
  // case ND_RETURN:
  //   if (node->lhs) {
  //     gen_expr(node->lhs);
  //     Type *ty = node->lhs->ty;

  //     switch (ty->kind) {
  //     case TY_STRUCT:
  //     case TY_UNION:
  //       if (ty->size <= 16)
  //         copy_struct_reg();
  //       else
  //         copy_struct_mem();
  //       break;
  //     }
  //   }

  //   println("  jmp .L.return.%s", current_fn->name);
  //   return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    return;
    // case ND_ASM:
    //   println("  %s", node->asm_str);
    //   return;
  }

  error_tok(node->tok, "invalid statement");
}

static void emit_text(Obj *prog) {
  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_function || !fn->is_definition)
      continue;

    // No code is emitted for "static inline" functions
    // if no one is referencing them.
    // if (!fn->is_live)
    //   continue;

    if (fn->is_static)
      println("func $%s", fn->name);
    else
      println("func %s", fn->name);

    // Save passed-by-register arguments to the stack
    // int gp = 0, fp = 0;
    // for (Obj *var = fn->params; var; var = var->next) {
    //   if (var->offset > 0)
    //     continue;

    //   Type *ty = var->ty;

    //   switch (ty->kind) {
    //   case TY_STRUCT:
    //   case TY_UNION:
    //     assert(ty->size <= 16);
    //     if (has_flonum(ty, 0, 8, 0))
    //       store_fp(fp++, var->offset, MIN(8, ty->size));
    //     else
    //       store_gp(gp++, var->offset, MIN(8, ty->size));

    //     if (ty->size > 8) {
    //       if (has_flonum(ty, 8, 16, 0))
    //         store_fp(fp++, var->offset + 8, ty->size - 8);
    //       else
    //         store_gp(gp++, var->offset + 8, ty->size - 8);
    //     }
    //     break;
    //   case TY_FLOAT:
    //   case TY_DOUBLE:
    //     store_fp(fp++, var->offset, ty->size);
    //     break;
    //   default:
    //     store_gp(gp++, var->offset, ty->size);
    //   }
    // }

    // // Emit code
    gen_stmt(fn->body);
    // assert(depth == 0);

    // [https://www.sigbus.info/n1570#5.1.2.2.3p1] The C spec defines
    // a special rule for the main function. Reaching the end of the
    // main function is equivalent to returning 0, even though the
    // behavior is undefined for the other functions.
    // if (strcmp(fn->name, "main") == 0)
    //   println("  mov $0, %%rax");

    println("endfunc");
  }
}

void codegen_target_tac(Obj *prog, FILE *out) {
  OUT = out;

  emit_text(prog);
}
