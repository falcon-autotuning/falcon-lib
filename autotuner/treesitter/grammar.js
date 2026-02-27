/**
 * TreeSitter grammar for the Falcon Autotuner DSL (.fal files)
 * Mirrors the Bison grammar in autotuner/compiler/src/parser.y
 */
module.exports = grammar({
  name: 'falcon',  // Must match the compiled .so symbol: tree_sitter_falcon

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
      'int', 'float', 'bool', 'string',
      'Quantity', 'Config', 'Connection', 'Connections',
      'Gname', 'DeviceCharacteristic', 'DeviceCharacteristicQuery',
      'Error', 'FatalError', 'void',
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
      optional(field('uses', $.requires_clause)),
      // autotuner_var_decl is a DISTINCT node from var_decl_stmt so highlights.scm
      // can color autotuner-scope declarations differently from state-scope ones.
      repeat($.autotuner_var_decl),
      field('entry', $.entry_stmt),
      repeat($.state_decl),
      '}',
    ),

    // Body-level declarations/assignments inside the autotuner (before start ->)
    // Distinct node type from var_decl_stmt (which lives inside states).
    autotuner_var_decl: $ => choice(
      // Type-annotated declaration:  int counter;  or  int total = 0;
      seq(
        field('type', $.type),
        field('name', $.identifier),
        optional(seq('=', field('init', $.expr))),
        ';',
      ),
      // Assignment without type:  sum = 0;
      seq(
        field('targets', $.identifier),
        repeat(seq(',', field('targets', $.identifier))),
        '=',
        field('value', $.expr),
        ';',
      ),
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
    // Statements (inside states)
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

    assign_stmt: $ => seq(
      field('targets', $.identifier),
      repeat(seq(',', field('targets', $.identifier))),
      '=',
      field('value', $.expr),
      ';',
    ),

    if_stmt: $ => seq(
      'if', '(',
      field('condition', $.expr),
      ')', '{',
      repeat($.stmt),
      '}',
      repeat($.elif_clause),
      optional($.else_clause),
    ),

    elif_clause: $ => seq(
      'elif', '(',
      field('condition', $.expr),
      ')', '{',
      repeat($.stmt),
      '}',
    ),

    else_clause: $ => seq(
      'else', '{',
      repeat($.stmt),
      '}',
    ),

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
    // -----------------------------------------------------------------------
    expr: $ => choice(
      $.binary_expr,
      $.unary_expr,
      $.call_expr,
      $.member_expr,
      $.index_expr,
      $.primary_expr,
    ),

    binary_expr: $ => choice(
      prec.left(1, seq($.expr, '||', $.expr)),
      prec.left(2, seq($.expr, '&&', $.expr)),
      prec.left(3, seq($.expr, choice('==', '!=', '<', '>', '<=', '>='), $.expr)),
      prec.left(4, seq($.expr, choice('+', '-'), $.expr)),
      prec.left(5, seq($.expr, choice('*', '/'), $.expr)),
    ),

    unary_expr: $ => prec.right(6, choice(
      seq('-', $.expr),
      seq('!', $.expr),
    )),

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

    member_expr: $ => prec.left(7, seq(
      field('object', $.expr),
      '.',
      field('member', $.identifier),
    )),

    index_expr: $ => prec.left(7, seq(
      field('object', $.expr),
      '[',
      field('index', $.expr),
      ']',
    )),

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
    string_literal: $ => seq('"', /[^"\\]*/, '"'),
    identifier: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,
  },
});

function commaSep(rule) { return optional(commaSep1(rule)); }
function commaSep1(rule) { return seq(rule, repeat(seq(',', rule))); }
