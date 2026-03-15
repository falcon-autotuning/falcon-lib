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
#line 13 "./compiler/src/parser.y"

  #include <string>
  #include <vector>
  #include <memory>
  #include <optional>
  #include <set>
  #include "falcon-atc/AST.hpp"
  #include "falcon-atc/ParseError.hpp"

#line 53 "parser.tab.cc"


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
# define YYDEBUG 1
#endif

#line 4 "./compiler/src/parser.y"
namespace falcon { namespace atc {
#line 189 "parser.tab.cc"


  /// A point in a source file.
  class position
  {
  public:
    /// Type for file name.
    typedef const std::string filename_type;
    /// Type for line and column numbers.
    typedef int counter_type;

    /// Construct a position.
    explicit position (filename_type* f = YY_NULLPTR,
                       counter_type l = 1,
                       counter_type c = 1)
      : filename (f)
      , line (l)
      , column (c)
    {}


    /// Initialization.
    void initialize (filename_type* fn = YY_NULLPTR,
                     counter_type l = 1,
                     counter_type c = 1)
    {
      filename = fn;
      line = l;
      column = c;
    }

    /** \name Line and Column related manipulators
     ** \{ */
    /// (line related) Advance to the COUNT next lines.
    void lines (counter_type count = 1)
    {
      if (count)
        {
          column = 1;
          line = add_ (line, count, 1);
        }
    }

    /// (column related) Advance to the COUNT next columns.
    void columns (counter_type count = 1)
    {
      column = add_ (column, count, 1);
    }
    /** \} */

    /// File name to which this position refers.
    filename_type* filename;
    /// Current line number.
    counter_type line;
    /// Current column number.
    counter_type column;

  private:
    /// Compute max (min, lhs+rhs).
    static counter_type add_ (counter_type lhs, counter_type rhs, counter_type min)
    {
      return lhs + rhs < min ? min : lhs + rhs;
    }
  };

  /// Add \a width columns, in place.
  inline position&
  operator+= (position& res, position::counter_type width)
  {
    res.columns (width);
    return res;
  }

  /// Add \a width columns.
  inline position
  operator+ (position res, position::counter_type width)
  {
    return res += width;
  }

  /// Subtract \a width columns, in place.
  inline position&
  operator-= (position& res, position::counter_type width)
  {
    return res += -width;
  }

  /// Subtract \a width columns.
  inline position
  operator- (position res, position::counter_type width)
  {
    return res -= width;
  }

  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param pos a reference to the position to redirect
   */
  template <typename YYChar>
  std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const position& pos)
  {
    if (pos.filename)
      ostr << *pos.filename << ':';
    return ostr << pos.line << '.' << pos.column;
  }

  /// Two points in a source file.
  class location
  {
  public:
    /// Type for file name.
    typedef position::filename_type filename_type;
    /// Type for line and column numbers.
    typedef position::counter_type counter_type;

    /// Construct a location from \a b to \a e.
    location (const position& b, const position& e)
      : begin (b)
      , end (e)
    {}

    /// Construct a 0-width location in \a p.
    explicit location (const position& p = position ())
      : begin (p)
      , end (p)
    {}

    /// Construct a 0-width location in \a f, \a l, \a c.
    explicit location (filename_type* f,
                       counter_type l = 1,
                       counter_type c = 1)
      : begin (f, l, c)
      , end (f, l, c)
    {}


    /// Initialization.
    void initialize (filename_type* f = YY_NULLPTR,
                     counter_type l = 1,
                     counter_type c = 1)
    {
      begin.initialize (f, l, c);
      end = begin;
    }

    /** \name Line and Column related manipulators
     ** \{ */
  public:
    /// Reset initial location to final location.
    void step ()
    {
      begin = end;
    }

    /// Extend the current location to the COUNT next columns.
    void columns (counter_type count = 1)
    {
      end += count;
    }

    /// Extend the current location to the COUNT next lines.
    void lines (counter_type count = 1)
    {
      end.lines (count);
    }
    /** \} */


  public:
    /// Beginning of the located region.
    position begin;
    /// End of the located region.
    position end;
  };

  /// Join two locations, in place.
  inline location&
  operator+= (location& res, const location& end)
  {
    res.end = end.end;
    return res;
  }

  /// Join two locations.
  inline location
  operator+ (location res, const location& end)
  {
    return res += end;
  }

  /// Add \a width columns to the end position, in place.
  inline location&
  operator+= (location& res, location::counter_type width)
  {
    res.columns (width);
    return res;
  }

  /// Add \a width columns to the end position.
  inline location
  operator+ (location res, location::counter_type width)
  {
    return res += width;
  }

  /// Subtract \a width columns to the end position, in place.
  inline location&
  operator-= (location& res, location::counter_type width)
  {
    return res += -width;
  }

