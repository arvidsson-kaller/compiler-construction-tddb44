#ifndef __SYMTAB_HH__
#define __SYMTAB_HH__

#include "error.hh"

// Set this #define to 0 after the scanner works.
#if defined(LAB1)
#define TEST_SCANNER 1
#else
#define TEST_SCANNER 0
#endif

/* Just to make the code more readable. Not true new types like in Pascal. */
typedef long pool_index;
typedef long hash_index;
typedef long sym_index;
typedef int block_level;

/* Since constants can be either integers or reals, we use a union to hold the
   value for now. */
typedef union {
    long   ival;
    double rval;
} constant_value;

/* The various types of symbol tags that can appear. If C++ had had an
   instanceof operator like Java, we wouldn't need this... */
// enumerated 0-7
enum symbol_types {
    SYM_ARRAY,
    SYM_FUNC,
    SYM_PROC,
    SYM_VAR,
    SYM_PARAM,
    SYM_CONST,
    SYM_NAMETYPE,
    SYM_UNDEF
};
typedef enum symbol_types sym_type;

/* Some numerical constants we use in the symbol table. */

/*!
 *  Max allowed nesting levels (size of block table).
 */
const block_level MAX_BLOCK = 8;

/*!
 *  Max size of hash table.
 */
const hash_index MAX_HASH = 512;

/*!
 *  Base size of string pool.
 */
const pool_index BASE_POOL_SIZE = 1024;

/*!
 *  Max size of symbol table.
 */
const sym_index MAX_SYM = 1024;

/*!
 *  Signifies 'no symbol'.
 */
const sym_index NULL_SYM = -1;

/*!
 * Signifies a non-int array size.
 * The code using this constant has already been written for you.
 */
const int ILLEGAL_ARRAY_CARD = -1;

/*!
   Sets a limit for max nr of temporary variables. Should never be reached
   unless someone really, really starts to dig writing huge programs in
   Diesel. See quads.cc.
 */
const int MAX_TEMP_VARS = 999999;
const int MAX_TEMP_VAR_LENGTH = 8;

/* The various symbol classes, predefined. */
class constant_symbol;
class variable_symbol;
class array_symbol;
class parameter_symbol;
class procedure_symbol;
class function_symbol;
class nametype_symbol;

class symbol_table;

/*! \brief A global pointer to the ::symbol_table.

 You will use functions in the ::symbol_table to install strings
 (and, if you want to make your compiler use shared strings,
 to reduce memory requirements, to look-up and identify symbols,
 as well as forget strings).
 */
extern symbol_table *sym_tab; // implementation in symtab.cc



/* Global symbol table variables. These indexes point to symbols in the symbol
   table which represent information about types. Declared "for real" in
   symbol.cc. */
extern sym_index void_type;
extern sym_index integer_type;
extern sym_index real_type;




/**********************************
 *** THE VARIOUS SYMBOL CLASSES ***
 **********************************/

/*! The symbol table consists of entries of subclasses to symbol. This class
   contains data that are common to all symbol types.
   This class is never used directly. Use the derived classes instead. */
class symbol
{
protected:
    // Every symbol must define a print method, which is called when the
    // symbol is sent to an outstream.
    virtual void print(ostream &);

    // This is used later on to control the level of detail given when printing
    // a symbol. If you're not used to C++, don't worry: You don't really need
    // to understand how this works to complete the lab course. :) */
    friend ostream &long_symbols(ostream &);

    friend ostream &summary_symbols(ostream &);

    friend ostream &short_symbols(ostream &);

    enum format_types { LONG_FORMAT, SUMMARY_FORMAT, SHORT_FORMAT };

    typedef enum format_types format_type;

    static format_type output_format;

public:
    /*! \brief Index to the string_pool, ie, its name.

      The object’s identifier is stored in the string table (i.e. the
      spelling, see previous lab). Index to string table (called ``id`` in
      the lab skeleton) is a value denoting where an identifier starts in
      the string table. Example:

      The scanner from lab 1 has created a string table that look like
      this (the number denotes the length of the following identifier):

      ``7GLOBAL.4VOID7INTEGER...``

      The id to ``GLOBAL.`` is 0 because it starts on the first
      position in the string table (the index to the first position in an
      array in C++ is 0).
    */
    pool_index id;

