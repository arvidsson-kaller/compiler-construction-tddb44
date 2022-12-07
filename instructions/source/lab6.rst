Lab 6: Intermediary Code
========================

In this lab we will study intermediary code generation, especially the
representation called *quadruples*.

Introduction
------------

Intermediary code:

* is closer to machine code without being machine-dependent.

* can handle temporary variables (intermediate storage of results).

* means higher portability. Intermediary code can easily be expanded to,
  for example, assembler – which we will study in the next lab.

* offers the possibility of performing code optimization such as code
  transposition and register allocation.

Various types of intermediary code are:

* Infix notation

* Postfix notation (reverse Polish notation)

* Three-address code:

  -  Triples

  -  Quadruples

The intermediary code we will use is quadruples where each instruction
has four fields:

.. code-block :: none

   operator    operand1    operand2    result

``operand1`` and ``operand2`` denote the operands and ``result`` denotes the
result of the operation, usually a temporary variable. The temporary
variables can be stored in registers or on the stack. In the lab the
temporary variables will be stored together with the local variables on
the stack.

Example: ::

  (A + B) * (C + D) - E

.. table:: Intermediate code generation using quadruples

   +------------+------------+------------+----------+
   | *Operator* | *Operand1* | *Operand2* | *Result* |
   +============+============+============+==========+
   |   \+       |   A        |   B        |   T1     |
   +------------+------------+------------+----------+
   |   \+       |   C        |   D        |   T2     |
   +------------+------------+------------+----------+
   |   \*       |   T1       |   T2       |   T3     |
   +------------+------------+------------+----------+
   |   \-       |   T3       |   E        |   T4     |
   +------------+------------+------------+----------+

where ``T1``, ``T2``, ``T3``, and ``T4`` are temporary variables.

As you perhaps have worked out, ``operand1``, ``operand2`` and ``result`` are
often indexes in the symbol table.

Specifications for DIESEL
-------------------------

Traversing the abstract syntax tree, after type checking and
optimization has been performed, one block at a time, generates
quadruples.

.. digraph :: diesel_lab6_ast_ex1
   :alt: Example of AST tree.
   :align: center

   node [fontname="Courier New"];
   ":=" -> A;
   ":=" -> idiv;
   idiv -> "+";
   "+" -> B;
   "+" -> C;
   idiv -> D;

The code given as an AST tree above is translated to:

    +-----------+----+----+----+
    | q_iplus   | 10 | 11 | 13 |
    +-----------+----+----+----+
    | q_idiv    | 13 | 12 | 14 |
    +-----------+----+----+----+
    | q_iassign | 14 | 0  | 9  |
    +-----------+----+----+----+

The numbers are :type:`sym_index` positions given in the following symbol table:

    +-------+--------+--------+--------+--------+--------+
    |   9   |   10   |   11   |   12   |   13   |   14   |
    +=======+========+========+========+========+========+
    |   A   |   B    |   C    |   D    |   T1   |   T2   |
    +-------+--------+--------+--------+--------+--------+

Note that operations are typed, i.e. there are both :enumerator:`q_iplus` (integer
addition) and :enumerator:`q_rplus` (real addition). The operation to select depends
on the node type if it is an arithmetic operation, but on the children’s
types if it is a relational operation. For example, if the tree node
operation is :class:`ast_add` and both operands are of integer type, we can use
:enumerator:`q_iplus`.

Handling real numbers
~~~~~~~~~~~~~~~~~~~~~

When generating assembler code, life will be a lot easier if we can assume that all real numbers are stored in 64 bits.
We can do this by storing real numbers as integers, in the IEEE format.
This is done by some pointer conversion magic in :func:`symbol_table::ieee()`, which is already written.
It takes as argument a double number, and returns a long representing it in the 64-bit IEEE format.

Whenever you generate a quad representing or treating a real number, such as a :enumerator:`q_rload` quad below, you will need to first convert the real number (which is of type ``double`` in our C++ program) to an IEEE integer. This is done quite conveniently:

.. code-block :: cpp

   sym_tab->ieee(value); // value is the double you want to convert.

Quadruple operations
~~~~~~~~~~~~~~~~~~~~

``(operation, operand1, operand2, result)``
  This is the structure used when describing the 39 different quadruple
  operations required for DIESEL. The result can usually also be considered the
  destination.

Load operations
+++++++++++++++

``(q_rload, long, -, sym_index)``
  Loads a temporary variable with a real number represented as a
  64-bit integer in the IEEE format. Note that the operand is
  interpreted as a real number, not as an index to the symbol table. The
  operation is needed to describe expressions such as 3.14 \* D.

  If :enumerator:`q_rload` did not exist, how could we know what it says here?:

  ``(q_rmult, 10785233, 114, 143)``

  Instead we generate the quadruples:

  ``(q_rload, 10785233, -, 143)`` [1]_

  ``(q_rmult, 143, 114, 144)``

  where we can determine that ``143`` is an index to the symbol table
  for a temporary variable which contains ``3.14``. The alternative
  would be to introduce extra fields in the operands specifying how they
  are to be interpreted.

``(q_iload, long, -, sym_index)``
  Loads a temporary variable with an integer constant, in a similar way to q_rload.

Store operations
++++++++++++++++
Store quadruples are usually preceded by an address calculation for an
array element (see :enumerator:`q_lindex`).

``(operation, sym_index, -, address)``
  .. list-table ::
    :header-rows: 1

    * - Operation
      - Description
    * - :enumerator:`q_rstore`
      - Assignment of a real number value to memory address.
    * - :enumerator:`q_istore`
      - Assignment of an integer value to memory address.

Unary operations
++++++++++++++++

``(operation, sym_index, -, sym_index)``
  .. list-table ::
    :header-rows: 1

    * - Operation
      - Description
    * - :enumerator:`q_inot`
      - Logical negation.
    * - :enumerator:`q_ruminus`
      - Unary minus for real numbers.
    * - :enumerator:`q_iuminus`
      - Unary minus for integers.
    * - :enumerator:`q_rassign`
      - Assignment of real value to real variable.
    * - :enumerator:`q_iassign`
      - Assignment of integer value to integer variable.
    * - :enumerator:`q_itor`
      - Type conversion from integer to real.

Binary operations
+++++++++++++++++

Note that logical operations and comparisons return ``1`` for true
and ``0`` for false although the input considers any non-zero argument
to be true.

``(operation, sym_index, sym_index, sym_index)``
  .. list-table ::
    :header-rows: 1

    * - Operation
      - Description
    * - :enumerator:`q_rplus`
      - Real number addition.
    * - :enumerator:`q_iplus`
      - Integer addition.
    * - :enumerator:`q_rminus`
      - Real number subtraction
    * - :enumerator:`q_iminus`
      - Integer subtraction.
    * - :enumerator:`q_ior`
      - Logical “or”.
    * - :enumerator:`q_iand`
      - Logical “and”.
    * - :enumerator:`q_rmult`
      - Real number multiplication.
    * - :enumerator:`q_imult`
      - Integer multiplication.
    * - :enumerator:`q_rdivide`
      - Real number division.
    * - :enumerator:`q_idivide`
      - Integer division.
    * - :enumerator:`q_imod`
      - Integer remainder.
    * - :enumerator:`q_req`
      - Real equality comparison.
    * - :enumerator:`q_ieq`
      - Integer equality comparison.
    * - :enumerator:`q_rne`
      - Real inequality comparison.
    * - :enumerator:`q_ine`
      - Integer inequality comparison.
    * - :enumerator:`q_rlt`
      - Real less than comparison.
    * - :enumerator:`q_ilt`
      - Integer less than comparison.
    * - :enumerator:`q_rgt`
      - Real greater than comparison.
    * - :enumerator:`q_igt`
      - Integer greater than comparison.

Subroutines
+++++++++++