  /// Subtract \a width columns to the end position.
  inline location
  operator- (location res, location::counter_type width)
  {
    return res -= width;
  }

  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param loc a reference to the location to redirect
   **
   ** Avoid duplicate information.
   */
  template <typename YYChar>
  std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const location& loc)
  {
    location::counter_type end_col
      = 0 < loc.end.column ? loc.end.column - 1 : 0;
    ostr << loc.begin;
    if (loc.end.filename
        && (!loc.begin.filename
            || *loc.begin.filename != *loc.end.filename))
      ostr << '-' << loc.end.filename << ':' << loc.end.line << '.' << end_col;
    else if (loc.begin.line < loc.end.line)
      ostr << '-' << loc.end.line << '.' << end_col;
    else if (loc.begin.column < end_col)
      ostr << '-' << end_col;
    return ostr;
  }


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
      // IDENTIFIER
      // DOUBLE
      // INTEGER
      // STRING
      // entry_state
      char dummy1[sizeof (std::string)];

      // assign_target
      char dummy2[sizeof (std::unique_ptr<AssignTarget>)];

      // autotuner_decl
      char dummy3[sizeof (std::unique_ptr<AutotunerDecl>)];

      // call_arg
      char dummy4[sizeof (std::unique_ptr<CallArg>)];

      // expr
      // postfix_expr
      // primary_expr
      char dummy5[sizeof (std::unique_ptr<Expr>)];

      // param_decl
      char dummy6[sizeof (std::unique_ptr<ParamDecl>)];

      // program
      char dummy7[sizeof (std::unique_ptr<Program>)];

      // routine_decl
      char dummy8[sizeof (std::unique_ptr<RoutineDecl>)];

      // state_decl
      char dummy9[sizeof (std::unique_ptr<StateDecl>)];

      // struct_routine_stmt
      // stmt
      char dummy10[sizeof (std::unique_ptr<Stmt>)];

      // struct_decl
      char dummy11[sizeof (std::unique_ptr<StructDecl>)];

      // type_spec
      char dummy12[sizeof (std::unique_ptr<TypeDescriptor>)];

      // struct_field_decl
      // var_decl_stmt
      char dummy13[sizeof (std::unique_ptr<VarDeclStmt>)];

      // assign_target_list
      char dummy14[sizeof (std::vector<AssignTarget>)];

      // autotuner_list
      char dummy15[sizeof (std::vector<AutotunerDecl>)];

      // call_arg_list
      char dummy16[sizeof (std::vector<CallArg>)];

      // routine_list
      // struct_routine_list
      char dummy17[sizeof (std::vector<RoutineDecl>)];

      // state_list
      char dummy18[sizeof (std::vector<StateDecl>)];

      // struct_decl_list
      char dummy19[sizeof (std::vector<StructDecl>)];

      // struct_field_list
      char dummy20[sizeof (std::vector<VarDeclStmt>)];

      // import_list
      // import_stmt
      // import_string_list
      // requires_clause
      // identifier_list
      char dummy21[sizeof (std::vector<std::string>)];

      // entry_params
      // expr_list
      char dummy22[sizeof (std::vector<std::unique_ptr<Expr>>)];

      // input_params
      // output_params
      // param_decl_list
      // state_input_params
      char dummy23[sizeof (std::vector<std::unique_ptr<ParamDecl>>)];

      // routine_body
      // autotuner_var_decls
      // stmt_list
      // elif_chain
      char dummy24[sizeof (std::vector<std::unique_ptr<Stmt>>)];
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

    /// Symbol locations.
    typedef location location_type;

    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const location_type& l, const std::string& m)
        : std::runtime_error (m)
        , location (l)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
        , location (s.location)
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;

      location_type location;
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
    TOK_ROUTINE = 263,             // ROUTINE
    TOK_STATE = 264,               // STATE
    TOK_STRUCT = 265,              // STRUCT
    TOK_IMPORT = 266,              // IMPORT
    TOK_START = 267,               // START
    TOK_USES = 268,                // USES
    TOK_TERMINAL = 269,            // TERMINAL
    TOK_IF = 270,                  // IF
    TOK_ELIF = 271,                // ELIF
    TOK_ELSE = 272,                // ELSE
    TOK_TRUE = 273,                // TRUE
    TOK_FALSE = 274,               // FALSE
    TOK_NIL = 275,                 // NIL
    TOK_FLOAT_KW = 276,            // FLOAT_KW
    TOK_INT_KW = 277,              // INT_KW
    TOK_BOOL_KW = 278,             // BOOL_KW
    TOK_STRING_KW = 279,           // STRING_KW
    TOK_ERROR_KW = 280,            // ERROR_KW
    TOK_ARROW = 281,               // ARROW
    TOK_LBRACKET = 282,            // LBRACKET
    TOK_RBRACKET = 283,            // RBRACKET
    TOK_LBRACE = 284,              // LBRACE
    TOK_RBRACE = 285,              // RBRACE
    TOK_LPAREN = 286,              // LPAREN
    TOK_RPAREN = 287,              // RPAREN
    TOK_ASSIGN = 288,              // ASSIGN
    TOK_COMMA = 289,               // COMMA
    TOK_SEMICOLON = 290,           // SEMICOLON
    TOK_DOT = 291,                 // DOT
    TOK_PLUS = 292,                // PLUS
    TOK_MINUS = 293,               // MINUS
    TOK_MUL = 294,                 // MUL
    TOK_DIV = 295,                 // DIV
    TOK_EQ = 296,                  // EQ
    TOK_NE = 297,                  // NE
    TOK_LL = 298,                  // LL
    TOK_GG = 299,                  // GG
    TOK_LE = 300,                  // LE
    TOK_GE = 301,                  // GE
    TOK_AND = 302,                 // AND
    TOK_OR = 303,                  // OR
    TOK_NOT = 304,                 // NOT
    TOK_UMINUS = 305               // UMINUS
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
        YYNTOKENS = 51, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of file"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_IDENTIFIER = 3,                        // IDENTIFIER
        S_DOUBLE = 4,                            // DOUBLE
        S_INTEGER = 5,                           // INTEGER
        S_STRING = 6,                            // STRING
        S_AUTOTUNER = 7,                         // AUTOTUNER
        S_ROUTINE = 8,                           // ROUTINE
        S_STATE = 9,                             // STATE
        S_STRUCT = 10,                           // STRUCT
        S_IMPORT = 11,                           // IMPORT
        S_START = 12,                            // START
        S_USES = 13,                             // USES
        S_TERMINAL = 14,                         // TERMINAL
        S_IF = 15,                               // IF
        S_ELIF = 16,                             // ELIF
        S_ELSE = 17,                             // ELSE
        S_TRUE = 18,                             // TRUE
        S_FALSE = 19,                            // FALSE
        S_NIL = 20,                              // NIL
        S_FLOAT_KW = 21,                         // FLOAT_KW
        S_INT_KW = 22,                           // INT_KW
        S_BOOL_KW = 23,                          // BOOL_KW
        S_STRING_KW = 24,                        // STRING_KW
        S_ERROR_KW = 25,                         // ERROR_KW
        S_ARROW = 26,                            // ARROW
        S_LBRACKET = 27,                         // LBRACKET
        S_RBRACKET = 28,                         // RBRACKET
        S_LBRACE = 29,                           // LBRACE
        S_RBRACE = 30,                           // RBRACE
        S_LPAREN = 31,                           // LPAREN
        S_RPAREN = 32,                           // RPAREN
        S_ASSIGN = 33,                           // ASSIGN
        S_COMMA = 34,                            // COMMA
        S_SEMICOLON = 35,                        // SEMICOLON
        S_DOT = 36,                              // DOT
        S_PLUS = 37,                             // PLUS
        S_MINUS = 38,                            // MINUS
        S_MUL = 39,                              // MUL
        S_DIV = 40,                              // DIV
        S_EQ = 41,                               // EQ
        S_NE = 42,                               // NE
        S_LL = 43,                               // LL
        S_GG = 44,                               // GG
        S_LE = 45,                               // LE
        S_GE = 46,                               // GE
        S_AND = 47,                              // AND
        S_OR = 48,                               // OR
        S_NOT = 49,                              // NOT
        S_UMINUS = 50,                           // UMINUS
        S_YYACCEPT = 51,                         // $accept
        S_program = 52,                          // program
        S_import_list = 53,                      // import_list
        S_autotuner_list = 54,                   // autotuner_list
        S_routine_list = 55,                     // routine_list
        S_struct_decl_list = 56,                 // struct_decl_list
        S_struct_decl = 57,                      // struct_decl
        S_58_1 = 58,                             // $@1
        S_struct_field_list = 59,                // struct_field_list
        S_struct_field_decl = 60,                // struct_field_decl
        S_struct_routine_list = 61,              // struct_routine_list
        S_62_2 = 62,                             // $@2
        S_routine_decl = 63,                     // routine_decl
        S_routine_body = 64,                     // routine_body
        S_struct_routine_stmt = 65,              // struct_routine_stmt
        S_import_stmt = 66,                      // import_stmt
        S_import_string_list = 67,               // import_string_list
        S_autotuner_decl = 68,                   // autotuner_decl
        S_69_3 = 69,                             // $@3
        S_input_params = 70,                     // input_params
        S_output_params = 71,                    // output_params
        S_param_decl_list = 72,                  // param_decl_list
        S_param_decl = 73,                       // param_decl
        S_state_input_params = 74,               // state_input_params
        S_type_spec = 75,                        // type_spec
        S_requires_clause = 76,                  // requires_clause
        S_identifier_list = 77,                  // identifier_list
        S_autotuner_var_decls = 78,              // autotuner_var_decls
        S_entry_state = 79,                      // entry_state
        S_entry_params = 80,                     // entry_params
        S_state_list = 81,                       // state_list
        S_state_decl = 82,                       // state_decl
        S_83_4 = 83,                             // $@4
        S_stmt_list = 84,                        // stmt_list
        S_stmt = 85,                             // stmt
        S_assign_target_list = 86,               // assign_target_list
        S_assign_target = 87,                    // assign_target
        S_elif_chain = 88,                       // elif_chain
        S_var_decl_stmt = 89,                    // var_decl_stmt
        S_expr = 90,                             // expr
        S_postfix_expr = 91,                     // postfix_expr
        S_primary_expr = 92,                     // primary_expr
        S_expr_list = 93,                        // expr_list
        S_call_arg_list = 94,                    // call_arg_list
        S_call_arg = 95                          // call_arg
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
    /// Provide access to semantic value and location.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol () YY_NOEXCEPT
        : value ()
        , location ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value ()
        , location (std::move (that.location))
      {
        switch (this->kind ())
    {
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        value.move< std::string > (std::move (that.value));
        break;

      case symbol_kind::S_assign_target: // assign_target
        value.move< std::unique_ptr<AssignTarget> > (std::move (that.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.move< std::unique_ptr<AutotunerDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_call_arg: // call_arg
        value.move< std::unique_ptr<CallArg> > (std::move (that.value));
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.move< std::unique_ptr<Expr> > (std::move (that.value));
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.move< std::unique_ptr<ParamDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_program: // program
        value.move< std::unique_ptr<Program> > (std::move (that.value));
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        value.move< std::unique_ptr<RoutineDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.move< std::unique_ptr<StateDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        value.move< std::unique_ptr<Stmt> > (std::move (that.value));
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        value.move< std::unique_ptr<StructDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_type_spec: // type_spec
        value.move< std::unique_ptr<TypeDescriptor> > (std::move (that.value));
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        value.move< std::unique_ptr<VarDeclStmt> > (std::move (that.value));
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        value.move< std::vector<AssignTarget> > (std::move (that.value));
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        value.move< std::vector<AutotunerDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        value.move< std::vector<CallArg> > (std::move (that.value));
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        value.move< std::vector<RoutineDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_state_list: // state_list
        value.move< std::vector<StateDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        value.move< std::vector<StructDecl> > (std::move (that.value));
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        value.move< std::vector<VarDeclStmt> > (std::move (that.value));
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        value.move< std::vector<std::string> > (std::move (that.value));
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        value.move< std::vector<std::unique_ptr<Expr>> > (std::move (that.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        value.move< std::vector<std::unique_ptr<ParamDecl>> > (std::move (that.value));
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        value.move< std::vector<std::unique_ptr<Stmt>> > (std::move (that.value));
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
      basic_symbol (typename Base::kind_type t, location_type&& l)
        : Base (t)
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const location_type& l)
        : Base (t)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::string&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::string& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<AssignTarget>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<AssignTarget>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<AutotunerDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<AutotunerDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<CallArg>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<CallArg>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<Expr>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<Expr>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<ParamDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<ParamDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<Program>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<Program>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<RoutineDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<RoutineDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<StateDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<StateDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<Stmt>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<Stmt>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<StructDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<StructDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<TypeDescriptor>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<TypeDescriptor>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::unique_ptr<VarDeclStmt>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::unique_ptr<VarDeclStmt>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<AssignTarget>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<AssignTarget>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<AutotunerDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<AutotunerDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<CallArg>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<CallArg>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<RoutineDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<RoutineDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<StateDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<StateDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<StructDecl>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<StructDecl>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<VarDeclStmt>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<VarDeclStmt>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<std::string>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<std::string>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<std::unique_ptr<Expr>>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<std::unique_ptr<Expr>>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<std::unique_ptr<ParamDecl>>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<std::unique_ptr<ParamDecl>>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, std::vector<std::unique_ptr<Stmt>>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const std::vector<std::unique_ptr<Stmt>>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
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
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        value.template destroy< std::string > ();
        break;

      case symbol_kind::S_assign_target: // assign_target
        value.template destroy< std::unique_ptr<AssignTarget> > ();
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.template destroy< std::unique_ptr<AutotunerDecl> > ();
        break;

      case symbol_kind::S_call_arg: // call_arg
        value.template destroy< std::unique_ptr<CallArg> > ();
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.template destroy< std::unique_ptr<Expr> > ();
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.template destroy< std::unique_ptr<ParamDecl> > ();
        break;

      case symbol_kind::S_program: // program
        value.template destroy< std::unique_ptr<Program> > ();
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        value.template destroy< std::unique_ptr<RoutineDecl> > ();
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.template destroy< std::unique_ptr<StateDecl> > ();
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        value.template destroy< std::unique_ptr<Stmt> > ();
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        value.template destroy< std::unique_ptr<StructDecl> > ();
        break;

      case symbol_kind::S_type_spec: // type_spec
        value.template destroy< std::unique_ptr<TypeDescriptor> > ();
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        value.template destroy< std::unique_ptr<VarDeclStmt> > ();
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        value.template destroy< std::vector<AssignTarget> > ();
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        value.template destroy< std::vector<AutotunerDecl> > ();
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        value.template destroy< std::vector<CallArg> > ();
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        value.template destroy< std::vector<RoutineDecl> > ();
        break;

      case symbol_kind::S_state_list: // state_list
        value.template destroy< std::vector<StateDecl> > ();
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        value.template destroy< std::vector<StructDecl> > ();
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        value.template destroy< std::vector<VarDeclStmt> > ();
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        value.template destroy< std::vector<std::string> > ();
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        value.template destroy< std::vector<std::unique_ptr<Expr>> > ();
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        value.template destroy< std::vector<std::unique_ptr<ParamDecl>> > ();
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        value.template destroy< std::vector<std::unique_ptr<Stmt>> > ();
        break;

      default:
        break;
    }

        Base::clear ();
      }

      /// The user-facing name of this symbol.
      std::string name () const YY_NOEXCEPT
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

      /// The location.
      location_type location;

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
      symbol_type (int tok, location_type l)
        : super_type (token_kind_type (tok), std::move (l))
#else
      symbol_type (int tok, const location_type& l)
        : super_type (token_kind_type (tok), l)
#endif
      {}
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, std::string v, location_type l)
        : super_type (token_kind_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const std::string& v, const location_type& l)
        : super_type (token_kind_type (tok), v, l)
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
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

    /// Report a syntax error.
    void error (const syntax_error& err);

    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static std::string symbol_name (symbol_kind_type yysymbol);

    // Implementation of make_symbol for each token kind.
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYEOF (location_type l)
      {
        return symbol_type (token::TOK_YYEOF, std::move (l));
      }
#else
      static
      symbol_type
      make_YYEOF (const location_type& l)
      {
        return symbol_type (token::TOK_YYEOF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYerror (location_type l)
      {
        return symbol_type (token::TOK_YYerror, std::move (l));
      }
#else
      static
      symbol_type
      make_YYerror (const location_type& l)
      {
        return symbol_type (token::TOK_YYerror, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYUNDEF (location_type l)
      {
        return symbol_type (token::TOK_YYUNDEF, std::move (l));
      }
#else
      static
      symbol_type
      make_YYUNDEF (const location_type& l)
      {
        return symbol_type (token::TOK_YYUNDEF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IDENTIFIER (std::string v, location_type l)
      {
        return symbol_type (token::TOK_IDENTIFIER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_IDENTIFIER (const std::string& v, const location_type& l)
      {
        return symbol_type (token::TOK_IDENTIFIER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DOUBLE (std::string v, location_type l)
      {
        return symbol_type (token::TOK_DOUBLE, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_DOUBLE (const std::string& v, const location_type& l)
      {
        return symbol_type (token::TOK_DOUBLE, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_INTEGER (std::string v, location_type l)
      {
        return symbol_type (token::TOK_INTEGER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_INTEGER (const std::string& v, const location_type& l)
      {
        return symbol_type (token::TOK_INTEGER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STRING (std::string v, location_type l)
      {
        return symbol_type (token::TOK_STRING, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_STRING (const std::string& v, const location_type& l)
      {
        return symbol_type (token::TOK_STRING, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_AUTOTUNER (location_type l)
      {
        return symbol_type (token::TOK_AUTOTUNER, std::move (l));
      }
#else
      static
      symbol_type
      make_AUTOTUNER (const location_type& l)
      {
        return symbol_type (token::TOK_AUTOTUNER, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ROUTINE (location_type l)
      {
        return symbol_type (token::TOK_ROUTINE, std::move (l));
      }
#else
      static
      symbol_type
      make_ROUTINE (const location_type& l)
      {
        return symbol_type (token::TOK_ROUTINE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STATE (location_type l)
      {
        return symbol_type (token::TOK_STATE, std::move (l));
      }
#else
      static
      symbol_type
      make_STATE (const location_type& l)
      {
        return symbol_type (token::TOK_STATE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STRUCT (location_type l)
      {
        return symbol_type (token::TOK_STRUCT, std::move (l));
      }
#else
      static
      symbol_type
      make_STRUCT (const location_type& l)
      {
        return symbol_type (token::TOK_STRUCT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IMPORT (location_type l)
      {
        return symbol_type (token::TOK_IMPORT, std::move (l));
      }
#else
      static
      symbol_type
      make_IMPORT (const location_type& l)
      {
        return symbol_type (token::TOK_IMPORT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_START (location_type l)
      {
        return symbol_type (token::TOK_START, std::move (l));
      }
#else
      static
      symbol_type
      make_START (const location_type& l)
      {
        return symbol_type (token::TOK_START, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_USES (location_type l)
      {
        return symbol_type (token::TOK_USES, std::move (l));
      }
#else
      static
      symbol_type
      make_USES (const location_type& l)
      {
        return symbol_type (token::TOK_USES, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_TERMINAL (location_type l)
      {
        return symbol_type (token::TOK_TERMINAL, std::move (l));
      }
#else
      static
      symbol_type
      make_TERMINAL (const location_type& l)
      {
        return symbol_type (token::TOK_TERMINAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IF (location_type l)
      {
        return symbol_type (token::TOK_IF, std::move (l));
      }
#else
      static
      symbol_type
      make_IF (const location_type& l)
      {
        return symbol_type (token::TOK_IF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ELIF (location_type l)
      {
        return symbol_type (token::TOK_ELIF, std::move (l));
      }
#else
      static
      symbol_type
      make_ELIF (const location_type& l)
      {
        return symbol_type (token::TOK_ELIF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ELSE (location_type l)
      {
        return symbol_type (token::TOK_ELSE, std::move (l));
      }
#else
      static
      symbol_type
      make_ELSE (const location_type& l)
      {
        return symbol_type (token::TOK_ELSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_TRUE (location_type l)
      {
        return symbol_type (token::TOK_TRUE, std::move (l));
      }
#else
      static
      symbol_type
      make_TRUE (const location_type& l)
      {
        return symbol_type (token::TOK_TRUE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FALSE (location_type l)
      {
        return symbol_type (token::TOK_FALSE, std::move (l));
      }
#else
      static
      symbol_type
      make_FALSE (const location_type& l)
      {
        return symbol_type (token::TOK_FALSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NIL (location_type l)
      {
        return symbol_type (token::TOK_NIL, std::move (l));
      }
#else
      static
      symbol_type
      make_NIL (const location_type& l)
      {
        return symbol_type (token::TOK_NIL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_FLOAT_KW (location_type l)
      {
        return symbol_type (token::TOK_FLOAT_KW, std::move (l));
      }
#else
      static
      symbol_type
      make_FLOAT_KW (const location_type& l)
      {
        return symbol_type (token::TOK_FLOAT_KW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_INT_KW (location_type l)
      {
        return symbol_type (token::TOK_INT_KW, std::move (l));
      }
#else
      static
      symbol_type
      make_INT_KW (const location_type& l)
      {
        return symbol_type (token::TOK_INT_KW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_BOOL_KW (location_type l)
      {
        return symbol_type (token::TOK_BOOL_KW, std::move (l));
      }
#else
      static
      symbol_type
      make_BOOL_KW (const location_type& l)
      {
        return symbol_type (token::TOK_BOOL_KW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_STRING_KW (location_type l)
      {
        return symbol_type (token::TOK_STRING_KW, std::move (l));
      }
#else
      static
      symbol_type
      make_STRING_KW (const location_type& l)
      {
        return symbol_type (token::TOK_STRING_KW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ERROR_KW (location_type l)
      {
        return symbol_type (token::TOK_ERROR_KW, std::move (l));
      }
#else
      static
      symbol_type
      make_ERROR_KW (const location_type& l)
      {
        return symbol_type (token::TOK_ERROR_KW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ARROW (location_type l)
      {
        return symbol_type (token::TOK_ARROW, std::move (l));
      }
#else
      static
      symbol_type
      make_ARROW (const location_type& l)
      {
        return symbol_type (token::TOK_ARROW, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LBRACKET (location_type l)
      {
        return symbol_type (token::TOK_LBRACKET, std::move (l));
      }
#else
      static
      symbol_type
      make_LBRACKET (const location_type& l)
      {
        return symbol_type (token::TOK_LBRACKET, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RBRACKET (location_type l)
      {
        return symbol_type (token::TOK_RBRACKET, std::move (l));
      }
#else
      static
      symbol_type
      make_RBRACKET (const location_type& l)
      {
        return symbol_type (token::TOK_RBRACKET, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LBRACE (location_type l)
      {
        return symbol_type (token::TOK_LBRACE, std::move (l));
      }
#else
      static
      symbol_type
      make_LBRACE (const location_type& l)
      {
        return symbol_type (token::TOK_LBRACE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RBRACE (location_type l)
      {
        return symbol_type (token::TOK_RBRACE, std::move (l));
      }
#else
      static
      symbol_type
      make_RBRACE (const location_type& l)
      {
        return symbol_type (token::TOK_RBRACE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LPAREN (location_type l)
      {
        return symbol_type (token::TOK_LPAREN, std::move (l));
      }
#else
      static
      symbol_type
      make_LPAREN (const location_type& l)
      {
        return symbol_type (token::TOK_LPAREN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RPAREN (location_type l)
      {
        return symbol_type (token::TOK_RPAREN, std::move (l));
      }
#else
      static
      symbol_type
      make_RPAREN (const location_type& l)
      {
        return symbol_type (token::TOK_RPAREN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ASSIGN (location_type l)
      {
        return symbol_type (token::TOK_ASSIGN, std::move (l));
      }
#else
      static
      symbol_type
      make_ASSIGN (const location_type& l)
      {
        return symbol_type (token::TOK_ASSIGN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_COMMA (location_type l)
      {
        return symbol_type (token::TOK_COMMA, std::move (l));
      }
#else
      static
      symbol_type
      make_COMMA (const location_type& l)
      {
        return symbol_type (token::TOK_COMMA, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SEMICOLON (location_type l)
      {
        return symbol_type (token::TOK_SEMICOLON, std::move (l));
      }
#else
      static
      symbol_type
      make_SEMICOLON (const location_type& l)
      {
        return symbol_type (token::TOK_SEMICOLON, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DOT (location_type l)
      {
        return symbol_type (token::TOK_DOT, std::move (l));
      }
#else
      static
      symbol_type
      make_DOT (const location_type& l)
      {
        return symbol_type (token::TOK_DOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PLUS (location_type l)
      {
        return symbol_type (token::TOK_PLUS, std::move (l));
      }
#else
      static
      symbol_type
      make_PLUS (const location_type& l)
      {
        return symbol_type (token::TOK_PLUS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MINUS (location_type l)
      {
        return symbol_type (token::TOK_MINUS, std::move (l));
      }
#else
      static
      symbol_type
      make_MINUS (const location_type& l)
      {
        return symbol_type (token::TOK_MINUS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MUL (location_type l)
      {
        return symbol_type (token::TOK_MUL, std::move (l));
      }
#else
      static
      symbol_type
      make_MUL (const location_type& l)
      {
        return symbol_type (token::TOK_MUL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DIV (location_type l)
      {
        return symbol_type (token::TOK_DIV, std::move (l));
      }
#else
      static
      symbol_type
      make_DIV (const location_type& l)
      {
        return symbol_type (token::TOK_DIV, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_EQ (location_type l)
      {
        return symbol_type (token::TOK_EQ, std::move (l));
      }
#else
      static
      symbol_type
      make_EQ (const location_type& l)
      {
        return symbol_type (token::TOK_EQ, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NE (location_type l)
      {
        return symbol_type (token::TOK_NE, std::move (l));
      }
#else
      static
      symbol_type
      make_NE (const location_type& l)
      {
        return symbol_type (token::TOK_NE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LL (location_type l)
      {
        return symbol_type (token::TOK_LL, std::move (l));
      }
#else
      static
      symbol_type
      make_LL (const location_type& l)
      {
        return symbol_type (token::TOK_LL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_GG (location_type l)
      {
        return symbol_type (token::TOK_GG, std::move (l));
      }
#else
      static
      symbol_type
      make_GG (const location_type& l)
      {
        return symbol_type (token::TOK_GG, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LE (location_type l)
      {
        return symbol_type (token::TOK_LE, std::move (l));
      }
#else
      static
      symbol_type
      make_LE (const location_type& l)
      {
        return symbol_type (token::TOK_LE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_GE (location_type l)
      {
        return symbol_type (token::TOK_GE, std::move (l));
      }
#else
      static
      symbol_type
      make_GE (const location_type& l)
      {
        return symbol_type (token::TOK_GE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_AND (location_type l)
      {
        return symbol_type (token::TOK_AND, std::move (l));
      }
#else
      static
      symbol_type
      make_AND (const location_type& l)
      {
        return symbol_type (token::TOK_AND, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_OR (location_type l)
      {
        return symbol_type (token::TOK_OR, std::move (l));
      }
#else
      static
      symbol_type
      make_OR (const location_type& l)
      {
        return symbol_type (token::TOK_OR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NOT (location_type l)
      {
        return symbol_type (token::TOK_NOT, std::move (l));
      }
#else
      static
      symbol_type
      make_NOT (const location_type& l)
      {
        return symbol_type (token::TOK_NOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_UMINUS (location_type l)
      {
        return symbol_type (token::TOK_UMINUS, std::move (l));
      }
#else
      static
      symbol_type
      make_UMINUS (const location_type& l)
      {
        return symbol_type (token::TOK_UMINUS, l);
      }
#endif


    class context
    {
    public:
      context (const Parser& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const YY_NOEXCEPT { return yyla_; }
      symbol_kind_type token () const YY_NOEXCEPT { return yyla_.kind (); }
      const location_type& location () const YY_NOEXCEPT { return yyla_.location; }

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


    /// Stored state numbers (used for stacks).
    typedef unsigned char state_type;

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

    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    static std::string yytnamerr_ (const char *yystr);

    /// For a symbol, its name in clear.
    static const char* const yytname_[];


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
      yylast_ = 866,     ///< Last index in yytable_.
      yynnts_ = 45,  ///< Number of nonterminal symbols.
      yyfinal_ = 3 ///< Termination state number.
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
      45,    46,    47,    48,    49,    50
    };
    // Last valid token kind.
    const int code_max = 305;

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
    , location (that.location)
  {
    switch (this->kind ())
    {
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        value.copy< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assign_target: // assign_target
        value.copy< std::unique_ptr<AssignTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.copy< std::unique_ptr<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_call_arg: // call_arg
        value.copy< std::unique_ptr<CallArg> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.copy< std::unique_ptr<Expr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.copy< std::unique_ptr<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_program: // program
        value.copy< std::unique_ptr<Program> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        value.copy< std::unique_ptr<RoutineDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.copy< std::unique_ptr<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        value.copy< std::unique_ptr<Stmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        value.copy< std::unique_ptr<StructDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_type_spec: // type_spec
        value.copy< std::unique_ptr<TypeDescriptor> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        value.copy< std::unique_ptr<VarDeclStmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        value.copy< std::vector<AssignTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        value.copy< std::vector<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        value.copy< std::vector<CallArg> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        value.copy< std::vector<RoutineDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_list: // state_list
        value.copy< std::vector<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        value.copy< std::vector<StructDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        value.copy< std::vector<VarDeclStmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        value.copy< std::vector<std::string> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        value.copy< std::vector<std::unique_ptr<Expr>> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        value.copy< std::vector<std::unique_ptr<ParamDecl>> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        value.copy< std::vector<std::unique_ptr<Stmt>> > (YY_MOVE (that.value));
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
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        value.move< std::string > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_assign_target: // assign_target
        value.move< std::unique_ptr<AssignTarget> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.move< std::unique_ptr<AutotunerDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_call_arg: // call_arg
        value.move< std::unique_ptr<CallArg> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.move< std::unique_ptr<Expr> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.move< std::unique_ptr<ParamDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_program: // program
        value.move< std::unique_ptr<Program> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        value.move< std::unique_ptr<RoutineDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.move< std::unique_ptr<StateDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        value.move< std::unique_ptr<Stmt> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        value.move< std::unique_ptr<StructDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_type_spec: // type_spec
        value.move< std::unique_ptr<TypeDescriptor> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        value.move< std::unique_ptr<VarDeclStmt> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        value.move< std::vector<AssignTarget> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        value.move< std::vector<AutotunerDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        value.move< std::vector<CallArg> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        value.move< std::vector<RoutineDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_state_list: // state_list
        value.move< std::vector<StateDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        value.move< std::vector<StructDecl> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        value.move< std::vector<VarDeclStmt> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        value.move< std::vector<std::string> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        value.move< std::vector<std::unique_ptr<Expr>> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        value.move< std::vector<std::unique_ptr<ParamDecl>> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        value.move< std::vector<std::unique_ptr<Stmt>> > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

    location = YY_MOVE (s.location);
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
#line 3218 "parser.tab.cc"






// Unqualified %code blocks.
#line 23 "./compiler/src/parser.y"

  #include <iostream>
  
  namespace falcon::atc {
    Parser::symbol_type yylex();
  }
  
  std::unique_ptr<falcon::atc::Program> program_root;
  std::vector<falcon::atc::ParseError> current_errors;
  
  // Scope tracking for variable declarations
  std::set<std::string> autotuner_scope;      // Autotuner-level variables
  std::set<std::string> autotuner_input_params;   // Input parameters (read-only)
  std::set<std::string> autotuner_output_params;  // Output parameters (read/write)
  std::set<std::string> state_local_scope;    // State-local variables
  std::set<std::string> state_input_params;    // Current state's input parameter
  
  void clear_autotuner_scope() {
    autotuner_scope.clear();
    autotuner_input_params.clear();
    autotuner_output_params.clear();
    state_local_scope.clear();
    state_input_params.clear();
  }
  
  void clear_state_scope() {
    state_local_scope.clear();
    state_input_params.clear();
  }  

  // -----------------------------------------------------------------------
  // Struct context tracking
  // -----------------------------------------------------------------------

  // Names of all struct types declared so far in this file.
  // Used by type_spec to allow struct names as types.
  std::set<std::string> struct_known_types;

  // The set of field names belonging to the struct currently being parsed.
  // Populated while parsing struct_field_list, cleared after each struct_decl.
  std::set<std::string> struct_field_scope;

  // True while we are inside a struct routine body.
  // When true, bare IDENTIFIER = expr is checked against struct_field_scope
  // first (becomes a StructFieldAssignStmt targeting "self").
  bool in_struct_routine = false;

  void enter_struct_routine() {
    in_struct_routine = true;
    // struct routine has its own mini-scope for input/output params
    autotuner_input_params.clear();
    autotuner_output_params.clear();
    state_local_scope.clear();
    state_input_params.clear();
  }

  void leave_struct_routine() {
    in_struct_routine = false;
    autotuner_input_params.clear();
    autotuner_output_params.clear();
    state_local_scope.clear();
    state_input_params.clear();
  }
  
  bool is_variable_declared(const std::string& name) {
    // When inside a struct routine, bare field names are implicitly in scope
    if (in_struct_routine && struct_field_scope.count(name) > 0) return true;
    return autotuner_scope.count(name) > 0 ||
           autotuner_input_params.count(name) > 0 ||
           autotuner_output_params.count(name) > 0 ||
           state_local_scope.count(name) > 0 ||
           state_input_params.count(name) > 0;
  }
  
  bool is_redeclaration(const std::string& name, bool in_state) {
    if (in_state) {
      // In state: can't redeclare autotuner-level vars, input params, output params, or state input param
      return autotuner_scope.count(name) > 0 ||
             autotuner_input_params.count(name) > 0 ||
             autotuner_output_params.count(name) > 0 ||
             state_input_params.count(name) > 0 ||
             state_local_scope.count(name) > 0;
    } else {
      // At autotuner level: can't redeclare autotuner vars, input params, or output params
      return autotuner_scope.count(name) > 0 ||
             autotuner_input_params.count(name) > 0 ||
             autotuner_output_params.count(name) > 0;
    }
  }

  void set_stmt_location(falcon::atc::Stmt* stmt, const falcon::atc::Parser::location_type& loc) {
    if (stmt) {
      stmt->filename = falcon::atc::current_filename;
      stmt->line = loc.begin.line;
      stmt->column = loc.begin.column;
    }
  }

#line 3325 "parser.tab.cc"


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

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


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
#line 3418 "parser.tab.cc"

  /// Build a parser object.
  Parser::Parser ()
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr)
#else

#endif
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
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assign_target: // assign_target
        value.YY_MOVE_OR_COPY< std::unique_ptr<AssignTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_call_arg: // call_arg
        value.YY_MOVE_OR_COPY< std::unique_ptr<CallArg> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.YY_MOVE_OR_COPY< std::unique_ptr<Expr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_program: // program
        value.YY_MOVE_OR_COPY< std::unique_ptr<Program> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<RoutineDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        value.YY_MOVE_OR_COPY< std::unique_ptr<Stmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        value.YY_MOVE_OR_COPY< std::unique_ptr<StructDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_type_spec: // type_spec
        value.YY_MOVE_OR_COPY< std::unique_ptr<TypeDescriptor> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        value.YY_MOVE_OR_COPY< std::unique_ptr<VarDeclStmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        value.YY_MOVE_OR_COPY< std::vector<AssignTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        value.YY_MOVE_OR_COPY< std::vector<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        value.YY_MOVE_OR_COPY< std::vector<CallArg> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        value.YY_MOVE_OR_COPY< std::vector<RoutineDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_list: // state_list
        value.YY_MOVE_OR_COPY< std::vector<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        value.YY_MOVE_OR_COPY< std::vector<StructDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        value.YY_MOVE_OR_COPY< std::vector<VarDeclStmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        value.YY_MOVE_OR_COPY< std::vector<std::string> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        value.YY_MOVE_OR_COPY< std::vector<std::unique_ptr<Expr>> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        value.YY_MOVE_OR_COPY< std::vector<std::unique_ptr<ParamDecl>> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        value.YY_MOVE_OR_COPY< std::vector<std::unique_ptr<Stmt>> > (YY_MOVE (that.value));
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
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assign_target: // assign_target
        value.move< std::unique_ptr<AssignTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.move< std::unique_ptr<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_call_arg: // call_arg
        value.move< std::unique_ptr<CallArg> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.move< std::unique_ptr<Expr> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.move< std::unique_ptr<ParamDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_program: // program
        value.move< std::unique_ptr<Program> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        value.move< std::unique_ptr<RoutineDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.move< std::unique_ptr<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        value.move< std::unique_ptr<Stmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        value.move< std::unique_ptr<StructDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_type_spec: // type_spec
        value.move< std::unique_ptr<TypeDescriptor> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        value.move< std::unique_ptr<VarDeclStmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        value.move< std::vector<AssignTarget> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        value.move< std::vector<AutotunerDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        value.move< std::vector<CallArg> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        value.move< std::vector<RoutineDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_state_list: // state_list
        value.move< std::vector<StateDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        value.move< std::vector<StructDecl> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        value.move< std::vector<VarDeclStmt> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        value.move< std::vector<std::string> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        value.move< std::vector<std::unique_ptr<Expr>> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        value.move< std::vector<std::unique_ptr<ParamDecl>> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        value.move< std::vector<std::unique_ptr<Stmt>> > (YY_MOVE (that.value));
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
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_assign_target: // assign_target
        value.copy< std::unique_ptr<AssignTarget> > (that.value);
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.copy< std::unique_ptr<AutotunerDecl> > (that.value);
        break;

      case symbol_kind::S_call_arg: // call_arg
        value.copy< std::unique_ptr<CallArg> > (that.value);
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.copy< std::unique_ptr<Expr> > (that.value);
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.copy< std::unique_ptr<ParamDecl> > (that.value);
        break;

      case symbol_kind::S_program: // program
        value.copy< std::unique_ptr<Program> > (that.value);
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        value.copy< std::unique_ptr<RoutineDecl> > (that.value);
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.copy< std::unique_ptr<StateDecl> > (that.value);
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        value.copy< std::unique_ptr<Stmt> > (that.value);
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        value.copy< std::unique_ptr<StructDecl> > (that.value);
        break;

      case symbol_kind::S_type_spec: // type_spec
        value.copy< std::unique_ptr<TypeDescriptor> > (that.value);
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        value.copy< std::unique_ptr<VarDeclStmt> > (that.value);
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        value.copy< std::vector<AssignTarget> > (that.value);
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        value.copy< std::vector<AutotunerDecl> > (that.value);
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        value.copy< std::vector<CallArg> > (that.value);
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        value.copy< std::vector<RoutineDecl> > (that.value);
        break;

      case symbol_kind::S_state_list: // state_list
        value.copy< std::vector<StateDecl> > (that.value);
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        value.copy< std::vector<StructDecl> > (that.value);
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        value.copy< std::vector<VarDeclStmt> > (that.value);
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        value.copy< std::vector<std::string> > (that.value);
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        value.copy< std::vector<std::unique_ptr<Expr>> > (that.value);
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        value.copy< std::vector<std::unique_ptr<ParamDecl>> > (that.value);
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        value.copy< std::vector<std::unique_ptr<Stmt>> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_assign_target: // assign_target
        value.move< std::unique_ptr<AssignTarget> > (that.value);
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        value.move< std::unique_ptr<AutotunerDecl> > (that.value);
        break;

      case symbol_kind::S_call_arg: // call_arg
        value.move< std::unique_ptr<CallArg> > (that.value);
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        value.move< std::unique_ptr<Expr> > (that.value);
        break;

      case symbol_kind::S_param_decl: // param_decl
        value.move< std::unique_ptr<ParamDecl> > (that.value);
        break;

      case symbol_kind::S_program: // program
        value.move< std::unique_ptr<Program> > (that.value);
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        value.move< std::unique_ptr<RoutineDecl> > (that.value);
        break;

      case symbol_kind::S_state_decl: // state_decl
        value.move< std::unique_ptr<StateDecl> > (that.value);
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        value.move< std::unique_ptr<Stmt> > (that.value);
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        value.move< std::unique_ptr<StructDecl> > (that.value);
        break;

      case symbol_kind::S_type_spec: // type_spec
        value.move< std::unique_ptr<TypeDescriptor> > (that.value);
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        value.move< std::unique_ptr<VarDeclStmt> > (that.value);
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        value.move< std::vector<AssignTarget> > (that.value);
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        value.move< std::vector<AutotunerDecl> > (that.value);
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        value.move< std::vector<CallArg> > (that.value);
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        value.move< std::vector<RoutineDecl> > (that.value);
        break;

      case symbol_kind::S_state_list: // state_list
        value.move< std::vector<StateDecl> > (that.value);
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        value.move< std::vector<StructDecl> > (that.value);
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        value.move< std::vector<VarDeclStmt> > (that.value);
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        value.move< std::vector<std::string> > (that.value);
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        value.move< std::vector<std::unique_ptr<Expr>> > (that.value);
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        value.move< std::vector<std::unique_ptr<ParamDecl>> > (that.value);
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        value.move< std::vector<std::unique_ptr<Stmt>> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
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
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
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

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

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
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
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
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_DOUBLE: // DOUBLE
      case symbol_kind::S_INTEGER: // INTEGER
      case symbol_kind::S_STRING: // STRING
      case symbol_kind::S_entry_state: // entry_state
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_assign_target: // assign_target
        yylhs.value.emplace< std::unique_ptr<AssignTarget> > ();
        break;

      case symbol_kind::S_autotuner_decl: // autotuner_decl
        yylhs.value.emplace< std::unique_ptr<AutotunerDecl> > ();
        break;

      case symbol_kind::S_call_arg: // call_arg
        yylhs.value.emplace< std::unique_ptr<CallArg> > ();
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_postfix_expr: // postfix_expr
      case symbol_kind::S_primary_expr: // primary_expr
        yylhs.value.emplace< std::unique_ptr<Expr> > ();
        break;

      case symbol_kind::S_param_decl: // param_decl
        yylhs.value.emplace< std::unique_ptr<ParamDecl> > ();
        break;

      case symbol_kind::S_program: // program
        yylhs.value.emplace< std::unique_ptr<Program> > ();
        break;

      case symbol_kind::S_routine_decl: // routine_decl
        yylhs.value.emplace< std::unique_ptr<RoutineDecl> > ();
        break;

      case symbol_kind::S_state_decl: // state_decl
        yylhs.value.emplace< std::unique_ptr<StateDecl> > ();
        break;

      case symbol_kind::S_struct_routine_stmt: // struct_routine_stmt
      case symbol_kind::S_stmt: // stmt
        yylhs.value.emplace< std::unique_ptr<Stmt> > ();
        break;

      case symbol_kind::S_struct_decl: // struct_decl
        yylhs.value.emplace< std::unique_ptr<StructDecl> > ();
        break;

      case symbol_kind::S_type_spec: // type_spec
        yylhs.value.emplace< std::unique_ptr<TypeDescriptor> > ();
        break;

      case symbol_kind::S_struct_field_decl: // struct_field_decl
      case symbol_kind::S_var_decl_stmt: // var_decl_stmt
        yylhs.value.emplace< std::unique_ptr<VarDeclStmt> > ();
        break;

      case symbol_kind::S_assign_target_list: // assign_target_list
        yylhs.value.emplace< std::vector<AssignTarget> > ();
        break;

      case symbol_kind::S_autotuner_list: // autotuner_list
        yylhs.value.emplace< std::vector<AutotunerDecl> > ();
        break;

      case symbol_kind::S_call_arg_list: // call_arg_list
        yylhs.value.emplace< std::vector<CallArg> > ();
        break;

      case symbol_kind::S_routine_list: // routine_list
      case symbol_kind::S_struct_routine_list: // struct_routine_list
        yylhs.value.emplace< std::vector<RoutineDecl> > ();
        break;

      case symbol_kind::S_state_list: // state_list
        yylhs.value.emplace< std::vector<StateDecl> > ();
        break;

      case symbol_kind::S_struct_decl_list: // struct_decl_list
        yylhs.value.emplace< std::vector<StructDecl> > ();
        break;

      case symbol_kind::S_struct_field_list: // struct_field_list
        yylhs.value.emplace< std::vector<VarDeclStmt> > ();
        break;

      case symbol_kind::S_import_list: // import_list
      case symbol_kind::S_import_stmt: // import_stmt
      case symbol_kind::S_import_string_list: // import_string_list
      case symbol_kind::S_requires_clause: // requires_clause
      case symbol_kind::S_identifier_list: // identifier_list
        yylhs.value.emplace< std::vector<std::string> > ();
        break;

      case symbol_kind::S_entry_params: // entry_params
      case symbol_kind::S_expr_list: // expr_list
        yylhs.value.emplace< std::vector<std::unique_ptr<Expr>> > ();
        break;

      case symbol_kind::S_input_params: // input_params
      case symbol_kind::S_output_params: // output_params
      case symbol_kind::S_param_decl_list: // param_decl_list
      case symbol_kind::S_state_input_params: // state_input_params
        yylhs.value.emplace< std::vector<std::unique_ptr<ParamDecl>> > ();
        break;

      case symbol_kind::S_routine_body: // routine_body
      case symbol_kind::S_autotuner_var_decls: // autotuner_var_decls
      case symbol_kind::S_stmt_list: // stmt_list
      case symbol_kind::S_elif_chain: // elif_chain
        yylhs.value.emplace< std::vector<std::unique_ptr<Stmt>> > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // program: import_list struct_decl_list autotuner_list routine_list
#line 178 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Program> > () = std::make_unique<Program>();
        yylhs.value.as < std::unique_ptr<Program> > ()->imports = std::move(yystack_[3].value.as < std::vector<std::string> > ());
        for (auto &s : yystack_[2].value.as < std::vector<StructDecl> > ()) {
          yylhs.value.as < std::unique_ptr<Program> > ()->structs.push_back(std::move(s));
        }
        yylhs.value.as < std::unique_ptr<Program> > ()->autotuners = std::move(yystack_[1].value.as < std::vector<AutotunerDecl> > ());
        yylhs.value.as < std::unique_ptr<Program> > ()->routines = std::move(yystack_[0].value.as < std::vector<RoutineDecl> > ());
        yylhs.value.as < std::unique_ptr<Program> > ()->build_indexes();
        program_root = std::move(yylhs.value.as < std::unique_ptr<Program> > ());
      }
#line 4401 "parser.tab.cc"
    break;

  case 3: // import_list: %empty
#line 193 "./compiler/src/parser.y"
              { yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>(); }
#line 4407 "parser.tab.cc"
    break;

  case 4: // import_list: import_list import_stmt
#line 195 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[1].value.as < std::vector<std::string> > ());
        yylhs.value.as < std::vector<std::string> > ().insert(yylhs.value.as < std::vector<std::string> > ().end(), std::make_move_iterator(yystack_[0].value.as < std::vector<std::string> > ().begin()), std::make_move_iterator(yystack_[0].value.as < std::vector<std::string> > ().end()));
      }
#line 4416 "parser.tab.cc"
    break;

  case 5: // autotuner_list: %empty
#line 203 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<AutotunerDecl> > () = std::vector<AutotunerDecl>();
      }
#line 4424 "parser.tab.cc"
    break;

  case 6: // autotuner_list: autotuner_list autotuner_decl
#line 207 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<AutotunerDecl> > () = std::move(yystack_[1].value.as < std::vector<AutotunerDecl> > ());
        yylhs.value.as < std::vector<AutotunerDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<AutotunerDecl> > ()));
      }
#line 4433 "parser.tab.cc"
    break;

  case 7: // routine_list: %empty
#line 215 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<RoutineDecl> > () = std::vector<RoutineDecl>();
      }
#line 4441 "parser.tab.cc"
    break;

  case 8: // routine_list: routine_list routine_decl
#line 219 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<RoutineDecl> > () = std::move(yystack_[1].value.as < std::vector<RoutineDecl> > ());
        yylhs.value.as < std::vector<RoutineDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<RoutineDecl> > ()));
      }
#line 4450 "parser.tab.cc"
    break;

  case 9: // struct_decl_list: %empty
#line 231 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<StructDecl> > () = std::vector<StructDecl>(); }
#line 4456 "parser.tab.cc"
    break;

  case 10: // struct_decl_list: struct_decl_list struct_decl
#line 233 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<StructDecl> > () = std::move(yystack_[1].value.as < std::vector<StructDecl> > ());
        yylhs.value.as < std::vector<StructDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<StructDecl> > ()));
      }
#line 4465 "parser.tab.cc"
    break;

  case 11: // $@1: %empty
#line 241 "./compiler/src/parser.y"
      {
        // Clear field scope for this new struct
        struct_field_scope.clear();
      }
#line 4474 "parser.tab.cc"
    break;

  case 12: // struct_decl: STRUCT IDENTIFIER LBRACE $@1 struct_field_list struct_routine_list RBRACE
#line 246 "./compiler/src/parser.y"
      {
        // Register the struct name so type_spec can use it from this point on
        struct_known_types.insert(yystack_[5].value.as < std::string > ());
        struct_field_scope.clear();
        yylhs.value.as < std::unique_ptr<StructDecl> > () = std::make_unique<StructDecl>(
            std::move(yystack_[5].value.as < std::string > ()),
            std::move(yystack_[2].value.as < std::vector<VarDeclStmt> > ()),
            std::move(yystack_[1].value.as < std::vector<RoutineDecl> > ()));
      }
#line 4488 "parser.tab.cc"
    break;

  case 13: // struct_field_list: %empty
#line 263 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<VarDeclStmt> > () = std::vector<VarDeclStmt>(); }
#line 4494 "parser.tab.cc"
    break;

  case 14: // struct_field_list: struct_field_list struct_field_decl
#line 265 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<VarDeclStmt> > () = std::move(yystack_[1].value.as < std::vector<VarDeclStmt> > ());
        yylhs.value.as < std::vector<VarDeclStmt> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<VarDeclStmt> > ()));
      }
#line 4503 "parser.tab.cc"
    break;

  case 15: // struct_field_decl: type_spec IDENTIFIER SEMICOLON
#line 273 "./compiler/src/parser.y"
      {
        struct_field_scope.insert(yystack_[1].value.as < std::string > ());
        auto decl = std::make_unique<VarDeclStmt>(
            std::move(*yystack_[2].value.as < std::unique_ptr<TypeDescriptor> > ()), std::move(yystack_[1].value.as < std::string > ()), std::nullopt);
        decl->decl_scope = VarDeclStmt::DeclScope::StructField;
        yylhs.value.as < std::unique_ptr<VarDeclStmt> > () = std::move(decl);
      }
#line 4515 "parser.tab.cc"
    break;

  case 16: // struct_field_decl: type_spec IDENTIFIER ASSIGN expr SEMICOLON
#line 281 "./compiler/src/parser.y"
      {
        struct_field_scope.insert(yystack_[3].value.as < std::string > ());
        auto decl = std::make_unique<VarDeclStmt>(
            std::move(*yystack_[4].value.as < std::unique_ptr<TypeDescriptor> > ()), std::move(yystack_[3].value.as < std::string > ()),
            std::make_optional(std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ())));
        decl->decl_scope = VarDeclStmt::DeclScope::StructField;
        yylhs.value.as < std::unique_ptr<VarDeclStmt> > () = std::move(decl);
      }
#line 4528 "parser.tab.cc"
    break;

  case 17: // struct_routine_list: %empty
#line 297 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<RoutineDecl> > () = std::vector<RoutineDecl>(); }
#line 4534 "parser.tab.cc"
    break;

  case 18: // $@2: %empty
#line 299 "./compiler/src/parser.y"
      {
        enter_struct_routine();
      }
#line 4542 "parser.tab.cc"
    break;

  case 19: // struct_routine_list: struct_routine_list $@2 routine_decl
#line 303 "./compiler/src/parser.y"
      {
        leave_struct_routine();
        yylhs.value.as < std::vector<RoutineDecl> > () = std::move(yystack_[2].value.as < std::vector<RoutineDecl> > ());
        yylhs.value.as < std::vector<RoutineDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<RoutineDecl> > ()));
      }
#line 4552 "parser.tab.cc"
    break;

  case 20: // routine_decl: ROUTINE IDENTIFIER input_params ARROW output_params LBRACE routine_body RBRACE
#line 315 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<RoutineDecl> > () = std::make_unique<RoutineDecl>(
            std::move(yystack_[6].value.as < std::string > ()),
            std::move(yystack_[5].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()),
            std::move(yystack_[3].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()),
            std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Stmt>> > ()));
      }
#line 4564 "parser.tab.cc"
    break;

  case 21: // routine_decl: ROUTINE IDENTIFIER input_params ARROW output_params
#line 325 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<RoutineDecl> > () = std::make_unique<RoutineDecl>(
            std::move(yystack_[3].value.as < std::string > ()),
            std::move(yystack_[2].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()),
            std::move(yystack_[0].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()));
      }
#line 4575 "parser.tab.cc"
    break;

  case 22: // routine_body: %empty
#line 339 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::vector<std::unique_ptr<Stmt>>(); }
#line 4581 "parser.tab.cc"
    break;

  case 23: // routine_body: routine_body struct_routine_stmt
#line 341 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Stmt>> > ());
        yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<Stmt> > ()));
      }
#line 4590 "parser.tab.cc"
    break;

  case 24: // struct_routine_stmt: var_decl_stmt
#line 350 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::move(yystack_[0].value.as < std::unique_ptr<VarDeclStmt> > ());
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[0].location);
      }
#line 4599 "parser.tab.cc"
    break;

  case 25: // struct_routine_stmt: IDENTIFIER ASSIGN expr SEMICOLON
#line 358 "./compiler/src/parser.y"
      {
        if (struct_field_scope.count(yystack_[3].value.as < std::string > ()) > 0) {
          // Implicit self.field = val
          yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<StructFieldAssignStmt>(
              std::make_unique<VarExpr>("self"),
              std::move(yystack_[3].value.as < std::string > ()),
              std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        } else if (is_variable_declared(yystack_[3].value.as < std::string > ())) {
          if (autotuner_input_params.count(yystack_[3].value.as < std::string > ()) > 0 ||
              state_input_params.count(yystack_[3].value.as < std::string > ()) > 0) {
            error(yystack_[3].location, "Cannot assign to read-only parameter: " + yystack_[3].value.as < std::string > ());
            YYABORT;
          }
          std::vector<AssignTarget> targets;
          targets.emplace_back(yystack_[3].value.as < std::string > ());
          yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<AssignStmt>(
              std::move(targets), std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        } else {
          error(yystack_[3].location, "Undeclared variable in struct routine: " + yystack_[3].value.as < std::string > ());
          YYABORT;
        }
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[3].location);
      }
#line 4627 "parser.tab.cc"
    break;

  case 26: // struct_routine_stmt: expr DOT IDENTIFIER ASSIGN expr SEMICOLON
#line 383 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<StructFieldAssignStmt>(
            std::move(yystack_[5].value.as < std::unique_ptr<Expr> > ()),
            std::move(yystack_[3].value.as < std::string > ()),
            std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[5].location);
      }
#line 4639 "parser.tab.cc"
    break;

  case 27: // struct_routine_stmt: IF LPAREN expr RPAREN LBRACE routine_body RBRACE elif_chain
#line 392 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<IfStmt>(
            std::move(yystack_[5].value.as < std::unique_ptr<Expr> > ()),
            std::move(yystack_[2].value.as < std::vector<std::unique_ptr<Stmt>> > ()),
            std::move(yystack_[0].value.as < std::vector<std::unique_ptr<Stmt>> > ()));
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[7].location);
      }
#line 4651 "parser.tab.cc"
    break;

  case 28: // struct_routine_stmt: expr SEMICOLON
#line 401 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<ExprStmt>(std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[1].location);
      }
#line 4660 "parser.tab.cc"
    break;

  case 29: // import_stmt: IMPORT STRING SEMICOLON
#line 413 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>{std::move(yystack_[1].value.as < std::string > ())}; }
#line 4666 "parser.tab.cc"
    break;

  case 30: // import_stmt: IMPORT LPAREN import_string_list RPAREN
#line 415 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[1].value.as < std::vector<std::string> > ()); }
#line 4672 "parser.tab.cc"
    break;

  case 31: // import_string_list: STRING
#line 420 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>{std::move(yystack_[0].value.as < std::string > ())}; }
#line 4678 "parser.tab.cc"
    break;

  case 32: // import_string_list: import_string_list STRING
#line 422 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[1].value.as < std::vector<std::string> > ()); 
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ())); 
      }
#line 4687 "parser.tab.cc"
    break;

  case 33: // $@3: %empty
#line 435 "./compiler/src/parser.y"
      {
        // Clear scope when starting new autotuner
        clear_autotuner_scope();
      }
#line 4696 "parser.tab.cc"
    break;

  case 34: // autotuner_decl: AUTOTUNER IDENTIFIER $@3 input_params ARROW output_params LBRACE requires_clause autotuner_var_decls entry_state entry_params SEMICOLON state_list RBRACE
#line 448 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<AutotunerDecl> > () = std::make_unique<AutotunerDecl>(
          std::move(yystack_[12].value.as < std::string > ()),
          std::move(yystack_[10].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()),
          std::move(yystack_[8].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()),
          std::move(yystack_[6].value.as < std::vector<std::string> > ()),
          std::move(yystack_[5].value.as < std::vector<std::unique_ptr<Stmt>> > ()),
          std::move(yystack_[4].value.as < std::string > ()),
          std::move(yystack_[3].value.as < std::vector<std::unique_ptr<Expr>> > ()),
          std::move(yystack_[1].value.as < std::vector<StateDecl> > ())
        );
      }
#line 4713 "parser.tab.cc"
    break;

  case 35: // input_params: LPAREN param_decl_list RPAREN
#line 468 "./compiler/src/parser.y"
      {
        // Register input parameters in scope (read-only)
        for (const auto& param : yystack_[1].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()) {
          if (autotuner_input_params.count(param->name) > 0) {
            error(yystack_[1].location, "Duplicate input parameter: " + param->name);
            YYABORT;
          }
          autotuner_input_params.insert(param->name);
        }
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<ParamDecl>> > ());
      }
#line 4729 "parser.tab.cc"
    break;

  case 36: // input_params: LPAREN RPAREN
#line 480 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::vector<std::unique_ptr<ParamDecl>>(); 
      }
#line 4737 "parser.tab.cc"
    break;

  case 37: // input_params: %empty
#line 484 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::vector<std::unique_ptr<ParamDecl>>(); 
      }
#line 4745 "parser.tab.cc"
    break;

  case 38: // output_params: LPAREN param_decl_list RPAREN
#line 491 "./compiler/src/parser.y"
      {
        // Register output parameters in scope (read/write)
        for (const auto& param : yystack_[1].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()) {
          if (autotuner_output_params.count(param->name) > 0) {
            error(yystack_[1].location, "Duplicate output parameter: " + param->name);
            YYABORT;
          }
          if (autotuner_input_params.count(param->name) > 0) {
            error(yystack_[1].location, "Output parameter '" + param->name + "' conflicts with input parameter");
            YYABORT;
          }
          autotuner_output_params.insert(param->name);
        }
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<ParamDecl>> > ());
      }
#line 4765 "parser.tab.cc"
    break;

  case 39: // output_params: %empty
#line 507 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::vector<std::unique_ptr<ParamDecl>>(); 
      }
#line 4773 "parser.tab.cc"
    break;

  case 40: // param_decl_list: param_decl
#line 514 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::vector<std::unique_ptr<ParamDecl>>();
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<ParamDecl> > ()));
      }
#line 4782 "parser.tab.cc"
    break;

  case 41: // param_decl_list: param_decl_list COMMA param_decl
#line 519 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::move(yystack_[2].value.as < std::vector<std::unique_ptr<ParamDecl>> > ());
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<ParamDecl> > ()));
      }
#line 4791 "parser.tab.cc"
    break;

  case 42: // param_decl: type_spec IDENTIFIER
#line 527 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<ParamDecl> > () = std::make_unique<ParamDecl>(std::move(*yystack_[1].value.as < std::unique_ptr<TypeDescriptor> > ()), std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4799 "parser.tab.cc"
    break;

  case 43: // param_decl: type_spec IDENTIFIER ASSIGN expr
#line 531 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<ParamDecl> > () = std::make_unique<ParamDecl>(
          std::move(*yystack_[3].value.as < std::unique_ptr<TypeDescriptor> > ()), 
          std::move(yystack_[2].value.as < std::string > ()), 
          std::make_optional(std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()))
        );
      }
#line 4811 "parser.tab.cc"
    break;

  case 44: // state_input_params: LPAREN param_decl_list RPAREN
#line 542 "./compiler/src/parser.y"
      {
        // Register input parameters in scope (read-only)
        for (const auto& param : yystack_[1].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()) {
          if (is_redeclaration(param->name, true)) {
            error(yystack_[1].location, "State input parameter '" + param->name + "' conflicts with autotuner-level declaration");
            YYABORT;
          }
          state_input_params.insert(param->name);
        }
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<ParamDecl>> > ());
      }
#line 4827 "parser.tab.cc"
    break;

  case 45: // state_input_params: LPAREN RPAREN
#line 554 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::vector<std::unique_ptr<ParamDecl>>(); 
      }
#line 4835 "parser.tab.cc"
    break;

  case 46: // state_input_params: %empty
#line 558 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<ParamDecl>> > () = std::vector<std::unique_ptr<ParamDecl>>(); 
      }
#line 4843 "parser.tab.cc"
    break;

  case 47: // type_spec: INT_KW
#line 569 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<TypeDescriptor> > () = std::make_unique<TypeDescriptor>(ParamType::Int); }
#line 4849 "parser.tab.cc"
    break;

  case 48: // type_spec: FLOAT_KW
#line 571 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<TypeDescriptor> > () = std::make_unique<TypeDescriptor>(ParamType::Float); }
#line 4855 "parser.tab.cc"
    break;

  case 49: // type_spec: BOOL_KW
#line 573 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<TypeDescriptor> > () = std::make_unique<TypeDescriptor>(ParamType::Bool); }
#line 4861 "parser.tab.cc"
    break;

  case 50: // type_spec: STRING_KW
#line 575 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<TypeDescriptor> > () = std::make_unique<TypeDescriptor>(ParamType::String); }
#line 4867 "parser.tab.cc"
    break;

  case 51: // type_spec: ERROR_KW
#line 577 "./compiler/src/parser.y"
      { yylhs.value.as < std::unique_ptr<TypeDescriptor> > () = std::make_unique<TypeDescriptor>(ParamType::Error, "Error"); }
#line 4873 "parser.tab.cc"
    break;

  case 52: // type_spec: IDENTIFIER
#line 579 "./compiler/src/parser.y"
      {
        // Allow user-defined struct type names declared earlier in this file.
        if (struct_known_types.count(yystack_[0].value.as < std::string > ()) == 0) {
          error(yystack_[0].location, "Unknown type '" + yystack_[0].value.as < std::string > () + "' — "
                "did you forget to declare a struct with this name before use?");
          YYABORT;
        }
        yylhs.value.as < std::unique_ptr<TypeDescriptor> > () = std::make_unique<TypeDescriptor>(
            TypeDescriptor::make_struct(yystack_[0].value.as < std::string > ()));
      }
#line 4888 "parser.tab.cc"
    break;

  case 53: // requires_clause: USES identifier_list SEMICOLON
#line 596 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[1].value.as < std::vector<std::string> > ()); 
      }
#line 4896 "parser.tab.cc"
    break;

  case 54: // requires_clause: %empty
#line 600 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>(); 
      }
#line 4904 "parser.tab.cc"
    break;

  case 55: // identifier_list: IDENTIFIER
#line 607 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>();
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4913 "parser.tab.cc"
    break;

  case 56: // identifier_list: identifier_list COMMA IDENTIFIER
#line 612 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::string> > () = std::move(yystack_[2].value.as < std::vector<std::string> > ());
        yylhs.value.as < std::vector<std::string> > ().push_back(std::move(yystack_[0].value.as < std::string > ()));
      }
#line 4922 "parser.tab.cc"
    break;

  case 57: // identifier_list: %empty
#line 617 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::string> > () = std::vector<std::string>(); 
      }
#line 4930 "parser.tab.cc"
    break;

  case 58: // autotuner_var_decls: %empty
#line 624 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::vector<std::unique_ptr<Stmt>>();
      }
#line 4938 "parser.tab.cc"
    break;

  case 59: // autotuner_var_decls: autotuner_var_decls stmt
#line 628 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Stmt>> > ());
        yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<Stmt> > ()));
      }
#line 4947 "parser.tab.cc"
    break;

  case 60: // entry_state: START ARROW IDENTIFIER
#line 636 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ()); 
      }
#line 4955 "parser.tab.cc"
    break;

  case 61: // entry_params: LPAREN expr_list RPAREN
#line 643 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Expr>> > ()); 
      }
#line 4963 "parser.tab.cc"
    break;

  case 62: // entry_params: %empty
#line 647 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > () = std::vector<std::unique_ptr<Expr>>(); 
      }
#line 4971 "parser.tab.cc"
    break;

  case 63: // state_list: state_decl
#line 658 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<StateDecl> > () = std::vector<StateDecl>();
        yylhs.value.as < std::vector<StateDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<StateDecl> > ()));
      }
#line 4980 "parser.tab.cc"
    break;

  case 64: // state_list: state_list state_decl
#line 663 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<StateDecl> > () = std::move(yystack_[1].value.as < std::vector<StateDecl> > ());
        yylhs.value.as < std::vector<StateDecl> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<StateDecl> > ()));
      }
#line 4989 "parser.tab.cc"
    break;

  case 65: // $@4: %empty
#line 671 "./compiler/src/parser.y"
      {
        // Clear state-local scope when entering new state
        clear_state_scope();
      }
#line 4998 "parser.tab.cc"
    break;

  case 66: // state_decl: STATE IDENTIFIER $@4 state_input_params LBRACE stmt_list RBRACE
#line 679 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<StateDecl> > () = std::make_unique<StateDecl>(
          std::move(yystack_[5].value.as < std::string > ()), 
          std::move(yystack_[3].value.as < std::vector<std::unique_ptr<ParamDecl>> > ()), 
          std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Stmt>> > ())
        );
      }
#line 5010 "parser.tab.cc"
    break;

  case 67: // stmt_list: %empty
#line 694 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::vector<std::unique_ptr<Stmt>>(); }
#line 5016 "parser.tab.cc"
    break;

  case 68: // stmt_list: stmt_list stmt
#line 696 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Stmt>> > ());
        yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<Stmt> > ()));
      }
#line 5025 "parser.tab.cc"
    break;

  case 69: // stmt: var_decl_stmt
#line 704 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::move(yystack_[0].value.as < std::unique_ptr<VarDeclStmt> > ()); 
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[0].location);
      }
#line 5034 "parser.tab.cc"
    break;

  case 70: // stmt: IDENTIFIER ASSIGN expr SEMICOLON
#line 709 "./compiler/src/parser.y"
      {
        if (!is_variable_declared(yystack_[3].value.as < std::string > ())) {
          error(yystack_[3].location, "Assignment to undeclared variable: " + yystack_[3].value.as < std::string > ());
          YYABORT;
        }
        if (autotuner_input_params.count(yystack_[3].value.as < std::string > ()) > 0 ||
            state_input_params.count(yystack_[3].value.as < std::string > ()) > 0) {
          error(yystack_[3].location, "Cannot assign to read-only input parameter: " + yystack_[3].value.as < std::string > ());
          YYABORT;
        }
        std::vector<AssignTarget> targets;
        targets.emplace_back(yystack_[3].value.as < std::string > ());
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<AssignStmt>(std::move(targets), std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[3].location);
      }
#line 5054 "parser.tab.cc"
    break;

  case 71: // stmt: assign_target_list ASSIGN expr SEMICOLON
#line 726 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<AssignStmt>(std::move(yystack_[3].value.as < std::vector<AssignTarget> > ()), std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[3].location);
      }
#line 5063 "parser.tab.cc"
    break;

  case 72: // stmt: expr DOT IDENTIFIER ASSIGN expr SEMICOLON
#line 732 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<StructFieldAssignStmt>(
            std::move(yystack_[5].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[3].value.as < std::string > ()), std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[5].location);
      }
#line 5073 "parser.tab.cc"
    break;

  case 73: // stmt: expr SEMICOLON
#line 738 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<ExprStmt>(std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()));
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[1].location);
      }
#line 5082 "parser.tab.cc"
    break;

  case 74: // stmt: IF LPAREN expr RPAREN LBRACE stmt_list RBRACE elif_chain
#line 743 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<IfStmt>(
          std::move(yystack_[5].value.as < std::unique_ptr<Expr> > ()),
          std::move(yystack_[2].value.as < std::vector<std::unique_ptr<Stmt>> > ()),
          std::move(yystack_[0].value.as < std::vector<std::unique_ptr<Stmt>> > ())
        );
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[7].location);
      }
#line 5095 "parser.tab.cc"
    break;

  case 75: // stmt: ARROW IDENTIFIER SEMICOLON
#line 752 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<TransitionStmt>(
          std::move(yystack_[1].value.as < std::string > ())
        );
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[2].location);
      }
#line 5106 "parser.tab.cc"
    break;

  case 76: // stmt: ARROW IDENTIFIER LPAREN expr_list RPAREN SEMICOLON
#line 759 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<TransitionStmt>(
          std::move(yystack_[4].value.as < std::string > ()), 
          std::move(yystack_[2].value.as < std::vector<std::unique_ptr<Expr>> > ())
        );
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[5].location);
      }
#line 5118 "parser.tab.cc"
    break;

  case 77: // stmt: TERMINAL SEMICOLON
#line 767 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Stmt> > () = std::make_unique<TerminalStmt>();
        set_stmt_location(yylhs.value.as < std::unique_ptr<Stmt> > ().get(), yystack_[1].location);
      }
#line 5127 "parser.tab.cc"
    break;

  case 78: // assign_target_list: assign_target COMMA assign_target
#line 778 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<AssignTarget> > () = std::vector<AssignTarget>();
        yylhs.value.as < std::vector<AssignTarget> > ().push_back(std::move(*yystack_[2].value.as < std::unique_ptr<AssignTarget> > ()));
        yylhs.value.as < std::vector<AssignTarget> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<AssignTarget> > ()));
      }
#line 5137 "parser.tab.cc"
    break;

  case 79: // assign_target_list: assign_target_list COMMA assign_target
#line 784 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<AssignTarget> > () = std::move(yystack_[2].value.as < std::vector<AssignTarget> > ());
        yylhs.value.as < std::vector<AssignTarget> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<AssignTarget> > ()));
      }
#line 5146 "parser.tab.cc"
    break;

  case 80: // assign_target: IDENTIFIER
#line 792 "./compiler/src/parser.y"
      {
        if (!is_variable_declared(yystack_[0].value.as < std::string > ())) {
          error(yystack_[0].location, "Assignment to undeclared variable: " + yystack_[0].value.as < std::string > ());
          YYABORT;
        }
        yylhs.value.as < std::unique_ptr<AssignTarget> > () = std::make_unique<AssignTarget>(yystack_[0].value.as < std::string > ());
      }
#line 5158 "parser.tab.cc"
    break;

  case 81: // assign_target: expr DOT IDENTIFIER
#line 800 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<AssignTarget> > () = std::make_unique<AssignTarget>(std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::string > ()));
      }
#line 5166 "parser.tab.cc"
    break;

  case 82: // elif_chain: ELIF LPAREN expr RPAREN LBRACE stmt_list RBRACE elif_chain
#line 807 "./compiler/src/parser.y"
      {
        std::vector<std::unique_ptr<Stmt>> else_vec;
        else_vec.push_back(std::make_unique<IfStmt>(
          std::move(yystack_[5].value.as < std::unique_ptr<Expr> > ()),
          std::move(yystack_[2].value.as < std::vector<std::unique_ptr<Stmt>> > ()),
          std::move(yystack_[0].value.as < std::vector<std::unique_ptr<Stmt>> > ())
        ));
        yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::move(else_vec);
      }
#line 5180 "parser.tab.cc"
    break;

  case 83: // elif_chain: ELSE LBRACE stmt_list RBRACE
#line 817 "./compiler/src/parser.y"
      { yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Stmt>> > ()); }
#line 5186 "parser.tab.cc"
    break;

  case 84: // elif_chain: %empty
#line 819 "./compiler/src/parser.y"
              { yylhs.value.as < std::vector<std::unique_ptr<Stmt>> > () = std::vector<std::unique_ptr<Stmt>>(); }
#line 5192 "parser.tab.cc"
    break;

  case 85: // var_decl_stmt: type_spec IDENTIFIER SEMICOLON
#line 824 "./compiler/src/parser.y"
      {
        // Determine if we're in a state or at autotuner level
        bool in_state = !state_local_scope.empty() || !state_input_params.empty() || 
                       (current_errors.empty()); // Heuristic: if no errors, assume we're parsing normally
        
        // Check for redeclaration
        if (is_redeclaration(yystack_[1].value.as < std::string > (), in_state)) {
          error(yystack_[1].location, "Redeclaration of variable '" + yystack_[1].value.as < std::string > () + "' (conflicts with existing declaration)");
          YYABORT;
        }
        
        // Add to appropriate scope
        if (in_state) {
          state_local_scope.insert(yystack_[1].value.as < std::string > ());
        } else {
          autotuner_scope.insert(yystack_[1].value.as < std::string > ());
        }
        
        yylhs.value.as < std::unique_ptr<VarDeclStmt> > () = std::make_unique<VarDeclStmt>(
          std::move(*yystack_[2].value.as < std::unique_ptr<TypeDescriptor> > ()), 
          std::move(yystack_[1].value.as < std::string > ()), 
          std::nullopt
        );
      }
#line 5221 "parser.tab.cc"
    break;

  case 86: // var_decl_stmt: type_spec IDENTIFIER ASSIGN expr SEMICOLON
#line 849 "./compiler/src/parser.y"
      {
        // Determine if we're in a state or at autotuner level
        bool in_state = !state_local_scope.empty() || !state_input_params.empty();
        
        // Check for redeclaration
        if (is_redeclaration(yystack_[3].value.as < std::string > (), in_state)) {
          error(yystack_[3].location, "Redeclaration of variable '" + yystack_[3].value.as < std::string > () + "' (conflicts with existing declaration)");
          YYABORT;
        }
        
        // Add to appropriate scope
        if (in_state) {
          state_local_scope.insert(yystack_[3].value.as < std::string > ());
        } else {
          autotuner_scope.insert(yystack_[3].value.as < std::string > ());
        }
        
        yylhs.value.as < std::unique_ptr<VarDeclStmt> > () = std::make_unique<VarDeclStmt>(
          std::move(*yystack_[4].value.as < std::unique_ptr<TypeDescriptor> > ()), 
          std::move(yystack_[3].value.as < std::string > ()), 
          std::make_optional(std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()))
        );
      }
#line 5249 "parser.tab.cc"
    break;

  case 87: // expr: primary_expr
#line 880 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()); 
      }
#line 5257 "parser.tab.cc"
    break;

  case 88: // expr: postfix_expr
#line 884 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()); 
      }
#line 5265 "parser.tab.cc"
    break;

  case 89: // expr: expr PLUS expr
#line 888 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("+", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5273 "parser.tab.cc"
    break;

  case 90: // expr: expr MINUS expr
#line 892 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("-", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5281 "parser.tab.cc"
    break;

  case 91: // expr: expr MUL expr
#line 896 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("*", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5289 "parser.tab.cc"
    break;

  case 92: // expr: expr DIV expr
#line 900 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("/", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5297 "parser.tab.cc"
    break;

  case 93: // expr: expr EQ expr
#line 904 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("==", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5305 "parser.tab.cc"
    break;

  case 94: // expr: expr NE expr
#line 908 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("!=", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5313 "parser.tab.cc"
    break;

  case 95: // expr: expr LL expr
#line 912 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("<", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5321 "parser.tab.cc"
    break;

  case 96: // expr: expr GG expr
#line 916 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>(">", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5329 "parser.tab.cc"
    break;

  case 97: // expr: expr LE expr
#line 920 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("<=", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5337 "parser.tab.cc"
    break;

  case 98: // expr: expr GE expr
#line 924 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>(">=", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5345 "parser.tab.cc"
    break;

  case 99: // expr: expr AND expr
#line 928 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("&&", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5353 "parser.tab.cc"
    break;

  case 100: // expr: expr OR expr
#line 932 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<BinaryExpr>("||", std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5361 "parser.tab.cc"
    break;

  case 101: // expr: NOT expr
#line 936 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<UnaryExpr>("!", std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5369 "parser.tab.cc"
    break;

  case 102: // expr: MINUS expr
#line 940 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<UnaryExpr>("-", std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ())); 
      }
#line 5377 "parser.tab.cc"
    break;

  case 103: // postfix_expr: expr DOT IDENTIFIER
#line 947 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<MemberExpr>(
          std::move(yystack_[2].value.as < std::unique_ptr<Expr> > ()), 
          std::move(yystack_[0].value.as < std::string > ())
        ); 
      }
#line 5388 "parser.tab.cc"
    break;

  case 104: // postfix_expr: expr DOT IDENTIFIER LPAREN expr_list RPAREN
#line 954 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<MethodCallExpr>(
          std::move(yystack_[5].value.as < std::unique_ptr<Expr> > ()), 
          std::move(yystack_[3].value.as < std::string > ()), 
          std::move(yystack_[1].value.as < std::vector<std::unique_ptr<Expr>> > ())
        ); 
      }
