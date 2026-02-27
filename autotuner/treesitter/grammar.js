/**
 * TreeSitter grammar for the Falcon Autotuner DSL (.fal files)
 * Mirrors the Bison grammar in autotuner/compiler/src/parser.y
 */
module.exports = grammar({
  name: 'falcon',

  extras: $ => [
    /\s/,
    $.comment,
  ],

  rules: {
    program: $ => repeat(choice(
      $.autotuner_decl,
      $.routine_decl,
    )),

    // -----------------------------------------------------------------------
    // Comments
    // -----------------------------------------------------------------------
    comment: $ => token(choice(
      seq('//', /.*/),
      seq('/*', /[^*]*\*+([^/*][^*]*\*+)*/, '/'),
    )),

    // -----------------------------------------------------------------------
    // Types
    // -----------------------------------------------------------------------
    type: $ => choice(
      'int',
      'float',
      'bool',
      'string',
      'Quantity',
      'Config',
      'Connection',
      'Connections',
      'Gname',
      'DeviceCharacteristic',
      'DeviceCharacteristicQuery',
      'Error',
      'FatalError',
      'void',
    ),

    // -----------------------------------------------------------------------
    // Parameter declarations
    // -----------------------------------------------------------------------
    param_decl: $ => seq(
      field('type', $.type),
      field('name', $.identifier),
      optional(seq('=', field('default', $.expr))),
    ),

    param_list: $ => seq('(', commaSep($.param_decl), ')'),

    // -----------------------------------------------------------------------
    // Autotuner declaration
    // parser.y order: requires_clause → autotuner_var_decls → entry → state_list
    // -----------------------------------------------------------------------
    autotuner_decl: $ => seq(
      'autotuner',
      field('name', $.identifier),
      field('inputs', $.param_list),
      '->',
      field('outputs', $.param_list),
      '{',
      // uses clause comes BEFORE body stmts — parser.y: requires_clause first
      optional(field('uses', $.requires_clause)),
      // body-level stmts (var decls, assignments) — parser.y: autotuner_var_decls → stmt*
      repeat($.stmt),
      field('entry', $.entry_stmt),
      repeat($.state_decl),
      '}',
    ),

    // parser.y: USES identifier_list SEMICOLON — no parens, semicolon-terminated
    requires_clause: $ => seq(
      'uses',
      commaSep1($.identifier),
      ';',
    ),

    entry_stmt: $ => seq(
      'start',
      '->',
      field('target', $.identifier),
      optional(seq('(', commaSep($.expr), ')')),
      ';',
    ),

    // -----------------------------------------------------------------------
    // Routine declaration
    // -----------------------------------------------------------------------
    routine_decl: $ => seq(
      'routine',
      field('name', $.identifier),
      field('inputs', $.param_list),
      '->',
      field('outputs', $.param_list),
    ),

    // -----------------------------------------------------------------------
    // State declaration
    // parser.y: STATE IDENTIFIER state_input_params LBRACE stmt_list RBRACE
    // -----------------------------------------------------------------------
    state_decl: $ => seq(
      'state',
      field('name', $.identifier),
      optional(field('params', $.param_list)),
      '{',
      repeat($.stmt),
      '}',
    ),

    // -----------------------------------------------------------------------
    // Statements
    // -----------------------------------------------------------------------
    stmt: $ => choice(
      $.var_decl_stmt,
      $.assign_stmt,
      $.if_stmt,
      $.transition_stmt,
      $.terminal_stmt,
      $.expr_stmt,
    ),

    var_decl_stmt: $ => seq(
      field('type', $.type),
      field('name', $.identifier),
      optional(seq('=', field('init', $.expr))),
      ';',
    ),

    // FIX: field('targets', ...) and field('value', ...) labels required by corpus.
    // parser.y: identifier_list ASSIGN expr SEMICOLON
    assign_stmt: $ => seq(
      field('targets', $.identifier),
      repeat(seq(',', field('targets', $.identifier))),
      '=',
      field('value', $.expr),
      ';',
    ),

    // parser.y: IF LPAREN expr RPAREN LBRACE stmt_list RBRACE elif_chain
    // elif and else are separate named nodes matching elif_clause / else_clause
    if_stmt: $ => seq(
      'if',
      '(',
      field('condition', $.expr),
      ')',
      '{',
      repeat($.stmt),
      '}',
      repeat($.elif_clause),
      optional($.else_clause),
    ),

    // parser.y elif_chain: ELIF LPAREN expr RPAREN LBRACE stmt_list RBRACE elif_chain
    elif_clause: $ => seq(
      'elif',
      '(',
      field('condition', $.expr),
      ')',
      '{',
      repeat($.stmt),
      '}',
    ),

    // parser.y elif_chain: ELSE LBRACE stmt_list RBRACE
    else_clause: $ => seq(
      'else',
      '{',
      repeat($.stmt),
      '}',
    ),

    // parser.y: ARROW IDENTIFIER SEMICOLON
    //         | ARROW IDENTIFIER LPAREN expr_list RPAREN SEMICOLON
    transition_stmt: $ => seq(
      '->',
      field('target', $.identifier),
      optional(seq('(', commaSep($.expr), ')')),
      ';',
    ),

    terminal_stmt: $ => seq('terminal', ';'),

    expr_stmt: $ => seq($.expr, ';'),

    // -----------------------------------------------------------------------
    // Expressions
    // parser.y: expr → primary_expr | postfix_expr | binary | unary
    // expr is always a wrapper node; atoms live inside primary_expr
    // -----------------------------------------------------------------------
    expr: $ => choice(
      $.binary_expr,
      $.unary_expr,
      $.call_expr,
      $.member_expr,
      $.index_expr,
      $.primary_expr,
    ),

    // parser.y precedence (lowest→highest): OR(1) AND(2) cmp(3) +/-(4) *//( 5)
    binary_expr: $ => choice(
      prec.left(1, seq($.expr, '||', $.expr)),
      prec.left(2, seq($.expr, '&&', $.expr)),
      prec.left(3, seq($.expr, choice('==', '!=', '<', '>', '<=', '>='), $.expr)),
      prec.left(4, seq($.expr, choice('+', '-'), $.expr)),
      prec.left(5, seq($.expr, choice('*', '/'), $.expr)),
    ),

    // parser.y: NOT expr | MINUS expr %prec UMINUS
    unary_expr: $ => prec.right(6, choice(
      seq('-', $.expr),
      seq('!', $.expr),
    )),

    // parser.y postfix_expr: IDENTIFIER LPAREN call_arg_list RPAREN
    call_expr: $ => seq(
      field('func', $.identifier),
      '(',
      commaSep(choice($.named_arg, $.expr)),
      ')',
    ),

    named_arg: $ => seq(
      field('name', $.identifier),
      '=',
      field('value', $.expr),
    ),

    // parser.y postfix_expr: expr DOT IDENTIFIER [LPAREN expr_list RPAREN]
    // Uses '.' not '::'
    member_expr: $ => prec.left(7, seq(
      field('object', $.expr),
      '.',
      field('member', $.identifier),
    )),

    // parser.y postfix_expr: expr LBRACKET expr RBRACKET
    index_expr: $ => prec.left(7, seq(
      field('object', $.expr),
      '[',
      field('index', $.expr),
      ']',
    )),

    // Atom wrapper — parser.y primary_expr: INTEGER | DOUBLE | STRING | TRUE
    //               | FALSE | NIL | IDENTIFIER | CONFIG_VAR | LPAREN expr RPAREN
    primary_expr: $ => choice(
      $.int_literal,
      $.float_literal,
      $.bool_literal,
      $.string_literal,
      $.nil_literal,
      $.identifier,
      seq('(', $.expr, ')'),
    ),

    // -----------------------------------------------------------------------
    // Literals
    // -----------------------------------------------------------------------
    int_literal: $ => /[0-9]+/,
    float_literal: $ => /[0-9]+\.[0-9]*/,
    bool_literal: $ => choice('true', 'false'),
    nil_literal: $ => 'nil',

    string_literal: $ => seq(
      '"',
      /[^"\\]*/,
      '"',
    ),

    identifier: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,
  },
});

// Helpers
function commaSep(rule) {
  return optional(commaSep1(rule));
}

function commaSep1(rule) {
  return seq(rule, repeat(seq(',', rule)));
}