``(q_call, sym_index, no.params, sym_index)``
  :enumerator:`q_call` is the call of a procedure or function whose position in the symbol table is given by ``operand1`` and the number of parameters is given by ``operand2``.
  The quadruple is preceded by zero or more parameters (see :enumerator:`q_param`).
  A function returns the value in result.
  For a procedure the result should be :var:`NULL_SYM`.

``(operation, labelno., sym_index, -)``
  Returns a value from a function.
  ``operand2`` is the function’s result value.
  ``operand1`` specifies the number on the label which is placed at the end of the function (:member:`~quad_list::last_label`).

  .. list-table ::
    :header-rows: 1

    * - Operation
      - Description
    * - :enumerator:`q_rreturn`
      - Returns a real number.
    * - :enumerator:`q_ireturn`
      - Returns an integer.

``(q_param, sym_index, -, -)``
  Specifies that ``operand1`` is to be sent as an argument (actual
  parameter) to a procedure/function.

Example:

::

      foo(a, bar(b), c);

gives us:

+---------+----+---+----+
| q_param | 11 | 0 | 0  |
+---------+----+---+----+
| q_param | 10 | 0 | 0  |
+---------+----+---+----+
| q_call  | 13 | 1 | 14 |
+---------+----+---+----+
| q_param | 14 | 0 | 0  |
+---------+----+---+----+
| q_param | 9  | 0 | 0  |
+---------+----+---+----+
| q_call  | 12 | 3 | 0  |
+---------+----+---+----+

where the symbol table is defined below:

+-----+------+------+-------+-------+------+
|  9  |  10  |  11  |  12   |  13   |  14  |
+=====+======+======+=======+=======+======+
|  A  |  B   |  C   |  FOO  |  BAR  |  T1  |
+-----+------+------+-------+-------+------+

Pay special attention to the treatment of the case above, when the
result of a function call becomes the parameter of another function.
Understanding this will come in handy in the last lab.

Array indexing
++++++++++++++

``(q_lindex, sym_index, sym_index, sym_index)``
  Calculates the address (l-value index) for a new array element.
  This is used as the assignment is made to an array element, ``operand1`` is the array, ``operand2`` holds the index into the array.
  The address is put in result and can as far as we are concerned here be stored in an integer, since DIESEL does not allow user-defined types.

``(q_rrindex, sym_index, sym_index, sym_index)``
  Real r-value indexing.
  Retrieves the value of an element in an array (of real).
  ``operand1`` is the array, ``operand2`` holds the index into the array.
  The element’s value is put in result.

``(q_irindex, sym_index, sym_index, sym_index)``
  Integer r-value indexing.
  Retrieves the value of an element in an array (of integer).
  ``operand1`` is the array, ``operand2`` holds the index into the array.
  The element’s value is put in result.

The example illustrates :enumerator:`q_lindex` and :enumerator:`q_irindex` (a is assumed to be
declared as an integer array) :

::

      a[a[1]] := a[2];

gives us:

+-----------+----+----+----+
| q_iload   | 2  | 0  | 10 |
+-----------+----+----+----+
| q_irindex | 9  | 10 | 11 |
+-----------+----+----+----+
| q_iload   | 1  | 0  | 12 |
+-----------+----+----+----+
| q_irindex | 9  | 12 | 13 |
+-----------+----+----+----+
| q_lindex  | 9  | 13 | 14 |
+-----------+----+----+----+
| q_istore  | 11 | 0  | 14 |
+-----------+----+----+----+

where the symbol table is given below:

+-----+------+------+------+------+------+
|  9  |  10  |  11  |  12  |  13  |  14  |
+=====+======+======+======+======+======+
|  A  |  T1  |  T2  |  T3  |  T4  |  T5  |
+-----+------+------+------+------+------+

Other operations
++++++++++++++++

``(q_labl, labelno., -, -)``
  Marks a position to where a jump can be made.

``(q_jmp, labelno., -, -)``
  Unconditional jump to the specified position.

``(q_jmpf, labelno., -, -)``
  Conditional jump to the specified position.
  The jump is made if ``operand2`` is zero (hence the name, "jump if false").

