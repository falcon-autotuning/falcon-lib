; ============================================================
; Keywords
; ============================================================
(autotuner_decl "autotuner" @keyword)
(routine_decl "routine" @keyword)
(state_decl "state" @keyword)
(entry_stmt "start" @keyword)
(terminal_stmt "terminal" @keyword)
"uses" @keyword

; Conditionals
(if_stmt "if" @keyword.conditional)
(else_clause "else" @keyword.conditional)
(elif_clause "elif" @keyword.conditional)

; Operators / punctuation
(transition_stmt "->" @operator)
(entry_stmt "->" @operator)
"->" @operator

; ============================================================
; Types
; ============================================================
(type) @type

; ============================================================
; Function / autotuner / routine names
; ============================================================
(autotuner_decl name: (identifier) @function)
(routine_decl   name: (identifier) @function)
(state_decl     name: (identifier) @function.method)

; ============================================================
; Variables / identifiers
; ============================================================
(identifier) @variable

; ============================================================
; Literals
; ============================================================
(string_literal) @string
(int_literal)    @number
(float_literal)  @number.float
(bool_literal)   @boolean
(nil_literal)    @constant.builtin

; ============================================================
; Comments
; ============================================================
(comment) @comment @spell
