/**
 * TreeSitter grammar for the Falcon Autotuner DSL (.fal files)
 */
module.exports = grammar({
  name: 'fal',

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

    tuple_type: $ => seq(
      '(',
      commaSep1($.type),
      ')',
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
    // -----------------------------------------------------------------------
    autotuner_decl: $ => seq(
      'autotuner',
      field('name', $.identifier),
      field('inputs', $.param_list),
      '->',
      field('outputs', $.param_list),
      optional(seq('uses', '(', commaSep($.identifier), ')')),
      '{',
      repeat($.autotuner_var_decl),
      field('entry', $.entry_stmt),
      repeat($.state_decl),
      '}',
    ),

    autotuner_var_decl: $ => seq(
      field('type', $.type),
      field('name', $.identifier),
      optional(seq('=', field('init', $.expr))),
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

    assign_stmt: $ => seq(
      commaSep1($.identifier),
      '=',
      $.expr,
      ';',
    ),

    if_stmt: $ => seq(
      'if',
      '(',
      field('condition', $.expr),
      ')',
      '{',
      repeat($.stmt),
      '}',
      optional(seq(
        'else',
        '{',
        repeat($.stmt),
        '}',
      )),
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
    // Expressions (by increasing precedence)
    // -----------------------------------------------------------------------
    expr: $ => choice(
      $.binary_expr,
      $.unary_expr,
      $.call_expr,
      $.member_expr,
      $.index_expr,
      $.identifier,
      $.literal,
      $.nil_literal,
      seq('(', $.expr, ')'),
    ),

    binary_expr: $ => choice(
      prec.left(1, seq($.expr, choice('||'), $.expr)),
      prec.left(2, seq($.expr, choice('&&'), $.expr)),
      prec.left(3, seq($.expr, choice('==', '!=', '<', '>', '<=', '>='), $.expr)),
      prec.left(4, seq($.expr, choice('+', '-'), $.expr)),
      prec.left(5, seq($.expr, choice('*', '/'), $.expr)),
    ),

    unary_expr: $ => prec(6, choice(
      seq('-', $.expr),
      seq('!', $.expr),
    )),

    call_expr: $ => seq(
      field('func', choice($.member_expr, $.identifier)),
      '(',
      commaSep(choice($.named_arg, $.expr)),
      ')',
    ),

    named_arg: $ => seq(
      $.identifier,
      '=',
      $.expr,
    ),

    member_expr: $ => seq(
      field('object', choice($.identifier, $.call_expr, $.index_expr)),
      '::',
      field('member', $.identifier),
    ),

    index_expr: $ => seq(
      field('object', $.identifier),
      '[',
      $.expr,
      ']',
    ),

    // -----------------------------------------------------------------------
    // Literals
    // -----------------------------------------------------------------------
    literal: $ => choice(
      $.int_literal,
      $.float_literal,
      $.bool_literal,
      $.string_literal,
    ),

    int_literal: $ => /[0-9]+/,

    float_literal: $ => /[0-9]+\.[0-9]*/,

    bool_literal: $ => choice('true', 'false'),

    string_literal: $ => seq(
      '"',
      /[^"\\]*/,
      '"',
    ),

    nil_literal: $ => 'nil',

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
