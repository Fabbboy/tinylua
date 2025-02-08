#include "lparse.h"
#include "lanalyzer.h"
#include "last.h"
#include "llex.h"
#include "llog.h"
#include "ltypes.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

static lexpr_t *factor(lparser_t *parser);
static lexpr_t *term(lparser_t *parser);
static lexpr_t *expr(lparser_t *parser);

static bool next(lparser_t *parser, tok_t *result, kind_t *expected,
                 u32 expected_len) {
  if (expected_len > MAX_EXPECTED_KINDS) {
    ERROR_LOG("Expected list too long\n");
    expected_len = MAX_EXPECTED_KINDS;
  }

  kind_t got = lexer_next(parser->lexer);
  *result = parser->lexer->currTok;

  if (got == KIND_ERR) {
    lparse_err_t *err = new_lexer_err(*result);
    list_push(&parser->errs, err);
    return false;
  }

  if (expected_len == 0) {
    return true;
  }

  for (u32 i = 0; i < expected_len; i++) {
    if (expected[i] == got) {
      return true;
    }
  }

  lparse_err_t *err = new_unexpected_err(*result, expected, expected_len);
  list_push(&parser->errs, err);

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
  tok_t curr = parser->lexer->currTok;
  while (curr.type != KIND_EOF) {
    for (u32 i = 0; i < expected_len; i++) {
      if (expected[i] == curr.type) {
        return;
      }
    }

    lexer_next(parser->lexer);
    curr = parser->lexer->nextTok;
  }
}

static lexpr_t *factor(lparser_t *parser) {
  EXP_LIST(exp_number, KIND_NUMBER, KIND_DECIMAL, KIND_LPAREN, KIND_IDENT);
  tok_t tok;
  if (!next(parser, &tok, exp_number, exp_number_len)) {
    return NULL;
  }

  lexpr_t *_expr = new_lexpr(LEXPK_LITERAL);
  switch (tok.type) {
  case KIND_NUMBER:
    _expr->literal.vt = VT_INT;
    _expr->literal.ival = atol(tok.start);
    break;
  case KIND_DECIMAL:
    _expr->literal.vt = VT_FLOAT;
    _expr->literal.fval = atof(tok.start);
    break;
  case KIND_LPAREN:
    _expr = expr(parser);
    EXP_LIST(exp_rparen, KIND_RPAREN);
    if (!next(parser, &tok, exp_rparen, exp_rparen_len)) {
      ERROR_LOG("Expected ')' after expression\n");
      return NULL;
    }
    break;
  case KIND_IDENT:
    _expr->kind = LEXPK_VAR;
    _expr->variable.name = tok;
    break;
  default:
    UNREACHABLE
  }

  return _expr;
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

static lvar_stmt *varstmt(lparser_t *parser, linkage_t link) {
  EXP_LIST(exp_name, KIND_IDENT);
  tok_t name;
  tok_t assign;
  bool res;
  if (link == LINK_EXTERNAL) {
    name = parser->lexer->currTok;
  } else {
    res = next(parser, &name, exp_name, exp_name_len);
    if (!res) {
      return NULL;
    }
  }

  EXP_LIST(exp_colon, KIND_COLON);
  lvalue_type_t type = VT_UNTYPED;
  if (peek(parser, exp_colon, exp_colon_len)) {
    res = next(parser, &assign, exp_colon, exp_colon_len);
    if (!res) {
      return NULL;
    }

    EXP_LIST(exp_type, KIND_TYPE);
    tok_t type_tok;
    res = next(parser, &type_tok, exp_type, exp_type_len);
    if (!res) {
      return NULL;
    }
    type = type_tok.vt;
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
  return new_lvar_stmt(name, val, link, type);
}

void parser_init(lparser_t *parser, llexer_t *lexer, lanalyzer_t *analyzer) {
  CHECK_NULL(parser, )
  CHECK_NULL(lexer, )

  parser->lexer = lexer;
  parser->analyzer = analyzer;
  parser->ast = new_ast();
  parser->errs = new_list(8);
};

perr_list_t *parser_parse(lparser_t *parser) {
  CHECK_NULL(parser, NULL);
  tok_t tok;
  EXP_LIST(expected, KIND_EOF, KIND_LOCAL, KIND_IDENT);
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
      stmt.var = varstmt(parser, LINK_INTERNAL);
      if (stmt.var == NULL) {
        DEBUG_LOG("Failed to parse varstmt\n");
        sync(parser, expected, expected_len);
        continue;
      }
      ana_infer_type(parser->analyzer, stmt.var, true);
      list_push(&parser->ast.globals, stmt.var);
      break;
    case KIND_IDENT:
      stmt.var = varstmt(parser, LINK_EXTERNAL);
      if (stmt.var == NULL) {
        DEBUG_LOG("Failed to parse varstmt\n");
        sync(parser, expected, expected_len);
        continue;
      }
      ana_infer_type(parser->analyzer, stmt.var, true);
      list_push(&parser->ast.globals, stmt.var);
      break;
    default:
      UNREACHABLE;
    }

  } while (tok.type != KIND_EOF);

  if (list_len(&parser->errs) > 0) {
    return &parser->errs;
  }

  return NULL;
}

void parser_free(lparser_t *parser) {
  CHECK_NULL(parser, );
  list_free(&parser->errs, parse_err_free);
  ast_free(&parser->ast);
};