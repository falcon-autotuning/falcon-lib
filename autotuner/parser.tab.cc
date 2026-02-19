// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





// "%code requires" blocks.
#line 12 "./compiler/src/parser.y"

  #include <string>
  #include <vector>
  #include <memory>
  #include "falcon-atc/AST.hpp"
  
  namespace falcon::atc {
    class Expr;
    struct Program;
  }

#line 55 "parser.tab.cc"


# include <cstdlib> // std::abort
# include <iostream>
# include <stdexcept>
# include <string>
# include <vector>

#if defined __cplusplus
# define YY_CPLUSPLUS __cplusplus
#else
# define YY_CPLUSPLUS 199711L
#endif

// Support move semantics when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_MOVE           std::move
# define YY_MOVE_OR_COPY   move
# define YY_MOVE_REF(Type) Type&&
# define YY_RVREF(Type)    Type&&
# define YY_COPY(Type)     Type
#else
# define YY_MOVE
# define YY_MOVE_OR_COPY   copy
# define YY_MOVE_REF(Type) Type&
# define YY_RVREF(Type)    const Type&
# define YY_COPY(Type)     const Type&
#endif

// Support noexcept when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_NOEXCEPT noexcept
# define YY_NOTHROW
#else
# define YY_NOEXCEPT
# define YY_NOTHROW throw ()
#endif

// Support constexpr when possible.
#if 201703 <= YY_CPLUSPLUS
# define YY_CONSTEXPR constexpr
#else
# define YY_CONSTEXPR
#endif



#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

#line 4 "./compiler/src/parser.y"
namespace falcon { namespace atc {
#line 191 "parser.tab.cc"




  /// A Bison parser.
  class Parser
  {
  public:
#ifdef YYSTYPE
# ifdef __GNUC__
#  pragma GCC message "bison: do not #define YYSTYPE in C++, use %define api.value.type"
# endif
    typedef YYSTYPE value_type;
#else
  /// A buffer to store and retrieve objects.
  ///
  /// Sort of a variant, but does not keep track of the nature
  /// of the stored data, since that knowledge is available
  /// via the current parser state.
  class value_type
  {
  public:
    /// Type of *this.
    typedef value_type self_type;

    /// Empty construction.
    value_type () YY_NOEXCEPT
      : yyraw_ ()
    {}

    /// Construct and fill.
    template <typename T>
    value_type (YY_RVREF (T) t)
    {
      new (yyas_<T> ()) T (YY_MOVE (t));
    }

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    value_type (const self_type&) = delete;
    /// Non copyable.
    self_type& operator= (const self_type&) = delete;
#endif

    /// Destruction, allowed only if empty.
    ~value_type () YY_NOEXCEPT
    {}

# if 201103L <= YY_CPLUSPLUS
    /// Instantiate a \a T in here from \a t.
    template <typename T, typename... U>
    T&
    emplace (U&&... u)
    {
      return *new (yyas_<T> ()) T (std::forward <U>(u)...);
    }
# else
    /// Instantiate an empty \a T in here.
    template <typename T>
    T&
    emplace ()
    {
      return *new (yyas_<T> ()) T ();
    }

    /// Instantiate a \a T in here from \a t.
    template <typename T>
    T&
    emplace (const T& t)
    {
      return *new (yyas_<T> ()) T (t);
    }
# endif

    /// Instantiate an empty \a T in here.
    /// Obsolete, use emplace.
    template <typename T>
    T&
    build ()
    {
      return emplace<T> ();
    }

    /// Instantiate a \a T in here from \a t.
    /// Obsolete, use emplace.
    template <typename T>
    T&
    build (const T& t)
    {
      return emplace<T> (t);
    }

    /// Accessor to a built \a T.
    template <typename T>
    T&
    as () YY_NOEXCEPT
    {
      return *yyas_<T> ();
    }

    /// Const accessor to a built \a T (for %printer).
    template <typename T>
    const T&
    as () const YY_NOEXCEPT
    {
      return *yyas_<T> ();
    }

    /// Swap the content with \a that, of same type.
    ///
    /// Both variants must be built beforehand, because swapping the actual
    /// data requires reading it (with as()), and this is not possible on
    /// unconstructed variants: it would require some dynamic testing, which
    /// should not be the variant's responsibility.
    /// Swapping between built and (possibly) non-built is done with
    /// self_type::move ().
    template <typename T>
    void
    swap (self_type& that) YY_NOEXCEPT
    {
      std::swap (as<T> (), that.as<T> ());
    }

    /// Move the content of \a that to this.
    ///
    /// Destroys \a that.
    template <typename T>
    void
    move (self_type& that)
    {
# if 201103L <= YY_CPLUSPLUS
      emplace<T> (std::move (that.as<T> ()));
# else
      emplace<T> ();
      swap<T> (that);
# endif
      that.destroy<T> ();
    }

# if 201103L <= YY_CPLUSPLUS
    /// Move the content of \a that to this.
    template <typename T>
    void
    move (self_type&& that)
    {
      emplace<T> (std::move (that.as<T> ()));
      that.destroy<T> ();
    }
#endif

    /// Copy the content of \a that to this.
    template <typename T>
    void
    copy (const self_type& that)
    {
      emplace<T> (that.as<T> ());
    }

    /// Destroy the stored \a T.
    template <typename T>
    void
    destroy ()
    {
      as<T> ().~T ();
    }

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    value_type (const self_type&);
    /// Non copyable.
    self_type& operator= (const self_type&);
#endif

    /// Accessor to raw memory as \a T.
    template <typename T>
    T*
    yyas_ () YY_NOEXCEPT
    {
      void *yyp = yyraw_;
      return static_cast<T*> (yyp);
     }

    /// Const accessor to raw memory as \a T.
    template <typename T>
    const T*
    yyas_ () const YY_NOEXCEPT
    {
      const void *yyp = yyraw_;
      return static_cast<const T*> (yyp);
     }

    /// An auxiliary type to compute the largest semantic type.
    union union_type
    {
      // type_spec
      char dummy1[sizeof (ParamType)];

      // IDENTIFIER
      // DOUBLE
      // INTEGER
      // STRING
      // entry_clause
      char dummy2[sizeof (std::string)];

      // autotuner_decl
      char dummy3[sizeof (std::unique_ptr<AutotunerDecl>)];

      // measurement_opt
      // expr
      // primary_expr
      char dummy4[sizeof (std::unique_ptr<Expr>)];

      // loop_decl
      char dummy5[sizeof (std::unique_ptr<ForLoop>)];

      // mapping_decl
      char dummy6[sizeof (std::unique_ptr<Mapping>)];

      // sig_param_decl
      // param_decl
      char dummy7[sizeof (std::unique_ptr<ParamDecl>)];

      // program
      char dummy8[sizeof (std::unique_ptr<Program>)];

      // spec_decl
      char dummy9[sizeof (std::unique_ptr<SpecDecl>)];

      // state_decl
      char dummy10[sizeof (std::unique_ptr<StateDecl>)];

      // trans_target
      char dummy11[sizeof (std::unique_ptr<TransitionTarget>)];

      // assignment_list
      char dummy12[sizeof (std::vector<Assignment>)];

      // autotuners
      char dummy13[sizeof (std::vector<AutotunerDecl>)];

      // loop_list
      char dummy14[sizeof (std::vector<ForLoop>)];

      // mappings
      // mapping_list
      char dummy15[sizeof (std::vector<Mapping>)];

      // input_params
      // output_params
      // sig_param_list
      // params_block
      // param_list
      // state_params
      // state_temps
      char dummy16[sizeof (std::vector<ParamDecl>)];

      // spec_inputs
      // spec_outputs
      // spec_list
      char dummy17[sizeof (std::vector<SpecDecl>)];

      // loop_states
      // states
      // state_list
      char dummy18[sizeof (std::vector<StateDecl>)];

      // transition_list
      // transition_decl
      // simple_transition
      char dummy19[sizeof (std::vector<Transition>)];

      // generic_params
      // requires_clause
      // separated_idents
      // separated_strings
      char dummy20[sizeof (std::vector<std::string>)];

      // arg_list
      // nonempty_arg_list
      char dummy21[sizeof (std::vector<std::unique_ptr<Expr>>)];
    };

    /// The size of the largest semantic type.
    enum { size = sizeof (union_type) };

    /// A buffer to store semantic values.
    union
    {
      /// Strongest alignment constraints.
      long double yyalign_me_;
      /// A buffer large enough to store any of the semantic values.
      char yyraw_[size];
    };
  };

#endif
    /// Backward compatibility (Bison 3.8).
    typedef value_type semantic_type;


    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const std::string& m)
        : std::runtime_error (m)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;
    };

    /// Token kinds.
    struct token
    {
      enum token_kind_type
      {
        TOK_YYEMPTY = -2,
    TOK_YYEOF = 0,                 // "end of file"
    TOK_YYerror = 256,             // error
    TOK_YYUNDEF = 257,             // "invalid token"
    TOK_IDENTIFIER = 258,          // IDENTIFIER
    TOK_DOUBLE = 259,              // DOUBLE
    TOK_INTEGER = 260,             // INTEGER
    TOK_STRING = 261,              // STRING
    TOK_AUTOTUNER = 262,           // AUTOTUNER
    TOK_STATE = 263,               // STATE
    TOK_PARAMS = 264,              // PARAMS
    TOK_TEMP = 265,                // TEMP
    TOK_MEASUREMENT = 266,         // MEASUREMENT
    TOK_RUN = 267,                 // RUN
    TOK_START = 268,               // START
    TOK_REQUIRES = 269,            // REQUIRES
    TOK_TERMINAL = 270,            // TERMINAL
    TOK_IF = 271,                  // IF
    TOK_ELSE = 272,                // ELSE
    TOK_TRUE = 273,                // TRUE
    TOK_FALSE = 274,               // FALSE
    TOK_SUCCESS = 275,             // SUCCESS
    TOK_FAIL = 276,                // FAIL
    TOK_SPEC_INPUTS = 277,         // SPEC_INPUTS
    TOK_SPEC_OUTPUTS = 278,        // SPEC_OUTPUTS
    TOK_CONFIG_VAR = 279,          // CONFIG_VAR
    TOK_NEXT = 280,                // NEXT
    TOK_FOR = 281,                 // FOR
    TOK_IN = 282,                  // IN
    TOK_FLOAT_KW = 283,            // FLOAT_KW
    TOK_INT_KW = 284,              // INT_KW
    TOK_BOOL_KW = 285,             // BOOL_KW
    TOK_STRING_KW = 286,           // STRING_KW
    TOK_QUANTITY_KW = 287,         // QUANTITY_KW
    TOK_CONFIG_KW = 288,           // CONFIG_KW
    TOK_GROUP_KW = 289,            // GROUP_KW
    TOK_CONNECTION_KW = 290,       // CONNECTION_KW
    TOK_ARROW = 291,               // ARROW
    TOK_DOUBLECOLON = 292,         // DOUBLECOLON
    TOK_LBRACKET = 293,            // LBRACKET
    TOK_RBRACKET = 294,            // RBRACKET
    TOK_LBRACE = 295,              // LBRACE
    TOK_RBRACE = 296,              // RBRACE
    TOK_LPAREN = 297,              // LPAREN
    TOK_RPAREN = 298,              // RPAREN
    TOK_ASSIGN = 299,              // ASSIGN
    TOK_COMMA = 300,               // COMMA
    TOK_COLON = 301,               // COLON
    TOK_SEMICOLON = 302,           // SEMICOLON
    TOK_DOT = 303,                 // DOT
    TOK_PLUS = 304,                // PLUS
    TOK_MINUS = 305,               // MINUS
    TOK_MUL = 306,                 // MUL
    TOK_DIV = 307,                 // DIV
    TOK_EQ = 308,                  // EQ
    TOK_NE = 309,                  // NE
    TOK_LL = 310,                  // LL
    TOK_GG = 311,                  // GG
    TOK_LE = 312,                  // LE
    TOK_GE = 313,                  // GE
    TOK_AND = 314,                 // AND
    TOK_OR = 315,                  // OR
    TOK_NOT = 316,                 // NOT
    TOK_UMINUS = 317               // UMINUS
      };
      /// Backward compatibility alias (Bison 3.6).
      typedef token_kind_type yytokentype;
    };

    /// Token kind, as returned by yylex.
    typedef token::token_kind_type token_kind_type;

    /// Backward compatibility alias (Bison 3.6).
    typedef token_kind_type token_type;

