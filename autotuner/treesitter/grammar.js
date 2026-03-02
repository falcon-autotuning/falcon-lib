/**
 * TreeSitter grammar for the Falcon Autotuner DSL (.fal files)
 * Mirrors the Bison grammar in autotuner/compiler/src/parser.y
 *
 * Key changes:
 *   - Added conflict for struct_field_assign_stmt vs primary_expr
 *   - Removed duplicate 'type' fields/aliases
 *   - Used qualified_name everywhere appropriate
 *   - Fixed import/ffimport statement semicolon handling
 *   - Allowed interleaving of struct fields/routines in structs
 *   - Checked all field() usages for duplicates
 */
module.exports = grammar({
  name: 'falcon',  // Must match the compiled .so symbol: tree_sitter_falcon

  extras: $ => [
    /\s/,
    $.comment,
  ],

  // Resolve ambiguity between method_call_expr/member_expr and struct_field_assign_stmt/primary_expr
  conflicts: $ => [
    [$.method_call_expr, $.member_expr],
    [$.struct_field_assign_stmt, $.primary_expr],
    [$.autotuner_var_decl, $.assign_stmt],
    [$.var_decl_stmt, $.assign_stmt],
    [$.struct_field_assign_stmt, $.qualified_name],
    [$.struct_field_assign_stmt, $.assign_stmt],
  ],

  rules: {
    // -----------------------------------------------------------------------
    // Top-level program: sequence of imports, ffimports, structs, routines, autotuners
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
    // Struct declarations: fields and routines (interleaved)
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

    // Routine stub inside a struct — body is optional (FFI-backed method)
    struct_routine_decl: $ => seq(
      'routine',
      field('name', $.identifier),
      field('inputs', $.param_list),
      '->',
      field('outputs', $.param_list),
      optional(field('body', $.block))
    ),

    block: $ => seq(
      '{',
      repeat($.stmt),
      '}'
    ),

    // -----------------------------------------------------------------------
    // Routine declaration (top-level, no body)
    // -----------------------------------------------------------------------
    routine_decl: $ => seq(
      'routine',
      field('name', $.identifier),
      field('inputs', $.param_list),
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
      field('outputs', $.param_list), '{',
      repeat(choice($.autotuner_var_decl, $.assign_stmt, $.requires_clause)),
      field('entry', $.entry_stmt),
      repeat($.state_decl), '}'
    ),

    // Body-level declarations/assignments inside the autotuner
    autotuner_var_decl: $ => seq(
      field('type', alias($.identifier, $.type)),
      field('name', $.identifier),
      optional(seq('=', field('init', $.expr))),
      ';'
    ),

    // uses clause — supports Module::symbol qualified names
    requires_clause: $ => seq(
      'uses',
      commaSep1($.qualified_name),
      ';'
    ),

    // Entry statement: start -> target;
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
    // Statements (inside states)
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
    // If/elif/else statements
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
    // Struct field assignment: this.field = expr;
    // -----------------------------------------------------------------------
    struct_field_assign_stmt: $ => choice(
      seq('this', '.', field('field', $.identifier), '=', field('value', $.expr), ';'),
      seq(field('field', $.identifier), '=', field('value', $.expr), ';'), // bare field assignment
      seq(alias($.identifier, '_'), '.', field('field', $.identifier), '=', field('value', $.expr), ';')
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

    // Plain call: name(args) or Module::name(args)
    call_expr: $ => seq(
      field('func', $.qualified_name),
      '(',
      commaSep(choice($.named_arg, $.expr)),
      ')'
    ),

    // Method call: obj.method(args)
    method_call_expr: $ => seq(
      field('object', $.expr),
      '.',
      field('method', $.identifier),
      '(',
      commaSep(choice($.named_arg, $.expr)),
      ')'
    ),

    // Member access: obj.field
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
    // Primary expressions: literals, identifiers, qualified names, parens
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

// Helpers for comma separated lists
function commaSep1(rule) {
  return seq(rule, repeat(seq(',', rule)));
}
function commaSep(rule) {
  return optional(commaSep1(rule));
}