``(q_nop, -, -, -)``
  Illegal operation.

Implementation
--------------

In this lab, you will write the routines for converting the internal
form we have been working with so far, the *abstract syntax tree*, into
quadruples (quads).

This is done one block at a time by traversing the AST.
The quad generation is started from parser.y with a call to :func:`~ast_procedurehead::do_quads()`, which in turn starts a recursive call :func:`~ast_node::generate_quads()` which propagates down the AST for the program block we’re currently treating (if you’re starting to recognize a pattern from the semantic and optimizing labs
here, you’re definitely on to something).
This method returns a :type:`sym_index` to wherever its result was stored, if any.
A difference is that the :func:`~ast_procedurehead::do_quads()` method is declared in the classes :class:`ast_functionhead` and :class:`ast_procedurehead` instead of in a separate class, like in the two previous labs.
The reasons are that such a class would have been small enough to be trivial, and also to show a variant approach to making the interfaces between the compiler phases as clean as possible.

In some cases methods with other names than :func:`~ast_node::generate_quads()` need to be defined.
These cases can easily be found by reading the ``quads.cc`` file, and/or the ``ast.hh`` file.
Read the code comments for more information on the individual cases.
Studying the pre-written implementations of a few methods will prove extremely helpful in understanding what is going on here.

The result of the traversal is a :class:`quad_list` containing all the quads generated while traversing the AST for a program block.
New quads are added onto this list as they are created.
The :class:`quad_list` is a class of its own, declared in ``quads.hh``, which is written with future quad optimization in mind, even if this lab course currently does not include any such.

The files that you need to modify
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

quads.hh and quads.cc
   contain quad generation code for the AST nodes as
   well as the declaration and implementation of the quadruple,
   :class:`quad_list`, :class:`quad_list_element` and :class:`quad_list_iterator` classes.
   These are the files you will edit in this lab.

symtab.cc
   contains symbol table implementation.
   You will need to complete one more method in this lab.
   Other than that, use your version from the earlier labs.

Other files of interest
~~~~~~~~~~~~~~~~~~~~~~~

parser.y
   is the input file to bison.

ast.hh
   contains the definitions for the AST nodes.

ast.cc
   contains (part of) the implementations of the AST nodes.

codegen.hh and codegen.cc
   contain assembler generation code. You will not
   need to edit these files in this lab.

error.hh, error.cc, symtab.hh, symbol.cc, scanner.l, semantic.hh, semantic.cc, optimize.hh, and optimize.cc
   use your versions from
   the earlier labs.

main.cc
   this is the compiler wrapper, parsing flags and the like. Same as in
   the previous labs.

Makefile
   and **diesel** use the same files as in the last lab.

Declarations and routines available in *ast.hh*, *quads.hh*, and *quads.cc*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Type declarations
+++++++++++++++++

:type:`quad_op_type` contains the various quadruple operations.

Variable declarations
``````````````````````````````````````````

:member:`~quadruple::op_code` contains the :type:`quad_op_type` for a quadruple.

The ``operand1``, ``operand2``, and ``result`` fields described above are stored in the variables ``sym1``, ``sym2``, ``sym3``, ``int1``, ``int2``, and ``int3`` of the :class:`quadruple` class, respectively.
A more memory-efficient solution would have been to use unions for these three fields, but having them as separate variables lead to slightly less obfuscated code in the next lab.
You will have to refer to the list above (see also the comment at the top of the file ``quads.hh`` describing the quads and their arguments) to figure out which quad wants what arguments (you would have needed to do that with unions too).

:member:`quad_list::last_label` is the number of the :enumerator:`q_labl` quadruple which concludes each block.
Quadruple generation for a block always starts by the programmer deciding what number the final :enumerator:`q_labl` is to have.
Why? Well, it has to do with the ``return``-quadruples which in assembler are implemented as (forward) jumps to the block’s exit.
But the :enumerator:`q_labl` quadruple we want to jump to has not yet been generated.
To avoid ‘backpatching’ we therefore jump to the :enumerator:`q_labl` which is going to get the number :member:`~quad_list::last_label`.
It is generated eventually as the last quadruple in the block.
This is taken care of by the :func:`~ast_procedurehead::do_quads()` methods.