    /// Symbol kinds.
    struct symbol_kind
    {
      enum symbol_kind_type
      {
        YYNTOKENS = 63, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of file"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_IDENTIFIER = 3,                        // IDENTIFIER
        S_DOUBLE = 4,                            // DOUBLE
        S_INTEGER = 5,                           // INTEGER
        S_STRING = 6,                            // STRING
        S_AUTOTUNER = 7,                         // AUTOTUNER
        S_STATE = 8,                             // STATE
        S_PARAMS = 9,                            // PARAMS
        S_TEMP = 10,                             // TEMP
        S_MEASUREMENT = 11,                      // MEASUREMENT
        S_RUN = 12,                              // RUN
        S_START = 13,                            // START
        S_REQUIRES = 14,                         // REQUIRES
        S_TERMINAL = 15,                         // TERMINAL
        S_IF = 16,                               // IF
        S_ELSE = 17,                             // ELSE
        S_TRUE = 18,                             // TRUE
        S_FALSE = 19,                            // FALSE
        S_SUCCESS = 20,                          // SUCCESS
        S_FAIL = 21,                             // FAIL
        S_SPEC_INPUTS = 22,                      // SPEC_INPUTS
        S_SPEC_OUTPUTS = 23,                     // SPEC_OUTPUTS
        S_CONFIG_VAR = 24,                       // CONFIG_VAR
        S_NEXT = 25,                             // NEXT
        S_FOR = 26,                              // FOR
        S_IN = 27,                               // IN
        S_FLOAT_KW = 28,                         // FLOAT_KW
        S_INT_KW = 29,                           // INT_KW
        S_BOOL_KW = 30,                          // BOOL_KW
        S_STRING_KW = 31,                        // STRING_KW
        S_QUANTITY_KW = 32,                      // QUANTITY_KW
        S_CONFIG_KW = 33,                        // CONFIG_KW
        S_GROUP_KW = 34,                         // GROUP_KW
        S_CONNECTION_KW = 35,                    // CONNECTION_KW
        S_ARROW = 36,                            // ARROW
        S_DOUBLECOLON = 37,                      // DOUBLECOLON
        S_LBRACKET = 38,                         // LBRACKET
        S_RBRACKET = 39,                         // RBRACKET
        S_LBRACE = 40,                           // LBRACE
        S_RBRACE = 41,                           // RBRACE
        S_LPAREN = 42,                           // LPAREN
        S_RPAREN = 43,                           // RPAREN
        S_ASSIGN = 44,                           // ASSIGN
        S_COMMA = 45,                            // COMMA
        S_COLON = 46,                            // COLON
        S_SEMICOLON = 47,                        // SEMICOLON
        S_DOT = 48,                              // DOT
        S_PLUS = 49,                             // PLUS
        S_MINUS = 50,                            // MINUS
        S_MUL = 51,                              // MUL
        S_DIV = 52,                              // DIV
        S_EQ = 53,                               // EQ
        S_NE = 54,                               // NE
        S_LL = 55,                               // LL
        S_GG = 56,                               // GG
        S_LE = 57,                               // LE
        S_GE = 58,                               // GE
        S_AND = 59,                              // AND
        S_OR = 60,                               // OR
        S_NOT = 61,                              // NOT
        S_UMINUS = 62,                           // UMINUS
        S_YYACCEPT = 63,                         // $accept
        S_program = 64,                          // program
        S_autotuners = 65,                       // autotuners
        S_autotuner_decl = 66,                   // autotuner_decl
        S_input_params = 67,                     // input_params
        S_output_params = 68,                    // output_params
        S_sig_param_list = 69,                   // sig_param_list
        S_sig_param_decl = 70,                   // sig_param_decl
        S_generic_params = 71,                   // generic_params
        S_spec_inputs = 72,                      // spec_inputs
        S_spec_outputs = 73,                     // spec_outputs
        S_spec_list = 74,                        // spec_list
        S_spec_decl = 75,                        // spec_decl
        S_requires_clause = 76,                  // requires_clause
        S_separated_idents = 77,                 // separated_idents
        S_separated_strings = 78,                // separated_strings
        S_params_block = 79,                     // params_block
        S_param_list = 80,                       // param_list
        S_param_decl = 81,                       // param_decl
        S_type_spec = 82,                        // type_spec
        S_entry_clause = 83,                     // entry_clause
        S_loop_list = 84,                        // loop_list
        S_loop_decl = 85,                        // loop_decl
        S_loop_states = 86,                      // loop_states
        S_states = 87,                           // states
        S_state_list = 88,                       // state_list
        S_state_decl = 89,                       // state_decl
        S_state_params = 90,                     // state_params
        S_state_temps = 91,                      // state_temps
        S_measurement_opt = 92,                  // measurement_opt
        S_arg_list = 93,                         // arg_list
        S_nonempty_arg_list = 94,                // nonempty_arg_list
        S_transition_list = 95,                  // transition_list
        S_transition_decl = 96,                  // transition_decl
        S_simple_transition = 97,                // simple_transition
        S_assignment_list = 98,                  // assignment_list
        S_trans_target = 99,                     // trans_target
        S_mappings = 100,                        // mappings
        S_mapping_list = 101,                    // mapping_list
        S_mapping_decl = 102,                    // mapping_decl
        S_expr = 103,                            // expr
        S_primary_expr = 104                     // primary_expr
      };
    };

    /// (Internal) symbol kind.
    typedef symbol_kind::symbol_kind_type symbol_kind_type;

    /// The number of tokens.
    static const symbol_kind_type YYNTOKENS = symbol_kind::YYNTOKENS;

    /// A complete symbol.
    ///
    /// Expects its Base type to provide access to the symbol kind
    /// via kind ().
    ///
    /// Provide access to semantic value.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol () YY_NOEXCEPT
        : value ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value ()
      {
        switch (this->kind ())
    {
      case symbol_kind::S_type_spec: // type_spec
        value.move< ParamType > (std::move (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        value.move< std::string > (std::move (that.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.move< std::unique_ptr<AutotunerDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.move< std::unique_ptr<Expr> > (std::move (that.value));
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        value.move< std::unique_ptr<ForLoop> > (std::move (that.value));
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        value.move< std::unique_ptr<Mapping> > (std::move (that.value));
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        value.move< std::unique_ptr<ParamDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_program: // program
        value.move< std::unique_ptr<Program> > (std::move (that.value));
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        value.move< std::unique_ptr<SpecDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.move< std::unique_ptr<StateDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_trans_target: // trans_target
        value.move< std::unique_ptr<TransitionTarget> > (std::move (that.value));
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        value.move< std::vector<Assignment> > (std::move (that.value));
        break;

      case symbol_kind::S_autotuners: // autotuners
        value.move< std::vector<AutotunerDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_loop_list: // loop_list
        value.move< std::vector<ForLoop> > (std::move (that.value));
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        value.move< std::vector<Mapping> > (std::move (that.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        value.move< std::vector<ParamDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        value.move< std::vector<SpecDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        value.move< std::vector<StateDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        value.move< std::vector<Transition> > (std::move (that.value));
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        value.move< std::vector<std::string> > (std::move (that.value));
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        value.move< std::vector<std::unique_ptr<Expr>> > (std::move (that.value));
        break;

      default:
        break;
    }

      }
#endif

      /// Copy constructor.
      basic_symbol (const basic_symbol& that);

      /// Constructors for typed symbols.
#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t)
        : Base (t)
      {}
#else
      basic_symbol (typename Base::kind_type t)
        : Base (t)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ParamType&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ParamType& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::string&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::string& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<AutotunerDecl>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<AutotunerDecl>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<Expr>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<Expr>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<ForLoop>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<ForLoop>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<Mapping>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<Mapping>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<ParamDecl>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<ParamDecl>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<Program>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<Program>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<SpecDecl>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<SpecDecl>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<StateDecl>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<StateDecl>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<TransitionTarget>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<TransitionTarget>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<Assignment>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<Assignment>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<AutotunerDecl>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<AutotunerDecl>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<ForLoop>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<ForLoop>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<Mapping>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<Mapping>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<ParamDecl>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<ParamDecl>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<SpecDecl>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<SpecDecl>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<StateDecl>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<StateDecl>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<Transition>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<Transition>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<std::string>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<std::string>& v)
        : Base (t)
        , value (v)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<std::unique_ptr<Expr>>&& v)
        : Base (t)
        , value (std::move (v))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<std::unique_ptr<Expr>>& v)
        : Base (t)
        , value (v)
      {}
#endif

      /// Destroy the symbol.
      ~basic_symbol ()
      {
        clear ();
      }



      /// Destroy contents, and record that is empty.
      void clear () YY_NOEXCEPT
      {
        // User destructor.
        symbol_kind_type yykind = this->kind ();
        basic_symbol<Base>& yysym = *this;
        (void) yysym;
        switch (yykind)
        {
       default:
          break;
        }

        // Value type destructor.
switch (yykind)
    {
      case symbol_kind::S_type_spec: // type_spec
        value.template destroy< ParamType > ();
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        value.template destroy< std::string > ();
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.template destroy< std::unique_ptr<AutotunerDecl> > ();
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.template destroy< std::unique_ptr<Expr> > ();
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        value.template destroy< std::unique_ptr<ForLoop> > ();
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        value.template destroy< std::unique_ptr<Mapping> > ();
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        value.template destroy< std::unique_ptr<ParamDecl> > ();
        break;

      case symbol_kind::S_program: // program
        value.template destroy< std::unique_ptr<Program> > ();
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        value.template destroy< std::unique_ptr<SpecDecl> > ();
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.template destroy< std::unique_ptr<StateDecl> > ();
        break;

      case symbol_kind::S_trans_target: // trans_target
        value.template destroy< std::unique_ptr<TransitionTarget> > ();
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        value.template destroy< std::vector<Assignment> > ();
        break;

      case symbol_kind::S_autotuners: // autotuners
        value.template destroy< std::vector<AutotunerDecl> > ();
        break;

      case symbol_kind::S_loop_list: // loop_list
        value.template destroy< std::vector<ForLoop> > ();
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        value.template destroy< std::vector<Mapping> > ();
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        value.template destroy< std::vector<ParamDecl> > ();
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        value.template destroy< std::vector<SpecDecl> > ();
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        value.template destroy< std::vector<StateDecl> > ();
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        value.template destroy< std::vector<Transition> > ();
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        value.template destroy< std::vector<std::string> > ();
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        value.template destroy< std::vector<std::unique_ptr<Expr>> > ();
        break;

      default:
        break;
    }

        Base::clear ();
      }

      /// The user-facing name of this symbol.
      const char *name () const YY_NOEXCEPT
      {
        return Parser::symbol_name (this->kind ());
      }

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// Whether empty.
      bool empty () const YY_NOEXCEPT;

      /// Destructive move, \a s is emptied into this.
      void move (basic_symbol& s);

      /// The semantic value.
      value_type value;

    private:
#if YY_CPLUSPLUS < 201103L
      /// Assignment operator.
      basic_symbol& operator= (const basic_symbol& that);
#endif
    };

    /// Type access provider for token (enum) based symbols.
    struct by_kind
    {
      /// The symbol kind as needed by the constructor.
      typedef token_kind_type kind_type;

      /// Default constructor.
      by_kind () YY_NOEXCEPT;

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      by_kind (by_kind&& that) YY_NOEXCEPT;
#endif

      /// Copy constructor.
      by_kind (const by_kind& that) YY_NOEXCEPT;

      /// Constructor from (external) token numbers.
      by_kind (kind_type t) YY_NOEXCEPT;



      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_kind& that);

      /// The (internal) type number (corresponding to \a type).
      /// \a empty when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// The symbol kind.
      /// \a S_YYEMPTY when empty.
      symbol_kind_type kind_;
    };

    /// Backward compatibility for a private implementation detail (Bison 3.6).
    typedef by_kind by_type;

    /// "External" symbols: returned by the scanner.
    struct symbol_type : basic_symbol<by_kind>
    {
      /// Superclass.
      typedef basic_symbol<by_kind> super_type;

      /// Empty symbol.
      symbol_type () YY_NOEXCEPT {}

      /// Constructor for valueless symbols, and symbols from each type.
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok)
        : super_type (token_kind_type (tok))
#else
      symbol_type (int tok)
        : super_type (token_kind_type (tok))
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, std::string v)
        : super_type (token_kind_type (tok), std::move (v))
#else
      symbol_type (int tok, const std::string& v)
        : super_type (token_kind_type (tok), v)
#endif
      {}
    };

    /// Build a parser object.
    Parser ();
    virtual ~Parser ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    Parser (const Parser&) = delete;
    /// Non copyable.
    Parser& operator= (const Parser&) = delete;
#endif

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.
    /// \param msg    a description of the syntax error.
    virtual void error (const std::string& msg);

    /// Report a syntax error.
    void error (const syntax_error& err);

    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static const char *symbol_name (symbol_kind_type yysymbol);

    // Implementation of make_symbol for each token kind.
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYEOF ()
      {
        return symbol_type (token::TOK_YYEOF);
      }
