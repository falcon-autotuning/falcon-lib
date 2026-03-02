; ============================================================================
; Keywords (highlighted via context nodes, not raw tokens)
; ============================================================================
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

; ============================================================================
; Types
; ============================================================================
(type) @type

; ============================================================================
; Declaration names — priority 200 so they beat the (identifier) @variable fallback
; ============================================================================
(autotuner_decl name: (identifier) @type.definition
  (#set! priority 200))
(struct_decl name: (identifier) @type.definition
  (#set! priority 200))
(routine_decl name: (identifier) @function
  (#set! priority 200))
(struct_routine_decl name: (identifier) @function
  (#set! priority 200))
(state_decl name: (identifier) @function.method
  (#set! priority 200))

; ============================================================================
; Parameters — priority 150
; ============================================================================
(param_decl name: (identifier) @variable.parameter
  (#set! priority 150))

; ============================================================================
; Autotuner-scope declarations — priority 150
; ============================================================================
(autotuner_var_decl name:    (identifier) @variable.builtin
  (#set! priority 150))

; ============================================================================
; State-scope declarations — priority 150
; ============================================================================
(var_decl_stmt name: (identifier) @variable
  (#set! priority 150))

; ============================================================================
; Struct field declarations — priority 150
; ============================================================================
(struct_field_decl name: (identifier) @variable
  (#set! priority 150))

; ============================================================================
; Assignments and transitions — priority 150
; ============================================================================
(assign_stmt     targets: (identifier) @variable
  (#set! priority 150))
(entry_stmt      target:  (identifier) @label
  (#set! priority 150))
(transition_stmt target:  (identifier) @label
  (#set! priority 150))

; ============================================================================
; Calls and members — priority 150
; ============================================================================
(call_expr func: (qualified_name symbol: (identifier) @function.call)
  (#set! priority 160))
(call_expr func: (qualified_name module: (identifier) @module)
  (#set! priority 160))
(method_call_expr method: (identifier) @function.method.call
  (#set! priority 150))
(member_expr member: (identifier) @property
  (#set! priority 150))

; ============================================================================
; Module-qualified names in expressions / uses clauses: Module::symbol
; Priority 160 so it beats the plain @variable fallback at 100.
; ============================================================================
(qualified_name module: (identifier) @module
  (#set! priority 160))
(qualified_name symbol: (identifier) @variable
  (#set! priority 160))

; ============================================================================
; Import paths — @string.special.path follows nvim-treesitter conventions for
; file-path strings (distinct color from regular string data in many themes).
; ============================================================================
(import_stmt    path:    (string_literal) @string.special.path)
(ffimport_decl  wrapper: (string_literal) @string.special.path)

; ============================================================================
; Operators
; ============================================================================
(operator) @operator

; ============================================================================
; Punctuation
; ============================================================================
["{" "}"] @punctuation.bracket
["(" ")"] @punctuation.bracket
";"       @punctuation.delimiter
","       @punctuation.delimiter
"."       @punctuation.delimiter
"::"      @punctuation.delimiter

; ============================================================================
; Literals
; ============================================================================
(int_literal)    @number
(float_literal)  @number.float
(bool_literal)   @boolean
(nil_literal)    @constant.builtin
(string_literal) @string

; ============================================================================
; Comments
; ============================================================================
(comment) @comment @spell

; ; ============================================================================
; ; Fallback — priority 100 (default). Catches all remaining identifier nodes.
; ; Specific patterns above at priority 150-200 will override this.
; ; ============================================================================
; (identifier) @variable
