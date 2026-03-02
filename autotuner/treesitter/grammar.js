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

  conflicts: $ => [
    [$.method_call_expr, $.member_expr],
    [$.struct_field_assign_stmt, $.primary_expr],
    [$.autotuner_var_decl, $.assign_stmt],
    [$.var_decl_stmt, $.assign_stmt],
  ],

  rules: {
    // -----------------------------------------------------------------------
    // Top-level program
    // -----------------------------------------------------------------------
    program: $ => repeat(
      choice(
        $.import_stmt,
        $.ffimport_decl,
        $.struct_decl,
        $.routine_decl,
        $.autotuner_decl,
      )
    ),

    // -----------------------------------------------------------------------
    // Comments
    // -----------------------------------------------------------------------
    comment: $ => token(choice(
      seq('//', /.*/),
      seq('/*', /[^*]*\*+([^/*][^*]*\*+)*/, '/')
    )),

    // -----------------------------------------------------------------------
    // Import statements (single or multi-path)
    // -----------------------------------------------------------------------
    import_stmt: $ => choice(
      seq('import', field('path', $.string_literal), ';'),
      seq('import', '(', repeat1(field('paths', $.string_literal)), ')'),
    ),

    // -----------------------------------------------------------------------
    // FFI import declarations
    // -----------------------------------------------------------------------
    ffimport_decl: $ => seq(
      'ffimport',
      field('wrapper', $.string_literal),
      field('imports', seq('(', repeat($.string_literal), ')')),
      field('libs', seq('(', repeat($.string_literal), ')'))
    ),

    // -----------------------------------------------------------------------
    // Struct declarations: fields and routines may interleave in source but
    // parser.y enforces struct_field_list then struct_routine_list.
    // We keep interleaving here for resilience; the test corpus only uses
    // fields-first order anyway.
    // -----------------------------------------------------------------------
    struct_decl: $ => seq(
      'struct',
      field('name', $.identifier),
      '{',
      repeat(choice($.struct_field_decl, $.struct_routine_decl)),
      '}',
    ),

    struct_field_decl: $ => seq(
      field('type', alias($.identifier, $.type)),
      field('name', $.identifier),
      optional(seq('=', field('default', $.expr))),
      ';'
    ),

    // Routine inside a struct.
    // FIX: inputs is optional — parser.y's input_params has a %empty production,
    //      so `routine Value -> (int value) { ... }` (no input parens) is valid.
    // FIX: body uses struct_routine_body (not block) so statements are NOT
    //      wrapped in a `stmt` node, matching parser.y's routine_body_stmts.
    struct_routine_decl: $ => seq(
      'routine',
      field('name', $.identifier),
      optional(field('inputs', $.param_list)),  // ← was required, now optional
      '->',
      field('outputs', $.param_list),
      optional(field('body', $.struct_routine_body))  // ← dedicated node type
    ),

    // -----------------------------------------------------------------------
    // Block for state bodies — statements ARE wrapped in `stmt`
    // -----------------------------------------------------------------------
    block: $ => seq(
      '{',
      repeat($.stmt),
      '}'
    ),

    // -----------------------------------------------------------------------
    // Struct routine body — matches parser.y's routine_body_stmts.
    // Statements are NOT wrapped in `stmt`; they are direct children.
    // -----------------------------------------------------------------------
    struct_routine_body: $ => seq(
      '{',
      repeat($.struct_routine_stmt),
      '}'
    ),

    // Statements valid inside a struct routine body.
    // Matches parser.y's struct_routine_stmt alternatives.
    // Note: bare `identifier = expr ;` is handled as struct_field_assign_stmt
    // when the identifier is a known struct field (runtime context in Bison).
    // In tree-sitter we cannot replicate that context, so we use assign_stmt
    // for the bare form and struct_field_assign_stmt only for dot-notation.
    struct_routine_stmt: $ => choice(
      $.var_decl_stmt,
      $.struct_field_assign_stmt,
      $.assign_stmt,
      $.expr_stmt,
      $.if_stmt,
    ),

    // -----------------------------------------------------------------------
    // Top-level routine declaration
    // FIX: inputs is optional (same reason as struct_routine_decl)
    // -----------------------------------------------------------------------
    routine_decl: $ => seq(
      'routine',
      field('name', $.identifier),
      optional(field('inputs', $.param_list)),  // ← was required, now optional
      '->',
      field('outputs', $.param_list),
      optional(field('body', $.block))
    ),

    // -----------------------------------------------------------------------
    // Autotuner declaration
    // -----------------------------------------------------------------------
    autotuner_decl: $ => seq(
      'autotuner',
      field('name', $.identifier),
      optional(field('inputs', $.param_list)),
      '->',
      field('outputs', $.param_list),
      '{',
      repeat(choice($.autotuner_var_decl, $.assign_stmt, $.requires_clause)),
      field('entry', $.entry_stmt),
      repeat($.state_decl),
      '}'
    ),

    autotuner_var_decl: $ => seq(
      field('type', alias($.identifier, $.type)),
      field('name', $.identifier),
      optional(seq('=', field('init', $.expr))),
      ';'
    ),

    requires_clause: $ => seq(
      'uses',
      commaSep1($.qualified_name),
      ';'
    ),

    entry_stmt: $ => seq(
      field('target', $.identifier),
      '->',
      field('next', $.identifier),
      ';'
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
      '}'
    ),

    // -----------------------------------------------------------------------
    // Statements (inside states and autotuner bodies) — wrapped in `stmt`
    // -----------------------------------------------------------------------
    stmt: $ => choice(
      $.assign_stmt,
      $.var_decl_stmt,
      $.transition_stmt,
      $.terminal_stmt,
      $.if_stmt,
      $.struct_field_assign_stmt,
      $.expr_stmt,
    ),

    var_decl_stmt: $ => seq(
      field('type', alias($.identifier, $.type)),
      field('name', $.identifier),
      optional(seq('=', field('init', $.expr))),
      ';'
    ),

    assign_stmt: $ => seq(
      field('targets', commaSep1($.identifier)),
      '=',
      field('value', $.expr),
      ';'
    ),

    transition_stmt: $ => seq(
      '->',
      field('target', $.identifier),
      ';'
    ),

    terminal_stmt: $ => seq(
      'terminal',
      ';'
    ),

    // -----------------------------------------------------------------------
    // If/elif/else
    // -----------------------------------------------------------------------
    if_stmt: $ => seq(
      'if',
      '(',
      field('condition', $.expr),
      ')',
      '{',
      repeat($.stmt),
      '}',
      repeat($.elif_clause),
      optional($.else_clause)
    ),

    elif_clause: $ => seq(
      'elif',
      '(',
      field('condition', $.expr),
      ')',
      '{',
      repeat($.stmt),
      '}'
    ),

    else_clause: $ => seq(
      'else',
      '{',
      repeat($.stmt),
      '}'
    ),

    // -----------------------------------------------------------------------
    // Struct field assignment: obj.field = expr;  or  this.field = expr;
    //
    // prec(1) is required on both variants to resolve the generate-time
    // conflict: when tree-sitter sees `this .` or `identifier .` inside a
    // struct_routine_body it cannot immediately tell whether this is the start
    // of a struct_field_assign_stmt or a primary_expr/member_expr/method_call.
    // Giving this rule higher precedence tells tree-sitter to prefer the
    // statement interpretation.
    //
    // The bare-name variant (field_ = val ;) is intentionally omitted — it is
    // syntactically identical to assign_stmt and Bison resolves it only via
    // runtime struct_field_scope context which tree-sitter cannot replicate.
    // In practice, bare assignments inside struct routines will parse as
    // assign_stmt, which is correct for the value = a_; / value = a_ + b_;
    // cases (output params, not struct fields).
    // -----------------------------------------------------------------------
    struct_field_assign_stmt: $ => choice(
      prec(1, seq(
        'this', '.', field('field', $.identifier), '=', field('value', $.expr), ';'
      )),
      prec(1, seq(
        field('object', $.identifier), '.', field('field', $.identifier), '=', field('value', $.expr), ';'
      )),
    ),

    expr_stmt: $ => seq(
      $.expr,
      ';'
    ),

    // -----------------------------------------------------------------------
    // Expressions
    // -----------------------------------------------------------------------
    expr: $ => choice(
      $.binary_expr,
      $.call_expr,
      $.method_call_expr,
      $.member_expr,
      $.primary_expr,
    ),

    binary_expr: $ => prec.left(seq(
      field('left', $.expr),
      field('operator', choice(
        alias('+', $.operator),
        alias('-', $.operator),
        alias('*', $.operator),
        alias('/', $.operator),
        alias('==', $.operator),
        alias('!=', $.operator),
        alias('<', $.operator),
        alias('>', $.operator),
        alias('<=', $.operator),
        alias('>=', $.operator),
        alias('&&', $.operator),
        alias('||', $.operator)
      )),
      field('right', $.expr),
    )),

    operator: $ => token(choice(
      '+', '-', '*', '/', '==', '!=', '<', '>', '<=', '>=', '&&', '||'
    )),

    call_expr: $ => seq(
      field('func', $.qualified_name),
      '(',
      commaSep(choice($.named_arg, $.expr)),
      ')'
    ),

    method_call_expr: $ => seq(
      field('object', $.expr),
      '.',
      field('method', $.identifier),
      '(',
      commaSep(choice($.named_arg, $.expr)),
      ')'
    ),

    member_expr: $ => seq(
      field('object', $.expr),
      '.',
      field('member', $.identifier)
    ),

    named_arg: $ => seq(
      field('name', $.identifier),
      '=',
      field('value', $.expr)
    ),

    // -----------------------------------------------------------------------
    // Primary expressions
    // -----------------------------------------------------------------------
    primary_expr: $ => choice(
      $.int_literal,
      $.float_literal,
      $.bool_literal,
      $.string_literal,
      $.nil_literal,
      'this',
      $.qualified_name,
      seq('(', $.expr, ')')
    ),

    // -----------------------------------------------------------------------
    // Qualified names — Module::symbol OR plain symbol
    // -----------------------------------------------------------------------
    qualified_name: $ => seq(
      optional(seq(
        field('module', $.identifier),
        '::'
      )),
      field('symbol', $.identifier)
    ),

    // -----------------------------------------------------------------------
    // Parameter declarations
    // -----------------------------------------------------------------------
    param_list: $ => seq(
      '(',
      commaSep($.param_decl),
      ')'
    ),

    param_decl: $ => seq(
      field('type', alias($.identifier, $.type)),
      field('name', $.identifier)
    ),

    // -----------------------------------------------------------------------
    // Literals
    // -----------------------------------------------------------------------
    int_literal: $ => /\d+/,
    float_literal: $ => /\d+\.\d+/,
    bool_literal: $ => choice('true', 'false'),
    string_literal: $ => seq('"', /[^"]*/, '"'),
    nil_literal: $ => 'nil',

    identifier: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,
  }
});

function commaSep1(rule) {
  return seq(rule, repeat(seq(',', rule)));
}
function commaSep(rule) {
  return optional(commaSep1(rule));
}