#else
      static
      symbol_type
      make_YYEOF ()
      {
        return symbol_type (token::TOK_YYEOF);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYerror ()
      {
        return symbol_type (token::TOK_YYerror);
      }
#else
      static
      symbol_type
      make_YYerror ()
      {
        return symbol_type (token::TOK_YYerror);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYUNDEF ()
      {
        return symbol_type (token::TOK_YYUNDEF);
      }
#else
      static
      symbol_type
      make_YYUNDEF ()
      {
        return symbol_type (token::TOK_YYUNDEF);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IDENTIFIER (std::string v)
      {
        return symbol_type (token::TOK_IDENTIFIER, std::move (v));
      }
#else
      static
      symbol_type
      make_IDENTIFIER (const std::string& v)
      {
        return symbol_type (token::TOK_IDENTIFIER, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DOUBLE (std::string v)
      {
        return symbol_type (token::TOK_DOUBLE, std::move (v));
      }
#else
      static
      symbol_type
      make_DOUBLE (const std::string& v)
      {
        return symbol_type (token::TOK_DOUBLE, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_INTEGER (std::string v)
      {
        return symbol_type (token::TOK_INTEGER, std::move (v));
      }
#else
      static
      symbol_type
      make_INTEGER (const std::string& v)
      {
        return symbol_type (token::TOK_INTEGER, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STRING (std::string v)
      {
        return symbol_type (token::TOK_STRING, std::move (v));
      }
#else
      static
      symbol_type
      make_STRING (const std::string& v)
      {
        return symbol_type (token::TOK_STRING, v);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_AUTOTUNER ()
      {
        return symbol_type (token::TOK_AUTOTUNER);
      }
#else
      static
      symbol_type
      make_AUTOTUNER ()
      {
        return symbol_type (token::TOK_AUTOTUNER);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STATE ()
      {
        return symbol_type (token::TOK_STATE);
      }
#else
      static
      symbol_type
      make_STATE ()
      {
        return symbol_type (token::TOK_STATE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PARAMS ()
      {
        return symbol_type (token::TOK_PARAMS);
      }
#else
      static
      symbol_type
      make_PARAMS ()
      {
        return symbol_type (token::TOK_PARAMS);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_TEMP ()
      {
        return symbol_type (token::TOK_TEMP);
      }
#else
      static
      symbol_type
      make_TEMP ()
      {
        return symbol_type (token::TOK_TEMP);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MEASUREMENT ()
      {
        return symbol_type (token::TOK_MEASUREMENT);
      }
#else
      static
      symbol_type
      make_MEASUREMENT ()
      {
        return symbol_type (token::TOK_MEASUREMENT);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RUN ()
      {
        return symbol_type (token::TOK_RUN);
      }
#else
      static
      symbol_type
      make_RUN ()
      {
        return symbol_type (token::TOK_RUN);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_START ()
      {
        return symbol_type (token::TOK_START);
      }
#else
      static
      symbol_type
      make_START ()
      {
        return symbol_type (token::TOK_START);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_REQUIRES ()
      {
        return symbol_type (token::TOK_REQUIRES);
      }
#else
      static
      symbol_type
      make_REQUIRES ()
      {
        return symbol_type (token::TOK_REQUIRES);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_TERMINAL ()
      {
        return symbol_type (token::TOK_TERMINAL);
      }
#else
      static
      symbol_type
      make_TERMINAL ()
      {
        return symbol_type (token::TOK_TERMINAL);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IF ()
      {
        return symbol_type (token::TOK_IF);
      }
#else
      static
      symbol_type
      make_IF ()
      {
        return symbol_type (token::TOK_IF);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ELSE ()
      {
        return symbol_type (token::TOK_ELSE);
      }
#else
      static
      symbol_type
      make_ELSE ()
      {
        return symbol_type (token::TOK_ELSE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_TRUE ()
      {
        return symbol_type (token::TOK_TRUE);
      }
#else
      static
      symbol_type
      make_TRUE ()
      {
        return symbol_type (token::TOK_TRUE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FALSE ()
      {
        return symbol_type (token::TOK_FALSE);
      }
#else
      static
      symbol_type
      make_FALSE ()
      {
        return symbol_type (token::TOK_FALSE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SUCCESS ()
      {
        return symbol_type (token::TOK_SUCCESS);
      }
#else
      static
      symbol_type
      make_SUCCESS ()
      {
        return symbol_type (token::TOK_SUCCESS);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FAIL ()
      {
        return symbol_type (token::TOK_FAIL);
      }
#else
      static
      symbol_type
      make_FAIL ()
      {
        return symbol_type (token::TOK_FAIL);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SPEC_INPUTS ()
      {
        return symbol_type (token::TOK_SPEC_INPUTS);
      }
#else
      static
      symbol_type
      make_SPEC_INPUTS ()
      {
        return symbol_type (token::TOK_SPEC_INPUTS);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SPEC_OUTPUTS ()
      {
        return symbol_type (token::TOK_SPEC_OUTPUTS);
      }
#else
      static
      symbol_type
      make_SPEC_OUTPUTS ()
      {
        return symbol_type (token::TOK_SPEC_OUTPUTS);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_CONFIG_VAR ()
      {
        return symbol_type (token::TOK_CONFIG_VAR);
      }
#else
      static
      symbol_type
      make_CONFIG_VAR ()
      {
        return symbol_type (token::TOK_CONFIG_VAR);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NEXT ()
      {
        return symbol_type (token::TOK_NEXT);
      }
#else
      static
      symbol_type
      make_NEXT ()
      {
        return symbol_type (token::TOK_NEXT);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FOR ()
      {
        return symbol_type (token::TOK_FOR);
      }
#else
      static
      symbol_type
      make_FOR ()
      {
        return symbol_type (token::TOK_FOR);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IN ()
      {
        return symbol_type (token::TOK_IN);
      }
#else
      static
      symbol_type
      make_IN ()
      {
        return symbol_type (token::TOK_IN);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FLOAT_KW ()
      {
        return symbol_type (token::TOK_FLOAT_KW);
      }
#else
      static
      symbol_type
      make_FLOAT_KW ()
      {
        return symbol_type (token::TOK_FLOAT_KW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_INT_KW ()
      {
        return symbol_type (token::TOK_INT_KW);
      }
#else
      static
      symbol_type
      make_INT_KW ()
      {
        return symbol_type (token::TOK_INT_KW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_BOOL_KW ()
      {
        return symbol_type (token::TOK_BOOL_KW);
      }
#else
      static
      symbol_type
      make_BOOL_KW ()
      {
        return symbol_type (token::TOK_BOOL_KW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STRING_KW ()
      {
        return symbol_type (token::TOK_STRING_KW);
      }
#else
      static
      symbol_type
      make_STRING_KW ()
      {
        return symbol_type (token::TOK_STRING_KW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_QUANTITY_KW ()
      {
        return symbol_type (token::TOK_QUANTITY_KW);
      }
#else
      static
      symbol_type
      make_QUANTITY_KW ()
      {
        return symbol_type (token::TOK_QUANTITY_KW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_CONFIG_KW ()
      {
        return symbol_type (token::TOK_CONFIG_KW);
      }
#else
      static
      symbol_type
      make_CONFIG_KW ()
      {
        return symbol_type (token::TOK_CONFIG_KW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_GROUP_KW ()
      {
        return symbol_type (token::TOK_GROUP_KW);
      }
#else
      static
      symbol_type
      make_GROUP_KW ()
      {
        return symbol_type (token::TOK_GROUP_KW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_CONNECTION_KW ()
      {
        return symbol_type (token::TOK_CONNECTION_KW);
      }
#else
      static
      symbol_type
      make_CONNECTION_KW ()
      {
        return symbol_type (token::TOK_CONNECTION_KW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ARROW ()
      {
        return symbol_type (token::TOK_ARROW);
      }
#else
      static
      symbol_type
      make_ARROW ()
      {
        return symbol_type (token::TOK_ARROW);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DOUBLECOLON ()
      {
        return symbol_type (token::TOK_DOUBLECOLON);
      }
#else
      static
      symbol_type
      make_DOUBLECOLON ()
      {
        return symbol_type (token::TOK_DOUBLECOLON);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LBRACKET ()
      {
        return symbol_type (token::TOK_LBRACKET);
      }
#else
      static
      symbol_type
      make_LBRACKET ()
      {
        return symbol_type (token::TOK_LBRACKET);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RBRACKET ()
      {
        return symbol_type (token::TOK_RBRACKET);
      }
#else
      static
      symbol_type
      make_RBRACKET ()
      {
        return symbol_type (token::TOK_RBRACKET);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LBRACE ()
      {
        return symbol_type (token::TOK_LBRACE);
      }
#else
      static
      symbol_type
      make_LBRACE ()
      {
        return symbol_type (token::TOK_LBRACE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RBRACE ()
      {
        return symbol_type (token::TOK_RBRACE);
      }
#else
      static
      symbol_type
      make_RBRACE ()
      {
        return symbol_type (token::TOK_RBRACE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LPAREN ()
      {
        return symbol_type (token::TOK_LPAREN);
      }
#else
      static
      symbol_type
      make_LPAREN ()
      {
        return symbol_type (token::TOK_LPAREN);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RPAREN ()
      {
        return symbol_type (token::TOK_RPAREN);
      }
#else
      static
      symbol_type
      make_RPAREN ()
      {
        return symbol_type (token::TOK_RPAREN);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ASSIGN ()
      {
        return symbol_type (token::TOK_ASSIGN);
      }
#else
      static
      symbol_type
      make_ASSIGN ()
      {
        return symbol_type (token::TOK_ASSIGN);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_COMMA ()
      {
        return symbol_type (token::TOK_COMMA);
      }
#else
      static
      symbol_type
      make_COMMA ()
      {
        return symbol_type (token::TOK_COMMA);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_COLON ()
      {
        return symbol_type (token::TOK_COLON);
      }
#else
      static
      symbol_type
      make_COLON ()
      {
        return symbol_type (token::TOK_COLON);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SEMICOLON ()
      {
        return symbol_type (token::TOK_SEMICOLON);
      }
#else
      static
      symbol_type
      make_SEMICOLON ()
      {
        return symbol_type (token::TOK_SEMICOLON);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DOT ()
      {
        return symbol_type (token::TOK_DOT);
      }
#else
      static
      symbol_type
      make_DOT ()
      {
        return symbol_type (token::TOK_DOT);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PLUS ()
      {
        return symbol_type (token::TOK_PLUS);
      }
#else
      static
      symbol_type
      make_PLUS ()
      {
        return symbol_type (token::TOK_PLUS);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MINUS ()
      {
        return symbol_type (token::TOK_MINUS);
      }
#else
      static
      symbol_type
      make_MINUS ()
      {
        return symbol_type (token::TOK_MINUS);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MUL ()
      {
        return symbol_type (token::TOK_MUL);
      }
#else
      static
      symbol_type
      make_MUL ()
      {
        return symbol_type (token::TOK_MUL);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DIV ()
      {
        return symbol_type (token::TOK_DIV);
      }
#else
      static
      symbol_type
      make_DIV ()
      {
        return symbol_type (token::TOK_DIV);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_EQ ()
      {
        return symbol_type (token::TOK_EQ);
      }
#else
      static
      symbol_type
      make_EQ ()
      {
        return symbol_type (token::TOK_EQ);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NE ()
      {
        return symbol_type (token::TOK_NE);
      }
#else
      static
      symbol_type
      make_NE ()
      {
        return symbol_type (token::TOK_NE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LL ()
      {
        return symbol_type (token::TOK_LL);
      }
#else
      static
      symbol_type
      make_LL ()
      {
        return symbol_type (token::TOK_LL);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_GG ()
      {
        return symbol_type (token::TOK_GG);
      }
#else
      static
      symbol_type
      make_GG ()
      {
        return symbol_type (token::TOK_GG);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LE ()
      {
        return symbol_type (token::TOK_LE);
      }
#else
      static
      symbol_type
      make_LE ()
      {
        return symbol_type (token::TOK_LE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_GE ()
      {
        return symbol_type (token::TOK_GE);
      }
#else
      static
      symbol_type
      make_GE ()
      {
        return symbol_type (token::TOK_GE);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_AND ()
      {
        return symbol_type (token::TOK_AND);
      }
#else
      static
      symbol_type
      make_AND ()
      {
        return symbol_type (token::TOK_AND);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_OR ()
      {
        return symbol_type (token::TOK_OR);
      }
#else
      static
      symbol_type
      make_OR ()
      {
        return symbol_type (token::TOK_OR);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NOT ()
      {
        return symbol_type (token::TOK_NOT);
      }
#else
      static
      symbol_type
      make_NOT ()
      {
        return symbol_type (token::TOK_NOT);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_UMINUS ()
      {
        return symbol_type (token::TOK_UMINUS);
      }
#else
      static
      symbol_type
      make_UMINUS ()
      {
        return symbol_type (token::TOK_UMINUS);
      }
#endif


    class context
    {
    public:
      context (const Parser& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const YY_NOEXCEPT { return yyla_; }
      symbol_kind_type token () const YY_NOEXCEPT { return yyla_.kind (); }
      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      const Parser& yyparser_;
      const symbol_type& yyla_;
    };

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    Parser (const Parser&);
    /// Non copyable.
    Parser& operator= (const Parser&);
#endif

    /// Check the lookahead yytoken.
    /// \returns  true iff the token will be eventually shifted.
    bool yy_lac_check_ (symbol_kind_type yytoken) const;
    /// Establish the initial context if no initial context currently exists.
    /// \returns  true iff the token will be eventually shifted.
    bool yy_lac_establish_ (symbol_kind_type yytoken);
    /// Discard any previous initial lookahead context because of event.
    /// \param event  the event which caused the lookahead to be discarded.
    ///               Only used for debbuging output.
    void yy_lac_discard_ (const char* event);

    /// Stored state numbers (used for stacks).
    typedef short state_type;

    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

    /// Generate an error message.
    /// \param yyctx     the context in which the error occurred.
    virtual std::string yysyntax_error_ (const context& yyctx) const;
    /// Compute post-reduction state.
    /// \param yystate   the current state
    /// \param yysym     the nonterminal to push on the stack
    static state_type yy_lr_goto_state_ (state_type yystate, int yysym);

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT;

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT;

    static const short yypact_ninf_;
    static const signed char yytable_ninf_;

    /// Convert a scanner token kind \a t to a symbol kind.
    /// In theory \a t should be a token_kind_type, but character literals
    /// are valid, yet not members of the token_kind_type enum.
    static symbol_kind_type yytranslate_ (int t) YY_NOEXCEPT;



    // Tables.
    // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
    // STATE-NUM.
    static const short yypact_[];

    // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
    // Performed when YYTABLE does not specify something else to do.  Zero
    // means the default is an error.
    static const signed char yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const short yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const unsigned char yydefgoto_[];

    // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
    // positive, shift that token.  If negative, reduce the rule whose
    // number is the opposite.  If YYTABLE_NINF, syntax error.
    static const short yytable_[];

    static const short yycheck_[];

    // YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
    // state STATE-NUM.
    static const signed char yystos_[];

    // YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.
    static const signed char yyr1_[];

    // YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.
    static const signed char yyr2_[];


#if YYDEBUG
    // YYRLINE[YYN] -- Source line where rule number YYN was defined.
    static const short yyrline_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r) const;
    /// Print the state stack on the debug stream.
    virtual void yy_stack_print_ () const;

    /// Debugging level.
    int yydebug_;
    /// Debug stream.
    std::ostream* yycdebug_;

    /// \brief Display a symbol kind, value and location.
    /// \param yyo    The output stream.
    /// \param yysym  The symbol.
    template <typename Base>
    void yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yysym     The symbol.
    template <typename Base>
    void yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const;

  private:
    /// Type access provider for state based symbols.
    struct by_state
    {
      /// Default constructor.
      by_state () YY_NOEXCEPT;

      /// The symbol kind as needed by the constructor.
      typedef state_type kind_type;

      /// Constructor.
      by_state (kind_type s) YY_NOEXCEPT;

      /// Copy constructor.
      by_state (const by_state& that) YY_NOEXCEPT;

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_state& that);

      /// The symbol kind (corresponding to \a state).
      /// \a symbol_kind::S_YYEMPTY when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// The state number used to denote an empty symbol.
      /// We use the initial state, as it does not have a value.
      enum { empty_state = 0 };

      /// The state.
      /// \a empty when empty.
      state_type state;
    };

    /// "Internal" symbol: element of the stack.
    struct stack_symbol_type : basic_symbol<by_state>
    {
      /// Superclass.
      typedef basic_symbol<by_state> super_type;
      /// Construct an empty symbol.
      stack_symbol_type ();
      /// Move or copy construction.
      stack_symbol_type (YY_RVREF (stack_symbol_type) that);
      /// Steal the contents from \a sym to build this.
      stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) sym);
#if YY_CPLUSPLUS < 201103L
      /// Assignment, needed by push_back by some old implementations.
      /// Moves the contents of that.
      stack_symbol_type& operator= (stack_symbol_type& that);

      /// Assignment, needed by push_back by other implementations.
      /// Needed by some other old implementations.
      stack_symbol_type& operator= (const stack_symbol_type& that);
#endif
    };

    /// A stack with random access from its top.
    template <typename T, typename S = std::vector<T> >
    class stack
    {
    public:
      // Hide our reversed order.
      typedef typename S::iterator iterator;
      typedef typename S::const_iterator const_iterator;
      typedef typename S::size_type size_type;
      typedef typename std::ptrdiff_t index_type;

      stack (size_type n = 200) YY_NOEXCEPT
        : seq_ (n)
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Non copyable.
      stack (const stack&) = delete;
      /// Non copyable.
      stack& operator= (const stack&) = delete;
#endif

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      const T&
      operator[] (index_type i) const
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      T&
      operator[] (index_type i)
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Steal the contents of \a t.
      ///
      /// Close to move-semantics.
      void
      push (YY_MOVE_REF (T) t)
      {
        seq_.push_back (T ());
        operator[] (0).move (t);
      }

      /// Pop elements from the stack.
      void
      pop (std::ptrdiff_t n = 1) YY_NOEXCEPT
      {
        for (; 0 < n; --n)
          seq_.pop_back ();
      }

      /// Pop all elements from the stack.
      void
      clear () YY_NOEXCEPT
      {
        seq_.clear ();
      }

      /// Number of elements on the stack.
      index_type
      size () const YY_NOEXCEPT
      {
        return index_type (seq_.size ());
      }

      /// Iterator on top of the stack (going downwards).
      const_iterator
      begin () const YY_NOEXCEPT
      {
        return seq_.begin ();
      }

      /// Bottom of the stack.
      const_iterator
      end () const YY_NOEXCEPT
      {
        return seq_.end ();
      }

      /// Present a slice of the top of a stack.
      class slice
      {
      public:
        slice (const stack& stack, index_type range) YY_NOEXCEPT
          : stack_ (stack)
          , range_ (range)
        {}

        const T&
        operator[] (index_type i) const
        {
          return stack_[range_ - i];
        }

      private:
        const stack& stack_;
        index_type range_;
      };

    private:
#if YY_CPLUSPLUS < 201103L
      /// Non copyable.
      stack (const stack&);
      /// Non copyable.
      stack& operator= (const stack&);
#endif
      /// The wrapped container.
      S seq_;
    };


    /// Stack type.
    typedef stack<stack_symbol_type> stack_type;

    /// The stack.
    stack_type yystack_;
    /// The stack for LAC.
    /// Logically, the yy_lac_stack's lifetime is confined to the function
    /// yy_lac_check_. We just store it as a member of this class to hold
    /// on to the memory and to avoid frequent reallocations.
    /// Since yy_lac_check_ is const, this member must be mutable.
    mutable std::vector<state_type> yylac_stack_;
    /// Whether an initial LAC context was established.
    bool yy_lac_established_;


    /// Push a new state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param sym  the symbol
    /// \warning the contents of \a s.value is stolen.
    void yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a sym.value is stolen.
    void yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym);

    /// Pop \a n symbols from the stack.
    void yypop_ (int n = 1) YY_NOEXCEPT;

    /// Constants.
    enum
    {
      yylast_ = 458,     ///< Last index in yytable_.
      yynnts_ = 42,  ///< Number of nonterminal symbols.
      yyfinal_ = 6 ///< Termination state number.
    };



  };

  Parser::symbol_kind_type
  Parser::yytranslate_ (int t) YY_NOEXCEPT
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62
    };
    // Last valid token kind.
    const int code_max = 317;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

  // basic_symbol.
  template <typename Base>
  Parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value ()
  {
    switch (this->kind ())
    {
      case symbol_kind::S_type_spec: // type_spec
        value.copy< ParamType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        value.copy< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.copy< std::unique_ptr<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.copy< std::unique_ptr<Expr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        value.copy< std::unique_ptr<ForLoop> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        value.copy< std::unique_ptr<Mapping> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        value.copy< std::unique_ptr<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_program: // program
        value.copy< std::unique_ptr<Program> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        value.copy< std::unique_ptr<SpecDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.copy< std::unique_ptr<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_trans_target: // trans_target
        value.copy< std::unique_ptr<TransitionTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        value.copy< std::vector<Assignment> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuners: // autotuners
        value.copy< std::vector<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_list: // loop_list
        value.copy< std::vector<ForLoop> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        value.copy< std::vector<Mapping> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        value.copy< std::vector<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        value.copy< std::vector<SpecDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        value.copy< std::vector<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        value.copy< std::vector<Transition> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        value.copy< std::vector<std::string> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        value.copy< std::vector<std::unique_ptr<Expr>> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

  }




  template <typename Base>
  Parser::symbol_kind_type
  Parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  Parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  Parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    switch (this->kind ())
    {
      case symbol_kind::S_type_spec: // type_spec
        value.move< ParamType > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        value.move< std::string > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.move< std::unique_ptr<AutotunerDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.move< std::unique_ptr<Expr> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        value.move< std::unique_ptr<ForLoop> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        value.move< std::unique_ptr<Mapping> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        value.move< std::unique_ptr<ParamDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_program: // program
        value.move< std::unique_ptr<Program> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        value.move< std::unique_ptr<SpecDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.move< std::unique_ptr<StateDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_trans_target: // trans_target
        value.move< std::unique_ptr<TransitionTarget> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        value.move< std::vector<Assignment> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_autotuners: // autotuners
        value.move< std::vector<AutotunerDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_loop_list: // loop_list
        value.move< std::vector<ForLoop> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        value.move< std::vector<Mapping> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        value.move< std::vector<ParamDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        value.move< std::vector<SpecDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        value.move< std::vector<StateDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        value.move< std::vector<Transition> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        value.move< std::vector<std::string> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        value.move< std::vector<std::unique_ptr<Expr>> > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

  }

  // by_kind.
  Parser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  Parser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  Parser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  Parser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  void
  Parser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  Parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  Parser::symbol_kind_type
  Parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  Parser::symbol_kind_type
  Parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


#line 4 "./compiler/src/parser.y"
} } // falcon::atc
#line 3056 "parser.tab.cc"






// Unqualified %code blocks.
#line 24 "./compiler/src/parser.y"

  #include <iostream>
  
  namespace falcon::atc {
    // Correct signature for api.token.constructor - returns symbol_type directly
    Parser::symbol_type yylex();
  }
  
  std::unique_ptr<falcon::atc::Program> program_root;

#line 3075 "parser.tab.cc"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif



// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 4 "./compiler/src/parser.y"
namespace falcon { namespace atc {
#line 3149 "parser.tab.cc"

  /// Build a parser object.
  Parser::Parser ()
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      yy_lac_established_ (false)
  {}

  Parser::~Parser ()
  {}

  Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
  Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  Parser::symbol_kind_type
  Parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  Parser::stack_symbol_type::stack_symbol_type ()
  {}

  Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_type_spec: // type_spec
        value.YY_MOVE_OR_COPY< ParamType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.YY_MOVE_OR_COPY< std::unique_ptr<Expr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<ForLoop> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<Mapping> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_program: // program
        value.YY_MOVE_OR_COPY< std::unique_ptr<Program> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<SpecDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_trans_target: // trans_target
        value.YY_MOVE_OR_COPY< std::unique_ptr<TransitionTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        value.YY_MOVE_OR_COPY< std::vector<Assignment> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuners: // autotuners
        value.YY_MOVE_OR_COPY< std::vector<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_list: // loop_list
        value.YY_MOVE_OR_COPY< std::vector<ForLoop> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        value.YY_MOVE_OR_COPY< std::vector<Mapping> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        value.YY_MOVE_OR_COPY< std::vector<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        value.YY_MOVE_OR_COPY< std::vector<SpecDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        value.YY_MOVE_OR_COPY< std::vector<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        value.YY_MOVE_OR_COPY< std::vector<Transition> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        value.YY_MOVE_OR_COPY< std::vector<std::string> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        value.YY_MOVE_OR_COPY< std::vector<std::unique_ptr<Expr>> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s)
  {
    switch (that.kind ())
    {
      case symbol_kind::S_type_spec: // type_spec
        value.move< ParamType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.move< std::unique_ptr<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.move< std::unique_ptr<Expr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        value.move< std::unique_ptr<ForLoop> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        value.move< std::unique_ptr<Mapping> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        value.move< std::unique_ptr<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_program: // program
        value.move< std::unique_ptr<Program> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        value.move< std::unique_ptr<SpecDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.move< std::unique_ptr<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_trans_target: // trans_target
        value.move< std::unique_ptr<TransitionTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        value.move< std::vector<Assignment> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuners: // autotuners
        value.move< std::vector<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_list: // loop_list
        value.move< std::vector<ForLoop> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        value.move< std::vector<Mapping> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        value.move< std::vector<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        value.move< std::vector<SpecDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        value.move< std::vector<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        value.move< std::vector<Transition> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        value.move< std::vector<std::string> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        value.move< std::vector<std::unique_ptr<Expr>> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_type_spec: // type_spec
        value.copy< ParamType > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.copy< std::unique_ptr<AutotunerDecl> > (that.value);
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.copy< std::unique_ptr<Expr> > (that.value);
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        value.copy< std::unique_ptr<ForLoop> > (that.value);
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        value.copy< std::unique_ptr<Mapping> > (that.value);
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        value.copy< std::unique_ptr<ParamDecl> > (that.value);
        break;

      case symbol_kind::S_program: // program
        value.copy< std::unique_ptr<Program> > (that.value);
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        value.copy< std::unique_ptr<SpecDecl> > (that.value);
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.copy< std::unique_ptr<StateDecl> > (that.value);
        break;

      case symbol_kind::S_trans_target: // trans_target
        value.copy< std::unique_ptr<TransitionTarget> > (that.value);
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        value.copy< std::vector<Assignment> > (that.value);
        break;

      case symbol_kind::S_autotuners: // autotuners
        value.copy< std::vector<AutotunerDecl> > (that.value);
        break;

      case symbol_kind::S_loop_list: // loop_list
        value.copy< std::vector<ForLoop> > (that.value);
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        value.copy< std::vector<Mapping> > (that.value);
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        value.copy< std::vector<ParamDecl> > (that.value);
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        value.copy< std::vector<SpecDecl> > (that.value);
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        value.copy< std::vector<StateDecl> > (that.value);
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        value.copy< std::vector<Transition> > (that.value);
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        value.copy< std::vector<std::string> > (that.value);
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        value.copy< std::vector<std::unique_ptr<Expr>> > (that.value);
        break;

      default:
        break;
    }

    return *this;
  }

  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_type_spec: // type_spec
        value.move< ParamType > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.move< std::unique_ptr<AutotunerDecl> > (that.value);
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.move< std::unique_ptr<Expr> > (that.value);
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        value.move< std::unique_ptr<ForLoop> > (that.value);
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        value.move< std::unique_ptr<Mapping> > (that.value);
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        value.move< std::unique_ptr<ParamDecl> > (that.value);
        break;

      case symbol_kind::S_program: // program
        value.move< std::unique_ptr<Program> > (that.value);
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        value.move< std::unique_ptr<SpecDecl> > (that.value);
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.move< std::unique_ptr<StateDecl> > (that.value);
        break;

      case symbol_kind::S_trans_target: // trans_target
        value.move< std::unique_ptr<TransitionTarget> > (that.value);
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        value.move< std::vector<Assignment> > (that.value);
        break;

      case symbol_kind::S_autotuners: // autotuners
        value.move< std::vector<AutotunerDecl> > (that.value);
        break;

      case symbol_kind::S_loop_list: // loop_list
        value.move< std::vector<ForLoop> > (that.value);
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        value.move< std::vector<Mapping> > (that.value);
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        value.move< std::vector<ParamDecl> > (that.value);
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        value.move< std::vector<SpecDecl> > (that.value);
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        value.move< std::vector<StateDecl> > (that.value);
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        value.move< std::vector<Transition> > (that.value);
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        value.move< std::vector<std::string> > (that.value);
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        value.move< std::vector<std::unique_ptr<Expr>> > (that.value);
        break;

      default:
        break;
    }

    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  Parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " (";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  Parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  Parser::state_type
  Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  Parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  Parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::operator() ()
  {
    return parse ();
  }

  int
  Parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The return value of parse ().
    int yyresult;

    // Discard the LAC context in case there still is one left from a
    // previous invocation.
    yy_lac_discard_ ("init");

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex ());
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        if (!yy_lac_establish_ (yyla.kind ()))
          goto yyerrlab;

        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    yy_lac_discard_ ("shift");
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_type_spec: // type_spec
        yylhs.value.emplace< ParamType > ();
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_clause: // entry_clause
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        yylhs.value.emplace< std::unique_ptr<AutotunerDecl> > ();
        break;

      case symbol_kind::S_measurement_opt: // measurement_opt
      case symbol_kind::S_expr: // expr
      case symbol_kind::S_primary_expr: // primary_expr
        yylhs.value.emplace< std::unique_ptr<Expr> > ();
        break;

      case symbol_kind::S_loop_decl: // loop_decl
        yylhs.value.emplace< std::unique_ptr<ForLoop> > ();
        break;

      case symbol_kind::S_mapping_decl: // mapping_decl
        yylhs.value.emplace< std::unique_ptr<Mapping> > ();
        break;

      case symbol_kind::S_sig_param_decl: // sig_param_decl
      case symbol_kind::S_param_decl: // param_decl
        yylhs.value.emplace< std::unique_ptr<ParamDecl> > ();
        break;

      case symbol_kind::S_program: // program
        yylhs.value.emplace< std::unique_ptr<Program> > ();
        break;

      case symbol_kind::S_spec_decl: // spec_decl
        yylhs.value.emplace< std::unique_ptr<SpecDecl> > ();
        break;

      case symbol_kind::S_state_decl: // state_decl
        yylhs.value.emplace< std::unique_ptr<StateDecl> > ();
        break;

      case symbol_kind::S_trans_target: // trans_target
        yylhs.value.emplace< std::unique_ptr<TransitionTarget> > ();
        break;

      case symbol_kind::S_assignment_list: // assignment_list
        yylhs.value.emplace< std::vector<Assignment> > ();
        break;

      case symbol_kind::S_autotuners: // autotuners
        yylhs.value.emplace< std::vector<AutotunerDecl> > ();
        break;

      case symbol_kind::S_loop_list: // loop_list
        yylhs.value.emplace< std::vector<ForLoop> > ();
        break;

      case symbol_kind::S_mappings: // mappings
      case symbol_kind::S_mapping_list: // mapping_list
        yylhs.value.emplace< std::vector<Mapping> > ();
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_sig_param_list: // sig_param_list
      case symbol_kind::S_params_block: // params_block
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_state_params: // state_params
      case symbol_kind::S_state_temps: // state_temps
        yylhs.value.emplace< std::vector<ParamDecl> > ();
        break;

      case symbol_kind::S_spec_inputs: // spec_inputs
      case symbol_kind::S_spec_outputs: // spec_outputs
      case symbol_kind::S_spec_list: // spec_list
        yylhs.value.emplace< std::vector<SpecDecl> > ();
        break;

      case symbol_kind::S_loop_states: // loop_states
      case symbol_kind::S_states: // states
      case symbol_kind::S_state_list: // state_list
        yylhs.value.emplace< std::vector<StateDecl> > ();
        break;

      case symbol_kind::S_transition_list: // transition_list
      case symbol_kind::S_transition_decl: // transition_decl
      case symbol_kind::S_simple_transition: // simple_transition
        yylhs.value.emplace< std::vector<Transition> > ();
        break;

      case symbol_kind::S_generic_params: // generic_params
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_separated_idents: // separated_idents
      case symbol_kind::S_separated_strings: // separated_strings
        yylhs.value.emplace< std::vector<std::string> > ();
        break;

      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_nonempty_arg_list: // nonempty_arg_list
        yylhs.value.emplace< std::vector<std::unique_ptr<Expr>> > ();
        break;

      default:
        break;
    }



      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // program: autotuners
#line 83 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Program> > () = std::make_unique<Program>();
        yylhs.value.as < std::unique_ptr<Program> > ()->autotuners = std::move(yystack_[0].value.as < std::vector<AutotunerDecl> > ());
        program_root = std::move(yylhs.value.as < std::unique_ptr<Program> > ());
      }
#line 4085 "parser.tab.cc"
    break;

  case 3: // autotuners: autotuner_decl
#line 92 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<AutotunerDecl> > () = std::vector<AutotunerDecl>();
        yylhs.value.as < std::vector<AutotunerDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<AutotunerDecl> > ()));
      }
#line 4094 "parser.tab.cc"
    break;

  case 4: // autotuners: autotuners autotuner_decl
#line 97 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<AutotunerDecl> > () = std::move(yystack_[1].value.as < std::vector<AutotunerDecl> > ());
        yylhs.value.as < std::vector<AutotunerDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<AutotunerDecl> > ()));
      }
#line 4103 "parser.tab.cc"
    break;

  case 5: // autotuner_decl: AUTOTUNER IDENTIFIER input_params generic_params ARROW output_params LBRACE requires_clause spec_inputs spec_outputs params_block entry_clause loop_list states RBRACE
#line 118 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<AutotunerDecl> > () = std::make_unique<AutotunerDecl>(
          std::move(yystack_[13].value.as < std::string > ()),
          std::move(yystack_[12].value.as < std::vector<ParamDecl> > ()),
          std::move(yystack_[9].value.as < std::vector<ParamDecl> > ()),
          std::move(yystack_[11].value.as < std::vector<std::string> > ()),
          std::move(yystack_[6].value.as < std::vector<SpecDecl> > ()),
          std::move(yystack_[5].value.as < std::vector<SpecDecl> > ()),
          std::move(yystack_[7].value.as < std::vector<std::string> > ()),
          std::move(yystack_[4].value.as < std::vector<ParamDecl> > ()),
          std::move(yystack_[3].value.as < std::string > ()),
          std::move(yystack_[1].value.as < std::vector<StateDecl> > ()),
          std::move(yystack_[2].value.as < std::vector<ForLoop> > ())
        );
      }
#line 4123 "parser.tab.cc"
    break;

  case 6: // input_params: LPAREN sig_param_list RPAREN
#line 137 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[1].value.as < std::vector<ParamDecl> > ()); }
#line 4129 "parser.tab.cc"
    break;

  case 7: // input_params: %empty
#line 139 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>(); }
#line 4135 "parser.tab.cc"
    break;

  case 8: // output_params: LPAREN sig_param_list RPAREN
#line 144 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[1].value.as < std::vector<ParamDecl> > ()); }
#line 4141 "parser.tab.cc"
    break;

  case 9: // output_params: %empty
#line 146 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>(); }
#line 4147 "parser.tab.cc"
    break;

  case 10: // sig_param_list: sig_param_decl
#line 151 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>();
        yylhs.value.as < std::vector<ParamDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<ParamDecl> > ()));
      }
#line 4156 "parser.tab.cc"
    break;

  case 11: // sig_param_list: sig_param_list COMMA sig_param_decl
#line 156 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[2].value.as < std::vector<ParamDecl> > ());
        yylhs.value.as < std::vector<ParamDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<ParamDecl> > ()));
      }
#line 4165 "parser.tab.cc"
    break;

  case 12: // sig_param_list: %empty
#line 161 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>(); }
#line 4171 "parser.tab.cc"
    break;

  case 13: // sig_param_decl: type_spec IDENTIFIER
#line 166 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<ParamDecl> > () = std::make_unique<ParamDecl>();
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->name = std::move(yystack_[0].value.as < std::string > ());
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->type = yystack_[1].value.as < ParamType > ();
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->default_value = nullptr;
      }
#line 4182 "parser.tab.cc"
    break;

  case 14: // generic_params: LBRACKET separated_idents RBRACKET
#line 176 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[1].value.as < std::vector<std::string> > ()); }
#line 4188 "parser.tab.cc"
    break;

  case 15: // generic_params: %empty
#line 178 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>(); }
#line 4194 "parser.tab.cc"
    break;

  case 16: // spec_inputs: SPEC_INPUTS COLON LBRACKET spec_list RBRACKET
#line 183 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<SpecDecl> > () = std::move(yystack_[1].value.as < std::vector<SpecDecl> > ()); }
#line 4200 "parser.tab.cc"
    break;

  case 17: // spec_inputs: %empty
#line 185 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<SpecDecl> > () = std::vector<SpecDecl>(); }
#line 4206 "parser.tab.cc"
    break;

  case 18: // spec_outputs: SPEC_OUTPUTS COLON LBRACKET spec_list RBRACKET
#line 190 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<SpecDecl> > () = std::move(yystack_[1].value.as < std::vector<SpecDecl> > ()); }
#line 4212 "parser.tab.cc"
    break;

  case 19: // spec_outputs: %empty
#line 192 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<SpecDecl> > () = std::vector<SpecDecl>(); }
#line 4218 "parser.tab.cc"
    break;

  case 20: // spec_list: spec_decl
#line 197 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<SpecDecl> > () = std::vector<SpecDecl>();
        yylhs.value.as < std::vector<SpecDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<SpecDecl> > ()));
      }
#line 4227 "parser.tab.cc"
    break;

  case 21: // spec_list: spec_list spec_decl
#line 202 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<SpecDecl> > () = std::move(yystack_[1].value.as < std::vector<SpecDecl> > ());
        yylhs.value.as < std::vector<SpecDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<SpecDecl> > ()));
      }
#line 4236 "parser.tab.cc"
    break;

  case 22: // spec_list: %empty
#line 207 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<SpecDecl> > () = std::vector<SpecDecl>(); }
#line 4242 "parser.tab.cc"
    break;

  case 23: // spec_decl: type_spec IDENTIFIER LBRACKET IDENTIFIER RBRACKET
#line 212 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<SpecDecl> > () = std::make_unique<SpecDecl>(
          yystack_[4].value.as < ParamType > (), 
          std::move(yystack_[3].value.as < std::string > ()), 
          std::move(yystack_[1].value.as < std::string > ())
        );
      }
#line 4254 "parser.tab.cc"
    break;

  case 24: // spec_decl: type_spec IDENTIFIER
#line 220 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<SpecDecl> > () = std::make_unique<SpecDecl>(
          yystack_[1].value.as < ParamType > (), 
          std::move(yystack_[0].value.as < std::string > ()), 
          ""
        );
      }
#line 4266 "parser.tab.cc"
    break;

  case 25: // requires_clause: REQUIRES COLON LBRACKET separated_strings RBRACKET SEMICOLON
#line 231 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[2].value.as < std::vector<std::string> > ()); }
#line 4272 "parser.tab.cc"
    break;

  case 26: // requires_clause: %empty
#line 233 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>(); }
#line 4278 "parser.tab.cc"
    break;

  case 27: // separated_idents: IDENTIFIER
#line 238 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>();
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4287 "parser.tab.cc"
    break;

  case 28: // separated_idents: separated_idents COMMA IDENTIFIER
#line 243 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[2].value.as < std::vector<std::string> > ());
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4296 "parser.tab.cc"
    break;

  case 29: // separated_strings: STRING
#line 251 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>();
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4305 "parser.tab.cc"
    break;

  case 30: // separated_strings: IDENTIFIER
#line 256 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>();
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4314 "parser.tab.cc"
    break;

  case 31: // separated_strings: separated_strings COMMA STRING
#line 261 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[2].value.as < std::vector<std::string> > ());
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4323 "parser.tab.cc"
    break;

  case 32: // separated_strings: separated_strings COMMA IDENTIFIER
#line 266 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[2].value.as < std::vector<std::string> > ());
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4332 "parser.tab.cc"
    break;

  case 33: // params_block: PARAMS LBRACE param_list RBRACE
#line 274 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[1].value.as < std::vector<ParamDecl> > ()); }
#line 4338 "parser.tab.cc"
    break;

  case 34: // params_block: %empty
#line 276 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>(); }
#line 4344 "parser.tab.cc"
    break;

  case 35: // param_list: param_decl
#line 281 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>();
        yylhs.value.as < std::vector<ParamDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<ParamDecl> > ()));
      }
#line 4353 "parser.tab.cc"
    break;

  case 36: // param_list: param_list param_decl
#line 286 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[1].value.as < std::vector<ParamDecl> > ());
        yylhs.value.as < std::vector<ParamDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<ParamDecl> > ()));
      }
#line 4362 "parser.tab.cc"
    break;

  case 37: // param_decl: type_spec IDENTIFIER SEMICOLON
#line 294 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<ParamDecl> > () = std::make_unique<ParamDecl>();
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->name = std::move(yystack_[1].value.as < std::string > ());
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->type = yystack_[2].value.as < ParamType > ();
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->default_value = nullptr;
      }
#line 4373 "parser.tab.cc"
    break;

  case 38: // param_decl: type_spec IDENTIFIER ASSIGN expr SEMICOLON
#line 301 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<ParamDecl> > () = std::make_unique<ParamDecl>();
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->name = std::move(yystack_[3].value.as < std::string > ());
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->type = yystack_[4].value.as < ParamType > ();
        yylhs.value.as < std::unique_ptr<ParamDecl> > ()->default_value = nullptr;
      }
#line 4384 "parser.tab.cc"
    break;

  case 39: // type_spec: FLOAT_KW
#line 310 "./compiler/src/parser.y"
                  { yylhs.value.as < ParamType > () = ParamType::Float; }
#line 4390 "parser.tab.cc"
    break;

  case 40: // type_spec: INT_KW
#line 311 "./compiler/src/parser.y"
                  { yylhs.value.as < ParamType > () = ParamType::Int; }
#line 4396 "parser.tab.cc"
    break;

  case 41: // type_spec: BOOL_KW
#line 312 "./compiler/src/parser.y"
                  { yylhs.value.as < ParamType > () = ParamType::Bool; }
#line 4402 "parser.tab.cc"
    break;

  case 42: // type_spec: STRING_KW
#line 313 "./compiler/src/parser.y"
                  { yylhs.value.as < ParamType > () = ParamType::String; }
#line 4408 "parser.tab.cc"
    break;

  case 43: // type_spec: QUANTITY_KW
#line 314 "./compiler/src/parser.y"
                  { yylhs.value.as < ParamType > () = ParamType::Quantity; }
#line 4414 "parser.tab.cc"
    break;

  case 44: // type_spec: CONFIG_KW
#line 315 "./compiler/src/parser.y"
                  { yylhs.value.as < ParamType > () = ParamType::Config; }
#line 4420 "parser.tab.cc"
    break;

  case 45: // type_spec: GROUP_KW
#line 316 "./compiler/src/parser.y"
                  { yylhs.value.as < ParamType > () = ParamType::Group; }
#line 4426 "parser.tab.cc"
    break;

  case 46: // type_spec: CONNECTION_KW
#line 317 "./compiler/src/parser.y"
                    { yylhs.value.as < ParamType > () = ParamType::Connection; }
#line 4432 "parser.tab.cc"
    break;

  case 47: // entry_clause: START ARROW IDENTIFIER SEMICOLON
#line 322 "./compiler/src/parser.y"
      { yylhs.value.as < std::string > () = std::move(yystack_[1].value.as < std::string > ()); }
#line 4438 "parser.tab.cc"
    break;

  case 48: // loop_list: loop_decl
#line 327 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<ForLoop> > () = std::vector<ForLoop>();
        yylhs.value.as < std::vector<ForLoop> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<ForLoop> > ()));
      }
#line 4447 "parser.tab.cc"
    break;

  case 49: // loop_list: loop_list loop_decl
#line 332 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<ForLoop> > () = std::move(yystack_[1].value.as < std::vector<ForLoop> > ());
        yylhs.value.as < std::vector<ForLoop> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<ForLoop> > ()));
      }
#line 4456 "parser.tab.cc"
    break;

  case 50: // loop_list: %empty
#line 337 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ForLoop> > () = std::vector<ForLoop>(); }
#line 4462 "parser.tab.cc"
    break;

  case 51: // loop_decl: FOR IDENTIFIER IN expr LBRACE loop_states RBRACE
#line 342 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<ForLoop> > () = std::make_unique<ForLoop>(
          std::move(yystack_[5].value.as < std::string > ()),
          std::move(yystack_[3].value.as < std::unique_ptr<Expr> > ()),
          std::move(yystack_[1].value.as < std::vector<StateDecl> > ())
        );
      }
#line 4474 "parser.tab.cc"
    break;

  case 52: // loop_states: state_decl
#line 353 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<StateDecl> > () = std::vector<StateDecl>();
        yylhs.value.as < std::vector<StateDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<StateDecl> > ()));
      }
#line 4483 "parser.tab.cc"
    break;

  case 53: // loop_states: loop_states state_decl
#line 358 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<StateDecl> > () = std::move(yystack_[1].value.as < std::vector<StateDecl> > ());
        yylhs.value.as < std::vector<StateDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<StateDecl> > ()));
      }
#line 4492 "parser.tab.cc"
    break;

  case 54: // states: state_list
#line 366 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<StateDecl> > () = std::move(yystack_[0].value.as < std::vector<StateDecl> > ()); }
#line 4498 "parser.tab.cc"
    break;

  case 55: // state_list: state_decl
#line 371 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<StateDecl> > () = std::vector<StateDecl>();
        yylhs.value.as < std::vector<StateDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<StateDecl> > ()));
      }
#line 4507 "parser.tab.cc"
    break;

  case 56: // state_list: state_list state_decl
#line 376 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<StateDecl> > () = std::move(yystack_[1].value.as < std::vector<StateDecl> > ());
        yylhs.value.as < std::vector<StateDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<StateDecl> > ()));
      }
#line 4516 "parser.tab.cc"
    break;

  case 57: // state_list: %empty
#line 381 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<StateDecl> > () = std::vector<StateDecl>(); }
#line 4522 "parser.tab.cc"
    break;

  case 58: // state_decl: STATE IDENTIFIER LBRACKET IDENTIFIER RBRACKET LBRACE state_params state_temps measurement_opt transition_list RBRACE
#line 393 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<StateDecl> > () = std::make_unique<StateDecl>(
          std::move(yystack_[9].value.as < std::string > ()),
          std::move(yystack_[7].value.as < std::string > ()),
          std::move(yystack_[4].value.as < std::vector<ParamDecl> > ()),
          std::move(yystack_[3].value.as < std::vector<ParamDecl> > ()),
          std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()),
          false,
          std::move(yystack_[1].value.as < std::vector<Transition> > ())
        );
      }
#line 4538 "parser.tab.cc"
    break;

  case 59: // state_decl: STATE IDENTIFIER LBRACE state_params state_temps measurement_opt transition_list RBRACE
#line 411 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<StateDecl> > () = std::make_unique<StateDecl>(
          std::move(yystack_[6].value.as < std::string > ()),
          "",
          std::move(yystack_[4].value.as < std::vector<ParamDecl> > ()),
          std::move(yystack_[3].value.as < std::vector<ParamDecl> > ()),
          std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()),
          false,
          std::move(yystack_[1].value.as < std::vector<Transition> > ())
        );
      }
#line 4554 "parser.tab.cc"
    break;

  case 60: // state_params: PARAMS LBRACE param_list RBRACE
#line 426 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[1].value.as < std::vector<ParamDecl> > ()); }
#line 4560 "parser.tab.cc"
    break;

  case 61: // state_params: %empty
#line 428 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>(); }
#line 4566 "parser.tab.cc"
    break;

  case 62: // state_temps: TEMP LBRACE param_list RBRACE
#line 433 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::move(yystack_[1].value.as < std::vector<ParamDecl> > ()); }
#line 4572 "parser.tab.cc"
    break;

  case 63: // state_temps: %empty
#line 435 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<ParamDecl> > () = std::vector<ParamDecl>(); }
#line 4578 "parser.tab.cc"
    break;

  case 64: // measurement_opt: MEASUREMENT COLON IDENTIFIER LPAREN arg_list RPAREN SEMICOLON
#line 440 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<CallExpr>(
          std::move(yystack_[4].value.as < std::string > ()),
          std::move(yystack_[2].value.as < std::vector<std::unique_ptr<Expr>> > ())
        );
      }
#line 4589 "parser.tab.cc"
    break;

  case 65: // measurement_opt: RUN COLON IDENTIFIER LPAREN arg_list RPAREN SEMICOLON
#line 447 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<CallExpr>(
          std::move(yystack_[4].value.as < std::string > ()),
          std::move(yystack_[2].value.as < std::vector<std::unique_ptr<Expr>> > ())
        );
      }
#line 4600 "parser.tab.cc"
    break;

  case 66: // measurement_opt: %empty
#line 454 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = nullptr; }
#line 4606 "parser.tab.cc"
    break;

  case 67: // arg_list: nonempty_arg_list
#line 460 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::unique_ptr<Expr>> > () = std::move(yystack_[0].value.as < std::vector<std::unique_ptr<Expr>> > ()); }
#line 4612 "parser.tab.cc"
    break;

  case 68: // arg_list: %empty
#line 462 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::unique_ptr<Expr>> > () = std::vector<std::unique_ptr<Expr>>(); }
#line 4618 "parser.tab.cc"
    break;

  case 69: // nonempty_arg_list: expr
#line 467 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > () = std::vector<std::unique_ptr<Expr>>();
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4627 "parser.tab.cc"
    break;

  case 70: // nonempty_arg_list: nonempty_arg_list COMMA expr
#line 472 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > () = std::move(yystack_[2].value.as < std::vector<std::unique_ptr<Expr>> > ());
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4636 "parser.tab.cc"
    break;

  case 71: // transition_list: transition_decl
#line 480 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<Transition> > () = std::move(yystack_[0].value.as < std::vector<Transition> > ()); }
#line 4642 "parser.tab.cc"
    break;

  case 72: // transition_list: transition_list transition_decl
#line 482 "./compiler/src/parser.y"
      { 
        auto tmp = std::move(yystack_[1].value.as < std::vector<Transition> > ());
        // Manual move to avoid automove issues
        for (auto& t : yystack_[0].value.as < std::vector<Transition> > ()) {
          tmp.push_back(std::move(t));
        }
        yylhs.value.as < std::vector<Transition> > () = std::move(tmp);
      }
#line 4655 "parser.tab.cc"
    break;

  case 73: // transition_list: %empty
#line 491 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<Transition> > () = std::vector<Transition>(); }
#line 4661 "parser.tab.cc"
    break;

  case 74: // transition_decl: simple_transition
#line 496 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<Transition> > () = std::move(yystack_[0].value.as < std::vector<Transition> > ()); }
#line 4667 "parser.tab.cc"
    break;

  case 75: // transition_decl: IF LPAREN expr RPAREN transition_decl
#line 498 "./compiler/src/parser.y"
      { 
        // Store condition, then iterate
        auto cond_ptr = std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ());
        auto tmp = std::move(yystack_[0].value.as < std::vector<Transition> > ());
        for (auto &t : tmp) {
          if (t.condition) {
            t.condition = std::make_unique<BinaryExpr>(
              "&&", 
              cond_ptr->clone(), 
              std::move(t.condition)
            );
          } else {
            t.condition = cond_ptr->clone();
          }
        }
        yylhs.value.as < std::vector<Transition> > () = std::move(tmp);
      }
#line 4689 "parser.tab.cc"
    break;

  case 76: // transition_decl: IF LPAREN expr RPAREN simple_transition ELSE transition_decl
#line 516 "./compiler/src/parser.y"
      { 
        // Store all values first to avoid multiple accesses
        auto cond_ptr = std::move(yystack_[4].value.as < std::unique_ptr<Expr> > ());
        auto tmp_then = std::move(yystack_[2].value.as < std::vector<Transition> > ());
        auto tmp_else = std::move(yystack_[0].value.as < std::vector<Transition> > ());
        
        // Process then branch
        for (auto &t : tmp_then) {
          if (t.condition) {
            t.condition = std::make_unique<BinaryExpr>(
              "&&", 
              cond_ptr->clone(), 
              std::move(t.condition)
            );
          } else {
            t.condition = cond_ptr->clone();
          }
        }
        
        // Process else branch
        auto inv_cond = std::make_unique<UnaryExpr>("!", cond_ptr->clone());
        for (auto &t : tmp_else) {
          if (t.condition) {
            t.condition = std::make_unique<BinaryExpr>(
              "&&", 
              inv_cond->clone(), 
              std::move(t.condition)
            );
          } else {
            t.condition = inv_cond->clone();
          }
        }
        
        // Combine
        for (auto& t : tmp_else) {
          tmp_then.push_back(std::move(t));
        }
        yylhs.value.as < std::vector<Transition> > () = std::move(tmp_then);
      }
#line 4733 "parser.tab.cc"
    break;

  case 77: // simple_transition: SUCCESS SEMICOLON
#line 559 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Transition> > () = std::vector<Transition>();
        Transition t;
        t.is_success = true;
        yylhs.value.as < std::vector<Transition> > ().push_back(std::move(t));
      }
#line 4744 "parser.tab.cc"
    break;

  case 78: // simple_transition: FAIL STRING SEMICOLON
#line 566 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Transition> > () = std::vector<Transition>();
        Transition t;
        t.is_fail = true;
        t.error_message = std::move(yystack_[1].value.as < std::string > ());
        yylhs.value.as < std::vector<Transition> > ().push_back(std::move(t));
      }
#line 4756 "parser.tab.cc"
    break;

  case 79: // simple_transition: FAIL SEMICOLON
#line 574 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Transition> > () = std::vector<Transition>();
        Transition t;
        t.is_fail = true;
        yylhs.value.as < std::vector<Transition> > ().push_back(std::move(t));
      }
#line 4767 "parser.tab.cc"
    break;

  case 80: // simple_transition: TERMINAL SEMICOLON
#line 581 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Transition> > () = std::vector<Transition>();
        Transition t;
        t.target.state_name = "_TERMINAL_";
        yylhs.value.as < std::vector<Transition> > ().push_back(std::move(t));
      }
#line 4778 "parser.tab.cc"
    break;

  case 81: // simple_transition: ARROW trans_target SEMICOLON
#line 588 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Transition> > () = std::vector<Transition>();
        yylhs.value.as < std::vector<Transition> > ().emplace_back(
          nullptr, 
          std::move(*yystack_[1].value.as < std::unique_ptr<TransitionTarget> > ()), 
          std::vector<Assignment>()
        );
      }
#line 4791 "parser.tab.cc"
    break;

  case 82: // simple_transition: LBRACE assignment_list RBRACE ARROW trans_target SEMICOLON
#line 597 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Transition> > () = std::vector<Transition>();
        yylhs.value.as < std::vector<Transition> > ().emplace_back(
          nullptr, 
          std::move(*yystack_[1].value.as < std::unique_ptr<TransitionTarget> > ()), 
          std::move(yystack_[4].value.as < std::vector<Assignment> > ())
        );
      }
#line 4804 "parser.tab.cc"
    break;

  case 83: // simple_transition: separated_idents ASSIGN expr SEMICOLON
#line 606 "./compiler/src/parser.y"
      { 
        // Single assignment without semicolon as transition
        yylhs.value.as < std::vector<Transition> > () = std::vector<Transition>();
        std::vector<Assignment> asgns;
        asgns.emplace_back(std::move(yystack_[3].value.as < std::vector<std::string> > ()), std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        yylhs.value.as < std::vector<Transition> > ().emplace_back(
          nullptr, 
          TransitionTarget("", "", nullptr, {}),
          std::move(asgns)
        );
      }
#line 4820 "parser.tab.cc"
    break;

  case 84: // simple_transition: LBRACE transition_list RBRACE
#line 618 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<Transition> > () = std::move(yystack_[1].value.as < std::vector<Transition> > ()); }
#line 4826 "parser.tab.cc"
    break;

  case 85: // assignment_list: separated_idents ASSIGN expr SEMICOLON
#line 623 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Assignment> > () = std::vector<Assignment>();
        yylhs.value.as < std::vector<Assignment> > ().emplace_back(std::move(yystack_[3].value.as < std::vector<std::string> > ()), std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4835 "parser.tab.cc"
    break;

  case 86: // assignment_list: assignment_list separated_idents ASSIGN expr SEMICOLON
#line 628 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Assignment> > () = std::move(yystack_[4].value.as < std::vector<Assignment> > ());
        yylhs.value.as < std::vector<Assignment> > ().emplace_back(std::move(yystack_[3].value.as < std::vector<std::string> > ()), std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4844 "parser.tab.cc"
    break;

  case 87: // trans_target: IDENTIFIER LBRACKET expr RBRACKET mappings
#line 637 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<TransitionTarget> > () = std::make_unique<TransitionTarget>(
          "",
          std::move(yystack_[4].value.as < std::string > ()),
          std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()),
          std::move(yystack_[0].value.as < std::vector<Mapping> > ())
        );
      }
#line 4857 "parser.tab.cc"
    break;

  case 88: // trans_target: IDENTIFIER mappings
#line 646 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<TransitionTarget> > () = std::make_unique<TransitionTarget>(
          "",
          std::move(yystack_[1].value.as < std::string > ()),
          nullptr,
          std::move(yystack_[0].value.as < std::vector<Mapping> > ())
        );
      }
