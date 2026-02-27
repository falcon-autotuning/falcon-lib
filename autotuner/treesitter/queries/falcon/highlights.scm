;; =========================================================================
;; Keywords
;; =========================================================================
"autotuner"  @keyword
"routine"    @keyword
"state"      @keyword
"start"      @keyword
"uses"       @keyword
"terminal"   @keyword.return

"if"         @keyword.conditional
"elif"       @keyword.conditional
"else"       @keyword.conditional

"->"         @keyword.operator

;; =========================================================================
;; Declaration names (structure-level identifiers)
;; =========================================================================
(autotuner_decl name: (identifier) @type.definition)
(routine_decl   name: (identifier) @function)
(state_decl     name: (identifier) @label)

;; =========================================================================
;; Parameters — colored by which field they appear in on the parent
;; =========================================================================
(autotuner_decl
  inputs: (param_list
    (param_decl name: (identifier) @variable.parameter)))

(autotuner_decl
  outputs: (param_list
    (param_decl name: (identifier) @variable.member)))

(state_decl
  params: (param_list
    (param_decl name: (identifier) @variable.parameter.state)))

(param_decl type: (type) @type)

;; =========================================================================
;; Variable declarations — colored by scope
;; =========================================================================

;; Autotuner-scope: type + name form  (int counter;)
(autotuner_var_decl name: (identifier) @variable.autotuner)

;; Autotuner-scope: assignment form   (sum = 0;)
(autotuner_var_decl targets: (identifier) @variable.autotuner)

;; State-scope declarations
(stmt (var_decl_stmt name: (identifier) @variable.state))

;; =========================================================================
;; Assignment targets inside states
;; =========================================================================
(stmt (assign_stmt targets: (identifier) @variable))

;; =========================================================================
;; Transitions and entry
;; =========================================================================
(entry_stmt      target: (identifier) @label)
(transition_stmt target: (identifier) @label)

;; =========================================================================
;; Expressions — calls, members
;; =========================================================================
(call_expr   func:   (identifier) @function.call)
(member_expr member: (identifier) @variable.member)
(named_arg   name:   (identifier) @variable.parameter)

;; FIX: index_expr object field holds (expr), not (identifier) directly.
;; Match the expr node and capture the identifier inside it instead.
(index_expr
  object: (expr
    (primary_expr
      (identifier) @variable)))

;; =========================================================================
;; Operators
;; =========================================================================
["=" "+" "-" "*" "/" "==" "!=" "<" ">" "<=" ">=" "&&" "||" "!"] @operator

;; =========================================================================
;; Literals
;; =========================================================================
(int_literal)    @number
(float_literal)  @number.float
(bool_literal)   @boolean
(nil_literal)    @constant.builtin
(string_literal) @string

;; =========================================================================
;; Comments
;; =========================================================================
(comment) @comment @spell

;; =========================================================================
;; Punctuation
;; =========================================================================
["{" "}"] @punctuation.bracket
["(" ")"] @punctuation.bracket
["[" "]"] @punctuation.bracket
";"       @punctuation.delimiter
","       @punctuation.delimiter
"."       @punctuation.delimiter

;; =========================================================================
;; Fallback for any identifier not matched above
;; =========================================================================
(identifier) @variable
