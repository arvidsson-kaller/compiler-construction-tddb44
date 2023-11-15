#include "semantic.hh"

semantic *type_checker = new semantic();

ostream &operator<<(ostream &oss, sym_type tag)
{
    switch (tag)
    {
        case SYM_UNDEF:
            return oss << "UNDEFINED";
        case SYM_CONST:
            return oss << "CONSTANT";
        case SYM_VAR:
            return oss << "VARIABLE";
        case SYM_ARRAY:
            return oss << "ARRAY";
        case SYM_PARAM:
            return oss << "PARAMETER";
        case SYM_PROC:
            return oss << "PROCEDURE";
        case SYM_FUNC:
            return oss << "FUNCTION";
        case SYM_NAMETYPE:
            return oss << "NAMETYPE";
        default:
            return oss;
    }
};

/* Used to check that all functions contain return statements.
   Static means that it is only visible inside this file.
   It is set to false in do_typecheck() (ie, every time we start type checking
   a new block) and set to true if we find an ast_return node. See below. */
static bool has_return = false;


/* Interface for type checking a block of code represented as an AST node. */
void semantic::do_typecheck(symbol *env, ast_stmt_list *body)
{
    // Reset the variable, since we're checking a new block of code.
    has_return = false;
    if (body) {
        body->type_check();
    }

    // This is the only case we need this variable for - a function lacking
    // a return statement. All other cases are already handled in
    // ast_return::type_check(); see below.
    if (env->tag == SYM_FUNC && !has_return) {
        // Note: We could do this by overloading the do_typecheck() method -
        // one for ast_procedurehead and one for ast_functionhead, but this
        // will do... Hopefully people won't write empty functions often,
        // since in that case we won't have position information available.
        if (body != NULL) {
            type_error(body->pos) << "A function must return a value.\n";
        } else {
            type_error() << "A function must return a value.\n";
        }
    }
}

/* Compare formal vs. actual parameters recursively. */
bool semantic::chk_param(ast_id *env,
                         parameter_symbol *formals,
                         ast_expr_list *actuals)
{
    /* Your code here */
    if (actuals == NULL && formals == NULL)
    {
        return true;
    }
    if (actuals != NULL && formals != NULL)
    {
        if (chk_param(env, formals->preceding, actuals->preceding))
        {
            if (actuals->last_expr->type_check() != formals->type)
            {
                type_error(actuals->pos) << "Type discrepancy between formal and actual parameters." << endl;
                return false;
            }
            return true;
        }
        return false;
    }
    if (actuals == NULL)
    {
        type_error(env->pos) << "More formal than actual parameters." << endl;
        return false;
    }
    if (formals == NULL)
    {
        type_error(actuals->pos) << "More actual than formal parameters." << endl;
        return false;
    }

}

/* Check formal vs. actual parameters at procedure/function calls. */
void semantic::check_parameters(ast_id *call_id,
                                ast_expr_list *param_list)
{
    /* Your code here */
    symbol *sym = sym_tab->get_symbol(call_id->sym_p);
    parameter_symbol *formals;
    if (sym->tag == SYM_FUNC)
    {
        formals = sym->get_function_symbol()->last_parameter;
    }
    else if (sym->tag == SYM_PROC) 
    {
        formals = sym->get_procedure_symbol()->last_parameter;
    } 
    else 
    {
        type_error(call_id->pos) << "Parameters sent to unsupported type: " << sym->tag << endl;
        return;
    }

    chk_param(call_id, formals, param_list);
}



/* We overload this method for the various ast_node subclasses that can
   appear in the AST. By use of virtual (dynamic) methods, we ensure that
   the correct method is invoked even if the pointers in the AST refer to
   one of the abstract classes such as ast_expression or ast_statement. */
sym_index ast_node::type_check()
{
    fatal("Trying to type check abstract class ast_node.");
    return void_type;
}

sym_index ast_statement::type_check()
{
    fatal("Trying to type check abstract class ast_statement.");
    return void_type;
}

sym_index ast_expression::type_check()
{
    fatal("Trying to type check abstract class ast_expression.");
    return void_type;
}

sym_index ast_lvalue::type_check()
{
    fatal("Trying to type check abstract class ast_lvalue.");
    return void_type;
}

sym_index ast_binaryoperation::type_check()
{
    fatal("Trying to type check abstract class ast_binaryoperation.");
    return void_type;
}

sym_index ast_binaryrelation::type_check()
{
    fatal("Trying to type check abstract class ast_binaryrelation.");
    return void_type;
}