#line 4870 "parser.tab.cc"
    break;

  case 89: // trans_target: IDENTIFIER DOUBLECOLON IDENTIFIER LBRACKET expr RBRACKET mappings
#line 655 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<TransitionTarget> > () = std::make_unique<TransitionTarget>(
          std::move(yystack_[6].value.as < std::string > ()),
          std::move(yystack_[4].value.as < std::string > ()),
          std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()),
          std::move(yystack_[0].value.as < std::vector<Mapping> > ())
        );
      }
#line 4883 "parser.tab.cc"
    break;

  case 90: // trans_target: IDENTIFIER DOUBLECOLON IDENTIFIER mappings
#line 664 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<TransitionTarget> > () = std::make_unique<TransitionTarget>(
          std::move(yystack_[3].value.as < std::string > ()),
          std::move(yystack_[1].value.as < std::string > ()),
          nullptr,
          std::move(yystack_[0].value.as < std::vector<Mapping> > ())
        );
      }
#line 4896 "parser.tab.cc"
    break;

  case 91: // mappings: LBRACKET mapping_list RBRACKET
#line 676 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<Mapping> > () = std::move(yystack_[1].value.as < std::vector<Mapping> > ()); }
#line 4902 "parser.tab.cc"
    break;

  case 92: // mappings: %empty