#line 5400 "parser.tab.cc"
    break;

  case 105: // postfix_expr: expr LBRACKET expr RBRACKET
#line 962 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<IndexExpr>(
          std::move(yystack_[3].value.as < std::unique_ptr<Expr> > ()), 
          std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ())
        ); 
      }
#line 5411 "parser.tab.cc"
    break;

  case 106: // postfix_expr: IDENTIFIER LPAREN RPAREN
#line 969 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<CallExpr>(
          std::move(yystack_[2].value.as < std::string > ()),
          std::vector<CallArg>() // empty argument list
        );
      }
#line 5422 "parser.tab.cc"
    break;

  case 107: // postfix_expr: IDENTIFIER LPAREN call_arg_list RPAREN
#line 976 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<CallExpr>(
          std::move(yystack_[3].value.as < std::string > ()),
          std::move(yystack_[1].value.as < std::vector<CallArg> > ())
        );
      }
#line 5433 "parser.tab.cc"
    break;

  case 108: // primary_expr: INTEGER
#line 986 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<LiteralExpr>(std::stoll(yystack_[0].value.as < std::string > ())); 
      }
#line 5441 "parser.tab.cc"
    break;

  case 109: // primary_expr: DOUBLE
#line 990 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<LiteralExpr>(std::stod(yystack_[0].value.as < std::string > ())); 
      }