    /*!
     This field is only used to give us a way to differentiate between
     the various symbol classes when we need to do a safe downcast from
     the superclass. The default value of tag is ``::SYM_UNDEF``.
     */
    sym_type tag;

    /*!
     In practice type is used only by constants, variables, arrays,
     parameters and functions; all other symbol types are of type void.
     The type field contains an index to a name type which in DIESEL
     only can be ``::integer_type``, ``::real_type``, or ``::void_type``.
     These will be preinstalled in the symbol table.
     */
    sym_index type;

    /*!
     If the object you are searching for not is found immediately after
     the key transformation, you follow the hash link backwards in the table.
     */
    sym_index hash_link;

    /*!
     This link points back to the hash table from the symbol table and
     can be used to speed up certain types of lookups.
     It is possible to complete the lab without using it, though.
     */
    hash_index back_link;

    /*! The lexical level states how deeply nested the object is in the program.
     * For example, an object on the first level is global.
     * Eight levels is the maximum in Diesel.
     */
    block_level level;

    /*!
     Offset specifies which relative position the object has in the
     memory space which has been reserved for a given lexical level on
     the runtime stack.
     This is used during code generation.
     */
    int offset;

    // Constructor.
    symbol(pool_index);

    // Currently lacks print method/operator.
    // Currently lacks some other needed stuff like conversions to and
    //   from strings.

    // These functions must be defined by the appropriate subclasses to
    // return a symbol of the correct type. This is to be able to ensure
    // safe downcasts in C++, since we don't want to rely on RTTI information
    // which isn't as of yet supported in all compilers.
    // Calling the method in this class is considered an error and will cause
    // the compiler to abort. It should only be called in a derived class.
    // The "return NULL;" lines are just there to avoid compiler warnings.
    virtual constant_symbol *get_constant_symbol() {
        fatal("Illegal downcasting to constant from symbol class");
        return NULL;
    }
    virtual variable_symbol *get_variable_symbol() {
        fatal("Illegal downcasting to variable from symbol class");
        return NULL;
    }
    virtual array_symbol *get_array_symbol() {
        fatal("Illegal downcasting to array from symbol class");
        return NULL;
    }
    virtual parameter_symbol *get_parameter_symbol() {
        fatal("Illegal downcasting to parameter from symbol class");
        return NULL;
    }
    virtual procedure_symbol *get_procedure_symbol() {
        fatal("Illegal downcasting to procedure from symbol class");
        return NULL;
    }
    virtual function_symbol *get_function_symbol() {
        fatal("Illegal downcasting to function from symbol class");
        return NULL;
    }
    virtual nametype_symbol *get_nametype_symbol() {
        fatal("Illegal downcasting to nametype from symbol class");
        return NULL;
    }

    // Allow us to print a symbol by sending it to an outstream.
    friend ostream &operator<<(ostream &, symbol *);
};


/*! Derived symbol type, used for constants. */
class constant_symbol: public symbol
{
protected:
    virtual void print(ostream &);

public:
    /*!

     Value of a constant, can be int or float.
     The value is stored in a union; check the ``#type`` attribute to figure out if
     you should access ``ivar`` or ``rvar``.
     */
    constant_value const_value;

    // Constructor. Arguments: Identifier.
    constant_symbol(const pool_index);

    // Used to be able to safely downcast from symbol to constant_symbol.
    virtual constant_symbol *get_constant_symbol() {
        return this;
    }
};


/*! Derived symbol type, used for variables. */
class variable_symbol: public symbol
{
protected:
    virtual void print(ostream &);

public:
    // Constructor. Args: identifier.
    variable_symbol(const pool_index);

    // Used to be able to safely downcast from symbol to variable_symbol.
    virtual variable_symbol *get_variable_symbol() {
        return this;
    }
};