#line 678 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<Mapping> > () = std::vector<Mapping>(); }
#line 4908 "parser.tab.cc"
    break;

  case 93: // mapping_list: mapping_decl
#line 683 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Mapping> > () = std::vector<Mapping>();
        yylhs.value.as < std::vector<Mapping> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<Mapping> > ()));
      }
#line 4917 "parser.tab.cc"
    break;

  case 94: // mapping_list: mapping_list COMMA mapping_decl
#line 688 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<Mapping> > () = std::move(yystack_[2].value.as < std::vector<Mapping> > ());
        yylhs.value.as < std::vector<Mapping> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<Mapping> > ()));
      }
#line 4926 "parser.tab.cc"
    break;

  case 95: // mapping_decl: IDENTIFIER COLON IDENTIFIER
#line 696 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Mapping> > () = std::make_unique<Mapping>(
          std::move(yystack_[2].value.as < std::string > ()),
          std::move(yystack_[0].value.as < std::string > ())
        );
      }
#line 4937 "parser.tab.cc"
    break;

  case 96: // mapping_decl: IDENTIFIER
#line 703 "./compiler/src/parser.y"
      { 
        // Need to copy before move since we use it twice
        std::string name_copy = yystack_[0].value.as < std::string > ();
        yylhs.value.as < std::unique_ptr<Mapping> > () = std::make_unique<Mapping>(
          std::move(yystack_[0].value.as < std::string > ()),
          std::move(name_copy)
        );
      }