/* Type check a list of statements. */
sym_index ast_stmt_list::type_check()
{
    if (preceding != NULL) {
        preceding->type_check();
    }
    if (last_stmt != NULL) {
        last_stmt->type_check();
    }
    return void_type;
}


/* Type check a list of expressions. */
sym_index ast_expr_list::type_check()
{
    /* Your code here */
    if (preceding != NULL)
    {
        preceding->type_check();
    }
    if (last_expr != NULL)
    {
        last_expr->type_check();
    }
    return void_type;
}



/* Type check an elsif list. */
sym_index ast_elsif_list::type_check()
{
    /* Your code here */
    if (preceding != NULL)
    {
        preceding->type_check();
    }
    if (last_elsif != NULL)
    {
        last_elsif->type_check();
    }
    return void_type;
}


/* "type check" an indentifier. We need to separate nametypes from other types
   here, since all nametypes are of type void, but should return an index to
   itself in the symbol table as far as typechecking is concerned. */
sym_index ast_id::type_check()
{
    if (sym_tab->get_symbol(sym_p)->tag != SYM_NAMETYPE) {
        return type;
    }
    return sym_p;
}


sym_index ast_indexed::type_check()
{
    /* Your code here */
    if (index->type_check() != integer_type)
    {
        type_error(pos) << "List index of non-integer type: " << index << endl;
    }
    return id->type_check();
}



/* This convenience function is used to type check all binary operations
   in which implicit casting of integer to real is done: plus, minus,
   multiplication. We synthesize type information as well. */
sym_index semantic::type_check_and_cast_real(ast_binaryoperation *operation)
{
    sym_index left_type = operation->left->type_check();
    sym_index right_type = operation->right->type_check();
    if (left_type == void_type || right_type == void_type)
    {
        type_error(operation->pos) << "Binary operations with void types" << endl;
    }

    if (left_type == right_type)
    {
        return left_type;
    }
    
    if (left_type == real_type && right_type == integer_type)
    {
        operation->right = new ast_cast(operation->pos, operation->right);
    }
    else if(left_type == integer_type && right_type == real_type)
    {
        operation->left = new ast_cast(operation->pos, operation->left);
    }
    return real_type;
}

sym_index ast_add::type_check()
{
    /* Your code here */
    return type = type_checker->type_check_and_cast_real(this);
}

sym_index ast_sub::type_check()
{
    /* Your code here */
    return type = type_checker->type_check_and_cast_real(this);
}

sym_index ast_mult::type_check()
{
    /* Your code here */
    return type = type_checker->type_check_and_cast_real(this);
}

/* Divide is a special case, since it always returns real. We make sure the
   operands are cast to real too as needed. */
sym_index ast_divide::type_check()
{
    /* Your code here */
    sym_index left_type = left->type_check();
    sym_index right_type = right->type_check();
    if (left_type == integer_type)
    {
        left = new ast_cast(left->pos, left);
    }
    if (right_type == integer_type)
    {
        right = new ast_cast(right->pos, right);
    }
    return type = real_type;
}

/* This convenience method is used to type check all binary operations
   which only accept integer operands: AND, OR, MOD, DIV.
   The second argument is the name of the operator, so we can generate a
   good error message.
   All of these return integers, so we synthesize that.
   */
sym_index semantic::type_check_integers(ast_binaryoperation *operation, string s)
{
    /* Your code here */
    sym_index left_type = operation->left->type_check();
    sym_index right_type = operation->right->type_check();
    if (left_type != integer_type || right_type != integer_type)
    {
        type_error(operation->pos) << "Integer binary operation " << s << " with non-integer types" << endl;
    }
    return integer_type;
}

sym_index ast_or::type_check()
{
    /* Your code here */
    return type = type_checker->type_check_integers(this, "OR");
}

sym_index ast_and::type_check()
{
    /* Your code here */
    return type = type_checker->type_check_integers(this, "AND");
}

sym_index ast_idiv::type_check()
{
    /* Your code here */
    return type = type_checker->type_check_integers(this, "IDIV");
}

sym_index ast_mod::type_check()
{
    /* Your code here */
    return type = type_checker->type_check_integers(this, "MOD");
}



/* Convienience method for all binary relations, since they're all typechecked
   the same way. They all return integer types, 1 = true, 0 = false. */