/*! \brief Derived symbol type, used for arrays.

Study the following declaration:

\verbatim embed:rst
::

  var arr : array[10] of real;
\endverbatim

The type attribute (see above) in this case is an index to the name type
``real``. We see that we also need an attribute, ``#array_cardinality``,
which in this example would have the value ``10``. To be slightly
more general, we also have an attribute, ``#index_type``, which at
present can only point to the name type ``#integer_type``.
 */
class array_symbol: public symbol
{
protected:
    virtual void print(ostream &);

public:
    //! Points to the index type in the symbol table.
    sym_index index_type;

    //! Note: cardinality = nr of elements,
    int array_cardinality;

    // Constructor. Args: identifier.
    array_symbol(const pool_index);

    // Used to be able to safely downcast from symbol to array_symbol.
    virtual array_symbol *get_array_symbol() {
        return this;
    }
};


/*!
Parameters require particular checking, as the formal parameters are to
be bound to the actual ones. Assume the following declaration:

\verbatim embed:rst

::

  procedure p(i : integer; r : real);

which is then called with the following actual parameters:

::

  p(22/7, 355/113);

To perform a semantic check, we must find the formal parameters (``i``
and ``r``) in the symbol table and compare their type attributes with
the actual parameters. In this example a type conflict occurs when the
value of the expression ``22/7`` (which is a ``real``) is to be
bound to the ``integer`` variable ``i``.

For this reason the formal parameters are linked together in the symbol
table. The ``preceding`` attribute points to the previous parameter in
the link chain. The procedure (or function) points to the ``last``
parameter. It might seem counter-intuitive that the parameters are
stored backwards like this, but it will actually make life easier later
on.

.. figure:: pics/diesel_6_parameter_storage.png
   :alt: The order of storing the actual paramerers.

The handling of parameters is actually one of the trickiest parts to
understand correctly in this entire lab course. Some work has already
been done for you, but you will have to do a lot of it yourself. Make
sure to read the code comments carefully when dealing with parameters,
and be sure to have this lab material handy as well. The order of the
parameters will be flipped around during compiling more than once. Once
you have that sequence clear, getting them to work will be a lot easier.

\endverbatim

 */
class parameter_symbol: public symbol
{
protected:
    virtual void print(ostream &);

public:
    //! Nr of bytes parameter needs.
    int size;

    //! Link to preceding parameter, if any.
    parameter_symbol *preceding;

    // Constructor. Args: identifier.
    parameter_symbol(const pool_index);

    // Used to be able to safely downcast from symbol to parameter_symbol.
    virtual parameter_symbol *get_parameter_symbol() {
        return this;
    }
};


/*! Derived symbol type, used for procedures. */
class procedure_symbol: public symbol
{
protected:
    virtual void print(ostream &);

public:
    /*! \brief Activation record size.
     *
     * Specifies the memory space that is needed to store the local
     * variables belonging to the procedure/function on the runtime stack.
     * A procedure that declares an array of ten integers, two reals and
     * three integers requires (10+2+3)*8 bytes.
     */
    int ar_size;

    //! The label number which is assigned to the procedure or function in assembler code.
    int label_nr;

    /*!
       List of parameters. We store them in reverse order to make type-checking
       easier later on.
     */
    parameter_symbol *last_parameter;

    // Constructor. Args: identifier.
    procedure_symbol(const pool_index);

    // Used to be able to safely downcast from symbol to procedure_symbol.
    virtual procedure_symbol *get_procedure_symbol() {
        return this;
    }
};


/*! Derived symbol type, used for functions. */
class function_symbol: public symbol
{
protected:
    virtual void print(ostream &);

public:
    /*! \brief Activation record size.
     *
     * Specifies the memory space that is needed to store the local
     * variables belonging to the procedure/function on the runtime stack.
     * A procedure that declares an array of ten integers, two reals and
     * three integers requires (10+2+3)*8 bytes.
     */
    int ar_size;

    //! The label number which is assigned to the procedure or function in assembler code.
    int label_nr;

    /*!
       List of parameters. We store them in reverse order to make type-checking
       easier later on.
     */
    parameter_symbol *last_parameter;

    // Constructor. Args: identifier.
    function_symbol(const pool_index);