#line 4950 "parser.tab.cc"
    break;

  case 97: // expr: expr PLUS expr
#line 716 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("+", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4958 "parser.tab.cc"
    break;

  case 98: // expr: expr MINUS expr
#line 720 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("-", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4966 "parser.tab.cc"
    break;

  case 99: // expr: expr MUL expr
#line 724 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("*", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4974 "parser.tab.cc"
    break;

  case 100: // expr: expr DIV expr
#line 728 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("/", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4982 "parser.tab.cc"
    break;

  case 101: // expr: expr EQ expr
#line 732 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("==", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4990 "parser.tab.cc"
    break;

  case 102: // expr: expr NE expr
#line 736 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("!=", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 4998 "parser.tab.cc"
    break;

  case 103: // expr: expr LL expr
#line 740 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("<", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5006 "parser.tab.cc"
    break;

  case 104: // expr: expr GG expr
#line 744 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>(">", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5014 "parser.tab.cc"
    break;

  case 105: // expr: expr LE expr
#line 748 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("<=", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5022 "parser.tab.cc"
    break;

  case 106: // expr: expr GE expr
#line 752 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>(">=", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5030 "parser.tab.cc"
    break;

  case 107: // expr: expr AND expr
#line 756 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("&&", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5038 "parser.tab.cc"
    break;

  case 108: // expr: expr OR expr