sym_index semantic::check_binrel(ast_binaryrelation *relation)
{
    sym_index left_type = relation->left->type_check();
    sym_index right_type = relation->right->type_check();
    if (left_type == void_type || right_type == void_type)
    {
        type_error(relation->pos) << "Binary relation with void types" << endl;    
    }

    if (left_type == right_type)
    {
        return integer_type;
    }
    
    if (left_type == real_type && right_type == integer_type)
    {
        relation->right = new ast_cast(relation->pos, relation->right);
    }
    else if(left_type == integer_type && right_type == real_type)
    {
        relation->left = new ast_cast(relation->pos, relation->left);
    }
    return integer_type;
}

sym_index ast_equal::type_check()
{
    /* Your code here */
    return type_checker->check_binrel(this);
}

sym_index ast_notequal::type_check()
{
    /* Your code here */
    return type_checker->check_binrel(this);
}

sym_index ast_lessthan::type_check()
{
    /* Your code here */
    return type_checker->check_binrel(this);
}

sym_index ast_greaterthan::type_check()
{
    /* Your code here */
    return type_checker->check_binrel(this);
}



/*** The various classes derived from ast_statement. ***/

sym_index ast_procedurecall::type_check()
{
    /* Your code here */
    type_checker->check_parameters(this->id, this->parameter_list);
    return void_type;
}


sym_index ast_assign::type_check()
{
    /* Your code here */
    sym_index left_type = lhs->type_check();
    sym_index right_type = rhs->type_check();
    if (left_type == integer_type && right_type == real_type)
    {
        type_error(rhs->pos) << "Can't assign a real value to an integer variable." << endl;
    }
    if (left_type == real_type && right_type == integer_type)
    {
        rhs = new ast_cast(rhs->pos, rhs);
    }
    return void_type;
}


sym_index ast_while::type_check()
{
    if (condition->type_check() != integer_type) {
        type_error(condition->pos) << "while predicate must be of integer "
                                   << "type.\n";
    }

    if (body != NULL) {
        body->type_check();
    }
    return void_type;
}


sym_index ast_if::type_check()
{
    /* Your code here */
    condition->type_check();
    if (body != NULL)
    {
        body->type_check();
    }
    if (elsif_list != NULL)
    {
        elsif_list->type_check();    
    }
    if (else_body != NULL)
    {
        else_body->type_check();
    }
    return void_type;
}


sym_index ast_return::type_check()
{
    // This static global (meaning it is global for all methods in this file,
    // but not visible outside this file) variable is reset to 0 every time
    // we start type checking a new block of code. If we find a return
    // statement, we set it to 1. It is checked in the do_typecheck() method
    // of semantic.cc.
    has_return = true;

    // Get the current environment. We don't yet know if it's a procedure or
    // a function.
    symbol *tmp = sym_tab->get_symbol(sym_tab->current_environment());
    if (value == NULL) {
        // If the return value is NULL,
        if (tmp->tag != SYM_PROC)
            // ...and we're not inside a procedure, something is wrong.
        {
            type_error(pos) << "Must return a value from a function.\n";
        }
        return void_type;
    }

    sym_index value_type = value->type_check();

    // The return value is not NULL,
    if (tmp->tag != SYM_FUNC) {
        // ...so if we're not inside a function, something is wrong too.
        type_error(pos) << "Procedures may not return a value.\n";
        return void_type;
    }

    // Now we know it's a function and can safely downcast.
    function_symbol *func = tmp->get_function_symbol();

    // Must make sure that the return type matches the function's
    // declared return type.
    if (func->type != value_type) {
        type_error(value->pos) << "Bad return type from function.\n";
    }

    return void_type;
}


sym_index ast_functioncall::type_check()
{
    /* Your code here */
    symbol *sym = sym_tab->get_symbol(id->sym_p);
    if (sym->tag != SYM_FUNC)
    {
        type_error(pos) << sym->tag << " not of function type " << endl;
        return void_type;
    }    

    type_checker->check_parameters(id, parameter_list);
    return sym->type;
}

sym_index ast_uminus::type_check()
{
    /* Your code here */
    sym_index type1 = expr->type_check();
    if (type1 == void_type)
    {
        type_error(pos) << "Unary of type void" << endl;
    }
    
    return type = type1;
}

sym_index ast_not::type_check()
{
    /* Your code here */
    sym_index type = expr->type_check();
    if (type != integer_type)
    {
        type_error(pos) << "Logical negation on non-integer" << endl;
    }
    return integer_type;
}


sym_index ast_elsif::type_check()
{
    condition->type_check();
    if (body != NULL)
    {
        body->type_check();
    }
    return void_type;
}



sym_index ast_integer::type_check()
{
    return integer_type;
}

sym_index ast_real::type_check()
{
    return real_type;
}