    // Used to be able to safely downcast from symbol to function_symbol.
    virtual function_symbol *get_function_symbol() {
        return this;
    }
};


/*!
   Derived symbol type, used for nametypes. Since it contains no new data
   fields, we might as well use symbol directly. This subclass mainly exists
   for abstraction's sake.
  */
class nametype_symbol: public symbol
{
protected:
    virtual void print(ostream &);

public:

    // Constructor. Args: identifier.
    nametype_symbol(const pool_index);

    // Used to be able to safely downcast from symbol to nametype_symbol.
    virtual nametype_symbol *get_nametype_symbol() {
        return this;
    }
};


/* IO manipulators to control the level of detail output by sending a symbol
   to an ostream. NOTE: Do we really need these here, since they're already
   defined in the symbol class? - Yes, these are the ones that other classes
   have access to. */
ostream &short_symbols(ostream &);

ostream &summary_symbols(ostream &);

ostream &long_symbols(ostream &);

/* Allow a node to be sent to an outstream for printing. */
ostream &operator<<(ostream &, symbol *);




/******************************
 *** THE SYMBOL TABLE CLASS ***
 ******************************/


/* The symbol table. Presents an interfaced used by parser.y.
   The idea is that in parser.y, the code will look something like this:
   NOTE: Fix this comment.
   parameter   :   id ':' type
            {
    // Some way to check non-null, anyway.
                if (sym_tab->lookup_symbol($1->get_sym_ptr()) != NULL_SYM) {
        error() << $1 << " already defined\n" << flush;
                }
    sym_tab->add_parameter($1->get_pos(),
                           $1->get_id(),
               $3->get_type());
            }
            ;
*/
class symbol_table
{
private:
    // --- String pool variables. ---

    char *string_pool; /*!< \brief The actual string pool. */

    // Keep track of dynamic pool size.
    long pool_length;

    // Points to end of string pool
    long pool_pos;

    // --- Hash table variables. ---

    // The actual hash table.
    sym_index *hash_table;

    // --- Display variables. ---

    block_level current_level; /*!< \brief Current nesting depth. */

    /*!
     * Table of level sym_index pointers. They point at
     * the start of a new scope/block.
     */
    sym_index *block_table;

    // --- Symbol table variables. ---

    // The actual symbol table.
    symbol **sym_table;

    // Points to last symbol entered in the table.
    sym_index sym_pos;

    // Assembler label counter.
    int label_nr;

    // Temp variable counter.
    long temp_nr;

public:
    // NOTE: Some of these methods should be made private.

    symbol_table();

    // --- Utility methods. ---

    // Convert a double to ieee 64-bit represented as a long
    long ieee(double);

    // Return type byte size.
    int get_size(const sym_index);

    // --- String pool methods. ---
    /*!
    * Finds existing entry of character in string pool, used for shared strings.
    */
    pool_index find_existing_entry(char *);

    /*!
     Install a string (an identifier or a string constant) in the
     string pool. Returns the index to the installed string.
     */
    pool_index pool_install(char *);

    /*!
     Given a ``::pool_index`` into the string pool, returns the string it
     points to.
     */
    char *pool_lookup(const pool_index);

    /*!
     Compare two strings taking their respective ``::pool_index`` as arguments.
     Returns true if they are identical, and false otherwise. Note that
     it is not enough to just check that the indices are identical. Why
     not?
     */
    bool pool_compare(const pool_index, const pool_index);

    /*!
     Remove last installed entry from the string pool. This will only
     be useful if you opt to implement shared strings.
     */
    pool_index pool_forget(const pool_index);

    /*! \brief Remove double ``''`` in strings constants.

      Internalizes a string constant. You hopefully use this method in
      several labs.
     */
    char *fix_string(const char *);

    /*!
      Capitalizes a string. You hopefully use this method in
      several labs.
      */
    char *capitalize(const char *);

    // --- Hash table methods. ---

    //! Given a ``::pool_index`` to a string, return its hash index.
    hash_index hash(const pool_index);

    // --- Symbol table methods. ---

    symbol *get_symbol(const sym_index);

