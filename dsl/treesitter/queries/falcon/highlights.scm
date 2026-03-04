; Keywords
(autotuner_decl) @keyword
(routine_decl)   @keyword
(state_decl)     @keyword
(struct_decl)    @keyword
(terminal_stmt)  @keyword.return
(if_stmt)        @keyword.conditional
(elif_clause)    @keyword.conditional
(else_clause)    @keyword.conditional
(import_stmt)    @keyword.import
(ffimport_decl)  @keyword.import
"start"          @keyword

; Types
(type (type_args_open) @punctuation.bracket (#set! priority 300))
(type (type_args_close) @punctuation.bracket (#set! priority 300))
(type (identifier) @type.definition (#set! priority 200))
(entry_target) @function.method
(param_decl type: (type) @type.definition 
  param_name: (identifier) @variable (#set! priority 200))

; Declaration names — priority 200 to override fallback
(autotuner_decl name: (identifier) @type.definition (#set! priority 200))
(struct_decl name: (identifier) @type.definition (#set! priority 200))
(routine_decl name: (identifier) @function (#set! priority 200))
(struct_routine_decl name: (identifier) @function (#set! priority 200))
(state_decl name: (identifier) @function.method (#set! priority 200))

; Struct field declarations
(struct_field_decl name: (identifier) @variable)

; Assignments and transitions
(assign_stmt (variable) @variable)
(transition_stmt 
  target: (identifier) @function.method (#set! priority 201))

; Calls and members
(method_call_expr
  method: (identifier) @function.method.call (#set! priority 201))
;; Highlight object ("conn") as variable in method calls (priority must be high)
(method_call_expr
  object: (identifier) @variable (#set! priority 200))
(member_expr member: (identifier) @property)
(assign_stmt ; [23, 12] - [23, 30]
  targets: (variable) ; [23, 12] - [23, 17]
  value: (expr ; [23, 20] - [23, 29]
    (binary_expr ; [23, 20] - [23, 29]
      left: (expr ; [23, 20] - [23, 25]
        (primary_expr ; [23, 20] - [23, 25]
          (qualified_name ; [23, 20] - [23, 25]
            symbol: (identifier) @variable (#set! priority 202)))))))
(assign_stmt ; [7, 4] - [7, 16]
  targets: (variable) ; [7, 4] - [7, 7]
  value: (expr ; [7, 10] - [7, 15]
    (binary_expr ; [7, 10] - [7, 15]
      right: (expr ; [7, 14] - [7, 15]
        (primary_expr ; [7, 14] - [7, 15]
          (qualified_name ; [7, 14] - [7, 15]
            symbol: (identifier) @variable (#set! priority 202)))))))
(if_stmt ; [22, 8] - [28, 9]
  condition: (expr ; [22, 12] - [22, 38]
    (binary_expr ; [22, 12] - [22, 38]
      left: (expr ; [22, 12] - [22, 17]
        (primary_expr ; [22, 12] - [22, 17]
          (qualified_name ; [22, 12] - [22, 17]
            symbol: (identifier) @variable (#set! priority 202)))))))
(if_stmt ; [7, 4] - [12, 5]
  condition: (expr ; [7, 8] - [7, 25]
    (binary_expr ; [7, 8] - [7, 25]
      right: (expr ; [7, 16] - [7, 25]
        (primary_expr ; [7, 16] - [7, 25]
          (qualified_name ; [7, 16] - [7, 25]
            symbol: (identifier) @variable (#set! priority 202)))))))
(elif_clause ; [9, 4] - [11, 5]
  condition: (expr ; [9, 10] - [9, 20]
    (binary_expr ; [9, 10] - [9, 20]
      left: (expr ; [9, 10] - [9, 15]
        (primary_expr ; [9, 10] - [9, 15]
          (qualified_name ; [9, 10] - [9, 15]
            symbol: (identifier) @variable (#set! priority 202)))))))
(elif_clause ; [9, 4] - [11, 5]
  condition: (expr ; [9, 10] - [9, 20]
    (binary_expr ; [9, 10] - [9, 20]
      right: (expr ; [9, 10] - [9, 15]
        (primary_expr ; [9, 10] - [9, 15]
          (qualified_name ; [9, 10] - [9, 15]
            symbol: (identifier) @variable (#set! priority 202)))))))
(var_decl_stmt ; [6, 4] - [6, 25]
  variable_name: (variable) @variable (#set! priority 202))
(primary_expr
  (qualified_name
    symbol: (identifier) @variable (#set! priority 202)))
(primary_expr
  (expr
    (index_expr
      index: (expr
        (primary_expr
          (qualified_name
            symbol: (identifier) @variable (#set! priority 202)))))))
(method_call_expr ; [9, 8] - [9, 28]
  object: (identifier) ; [9, 8] - [9, 11]
  method: (identifier) ; [9, 12] - [9, 17]
  (expr ; [9, 18] - [9, 27]
    (primary_expr ; [9, 18] - [9, 27]
      (qualified_name ; [9, 18] - [9, 27]
        symbol: (identifier) @variable (#set! priority 202)))))
(autotuner_var_decl
  init: (expr
    (index_expr
      index: (expr
        (primary_expr
          (qualified_name
            symbol: (identifier) @variable (#set! priority 202)))))))
(assign_stmt ; [8, 8] - [8, 21]
  targets: (variable) ; [8, 8] - [8, 13]
  value: (expr ; [8, 16] - [8, 20]
    (primary_expr ; [8, 16] - [8, 20]
      (qualified_name ; [8, 16] - [8, 20]
        symbol: (identifier) @variable (#set! priority 202)))))
; Module-qualified names in expressions / uses clauses
(call_expr func: (qualified_name symbol: (identifier) @function.call))

; Highlight modules (supports 1-2 nesting levels)
;; Color module portion in io::println
(qualified_name
  module: (qualified_name
    symbol: (identifier) @module (#set! priority 201)))

;; Color function portion in io::println
(qualified_name
  symbol: (identifier) @function.call (#set! priority 200))

; index in get expression
(index_expr (_ (identifier) @variable))

; Import paths
(import_stmt    path:    (string_literal) @string.special.path)
(ffimport_decl  wrapper: (string_literal) @string.special.path)

; Operators
(operator) @operator

; Punctuation
["{" "}"] @punctuation.bracket
["(" ")"] @punctuation.bracket
["[" "]"] @punctuation.bracket
(direction) @punctuation.delimiter
";"       @punctuation.delimiter
","       @punctuation.delimiter
"."       @punctuation.delimiter
"::"      @punctuation.delimiter
"="       @operator.assignment

; Literals
(int_literal)    @number
(float_literal)  @number.float
(bool_literal)   @boolean
(nil_literal)    @number
(string_literal) @string

; Comments
(comment) @comment @spell

; Fallback — priority 100 (default)
(identifier) @variable