#line 760 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("||", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5046 "parser.tab.cc"
    break;

  case 109: // expr: NOT expr
#line 764 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<UnaryExpr>("!", std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5054 "parser.tab.cc"
    break;

  case 110: // expr: MINUS expr
#line 768 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<UnaryExpr>("-", std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5062 "parser.tab.cc"
    break;

  case 111: // expr: expr DOT IDENTIFIER
#line 772 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<MemberExpr>(std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::string > ()));
      }
#line 5070 "parser.tab.cc"
    break;

  case 112: // expr: primary_expr
#line 776 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()); }
#line 5076 "parser.tab.cc"
    break;

  case 113: // primary_expr: INTEGER
#line 781 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<ConstExpr>(std::stoll(yystack_[0].value.as < std::string > ())); }
#line 5082 "parser.tab.cc"
    break;

  case 114: // primary_expr: DOUBLE
#line 783 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<ConstExpr>(std::stod(yystack_[0].value.as < std::string > ())); }
#line 5088 "parser.tab.cc"
    break;

  case 115: // primary_expr: STRING
#line 785 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<ConstExpr>(std::move(yystack_[0].value.as < std::string > ())); }
#line 5094 "parser.tab.cc"
    break;

  case 116: // primary_expr: TRUE
#line 787 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<ConstExpr>(true); }
#line 5100 "parser.tab.cc"
    break;

  case 117: // primary_expr: FALSE
#line 789 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<ConstExpr>(false); }
#line 5106 "parser.tab.cc"
    break;

  case 118: // primary_expr: IDENTIFIER
#line 791 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<VarExpr>(std::move(yystack_[0].value.as < std::string > ())); }
#line 5112 "parser.tab.cc"
    break;

  case 119: // primary_expr: CONFIG_VAR
#line 793 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<VarExpr>("config"); }
#line 5118 "parser.tab.cc"
    break;

  case 120: // primary_expr: NEXT IDENTIFIER
#line 795 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<VarExpr>("next " + yystack_[0].value.as < std::string > ()); }
#line 5124 "parser.tab.cc"
    break;

  case 121: // primary_expr: LPAREN expr RPAREN
#line 797 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<Expr> > () = std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()); }
#line 5130 "parser.tab.cc"
    break;

  case 122: // primary_expr: IDENTIFIER LPAREN arg_list RPAREN
#line 799 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<CallExpr>(std::move(yystack_[3].value.as < std::string > ()), std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Expr>> > ()));
      }
#line 5138 "parser.tab.cc"
    break;