    /*! \brief Installs a symbol in the symbol table and returns its index.
      This method installs a symbol in the symbol table and returns its
      index. If the symbol already existed at the same lexical level
      (i.e. two objects with the same identifi and value of the
      attribute level), the object will not be installed, and the index
      of the symbol that already exists is returned.

      If a symbol is installed, its tag will be set to ``::SYM_UNDEF``
      (which is done by default in the symbol constructor). The
      appropriate tag will then be set in the ``enter_`` method that
      called ``symbol_table::install_symbol``. In this way name collisions can be
      discovered. For instance: The variable ``m`` is declared twice on
      the same lexical level, and that is of course not correct. When the
      first ``m`` is encountered the method ``symbol_table::enter_variable`` will try to
      install the symbol in the symbol table. It calls ``symbol_table::install_symbol``
      which creates and install a symbol of the correct type. The symbol’s
      tag is now set to ``::SYM_UNDEF``. When ``symbol_table::install_symbol`` finishes,
      the execution continues inside ``symbol_table::enter_variable`` and the method
      checks if the recent installed symbol’s tag is set to ``::SYM_UNDEF``,
      and if it’s true, it changes the tag to ``::SYM_VAR``. When the second
      ``m`` is encountered the procedure is repeated, but this time
      ``symbol_table::install_symbol`` returns the ``::sym_index`` to the first ``m``.
      When checking the tag of this symbol, ``symbol_table::enter_variable`` gets the
      answer ``::SYM_VAR`` and not ``::SYM_UNDEF``. Therefore
      ``symbol_table::enter_variable`` knows that a symbol with this name already exists
      in the symbol table at this lexical level and outputs an error
      message. The second ``m`` is never installed. Furthermore, the level
      attribute will be set to the current lexical level,
      ``symbol_table#current_level`` (which is currently zero).

      Note that the first argument to ``symbol_table::install_symbol`` is an index to
      the string table (``symbol_table#string_pool``). This is, of course, what the
      scanner outputs in the ``YYSTYPE::pool_p`` field of the ``::yylval`` union. If
      you had problems understanding what it was for earlier, hopefully
      some of that has become more clear now!

      The second argument is the tag that the symbol should have, it
      denotes which symbol class should be used (``constant_symbol``,
      ``variable_symbol``, etc.).
    */
    sym_index install_symbol(const pool_index, const sym_type tag);

    /*!
        Look for a symbol with the same identifier as the
        argument specifies in the string table, and returns its symbol
        table index. The search is performed according to the scoping rules,
        i.e. from the current lexical level and outwards. Make sure you
        understand what this means! If the object is not found,
        ``::NULL_SYM`` is returned.
     */
    sym_index lookup_symbol(const pool_index);

    /*!
     Very useful method for dumping the symbol table to stdout. The value of the argument determines what info will be printed:

     \verbatim embed:rst
     1
        Print one line of info per symbol table entry.

     2
        Print (only) the string table, showing the current pool_pos.

     3
        Print (only) the non-zero elements in the hash table.

     any other
        Print detailed information about every symbol in the symbol table.
        Watch out, though: this gets very long if you have more than a few symbols installed.
     \endverbatim
     */
    void  print(int);

    //! Given a symbol table index, return its pool_index, or 0 (zero) if no such symbol existed.
    pool_index get_symbol_id(const sym_index);

    /*!
     Given a symbol table index, returns its type (``::void_type``,
     ``::integer_type``, or ``::real_type``).
     If the symbol didn’t exist, returns ``::void_type``.
     */
    sym_index get_symbol_type(const sym_index);

    /*!
     Given a symbol table index, returns its tag type
     (i.e., the tag marking which symbol class it is:
     ``::SYM_CONST``, ``::SYM_VAR``).
     */
    sym_type get_symbol_tag(const sym_index);

    /*!
    Given a symbol table index to a symbol, and a symbol table index to a type (e.g. ``::integer_type`` etc.),
    sets the symbol to that type. You shouldn’t need to use it in this lab.
    */
    void set_symbol_type(const sym_index, const sym_index);

    // These two methods are used in quads.cc.

    // Generate next asm label.
    long get_next_label();