#line 5449 "parser.tab.cc"
    break;

  case 110: // primary_expr: STRING
#line 994 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<LiteralExpr>(std::move(yystack_[0].value.as < std::string > ())); 
      }
#line 5457 "parser.tab.cc"
    break;

  case 111: // primary_expr: TRUE
#line 998 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<LiteralExpr>(true); 
      }
#line 5465 "parser.tab.cc"
    break;

  case 112: // primary_expr: FALSE
#line 1002 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<LiteralExpr>(false); 
      }
#line 5473 "parser.tab.cc"
    break;

  case 113: // primary_expr: NIL
#line 1006 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<NilLiteralExpr>(); 
      }
#line 5481 "parser.tab.cc"
    break;

  case 114: // primary_expr: IDENTIFIER
#line 1010 "./compiler/src/parser.y"
      {
        // Validate that variable is declared (only check for variables, not function calls)
        // Note: We can't easily distinguish here, so we only warn, not error
        // The semantic analyzer will do a full check later
        
        yylhs.value.as < std::unique_ptr<Expr> > () = std::make_unique<VarExpr>(std::move(yystack_[0].value.as < std::string > ())); 
      }
#line 5493 "parser.tab.cc"
    break;

  case 115: // primary_expr: LPAREN expr RPAREN
#line 1018 "./compiler/src/parser.y"
      { 
        yylhs.value.as < std::unique_ptr<Expr> > () = std::move(yystack_[1].value.as < std::unique_ptr<Expr> > ()); 
      }
