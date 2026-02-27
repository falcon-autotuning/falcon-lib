; ============================================================================
; Keywords
; ============================================================================
"autotuner" @keyword
"routine"   @keyword
"state"     @keyword
"start"     @keyword
"uses"      @keyword
"terminal"  @keyword.return

"if"        @keyword.conditional
"elif"      @keyword.conditional
"else"      @keyword.conditional

"->"        @keyword.operator

; ============================================================================
; Types
; ============================================================================
(type) @type

; ============================================================================
; Declaration names — priority 200 so they beat the (identifier) @variable
; fallback at priority 100 below.
; ============================================================================
(autotuner_decl name: (identifier) @type.definition
  (#set! priority 200))

(routine_decl name: (identifier) @function
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
(autotuner_var_decl targets: (identifier) @variable.builtin
  (#set! priority 150))

; ============================================================================
; State-scope declarations — priority 150
; ============================================================================
(var_decl_stmt name: (identifier) @variable
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
(call_expr   func:   (identifier) @function.call
  (#set! priority 150))
(member_expr member: (identifier) @property
  (#set! priority 150))

; ============================================================================
; Operators
; ============================================================================
["=" "+" "-" "*" "/" "==" "!=" "<" ">" "<=" ">=" "&&" "||" "!"] @operator

; ============================================================================
; Punctuation
; ============================================================================
["{" "}"] @punctuation.bracket
["(" ")"] @punctuation.bracket
["[" "]"] @punctuation.bracket
";"       @punctuation.delimiter
","       @punctuation.delimiter
"."       @punctuation.delimiter

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

; ============================================================================
; Fallback — priority 100 (default). Catches all remaining identifier nodes:
; variable references in expressions (a, b, sum, product etc.)
; Specific patterns above at priority 150-200 will override this.
; ============================================================================
(identifier) @variable