    /*!
     Given a symbol table index to a type (e.g., ``::integer_type`` etc.),
     generates, installs and returns index to a temporary variable of that type.
     It is not meaningful to generate a temporary variable of ``::void_type``.

     Choose a method for giving the temporary variables names which can
     not collide with the user.

     \verbatim embed:rst
     .. note ::

       The implementation used for the traces returns identifiers with
       exactly length :var:`MAX_TEMP_VAR_LENGTH`.
       It pads the remainder of the identifier with spaces, giving temporary
       names such as:

       .. code-block :: cpp

         "$1      "
         "$2      "
         // ...
         "$1234   "
     \endverbatim
     */
    sym_index gen_temp_var(sym_index);

    // These functions are used to enter identifiers into the symbol table,
    // depending on their context (function, constant, etc).

    /*!
    Given position information, a ``::pool_index`` to an identifier’s name,
    a ``::sym_index`` to the desired type and an int representing the actual constant value,
    generate and install a constant of the desired type.
    */
    sym_index enter_constant(position_information *,
                             const pool_index,
                             const sym_index,
                             const long);

    /*!
    Given position information, a ``::pool_index`` to an identifier’s name,
    a ``::sym_index`` to the desired type and an int representing the actual constant value,
    generate and install a constant of the desired type.
    */
    sym_index enter_constant(position_information *,
                             const pool_index,
                             const sym_index,
                             const double);

    /*!
     Given position information, a ``::pool_index`` to an identifier’s name,
     and a sym_index to the desired type, generate and install a variable
     of the desired type.
     */
    sym_index enter_variable(position_information *,
                             const pool_index,
                             const sym_index);

    /*!
     This convenience method is used for installing temporary variables for
     which position information is not relevant. See quads.cc (lab6).
     */
    sym_index enter_variable(const pool_index, const sym_index);

    /*!
     Given position information, a ``::pool_index`` to an identifier’s name,
     a ``::sym_index`` to the desired type, and an array cardinality,
     generate and install an array of the desired type.
     */
    sym_index enter_array(position_information *,
                          const pool_index,
                          const sym_index,
                          const int);

    /*!
     Given position information, and a ``::pool_index`` to an identifier’s name,
     generate and install a function.

     The type is set later on since it will not be known at symbol install time.
     */
    sym_index enter_function(position_information *, const pool_index);

    /*!
     Given position information, and a ``::pool_index`` to an identifier’s name,
     generate and install a procedure.
     */
    sym_index enter_procedure(position_information *, const pool_index);

    /*!
     Given position information, a ``::pool_index`` to an identifier’s name,
     and a sym_index to the desired type, generate and install a parameter
     of the desired type.
     */
    sym_index enter_parameter(position_information *,
                              const pool_index,
                              const sym_index);

    /*!
     Given position information, and a ``::pool_index`` to an identifier’s name,
     generate and install a nametype of the desired type.

     Since DIESEL doesn’t allow user-defined types, you will never need
     to use this method. It is used from the symbol table class constructor
     to install the default types, though.

     NOTE: Should perhaps be marked private?
     */
    sym_index enter_nametype(position_information *, const pool_index);

    // --- Display methods. ---

    /*!
     * Returns the symbol table index to the current environment, i.e., procedure/function.
     */
    sym_index current_environment();

    /*! \brief Opens a new lexical level.

      The routine is called when
      the name of a procedure or function has been installed. Note that
      ``proc`` lies outside the lexical level that ``i`` and ``loc``
      have, in this example:

      \verbatim embed:rst
      ::

        procedure proc(i : integer);
          var loc : integer;
        begin
        end;
      \endverbatim

      The ``#block_table`` keeps track of the active blocks in the
      program. When we have treated the whole ``proc`` procedure above, we
      must make sure that the ``loc`` variable is not *“visible”*. We
      shall soon see how this is achieved.
     */
    void open_scope();

    /*! \brief Closes the current lexical level and returns a symbol table index to the new lexical level.

     The routine is called when we have finished with a procedure or function.
     The local variables that were there will then become “invisible”, i.e.,
     the following program code can not reference them.
    */
    sym_index close_scope();
};


#endif