#line 5501 "parser.tab.cc"
    break;

  case 116: // expr_list: expr
#line 1025 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > () = std::vector<std::unique_ptr<Expr>>();
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5510 "parser.tab.cc"
    break;

  case 117: // expr_list: expr_list COMMA expr
#line 1030 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > () = std::move(yystack_[2].value.as < std::vector<std::unique_ptr<Expr>> > ());
        yylhs.value.as < std::vector<std::unique_ptr<Expr>> > ().push_back(std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5519 "parser.tab.cc"
    break;

  case 118: // call_arg_list: call_arg
#line 1038 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<CallArg> > () = std::vector<CallArg>();
        yylhs.value.as < std::vector<CallArg> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<CallArg> > ()));
      }
#line 5528 "parser.tab.cc"
    break;

  case 119: // call_arg_list: call_arg_list COMMA call_arg
#line 1043 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::vector<CallArg> > () = std::move(yystack_[2].value.as < std::vector<CallArg> > ());
        yylhs.value.as < std::vector<CallArg> > ().push_back(std::move(*yystack_[0].value.as < std::unique_ptr<CallArg> > ()));
      }
#line 5537 "parser.tab.cc"
    break;

  case 120: // call_arg: expr
#line 1051 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<CallArg> > () = std::make_unique<CallArg>(std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5545 "parser.tab.cc"
    break;

  case 121: // call_arg: IDENTIFIER ASSIGN expr
