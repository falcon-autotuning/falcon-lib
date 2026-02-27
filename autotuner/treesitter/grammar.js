/**
 * TreeSitter grammar for the Falcon Autotuner DSL (.fal files)
 *
 * Kept strictly equivalent to autotuner/compiler/src/parser.y (Bison source).
 */
module.exports = grammar({
  name: 'fal',

  // Tells tree-sitter which rule is the "word" — used for keyword
  // disambiguation so that 'start', 'state', etc. can't match as identifiers.
  word: $ => $.identifier,

  extras: $ => [
    /\s/,
    $.comment,
  ],

  rules: {
    // =========================================================================
    // PROGRAM
    // program : autotuner_list routine_list
    // =========================================================================
    program: $ => repeat1(choice($.autotuner_decl, $.routine_decl)),

    // =========================================================================
    // COMMENTS  (not in Bison — handled by Flex)
    // =========================================================================
    comment: $ => token(choice(
      seq('//', /.*/),
      seq('/*', /[^*]*\*+([^/*][^*]*\*+)*/, '/'),
    )),

    // =========================================================================
    // TYPES  — type_spec in Bison
    // =========================================================================
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
      'Error',
    ),

    // =========================================================================
    // ROUTINE DECLARATION
    // routine_decl : ROUTINE IDENTIFIER input_params ARROW output_params
    // =========================================================================
    routine_decl: $ => seq(
      'routine',
      field('name', $.identifier),
      field('inputs', $.param_list),
      '->',
      field('outputs', $.param_list),
    ),

    // =========================================================================
    // AUTOTUNER DECLARATION
    // autotuner_decl : AUTOTUNER IDENTIFIER input_params ARROW output_params
    //                  LBRACE requires_clause autotuner_var_decls
    //                         entry_state entry_params SEMICOLON
    //                         state_list RBRACE
    // =========================================================================
    autotuner_decl: $ => seq(
      'autotuner',
      field('name', $.identifier),
      field('inputs', $.param_list),
      '->',
      field('outputs', $.param_list),
      '{',
      optional(field('uses', $.requires_clause)),
      // autotuner_var_decls: zero or more stmts before the entry
      // (Bison uses the generic stmt rule here — same as state body)
      repeat($.stmt),
      field('entry', $.entry_stmt),
      repeat($.state_decl),
      '}',
    ),

    // requires_clause : USES identifier_list SEMICOLON
    // NOTE: no parens — Bison syntax is:  uses foo, bar ;
    requires_clause: $ => seq(
      'uses',
      commaSep1($.identifier),
      ';',
    ),

    // entry_state + entry_params merged:
    // START ARROW IDENTIFIER (expr_list)? SEMICOLON
    entry_stmt: $ => seq(
      'start',
      '->',
      field('target', $.identifier),
      optional(seq('(', commaSep($.expr), ')')),
      ';',
    ),

    // =========================================================================
    // PARAMETERS
    // param_list : LPAREN param_decl_list RPAREN | LPAREN RPAREN | %empty
    // =========================================================================
    param_list: $ => seq('(', commaSep($.param_decl), ')'),

    param_decl: $ => seq(
      field('type', $.type),
      field('name', $.identifier),
      optional(seq('=', field('default', $.expr))),
    ),

    // =========================================================================
    // STATE DECLARATION
    // state_decl : STATE IDENTIFIER state_input_params LBRACE stmt_list RBRACE
    // state_input_params : LPAREN param_decl_list RPAREN | LPAREN RPAREN | %empty
    // =========================================================================
    state_decl: $ => seq(
      'state',
      field('name', $.identifier),
      optional(field('params', $.param_list)),
      '{',
      repeat($.stmt),
      '}',
    ),

    // =========================================================================
    // STATEMENTS  — stmt rule in Bison
    //
    //   stmt : var_decl_stmt
    //        | identifier_list ASSIGN expr SEMICOLON
    //        | expr SEMICOLON
    //        | IF LPAREN expr RPAREN LBRACE stmt_list RBRACE elif_chain
    //        | ARROW IDENTIFIER SEMICOLON
    //        | ARROW IDENTIFIER LPAREN expr_list RPAREN SEMICOLON
    //        | TERMINAL SEMICOLON
    // =========================================================================
    stmt: $ => choice(
      $.var_decl_stmt,
      $.assign_stmt,
      $.if_stmt,
      $.transition_stmt,
      $.terminal_stmt,
      $.expr_stmt,
    ),

    // var_decl_stmt : type_spec IDENTIFIER SEMICOLON
    //               | type_spec IDENTIFIER ASSIGN expr SEMICOLON
    var_decl_stmt: $ => seq(
      field('type', $.type),
      field('name', $.identifier),
      optional(seq('=', field('init', $.expr))),
      ';',
    ),

    // identifier_list ASSIGN expr SEMICOLON
    // (Bison allows multi-assign:  a, b = expr ;)
    assign_stmt: $ => seq(
      field('targets', commaSep1($.identifier)),
      '=',
      field('value', $.expr),
      ';',
    ),

    // IF LPAREN expr RPAREN LBRACE stmt_list RBRACE elif_chain
    // elif_chain : ELIF ( expr ) { stmts } elif_chain
    //            | ELSE { stmts }
    //            | %empty
    if_stmt: $ => seq(
      'if',
      '(',
      field('condition', $.expr),
      ')',
      '{',
      repeat($.stmt),
      '}',
      repeat($.elif_clause),   // zero or more elif branches
      optional($.else_clause), // optional final else
    ),

    elif_clause: $ => seq(
      'elif',
      '(',
      field('condition', $.expr),
      ')',
      '{',
      repeat($.stmt),
      '}',
    ),

    else_clause: $ => seq(
      'else',
      '{',
      repeat($.stmt),
      '}',
    ),

    // ARROW IDENTIFIER SEMICOLON
    // ARROW IDENTIFIER LPAREN expr_list RPAREN SEMICOLON
    transition_stmt: $ => seq(
      '->',
      field('target', $.identifier),
      optional(seq('(', commaSep($.expr), ')')),
      ';',
    ),

    // TERMINAL SEMICOLON
    terminal_stmt: $ => seq('terminal', ';'),

    // expr SEMICOLON
    expr_stmt: $ => seq($.expr, ';'),

    // =========================================================================
    // EXPRESSIONS  — mirrors Bison precedence table exactly:
    //   %left OR
    //   %left AND
    //   %left EQ NE
    //   %left LL GG LE GE
    //   %left PLUS MINUS
    //   %left MUL DIV
    //   %right NOT UMINUS
    //   %left DOT LBRACKET LPAREN   (postfix)
    // =========================================================================
    expr: $ => choice(
      $.binary_expr,
      $.unary_expr,
      $.postfix_expr,
      $.primary_expr,
    ),

    binary_expr: $ => choice(
      prec.left(1, seq($.expr, '||', $.expr)),
      prec.left(2, seq($.expr, '&&', $.expr)),
      prec.left(3, seq($.expr, '==', $.expr)),
      prec.left(3, seq($.expr, '!=', $.expr)),
      prec.left(4, seq($.expr, '<', $.expr)),
      prec.left(4, seq($.expr, '>', $.expr)),
      prec.left(4, seq($.expr, '<=', $.expr)),
      prec.left(4, seq($.expr, '>=', $.expr)),
      prec.left(5, seq($.expr, '+', $.expr)),
      prec.left(5, seq($.expr, '-', $.expr)),
      prec.left(6, seq($.expr, '*', $.expr)),
      prec.left(6, seq($.expr, '/', $.expr)),
    ),

    unary_expr: $ => choice(
      prec.right(7, seq('!', $.expr)),
      prec.right(7, seq('-', $.expr)),
    ),

    postfix_expr: $ => choice(
      prec.left(8, seq(
        field('object', $.expr),
        '.',
        field('member', $.identifier),
      )),
      prec.left(8, seq(
        field('object', $.expr),
        '.',
        field('method', $.identifier),
        '(',
        commaSep($.call_arg),
        ')',
      )),
      prec.left(8, seq(
        field('object', $.expr),
        '[',
        field('index', $.expr),
        ']',
      )),
      prec.left(8, seq(
        field('func', $.identifier),
        '(',
        commaSep($.call_arg),
        ')',
      )),
    ),

    primary_expr: $ => choice(
      $.int_literal,
      $.float_literal,
      $.string_literal,
      $.bool_literal,
      $.nil_literal,
      $.config_var,
      $.identifier,
      seq('(', $.expr, ')'),
    ),

    // call_arg : expr | IDENTIFIER ASSIGN expr
    call_arg: $ => choice(
      $.expr,
      seq(field('name', $.identifier), '=', field('value', $.expr)),
    ),

    // =========================================================================
    // LITERALS
    // =========================================================================
    int_literal: $ => /[0-9]+/,
    float_literal: $ => /[0-9]+\.[0-9]*/,
    bool_literal: $ => choice('true', 'false'),
    nil_literal: $ => 'nil',
    string_literal: $ => seq('"', /[^"\\]*/, '"'),

    // CONFIG_VAR — the 'config' keyword used as an expression value
    config_var: $ => 'config',

    // identifier MUST be last and MUST match the word: property above.
    identifier: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,
  },
});

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
function commaSep(rule) {
  return optional(commaSep1(rule));
}

function commaSep1(rule) {
  return seq(rule, repeat(seq(',', rule)));
}
