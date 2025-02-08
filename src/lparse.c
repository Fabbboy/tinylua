#include "lparse.h"
#include "last.h"
#include "llex.h"
#include "llog.h"
#include "ltypes.h"
#include <stdbool.h>
#include <stdlib.h>

#define EXP_LIST(name, ...)                                                    \
  kind_t name[] = {__VA_ARGS__};                                               \
  u32 name##_len = sizeof(name) / sizeof(name[0]);

const int precedence[] = {
    ['+'] = 1,
    ['-'] = 1,
    ['*'] = 2,
    ['/'] = 2,
};

const bin_op_t bin_ops[] = {
    ['+'] = BO_ADD,
    ['-'] = BO_SUB,
    ['*'] = BO_MUL,
    ['/'] = BO_DIV,
};

static bool next(lparser_t *parser, tok_t *result, kind_t expected[],
                 u32 expected_len) {
  kind_t got = lexer_next(parser->lexer);
  *result = parser->lexer->currTok;

  for (u32 i = 0; i < expected_len; i++) {
    if (expected[i] == got) {
      return true;
    }
  }

  fbuf_write(&parser->errBuf, "Expected one of [");
  for (u32 i = 0; i < expected_len; i++) {
    fbuf_write(&parser->errBuf, "%s%s", kind_names[expected[i]],
               (i == expected_len - 1) ? "" : ", ");
  }
  fbuf_write(&parser->errBuf, "], but got: %s\n", kind_names[got]);
  ERROR_LOG("%s", fbuf_get(&parser->errBuf));
  fbuf_reset(&parser->errBuf);
  return false;
}

static bool peek(lparser_t *parser, kind_t expected[], u32 expected_len) {
  kind_t got = lexer_peek(parser->lexer);
  for (u32 i = 0; i < expected_len; i++) {
    if (expected[i] == got) {
      return true;
    }
  }

  return false;
}

static void sync(lparser_t *parser, kind_t expected[], u32 expected_len) {
  kind_t got = parser->lexer->currTok.type;
  for (u32 i = 0; i < expected_len; i++) {
    if (expected[i] == got) {
      return;
    }
  }

  while (got != KIND_EOF) {
    for (u32 i = 0; i < expected_len; i++) {
      if (expected[i] == got) {
        return;
      }
    }

    got = lexer_next(parser->lexer);
  }
}

static lexpr_t *factor(lparser_t *parser) {
  EXP_LIST(exp_number, KIND_NUMBER, KIND_DECIMAL);
  tok_t tok;
  if (!next(parser, &tok, exp_number, exp_number_len)) {
    return NULL;
  }

  lexpr_t *expr = new_lexpr(LEXPK_LITERAL);
  switch (tok.type) {
  case KIND_NUMBER:
    expr->literal.vt = VT_INT;
    expr->literal.ival = atol(tok.start);
    break;
  case KIND_DECIMAL:
    expr->literal.vt = VT_FLOAT;
    expr->literal.fval = atof(tok.start);
    break;
  default:
    UNREACHABLE
  }

  return expr;
}

static lexpr_t *term(lparser_t *parser) {
  lexpr_t *lhs = factor(parser);
  if (!lhs)
    return NULL;

  EXP_LIST(exp_muldiv, KIND_STAR, KIND_SLASH);
  while (peek(parser, exp_muldiv, exp_muldiv_len)) {
    tok_t op;
    next(parser, &op, exp_muldiv, exp_muldiv_len);

    lexpr_t *rhs = factor(parser);
    if (!rhs)
      return NULL;

    lexpr_t *binary = new_lexpr(LEXPK_BINARY);
    binary->binary.op = bin_ops[(u32)op.start[0]];
    binary->binary.left = lhs;
    binary->binary.right = rhs;
    lhs = binary;
  }

  return lhs;
}

static lexpr_t *expr(lparser_t *parser) {
  lexpr_t *lhs = term(parser);
  if (!lhs)
    return NULL;

  EXP_LIST(exp_addsub, KIND_PLUS, KIND_MINUS);
  while (peek(parser, exp_addsub, exp_addsub_len)) {
    tok_t op;
    next(parser, &op, exp_addsub, exp_addsub_len);

    lexpr_t *rhs = term(parser);
    if (!rhs)
      return NULL;

    lexpr_t *binary = new_lexpr(LEXPK_BINARY);
    binary->binary.op = bin_ops[(u32)op.start[0]];
    binary->binary.left = lhs;
    binary->binary.right = rhs;
    lhs = binary;
  }

  return lhs;
}

static lvar_stmt *varstmt(lparser_t *parser) {
  EXP_LIST(exp_name, KIND_IDENT);
  tok_t name;
  tok_t assign;
  bool res = next(parser, &name, exp_name, exp_name_len);
  if (!res) {
    return NULL;
  }

  EXP_LIST(exp_assign, KIND_ASSIGN);
  res = next(parser, &assign, exp_assign, exp_assign_len);
  if (!res) {
    return NULL;
  }

  lexpr_t *val = expr(parser);
  if (val == NULL) {
    return NULL;
  }
  return new_lvar_stmt(name, val);
}

void parser_init(lparser_t *parser, llexer_t *lexer) {
  CHECK_NULL(parser, )
  CHECK_NULL(lexer, )

  parser->lexer = lexer;
  parser->ast = new_ast();
  parser->errBuf = new_fbuf(256);
};

void parser_parse(lparser_t *parser) {
  CHECK_NULL(parser, )
  tok_t tok;
  kind_t expected[] = {KIND_EOF, KIND_LOCAL};
  u32 expected_len = sizeof(expected) / sizeof(expected[0]);
  union {
    lvar_stmt *var;
  } stmt;
  do {
    bool res = next(parser, &tok, expected, expected_len);
    if (!res)
      continue;

    if (tok.type == KIND_EOF)
      break;

    switch (tok.type) {
    case KIND_LOCAL:
      stmt.var = varstmt(parser);
      if (stmt.var == NULL) {
        DEBUG_LOG("Failed to parse varstmt\n");
        sync(parser, expected, expected_len);
        continue;
      }
      list_push(&parser->ast.globals, stmt.var);
      break;
    default:
      UNREACHABLE
    }

  } while (tok.type != KIND_EOF);

  return;
}

void parser_free(lparser_t *parser) {
  CHECK_NULL(parser, );
  fbuf_free(&parser->errBuf);
  ast_free(&parser->ast);
};