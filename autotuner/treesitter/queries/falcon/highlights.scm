;; Keywords
"autotuner"  @keyword
"routine"    @keyword
"state"      @keyword
"start"      @keyword
"uses"       @keyword
"terminal"   @keyword
"if"         @keyword.conditional
"elif"       @keyword.conditional
"else"       @keyword.conditional

;; Types
(type) @type

;; Identifiers
(identifier) @variable

;; Named fields — highlight declaration names as function/type names
(autotuner_decl name: (identifier) @type.definition)
(routine_decl   name: (identifier) @function)
(state_decl     name: (identifier) @label)

;; Entry / transition targets
(entry_stmt     target: (identifier) @label)
(transition_stmt target: (identifier) @label)

;; Parameters
(param_decl name: (identifier) @variable.parameter)

;; Assignment targets
(assign_stmt targets: (identifier) @variable)

;; Literals
(int_literal)    @number
(float_literal)  @number.float
(bool_literal)   @boolean
(string_literal) @string
(nil_literal)    @constant.builtin

;; Operators / punctuation
"->"   @operator
"="    @operator
"+"    @operator
"-"    @operator
"*"    @operator
"/"    @operator
"=="   @operator
"!="   @operator
"<"    @operator
">"    @operator
"<="   @operator
">="   @operator
"&&"   @operator
"||"   @operator
"!"    @operator

;; Comments
(comment) @comment

;; Brackets
"{" @punctuation.bracket
"}" @punctuation.bracket
"(" @punctuation.bracket
")" @punctuation.bracket
"[" @punctuation.bracket
"]" @punctuation.bracket
";" @punctuation.delimiter
"," @punctuation.delimiter
"." @punctuation.delimiter
