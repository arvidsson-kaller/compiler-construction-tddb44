#ifndef __SEMANTIC_HH__
#define __SEMANTIC_HH__

#include "ast.hh"

class semantic;


// Defined in semantic.cc.
extern semantic *type_checker;


class semantic
{
private:
    // Check formal vs actual parameters. This function recurses down the
    // parameter lists. Called by check_parameters.
    bool chk_param(ast_id *, parameter_symbol *, ast_expr_list *);
public:
    /*! \brief Compare formal vs. actual parameters in function/procedure calls.

    It is also responsible for making sure that parameters are type-checked.
    You will have to write this method yourself, and also figure out from where to call it.
    */
    void check_parameters(ast_id *, ast_expr_list *);

    /*! \brief Initiate type checking of a block of code.

    Performs some initialisation before starting recursive type checking
    of a block.
    \param env the symbol representing the block to be checked (i.e., a function or procedure symbol)
    \param body an ``ast_stmt_list`` node representing the block body to be checked.

    The function has already been written for you.
    */
    void do_typecheck(symbol *env, ast_stmt_list *body);

    // Perform type checking on a procedure/function/program body. Note that
    // the body is represented as an ast_stmt_list. See the productions for
    // comp_stmt and subprog_decl in parser.y.
    // NOTE: This should ideally be overloaded: One for binary_operations,
    // one for binary_relations, one for conditions (new abstract ast_node?),
    // one for expressions that aren't one of the above? Hmmm...

    sym_index type_check_integers(ast_binaryoperation *, string);

    sym_index check_binrel(ast_binaryrelation *);

    sym_index type_check_and_cast_real(ast_binaryoperation *);
};


#endif