#line 5142 "parser.tab.cc"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (YY_MOVE (msg));
      }


    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;


      // Shift the error token.
      yy_lac_discard_ ("error recovery");
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.what ());
  }

  const char *
  Parser::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "IDENTIFIER", "DOUBLE",
  "INTEGER", "STRING", "AUTOTUNER", "STATE", "PARAMS", "TEMP",
  "MEASUREMENT", "RUN", "START", "REQUIRES", "TERMINAL", "IF", "ELSE",
  "TRUE", "FALSE", "SUCCESS", "FAIL", "SPEC_INPUTS", "SPEC_OUTPUTS",
  "CONFIG_VAR", "NEXT", "FOR", "IN", "FLOAT_KW", "INT_KW", "BOOL_KW",
  "STRING_KW", "QUANTITY_KW", "CONFIG_KW", "GROUP_KW", "CONNECTION_KW",
  "ARROW", "DOUBLECOLON", "LBRACKET", "RBRACKET", "LBRACE", "RBRACE",
  "LPAREN", "RPAREN", "ASSIGN", "COMMA", "COLON", "SEMICOLON", "DOT",
  "PLUS", "MINUS", "MUL", "DIV", "EQ", "NE", "LL", "GG", "LE", "GE", "AND",
  "OR", "NOT", "UMINUS", "$accept", "program", "autotuners",
  "autotuner_decl", "input_params", "output_params", "sig_param_list",
  "sig_param_decl", "generic_params", "spec_inputs", "spec_outputs",
  "spec_list", "spec_decl", "requires_clause", "separated_idents",
  "separated_strings", "params_block", "param_list", "param_decl",
  "type_spec", "entry_clause", "loop_list", "loop_decl", "loop_states",
  "states", "state_list", "state_decl", "state_params", "state_temps",
  "measurement_opt", "arg_list", "nonempty_arg_list", "transition_list",
  "transition_decl", "simple_transition", "assignment_list",
  "trans_target", "mappings", "mapping_list", "mapping_decl", "expr",
  "primary_expr", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  // Parser::context.
  Parser::context::context (const Parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  Parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

#if YYDEBUG
    // Execute LAC once. We don't care if it is successful, we
    // only do it for the sake of debugging output.
    if (!yyparser_.yy_lac_established_)
      yyparser_.yy_lac_check_ (yyla_.kind ());
#endif

    for (int yyx = 0; yyx < YYNTOKENS; ++yyx)
      {
        symbol_kind_type yysym = YY_CAST (symbol_kind_type, yyx);
        if (yysym != symbol_kind::S_YYerror
            && yysym != symbol_kind::S_YYUNDEF
            && yyparser_.yy_lac_check_ (yysym))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
      }
    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }




  bool
  Parser::yy_lac_check_ (symbol_kind_type yytoken) const
  {
    // Logically, the yylac_stack's lifetime is confined to this function.
    // Clear it, to get rid of potential left-overs from previous call.
    yylac_stack_.clear ();
    // Reduce until we encounter a shift and thereby accept the token.
#if YYDEBUG
    YYCDEBUG << "LAC: checking lookahead " << symbol_name (yytoken) << ':';
#endif
    std::ptrdiff_t lac_top = 0;
    while (true)
      {
        state_type top_state = (yylac_stack_.empty ()
                                ? yystack_[lac_top].state
                                : yylac_stack_.back ());
        int yyrule = yypact_[+top_state];
        if (yy_pact_value_is_default_ (yyrule)
            || (yyrule += yytoken) < 0 || yylast_ < yyrule
            || yycheck_[yyrule] != yytoken)
          {
            // Use the default action.
            yyrule = yydefact_[+top_state];
            if (yyrule == 0)
              {
                YYCDEBUG << " Err\n";
                return false;
              }
          }
        else
          {
            // Use the action from yytable.
            yyrule = yytable_[yyrule];
            if (yy_table_value_is_error_ (yyrule))
              {
                YYCDEBUG << " Err\n";
                return false;
              }
            if (0 < yyrule)
              {
                YYCDEBUG << " S" << yyrule << '\n';
                return true;
              }
            yyrule = -yyrule;
          }
        // By now we know we have to simulate a reduce.
        YYCDEBUG << " R" << yyrule - 1;
        // Pop the corresponding number of values from the stack.
        {
          std::ptrdiff_t yylen = yyr2_[yyrule];
          // First pop from the LAC stack as many tokens as possible.
          std::ptrdiff_t lac_size = std::ptrdiff_t (yylac_stack_.size ());
          if (yylen < lac_size)
            {
              yylac_stack_.resize (std::size_t (lac_size - yylen));
              yylen = 0;
            }
          else if (lac_size)
            {
              yylac_stack_.clear ();
              yylen -= lac_size;
            }
          // Only afterwards look at the main stack.
          // We simulate popping elements by incrementing lac_top.
          lac_top += yylen;
        }
        // Keep top_state in sync with the updated stack.
        top_state = (yylac_stack_.empty ()
                     ? yystack_[lac_top].state
                     : yylac_stack_.back ());
        // Push the resulting state of the reduction.
        state_type state = yy_lr_goto_state_ (top_state, yyr1_[yyrule]);
        YYCDEBUG << " G" << int (state);
        yylac_stack_.push_back (state);
      }
  }

  // Establish the initial context if no initial context currently exists.
  bool
  Parser::yy_lac_establish_ (symbol_kind_type yytoken)
  {
    /* Establish the initial context for the current lookahead if no initial
       context is currently established.

       We define a context as a snapshot of the parser stacks.  We define
       the initial context for a lookahead as the context in which the
       parser initially examines that lookahead in order to select a
       syntactic action.  Thus, if the lookahead eventually proves
       syntactically unacceptable (possibly in a later context reached via a
       series of reductions), the initial context can be used to determine
       the exact set of tokens that would be syntactically acceptable in the
       lookahead's place.  Moreover, it is the context after which any
       further semantic actions would be erroneous because they would be
       determined by a syntactically unacceptable token.

       yy_lac_establish_ should be invoked when a reduction is about to be
       performed in an inconsistent state (which, for the purposes of LAC,
       includes consistent states that don't know they're consistent because
       their default reductions have been disabled).

       For parse.lac=full, the implementation of yy_lac_establish_ is as
       follows.  If no initial context is currently established for the
       current lookahead, then check if that lookahead can eventually be
       shifted if syntactic actions continue from the current context.  */
    if (yy_lac_established_)
      return true;
    else
      {
#if YYDEBUG
        YYCDEBUG << "LAC: initial context established for "
                 << symbol_name (yytoken) << '\n';
#endif
        yy_lac_established_ = true;
        return yy_lac_check_ (yytoken);
      }
  }

  // Discard any previous initial lookahead context.
  void
  Parser::yy_lac_discard_ (const char* event)
  {
   /* Discard any previous initial lookahead context because of Event,
      which may be a lookahead change or an invalidation of the currently
      established initial context for the current lookahead.

      The most common example of a lookahead change is a shift.  An example
      of both cases is syntax error recovery.  That is, a syntax error
      occurs when the lookahead is syntactically erroneous for the
      currently established initial context, so error recovery manipulates
      the parser stacks to try to find a new initial context in which the
      current lookahead is syntactically acceptable.  If it fails to find
      such a context, it discards the lookahead.  */
    if (yy_lac_established_)
      {
        YYCDEBUG << "LAC: initial context discarded due to "
                 << event << '\n';
        yy_lac_established_ = false;
      }
  }


  int
  Parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
         In the first two cases, it might appear that the current syntax
         error should have been detected in the previous state when
         yy_lac_check was invoked.  However, at that time, there might
         have been a different syntax error that discarded a different
         initial context during error recovery, leaving behind the
         current lookahead.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short Parser::yypact_ninf_ = -191;

  const signed char Parser::yytable_ninf_ = -97;

  const short
  Parser::yypact_[] =
  {
       7,    53,    65,     7,  -191,    29,  -191,  -191,   169,    40,
    -191,  -191,  -191,  -191,  -191,  -191,  -191,  -191,    73,  -191,
      89,   104,    63,  -191,   169,  -191,  -191,   -30,    68,  -191,
    -191,   110,   169,    82,  -191,    76,   113,  -191,    85,   116,
     103,   112,   136,    10,   131,   124,   162,  -191,  -191,   -16,
     169,   137,   139,   163,   133,    97,   370,  -191,   179,   169,
     169,   183,   194,  -191,  -191,  -191,  -191,  -191,   184,   382,
     303,  -191,   218,   220,   221,     0,  -191,   222,  -191,  -191,
    -191,    57,   193,   215,   238,  -191,   202,   250,  -191,   300,
      56,  -191,  -191,    56,    88,  -191,  -191,  -191,   217,  -191,
    -191,  -191,  -191,  -191,  -191,   257,    56,    56,    56,   228,
    -191,   178,   258,   332,    56,  -191,   196,   292,   292,  -191,
     339,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,   250,   304,   313,   344,   312,   311,   325,
    -191,  -191,    39,    39,   292,   292,   115,   115,   115,   115,
     115,   115,   374,   338,    12,  -191,   327,   169,   328,    16,
    -191,    56,  -191,  -191,   332,   317,   169,   323,   324,   141,
     325,   344,  -191,   331,   354,   368,   359,   365,   361,    -1,
     415,   141,    41,    48,  -191,  -191,    16,  -191,   377,   378,
    -191,    56,  -191,   386,  -191,    96,   387,    92,   109,     4,
      56,  -191,  -191,   141,    56,    56,   214,  -191,   432,    90,
    -191,  -191,    56,  -191,   400,    98,   242,   132,   394,   395,
     141,   401,    37,    13,  -191,   135,   256,   415,    56,  -191,
    -191,   393,   396,  -191,   424,    90,  -191,   439,  -191,   441,
     407,  -191,   399,   270,  -191,  -191,   141,   157,  -191,   402,
    -191,   441,  -191,  -191,  -191,  -191,   407,  -191
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,     0,     0,     2,     3,     7,     1,     4,    12,    15,
      39,    40,    41,    42,    43,    44,    45,    46,     0,    10,
       0,     0,     0,     6,     0,    13,    27,     0,     9,    11,
      14,     0,    12,     0,    28,     0,    26,     8,     0,    17,
       0,     0,    19,     0,     0,     0,    34,    30,    29,     0,
      22,     0,     0,     0,     0,     0,     0,    20,     0,    22,
       0,     0,    50,    25,    32,    31,    16,    21,    24,     0,
       0,    35,     0,     0,     0,    57,    48,     0,    18,    33,
      36,     0,     0,     0,     0,    49,     0,    54,    55,     0,
       0,    37,    47,     0,     0,     5,    56,    23,   118,   114,
     113,   115,   116,   117,   119,     0,     0,     0,     0,     0,
     112,     0,     0,    61,    68,   120,     0,   110,   109,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,    67,    69,
     121,   111,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,    52,     0,     0,     0,    66,
     122,     0,    51,    53,    61,     0,     0,     0,     0,    73,
      70,    63,    60,     0,     0,     0,     0,     0,     0,     0,
       0,    73,     0,     0,    71,    74,    66,    62,     0,     0,
      80,     0,    77,     0,    79,    92,     0,     0,     0,     0,
       0,    59,    72,    73,    68,    68,     0,    78,     0,     0,
      88,    81,     0,    84,     0,     0,     0,     0,     0,     0,
       0,    92,   118,     0,    93,     0,     0,     0,     0,    83,
      58,     0,     0,    75,    74,     0,    90,     0,    91,     0,
      92,    83,     0,     0,    64,    65,     0,     0,    95,    96,
      94,     0,    87,    82,    86,    76,    92,    89
  };

  const short
  Parser::yypgoto_[] =
  {
    -191,  -191,  -191,   444,  -191,  -191,   417,   426,  -191,  -191,
    -191,   392,   -45,  -191,   -21,  -191,  -191,  -145,   -68,    -2,
    -191,  -191,   379,  -191,  -191,  -191,   -84,   288,   282,   269,
     -50,  -191,  -126,  -173,   236,  -191,   230,  -190,  -191,   219,
     -89,  -191
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,     2,     3,     4,     9,    33,    18,    19,    22,    42,
      46,    56,    57,    39,   182,    49,    53,    70,    71,    72,
      62,    75,    76,   154,    86,    87,    88,   136,   159,   169,
     137,   138,   183,   184,   185,   199,   196,   210,   223,   224,
     139,   110
  };

  const short
  Parser::yytable_[] =
  {
      27,   109,    80,    96,   111,   193,    20,    26,    84,    30,
     202,    67,   165,    47,     1,    31,    48,   116,   117,   118,
      84,   173,    20,    54,    67,   202,    74,   167,   168,    55,
      20,   236,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   202,   214,   194,   233,    58,   155,
     252,    26,   238,   162,    58,   198,     5,    58,   239,    98,
      99,   100,   101,   176,   177,     6,   257,    58,   178,   179,
     163,     8,   170,   255,   102,   103,   -96,   217,    21,   114,
     104,   105,   -96,   237,   180,   200,    31,   120,   181,   201,
     123,   124,    25,   222,    99,   100,   101,    80,   106,    28,
      64,    90,   206,    65,    91,    80,   107,    26,   102,   103,
      32,   216,    26,    34,   104,   105,    23,   108,    24,    37,
     225,    24,    36,   226,   176,   177,   112,    38,   113,   178,
     179,    40,   106,   208,   209,    26,   212,    31,    41,   243,
     107,    43,   228,    31,    26,   180,   247,   176,   177,   181,
     213,   108,   178,   179,   218,   219,   176,   177,    44,    45,
     197,   178,   179,   120,   121,   122,   123,   124,   180,    50,
      51,    52,   181,   230,   240,    59,    61,   180,   215,    60,
      63,   181,    68,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   256,    10,    11,    12,
      13,    14,    15,    16,    17,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,    73,
      74,    81,    77,    82,    83,    89,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   140,
      92,    94,    93,    95,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   220,    84,   114,
     115,   134,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   229,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   241,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   254,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,    10,    11,    12,    13,    14,    15,    16,    17,    97,
     120,   135,   141,   156,    79,    10,    11,    12,    13,    14,
      15,    16,    17,   157,   158,   160,   161,   188,   172,    10,
      11,    12,    13,    14,    15,    16,    17,   164,   166,   174,
     175,   189,   187,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,    10,    11,
      12,    13,    14,    15,    16,    17,   190,   191,   192,    66,
      10,    11,    12,    13,    14,    15,    16,    17,   195,   204,
     205,    78,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   207,   211,   221,   227,   231,   232,   235,
     244,   246,   248,   245,   249,   251,   253,     7,   237,    35,
      29,    69,   171,   186,    85,   203,   234,   242,   250
  };

  const short
  Parser::yycheck_[] =
  {
      21,    90,    70,    87,    93,     6,     8,     3,     8,    39,
     183,    56,   157,     3,     7,    45,     6,   106,   107,   108,
       8,   166,    24,    39,    69,   198,    26,    11,    12,    45,
      32,   221,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   217,    41,    47,   220,    50,   133,
     240,     3,    39,    41,    56,   181,     3,    59,    45,     3,
       4,     5,     6,    15,    16,     0,   256,    69,    20,    21,
     154,    42,   161,   246,    18,    19,    39,   203,    38,    42,
      24,    25,    45,    46,    36,    44,    45,    48,    40,    41,
      51,    52,     3,     3,     4,     5,     6,   165,    42,    36,
       3,    44,   191,     6,    47,   173,    50,     3,    18,    19,
      42,   200,     3,     3,    24,    25,    43,    61,    45,    43,
     209,    45,    40,   212,    15,    16,    38,    14,    40,    20,
      21,    46,    42,    37,    38,     3,    44,    45,    22,   228,
      50,    38,    44,    45,     3,    36,   235,    15,    16,    40,
      41,    61,    20,    21,   204,   205,    15,    16,    46,    23,
     181,    20,    21,    48,    49,    50,    51,    52,    36,    38,
      46,     9,    40,    41,    39,    38,    13,    36,   199,    40,
      47,    40,     3,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    39,    28,    29,    30,
      31,    32,    33,    34,    35,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    40,    36,
      26,     3,    38,     3,     3,     3,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    43,
      47,     3,    27,    41,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    43,     8,    42,
       3,     3,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    28,    29,    30,    31,    32,    33,    34,    35,    39,
      48,     9,     3,    39,    41,    28,    29,    30,    31,    32,
      33,    34,    35,    40,    10,    43,    45,     3,    41,    28,
      29,    30,    31,    32,    33,    34,    35,    40,    40,    46,
      46,     3,    41,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    28,    29,
      30,    31,    32,    33,    34,    35,    47,    42,    47,    39,
      28,    29,    30,    31,    32,    33,    34,    35,     3,    42,
      42,    39,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    47,    47,     3,    36,    43,    43,    38,
      47,    17,     3,    47,     3,    38,    47,     3,    46,    32,
      24,    59,   164,   171,    75,   186,   220,   227,   239
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     7,    64,    65,    66,     3,     0,    66,    42,    67,
      28,    29,    30,    31,    32,    33,    34,    35,    69,    70,
      82,    38,    71,    43,    45,     3,     3,    77,    36,    70,
      39,    45,    42,    68,     3,    69,    40,    43,    14,    76,
      46,    22,    72,    38,    46,    23,    73,     3,     6,    78,
      38,    46,     9,    79,    39,    45,    74,    75,    82,    38,
      40,    13,    83,    47,     3,     6,    39,    75,     3,    74,
      80,    81,    82,    36,    26,    84,    85,    38,    39,    41,
      81,     3,     3,     3,     8,    85,    87,    88,    89,     3,
      44,    47,    47,    27,     3,    41,    89,    39,     3,     4,
       5,     6,    18,    19,    24,    25,    42,    50,    61,   103,
     104,   103,    38,    40,    42,     3,   103,   103,   103,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    40,     3,     9,    90,    93,    94,   103,
      43,     3,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,    86,    89,    39,    40,    10,    91,
      43,    45,    41,    89,    40,    80,    40,    11,    12,    92,
     103,    90,    41,    80,    46,    46,    15,    16,    20,    21,
      36,    40,    77,    95,    96,    97,    91,    41,     3,     3,
      47,    42,    47,     6,    47,     3,    99,    77,    95,    98,
      44,    41,    96,    92,    42,    42,   103,    47,    37,    38,
     100,    47,    44,    41,    41,    77,   103,    95,    93,    93,
      43,     3,     3,   101,   102,   103,   103,    36,    44,    47,
      41,    43,    43,    96,    97,    38,   100,    46,    39,    45,
      39,    47,    99,   103,    47,    47,    17,   103,     3,     3,
     102,    38,   100,    47,    47,    96,    39,   100
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    63,    64,    65,    65,    66,    67,    67,    68,    68,
      69,    69,    69,    70,    71,    71,    72,    72,    73,    73,
      74,    74,    74,    75,    75,    76,    76,    77,    77,    78,
      78,    78,    78,    79,    79,    80,    80,    81,    81,    82,
      82,    82,    82,    82,    82,    82,    82,    83,    84,    84,
      84,    85,    86,    86,    87,    88,    88,    88,    89,    89,
      90,    90,    91,    91,    92,    92,    92,    93,    93,    94,
      94,    95,    95,    95,    96,    96,    96,    97,    97,    97,
      97,    97,    97,    97,    97,    98,    98,    99,    99,    99,
      99,   100,   100,   101,   101,   102,   102,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     1,     1,     2,    15,     3,     0,     3,     0,
       1,     3,     0,     2,     3,     0,     5,     0,     5,     0,
       1,     2,     0,     5,     2,     6,     0,     1,     3,     1,
       1,     3,     3,     4,     0,     1,     2,     3,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     2,
       0,     7,     1,     2,     1,     1,     2,     0,    11,     8,
       4,     0,     4,     0,     7,     7,     0,     1,     0,     1,
       3,     1,     2,     0,     1,     5,     7,     2,     3,     2,
       2,     3,     6,     4,     3,     4,     5,     5,     2,     7,
       4,     3,     0,     1,     3,     3,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     3,     4
  };




#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,    82,    82,    91,    96,   104,   136,   138,   143,   145,
     150,   155,   160,   165,   175,   177,   182,   184,   189,   191,
     196,   201,   206,   211,   219,   230,   232,   237,   242,   250,
     255,   260,   265,   273,   275,   280,   285,   293,   300,   310,
     311,   312,   313,   314,   315,   316,   317,   321,   326,   331,
     336,   341,   352,   357,   365,   370,   375,   380,   385,   404,
     425,   427,   432,   434,   439,   446,   453,   459,   461,   466,
     471,   479,   481,   490,   495,   497,   515,   558,   565,   573,
     580,   587,   596,   605,   617,   622,   627,   636,   645,   654,
     663,   675,   677,   682,   687,   695,   702,   715,   719,   723,
     727,   731,   735,   739,   743,   747,   751,   755,   759,   763,
     767,   771,   775,   780,   782,   784,   786,   788,   790,   792,
     794,   796,   798
  };

  void
  Parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  Parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


#line 4 "./compiler/src/parser.y"
} } // falcon::atc
#line 5917 "parser.tab.cc"

#line 804 "./compiler/src/parser.y"


void falcon::atc::Parser::error(const std::string& msg) {
  std::cerr << "Parse error: " << msg << std::endl;
}