#line 1055 "./compiler/src/parser.y"
      {
        yylhs.value.as < std::unique_ptr<CallArg> > () = std::make_unique<CallArg>(std::move(yystack_[2].value.as < std::string > ()), std::move(yystack_[0].value.as < std::unique_ptr<Expr> > ()));
      }
#line 5553 "parser.tab.cc"
    break;


#line 5557 "parser.tab.cc"

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
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
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

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
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
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  Parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
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

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
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
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
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


  const short Parser::yypact_ninf_ = -224;

  const signed char Parser::yytable_ninf_ = -82;

  const short
  Parser::yypact_[] =
  {
    -224,    59,    40,  -224,     1,    61,  -224,    41,   100,   112,
     128,  -224,  -224,  -224,    -2,    82,   117,   135,  -224,  -224,
    -224,  -224,  -224,   145,  -224,  -224,   119,   119,   329,   131,
     132,   138,  -224,  -224,  -224,  -224,  -224,  -224,  -224,   127,
     156,  -224,   -13,  -224,   171,   147,   147,  -224,   135,    58,
    -224,   329,   157,   329,   160,   165,  -224,   365,  -224,  -224,
     365,    63,   178,  -224,   164,  -224,  -224,  -224,  -224,  -224,
    -224,   365,   365,   365,   427,  -224,  -224,   735,  -224,   193,
    -224,   306,   113,   449,   -12,   -12,   365,  -224,   195,   365,
     365,   365,   365,   365,   365,   365,   365,   365,   365,   365,
     365,  -224,   -21,    43,    51,   168,  -224,  -224,   202,  -224,
     471,    68,  -224,   735,    70,  -224,  -224,   493,   175,    86,
      86,   -12,   -12,   820,   820,   133,   133,   133,   133,   800,
     779,   204,  -224,    19,   182,   174,   179,   211,   184,  -224,
      46,   191,  -224,   515,   365,   365,    77,  -224,   223,   365,
    -224,   389,  -224,   365,  -224,   365,   224,  -224,   365,    25,
     365,   196,   365,   412,   412,  -224,   227,   537,   559,   365,
    -224,    90,   735,  -224,   735,    95,   581,  -224,   603,   365,
    -224,    96,   225,   625,    39,  -224,   757,  -224,    44,  -224,
     203,   647,   365,  -224,   365,  -224,   206,   104,  -224,   230,
      -1,  -224,  -224,   238,   365,  -224,  -224,   669,   735,  -224,
     207,  -224,  -224,  -224,    52,   691,   342,  -224,   162,  -224,
     212,  -224,    71,    71,  -224,   317,   215,   214,   217,  -224,
    -224,  -224,   108,  -224,   365,  -224,  -224,   198,   713,   234,
    -224,   221,  -224,  -224,   270,    71,  -224
  };

  const signed char
  Parser::yydefact_[] =
  {
       3,     0,     9,     1,     0,     5,     4,     0,     0,     0,
       7,    10,    29,    31,     0,     0,     0,     2,     6,    32,
      30,    11,    33,     0,     8,    13,    37,    37,    17,     0,
       0,     0,    52,    48,    47,    49,    50,    51,    14,    18,
       0,    36,     0,    40,     0,    39,    39,    12,     0,     0,
      35,     0,    42,     0,     0,    21,    19,     0,    15,    41,
       0,     0,    54,    22,   114,   109,   108,   110,   111,   112,
     113,     0,     0,     0,     0,    88,    87,    43,    38,    57,
      58,     0,     0,     0,   102,   101,     0,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    55,     0,     0,   114,     0,    20,    23,     0,    24,
       0,   114,   106,   120,     0,   118,   115,     0,   103,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,     0,    53,   114,     0,     0,     0,     0,    62,    59,
       0,     0,    69,     0,     0,     0,     0,    28,     0,     0,
     107,     0,   105,     0,    56,     0,     0,    77,     0,     0,
       0,     0,     0,     0,     0,    73,     0,     0,     0,     0,
      85,   103,   121,   119,   116,     0,     0,    60,     0,     0,
      75,     0,     0,     0,   114,    79,     0,    78,   103,    25,
       0,     0,     0,   104,     0,    70,     0,     0,    61,     0,
       0,    63,    71,     0,     0,    22,    86,     0,   117,    67,
       0,    65,    34,    64,   103,     0,     0,    26,     0,    76,
      46,    72,    84,    84,    68,     0,     0,     0,     0,    27,
      74,    45,     0,    67,     0,    67,    44,     0,     0,     0,
      66,     0,    83,    67,     0,    84,    82
  };

  const short
  Parser::yypgoto_[] =
  {
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,  -224,   213,    57,  -224,  -224,  -224,  -224,  -224,   236,
     205,   -50,   216,  -224,   -28,  -224,  -224,  -224,  -224,  -224,
    -224,    66,  -224,  -223,   166,  -224,   -17,  -212,   -75,   -55,
    -224,  -224,  -151,  -224,   120
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,     1,     2,    10,    17,     5,    11,    25,    28,    38,
      39,    48,    24,    81,   107,     6,    14,    18,    26,    30,
      54,    42,    43,   226,   108,    80,   102,   103,   138,   161,
     200,   201,   220,   218,   224,   140,   141,   229,   142,   143,
      75,    76,   175,   114,   115
  };

  const short
  Parser::yytable_[] =
  {
      40,    44,    74,    61,    19,    77,   109,     7,   199,   181,
     237,   230,   239,   131,   132,    86,    83,    84,    85,    50,
     244,    51,   -52,    44,    88,    44,   110,   113,   197,   212,
      20,   117,     8,   246,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   133,    65,    66,    67,
      82,     4,   155,   -80,   -52,   134,   179,   135,   136,     3,
     180,    68,    69,    70,    33,    34,    35,    36,    37,   137,
      82,     9,   -80,   -80,    71,   153,    12,   204,   -81,   162,
     163,    72,    82,   153,   144,   -81,   -81,   227,   228,   167,
     168,    57,    73,    58,   172,    78,   113,    51,   174,    82,
     176,   149,   150,   178,   151,   174,    13,   183,   186,   186,
     169,    21,   170,    86,   191,    15,   111,    65,    66,    67,
      22,   153,    88,   192,   174,    91,    92,   193,   198,   194,
     194,    68,    69,    70,    32,    16,   210,   207,   194,   208,
     236,   109,    51,    23,    71,   112,   185,   187,    27,   215,
      29,    72,    33,    34,    35,    36,    37,    47,    45,    49,
      86,   110,    73,    41,    46,   133,    65,    66,    67,    88,
      89,    90,    91,    92,    52,   232,   135,   136,    53,   238,
      68,    69,    70,    33,    34,    35,    36,    37,   137,    62,
      60,    79,   223,    71,    63,    82,   101,    44,   118,   145,
      72,   133,    65,    66,    67,   146,   153,   154,   156,   157,
     158,    73,   135,   136,   159,   160,    68,    69,    70,    33,
      34,    35,    36,    37,   137,   164,   171,   177,   240,    71,
     188,   182,   205,   211,   199,   209,    72,   133,    65,    66,
      67,   214,   219,   225,   233,   234,   235,    73,   135,   136,
     243,    55,    68,    69,    70,    33,    34,    35,    36,    37,
     137,    56,   216,    31,   242,    71,   213,    59,     0,   139,
       0,   173,    72,   133,    65,    66,    67,     0,     0,     0,
       0,     0,     0,    73,   135,   136,     0,     0,    68,    69,
      70,    33,    34,    35,    36,    37,   137,     0,     0,     0,
     245,    71,     0,     0,     0,     0,     0,     0,    72,   104,
      65,    66,    67,     0,     0,     0,     0,     0,     0,    73,
      32,   105,     0,     0,    68,    69,    70,    33,    34,    35,
      36,    37,    32,     0,     0,     0,   106,    71,    33,    34,
      35,    36,    37,     0,    72,   104,    65,    66,    67,   231,
      33,    34,    35,    36,    37,    73,     0,   105,     0,     0,
      68,    69,    70,    33,    34,    35,    36,    37,    64,    65,
      66,    67,   222,    71,     0,     0,     0,     0,     0,     0,
      72,     0,     0,    68,    69,    70,     0,     0,     0,     0,
       0,    73,   111,    65,    66,    67,    71,     0,     0,     0,
       0,     0,     0,    72,     0,     0,     0,    68,    69,    70,
       0,     0,     0,     0,    73,   184,    65,    66,    67,     0,
      71,     0,     0,     0,     0,     0,     0,    72,     0,     0,
      68,    69,    70,     0,     0,     0,     0,     0,    73,     0,
       0,     0,     0,    71,     0,     0,     0,     0,     0,     0,
      72,     0,     0,     0,    86,     0,     0,     0,     0,     0,
       0,    73,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,    86,     0,     0,     0,
       0,   116,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,    86,     0,
       0,     0,     0,     0,     0,     0,   147,   148,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
      86,   152,     0,     0,     0,     0,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,    86,     0,     0,     0,     0,     0,     0,     0,
     165,   166,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,    86,     0,     0,     0,     0,     0,
       0,     0,   189,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,    86,     0,     0,     0,
       0,   190,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,    86,     0,
       0,     0,     0,     0,     0,     0,   195,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
      86,     0,     0,     0,     0,   196,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,    86,     0,     0,     0,     0,     0,     0,     0,
     202,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,    86,     0,     0,     0,     0,     0,
       0,     0,   206,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,    86,     0,     0,     0,
       0,     0,     0,     0,   217,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,    86,     0,
       0,     0,     0,     0,     0,     0,   221,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
      86,     0,     0,     0,     0,   241,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,    86,     0,     0,     0,     0,     0,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,    86,     0,     0,     0,     0,     0,
       0,     0,     0,   203,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,    86,     0,     0,     0,
       0,     0,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,    86,     0,     0,
       0,     0,     0,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    86,     0,     0,
       0,     0,     0,     0,     0,     0,    88,    89,    90,    91,
      92,     0,     0,    95,    96,    97,    98
  };

  const short
  Parser::yycheck_[] =
  {
      28,    29,    57,    53,     6,    60,    81,     6,     9,   160,
     233,   223,   235,    34,    35,    27,    71,    72,    73,    32,
     243,    34,     3,    51,    36,    53,    81,    82,   179,    30,
      32,    86,    31,   245,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,     3,     4,     5,     6,
      31,    11,    33,    34,     3,    12,    31,    14,    15,     0,
      35,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      31,    10,    33,    34,    31,    31,    35,    33,    34,    33,
      34,    38,    31,    31,    33,    33,    34,    16,    17,   144,
     145,    33,    49,    35,   149,    32,   151,    34,   153,    31,
     155,    33,    32,   158,    34,   160,     6,   162,   163,   164,
      33,    29,    35,    27,   169,     3,     3,     4,     5,     6,
       3,    31,    36,    33,   179,    39,    40,    32,    32,    34,
      34,    18,    19,    20,     3,     7,    32,   192,    34,   194,
      32,   216,    34,     8,    31,    32,   163,   164,     3,   204,
      31,    38,    21,    22,    23,    24,    25,    30,    26,     3,
      27,   216,    49,    32,    26,     3,     4,     5,     6,    36,
      37,    38,    39,    40,     3,   225,    14,    15,    31,   234,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    29,
      33,    13,    30,    31,    29,    31,     3,   225,     3,    31,
      38,     3,     4,     5,     6,     3,    31,     3,    26,    35,
      31,    49,    14,    15,     3,    31,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    34,     3,     3,    30,    31,
       3,    35,    29,     3,     9,    29,    38,     3,     4,     5,
       6,     3,    35,    31,    29,    31,    29,    49,    14,    15,
      29,    46,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    48,   205,    27,    30,    31,   200,    51,    -1,   103,
      -1,   151,    38,     3,     4,     5,     6,    -1,    -1,    -1,
      -1,    -1,    -1,    49,    14,    15,    -1,    -1,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,     3,
       4,     5,     6,    -1,    -1,    -1,    -1,    -1,    -1,    49,
       3,    15,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,     3,    -1,    -1,    -1,    30,    31,    21,    22,
      23,    24,    25,    -1,    38,     3,     4,     5,     6,    32,
      21,    22,    23,    24,    25,    49,    -1,    15,    -1,    -1,
      18,    19,    20,    21,    22,    23,    24,    25,     3,     4,
       5,     6,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    18,    19,    20,    -1,    -1,    -1,    -1,
      -1,    49,     3,     4,     5,     6,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    18,    19,    20,
      -1,    -1,    -1,    -1,    49,     3,     4,     5,     6,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      18,    19,    20,    -1,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    27,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    27,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      27,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      27,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    38,    39,
      40,    -1,    -1,    43,    44,    45,    46
  };

  const signed char
  Parser::yystos_[] =
  {
       0,    52,    53,     0,    11,    56,    66,     6,    31,    10,
      54,    57,    35,     6,    67,     3,     7,    55,    68,     6,
      32,    29,     3,     8,    63,    58,    69,     3,    59,    31,
      70,    70,     3,    21,    22,    23,    24,    25,    60,    61,
      75,    32,    72,    73,    75,    26,    26,    30,    62,     3,
      32,    34,     3,    31,    71,    71,    63,    33,    35,    73,
      33,    72,    29,    29,     3,     4,     5,     6,    18,    19,
      20,    31,    38,    49,    90,    91,    92,    90,    32,    13,
      76,    64,    31,    90,    90,    90,    27,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,     3,    77,    78,     3,    15,    30,    65,    75,    89,
      90,     3,    32,    90,    94,    95,    32,    90,     3,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    34,    35,     3,    12,    14,    15,    26,    79,    85,
      86,    87,    89,    90,    33,    31,     3,    35,    36,    33,
      32,    34,    28,    31,     3,    33,    26,    35,    31,     3,
      31,    80,    33,    34,    34,    35,    36,    90,    90,    33,
      35,     3,    90,    95,    90,    93,    90,     3,    90,    31,
      35,    93,    35,    90,     3,    87,    90,    87,     3,    35,
      32,    90,    33,    32,    34,    35,    32,    93,    32,     9,
      81,    82,    35,    36,    33,    29,    35,    90,    90,    29,
      32,     3,    30,    82,     3,    90,    64,    35,    84,    35,
      83,    35,    30,    30,    85,    31,    74,    16,    17,    88,
      88,    32,    72,    29,    31,    29,    32,    84,    90,    84,
      30,    32,    30,    29,    84,    30,    88
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    51,    52,    53,    53,    54,    54,    55,    55,    56,
      56,    58,    57,    59,    59,    60,    60,    61,    62,    61,
      63,    63,    64,    64,    65,    65,    65,    65,    65,    66,
      66,    67,    67,    69,    68,    70,    70,    70,    71,    71,
      72,    72,    73,    73,    74,    74,    74,    75,    75,    75,
      75,    75,    75,    76,    76,    77,    77,    77,    78,    78,
      79,    80,    80,    81,    81,    83,    82,    84,    84,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    86,    86,
      87,    87,    88,    88,    88,    89,    89,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    91,    91,    91,    91,    91,    92,    92,
      92,    92,    92,    92,    92,    92,    93,    93,    94,    94,
      95,    95
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     4,     0,     2,     0,     2,     0,     2,     0,
       2,     0,     7,     0,     2,     3,     5,     0,     0,     3,
       8,     5,     0,     2,     1,     4,     6,     8,     2,     3,
       4,     1,     2,     0,    14,     3,     2,     0,     3,     0,
       1,     3,     2,     4,     3,     2,     0,     1,     1,     1,
       1,     1,     1,     3,     0,     1,     3,     0,     0,     2,
       3,     3,     0,     1,     2,     0,     7,     0,     2,     1,
       4,     4,     6,     2,     8,     3,     6,     2,     3,     3,
       1,     3,     8,     4,     0,     3,     5,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     3,     6,     4,     3,     4,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     3,
       1,     3
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const Parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFIER", "DOUBLE",
  "INTEGER", "STRING", "AUTOTUNER", "ROUTINE", "STATE", "STRUCT", "IMPORT",
  "START", "USES", "TERMINAL", "IF", "ELIF", "ELSE", "TRUE", "FALSE",
  "NIL", "FLOAT_KW", "INT_KW", "BOOL_KW", "STRING_KW", "ERROR_KW", "ARROW",
  "LBRACKET", "RBRACKET", "LBRACE", "RBRACE", "LPAREN", "RPAREN", "ASSIGN",
  "COMMA", "SEMICOLON", "DOT", "PLUS", "MINUS", "MUL", "DIV", "EQ", "NE",
  "LL", "GG", "LE", "GE", "AND", "OR", "NOT", "UMINUS", "$accept",
  "program", "import_list", "autotuner_list", "routine_list",
  "struct_decl_list", "struct_decl", "$@1", "struct_field_list",
  "struct_field_decl", "struct_routine_list", "$@2", "routine_decl",
  "routine_body", "struct_routine_stmt", "import_stmt",
  "import_string_list", "autotuner_decl", "$@3", "input_params",
  "output_params", "param_decl_list", "param_decl", "state_input_params",
  "type_spec", "requires_clause", "identifier_list", "autotuner_var_decls",
  "entry_state", "entry_params", "state_list", "state_decl", "$@4",
  "stmt_list", "stmt", "assign_target_list", "assign_target", "elif_chain",
  "var_decl_stmt", "expr", "postfix_expr", "primary_expr", "expr_list",
  "call_arg_list", "call_arg", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,   177,   177,   193,   194,   202,   206,   214,   218,   230,
     232,   241,   240,   262,   264,   272,   280,   296,   299,   298,
     312,   323,   338,   340,   349,   357,   382,   391,   400,   412,
     414,   419,   421,   435,   434,   467,   479,   483,   490,   506,
     513,   518,   526,   530,   541,   553,   557,   568,   570,   572,
     574,   576,   578,   595,   599,   606,   611,   616,   623,   627,
     635,   642,   646,   657,   662,   671,   670,   693,   695,   703,
     708,   725,   731,   737,   742,   751,   758,   766,   777,   783,
     791,   799,   806,   816,   819,   823,   848,   879,   883,   887,
     891,   895,   899,   903,   907,   911,   915,   919,   923,   927,
     931,   935,   939,   946,   953,   961,   968,   975,   985,   989,
     993,   997,  1001,  1005,  1009,  1017,  1024,  1029,  1037,  1042,
    1050,  1054
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
#line 6321 "parser.tab.cc"

#line 1060 "./compiler/src/parser.y"


void falcon::atc::Parser::error(const location_type& loc, const std::string& msg) {
  ParseError err{
    loc.begin.line,
    loc.begin.column,
    loc.end.line,
    loc.end.column,
    msg
  };
  
  current_errors.push_back(err);
}