:member:`~quad_list::quad_nr` is used to number the quadruples.
Numbering is used when the quadruples are written in the trace files; it has no significance to the compiling itself other than to assist debugging.
:member:`~quad_list::head` and :member:`~quad_list::tail` are simply pointers to the first and last elements on a quad list.
The list needs to be able to be iterated over, both for debug printouts and for assembler expansion later on (and optimization, if it were to be implemented).

Class declarations
++++++++++++++++++

:class:`quadruple` is the class implementing the actual quads. It contains the
variables ``sym1``, ``sym2``, ... described above, and a ``quad_op`` denoting
what quad it is. The class has a few constructors which support the
various argument combinations that can arise. For the fields that don’t
require an argument, pass a :var:`NULL_SYM` to the constructor.

:class:`quad_list_element` is a wrapper class for the quads seen as list
elements, and contains links to the next quad on the list. It’s only
purpose is to keep the code representation (the :class:`quadruple` class)
separate from the list implementation (the :class:`quad_list` class).

:class:`quad_list_iterator` is a convenience class for iterating over a
:class:`quad_list`. You will not have to bother yourself with this class, all
code using it is already written.

:class:`quad_list` is the class that actually contains the list of quads. The
``+`` operator has been overloaded to make it simple to add new quads to a
quad list. Look at the prewritten methods for examples. It supports
iteration over the list by use of the :class:`quad_list_iterator` class.
It also contains the :member:`~quad_list::last_label` which is the number of the last :enumerator:`q_labl` quad the list has.
Each program block will be transformed into a :class:`quad_list`, which will in turn be passed as an argument to the assembler code generator in the next lab.
The list is created in ``parser.y`` at the appropriate places.
See that file for more information.

The following routines have already been written:

.. doxygenfunction :: ast_procedurehead::do_quads
.. doxygenfunction :: ast_functionhead::do_quads

Furthermore, the implementation for :class:`ast_while` nodes, as well as assignments (which need some special treatment) have been written for you.
Study them carefully; they will be very helpful as examples.

Program specification
~~~~~~~~~~~~~~~~~~~~~

#. Complete the empty :func:`~ast_node::generate_quads` method bodies in ``quads.cc``.

#. In file ``sym_tab.cc``: Complete the empty method body for

   .. doxygenfunction :: symbol_table::gen_temp_var(sym_index)

#. Pay special attention to the handling of parameters. Make sure they
   are generated in the correct order.

#. Try to generalise, e.g. treat all binary operators using the same
   routine, all binary relations using another routine, etc. Example:

   .. code-block :: cpp

         sym_index ast_add::generate_quads(quad_list &q) {
            return do_binaryoperation(q, q_iplus, q_rplus, this);
         }


Debugging help
~~~~~~~~~~~~~~

You can send any individual quad, as well as a quad list, directly to
``cout``. Also note that there is a flag to the ``diesel`` script, ``-q``,
which will print the quad list for each block during compiling.

How do I know it’s correct?
~~~~~~~~~~~~~~~~~~~~~~~~~~~

An example execution is included as a reference. Start out by compiling
simple test programs, and compare the lists you get to what you believe
they should look like. Test one AST node type at a time if you run into
trouble.

Reporting your work
-------------------

* For the demonstration, run the test program
  for the test case ``quadtest1.d`` and discuss your code.
* Hand in your code as described in :ref:`handing-in-results`.

Example execution
-----------------

Running ``make lab6`` will take the sample input and compare it to the
sample output. You should test your code on more examples than this.

.. code-block:: bash

  ./diesel -b -q -y ../testpgm/quadtest1.d

.. literalinclude:: ../../code/trace/quadtest1.trace
  :language: none
  :caption: trace/quadtest1.trace

.. [1]
   ``10785233`` is the decimal representation of ``3.14`` stored as a 64-bit (double-precision) float in IEEE format.
