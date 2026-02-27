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
; Declaration names
;
; autotuner_decl name  → @type.definition  (class-like, e.g. "Calculator")
; routine_decl   name  → @function         (function-like)
; state_decl     name  → @label            (label-like, e.g. "calculate", "done")
; ============================================================================
(autotuner_decl name: (identifier) @type.definition)
(routine_decl   name: (identifier) @function)
(state_decl     name: (identifier) @label)

; ============================================================================
; Parameters
;
; Input params  → @variable.parameter  (typically blue)
; Output params → @variable.member     (typically teal/cyan)
; State params  → custom scope color
; ============================================================================
(autotuner_decl inputs:  (param_list (param_decl name: (identifier) @variable.parameter)))
(autotuner_decl outputs: (param_list (param_decl name: (identifier) @variable.member)))
(state_decl     params:  (param_list (param_decl name: (identifier) @variable.parameter.state)))

; ============================================================================
; Variable declarations — colored by scope
;
; autotuner_var_decl  = declared at autotuner body level  → warm orange
; var_decl_stmt       = declared inside a state           → cool blue
; ============================================================================

; Autotuner-scope: type + name form   (int counter;  /  int total = 0;)
(autotuner_var_decl name: (identifier) @variable.autotuner)

; Autotuner-scope: assignment form    (sum = 0;  before start ->)
(autotuner_var_decl targets: (identifier) @variable.autotuner)

; State-scope variable declarations
(var_decl_stmt name: (identifier) @variable.state)

; ============================================================================
; Assignment targets inside states  → plain @variable
; ============================================================================
(assign_stmt targets: (identifier) @variable)

; ============================================================================
; Entry and transition targets  → @label (same color as state names)
; ============================================================================
(entry_stmt      target: (identifier) @label)
(transition_stmt target: (identifier) @label)

; ============================================================================
; Calls and member access
; ============================================================================
(call_expr   func:   (identifier) @function.call)
(member_expr member: (identifier) @variable.member)
(named_arg   name:   (identifier) @variable.parameter)

; ============================================================================
; Operators
; ============================================================================
["=" "+" "-" "*" "/" "==" "!=" "<" ">" "<=" ">=" "&&" "||" "!"] @operator

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
; Punctuation
; ============================================================================
["{" "}"] @punctuation.bracket
["(" ")"] @punctuation.bracket
["[" "]"] @punctuation.bracket
";"       @punctuation.delimiter
","       @punctuation.delimiter
"."       @punctuation.delimiter

; ============================================================================
; Fallback — any identifier not caught by a more specific rule above
; This covers variable references in expressions (a, b, sum, product, etc.)
; ============================================================================
(identifier) @variable
