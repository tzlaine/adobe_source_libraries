#include <adobe/adam_function.hpp>

#include <adobe/dictionary.hpp>
#include <adobe/implementation/lex_shared.hpp>
#include <adobe/implementation/token.hpp>

#include <boost/container/flat_set.hpp>


namespace adobe {

namespace {

    typedef std::pair<any_regular_t, bool> stack_variable_t;
    typedef closed_hash_map<name_t, stack_variable_t> stack_frame_t;

    struct variable_declarer_t
    {
        variable_declarer_t(sheet_t& sheet, std::vector<stack_frame_t>& stack) :
            m_sheet(sheet),
            m_stack(stack)
            {}

        void operator()(name_t name, const array_t& expression, bool const_) const
            {
                if (!m_stack.empty()) {
                    if (m_stack.back().count(name)) {
                        throw std::runtime_error(
                            make_string("Attempted to re-declare variable ", name.c_str())
                        );
                    }
                    stack_variable_t& stack_variable = m_stack.back()[name];
                    if (!expression.empty())
                        stack_variable.first = m_sheet.inspect(expression);
                    stack_variable.second = const_;
                } else {
                    if (const_) {
                        m_sheet.add_constant(name, line_position_t(), expression);
                    } else {
                        m_sheet.add_interface(name,
                                              false,
                                              line_position_t(),
                                              expression,
                                              line_position_t(),
                                              array_t());
                    }
                }
            }

        sheet_t& m_sheet;
        std::vector<stack_frame_t>& m_stack;
    };

    struct variable_getter_t
    {
        variable_getter_t(sheet_t& sheet, std::vector<stack_frame_t>& stack) :
            m_sheet(sheet),
            m_stack(stack)
            {}

        any_regular_t operator()(name_t name) const
            {
                for (auto it = m_stack.rbegin(), end_it = m_stack.rend(); it != end_it; ++it) {
                    stack_frame_t::const_iterator frame_it = it->find(name);
                    if (frame_it != it->end())
                        return frame_it->second.first;
                }
                return m_sheet.get(name);
            }

        sheet_t& m_sheet;
        std::vector<stack_frame_t>& m_stack;
    };

    struct variable_setter_t
    {
        variable_setter_t(sheet_t& sheet, std::vector<stack_frame_t>& stack) :
            m_sheet(sheet),
            m_stack(stack)
            {}

        void operator()(name_t name, const any_regular_t& value) const
            {
                for (auto it = m_stack.rbegin(), end_it = m_stack.rend(); it != end_it; ++it) {
                    stack_frame_t::iterator frame_it = it->find(name);
                    if (frame_it == it->end())
                        continue;
                    if (frame_it->second.second) {
                        throw std::runtime_error(
                            make_string("Attempted to set the value of const variable ", name.c_str())
                        );
                    }
                    frame_it->second.first = value;
                    return;
                }
                m_sheet.set(name, value);
                m_sheet.update();
            }

        sheet_t& m_sheet;
        std::vector<stack_frame_t>& m_stack;
    };

    struct scoped_stack_frame_t
    {
        scoped_stack_frame_t(std::vector<stack_frame_t>& stack) :
            m_stack(stack)
            { m_stack.push_back(stack_frame_t()); }
        ~scoped_stack_frame_t()
            { m_stack.pop_back(); }

        std::vector<stack_frame_t>& m_stack;
    };

    struct lvalue_t
    {
        lvalue_t(const variable_getter_t& get, const array_t& expression) :
            m_cell_name(expression[0].cast<name_t>()),
            m_cell_value(new any_regular_t(get(m_cell_name))),
            m_lvalue(m_cell_value.get())
            {}
        name_t m_cell_name;
        boost::shared_ptr<any_regular_t> m_cell_value;
        any_regular_t* m_lvalue;
    };

    lvalue_t evaluate_lvalue_expression(sheet_t& sheet,
                                        const variable_getter_t& get,
                                        const array_t& expression)
    {
        lvalue_t retval(get, expression);
        array_t value_stack;
        for (array_t::const_iterator it(expression.begin()); it != expression.end(); ++it) {
            name_t op;
            if (it->cast(op) && op.c_str()[0] == '.') {
                if (op == variable_k) {
                    value_stack.pop_back();
                } else if (op == index_k) {
                    // TODO: {named,numeric}_index_lookup
                    if (value_stack.back().type_info() == boost::typeindex::type_id<name_t>()) {
                        retval.m_lvalue =
                            &retval.m_lvalue->cast<dictionary_t>()[value_stack.back().cast<name_t>()];
                    } else {
                        std::size_t index(static_cast<std::size_t>(value_stack.back().cast<double>()));
                        array_t& array = retval.m_lvalue->cast<array_t>();
                        if (array.size() <= index)
                            throw std::runtime_error("lvalue index: array index out of range");
                        retval.m_lvalue = &array[index];
                    }
                    value_stack.pop_back();
                } else if (op == ifelse_k) {
                    array_t else_expr = value_stack.back().cast<array_t>();
                    value_stack.pop_back();
                    array_t then_expr = value_stack.back().cast<array_t>();
                    value_stack.pop_back();
                    bool condition =
                        sheet.inspect(value_stack.back().cast<array_t>()).cast<bool>();
                    value_stack.pop_back();
                    retval = evaluate_lvalue_expression(
                        sheet,
                        get,
                        condition ? then_expr : else_expr
                    );
                }
            } else {
                value_stack.push_back(*it);
            }
        }
        return retval;
    }

    any_regular_t exec_statement(const array_t& statement,
                                 sheet_t& local_scope,
                                 std::vector<stack_frame_t>& stack,
                                 bool in_loop,
                                 bool& block_continue,
                                 bool& block_break,
                                 bool& function_done);

    any_regular_t exec_block(array_t::const_iterator first,
                             array_t::const_iterator last,
                             sheet_t& local_scope,
                             std::vector<stack_frame_t>& stack,
                             bool in_loop,
                             bool& block_continue,
                             bool& block_break,
                             bool& function_done)
    {
        for (; first != last; ++first) {
            any_regular_t value = exec_statement(first->cast<array_t>(),
                                                 local_scope,
                                                 stack,
                                                 in_loop,
                                                 block_continue,
                                                 block_break,
                                                 function_done);
            if (block_continue || block_break)
                break;
            if (function_done)
                return value;
        }
        return any_regular_t();
    }

    any_regular_t exec_statement(const array_t& statement,
                                 sheet_t& local_scope,
                                 std::vector<stack_frame_t>& stack,
                                 bool in_loop,
                                 bool& block_continue,
                                 bool& block_break,
                                 bool& function_done)
    {
        variable_declarer_t declare(local_scope, stack);
        variable_getter_t get(local_scope, stack);
        variable_setter_t set(local_scope, stack);

        name_t op;
        statement.back().cast(op);

        if (op == assign_k) {
            any_regular_t value =
                local_scope.inspect(array_t(statement.begin() + 1, statement.end() - 1));
            lvalue_t l_value =
                evaluate_lvalue_expression(local_scope, get, statement[0].cast<array_t>());
            *l_value.m_lvalue = value;
            set(l_value.m_cell_name, *l_value.m_cell_value);
        } else if (op == const_decl_k || op == decl_k) {
            declare(statement[0].cast<name_t>(),
                    statement[1].cast<array_t>(),
                    op == const_decl_k);
        } else if (op == stmt_ifelse_k) {
            const array_t& condition_expr =
               statement[0].cast<array_t>();
            const bool condition =
                local_scope.inspect(condition_expr).cast<bool>();
            const array_t& stmt_block =
                (condition ? statement[1] : statement[2]).cast<array_t>();
            scoped_stack_frame_t local_stack_frame(stack);
            any_regular_t value = exec_block(stmt_block.begin(),
                                             stmt_block.end(),
                                             local_scope,
                                             stack,
                                             in_loop,
                                             block_continue,
                                             block_break,
                                             function_done);
            if (function_done)
                return value;
        } else if (op == simple_for_k) {
            scoped_stack_frame_t local_stack_frame(stack);
            name_t loop_var_0 = statement[0].cast<name_t>();
            declare(loop_var_0, array_t(), false);
            name_t loop_var_1 = statement[1].cast<name_t>();
            if (loop_var_1)
                declare(loop_var_1, array_t(), false);
            const any_regular_t sequence =
                local_scope.inspect(statement[2].cast<array_t>());
            const array_t& stmt_block = statement[3].cast<array_t>();
            if (sequence.type_info() == boost::typeindex::type_id<array_t>()) {
                if (loop_var_1)
                    throw std::runtime_error("Two loop variables passed to a for loop over an array");
                const array_t& array = sequence.cast<array_t>();
                for (array_t::const_iterator it = array.begin(), end_it = array.end();
                     it != end_it;
                     ++it) {
                    scoped_stack_frame_t local_stack_frame(stack);
                    set(loop_var_0, *it);
                    any_regular_t value = exec_block(stmt_block.begin(),
                                                     stmt_block.end(),
                                                     local_scope,
                                                     stack,
                                                     true,
                                                     block_continue,
                                                     block_break,
                                                     function_done);
                    block_continue = false;
                    if (block_break) {
                        block_break = false;
                        break;
                    }
                    if (function_done)
                        return value;
                }
            } else if (sequence.type_info() == boost::typeindex::type_id<dictionary_t>()) {
                const dictionary_t& dictionary =
                    sequence.cast<dictionary_t>();
                for (dictionary_t::const_iterator
                         it = dictionary.begin(), end_it = dictionary.end();
                     it != end_it;
                     ++it) {
                    if (loop_var_1) {
                        set(loop_var_0, any_regular_t(it->first));
                        set(loop_var_1, it->second);
                    } else {
                        dictionary_t value;
                        value["key"_name] = any_regular_t(it->first);
                        value["value"_name] = it->second;
                        set(loop_var_0, any_regular_t(value));
                    }
                    scoped_stack_frame_t local_stack_frame(stack);
                    any_regular_t value = exec_block(stmt_block.begin(),
                                                     stmt_block.end(),
                                                     local_scope,
                                                     stack,
                                                     true,
                                                     block_continue,
                                                     block_break,
                                                     function_done);
                    block_continue = false;
                    if (block_break) {
                        block_break = false;
                        break;
                    }
                    if (function_done)
                        return value;
                }
            }
        } else if (op == complex_for_k) {
            scoped_stack_frame_t local_stack_frame(stack);
            const array_t& vars_array = statement[0].cast<array_t>();
            for (std::size_t i = 0; i < vars_array.size(); i += 3) {
                name_t var_name = vars_array[i + 0].cast<name_t>();
                const array_t& initializer =
                    vars_array[i + 1].cast<array_t>();
                declare(var_name, initializer, false);
            }
            const array_t& condition = statement[1].cast<array_t>();
            const array_t& assignments_array =
                statement[2].cast<array_t>();
            const array_t& stmt_block = statement[3].cast<array_t>();
            const any_regular_t assign_token(assign_k);
            any_regular_t condition_result = local_scope.inspect(condition);
            while (condition_result.cast<bool>()) {
                {
                    scoped_stack_frame_t local_stack_frame(stack);
                    any_regular_t value = exec_block(stmt_block.begin(),
                                                     stmt_block.end(),
                                                     local_scope,
                                                     stack,
                                                     true,
                                                     block_continue,
                                                     block_break,
                                                     function_done);
                    block_continue = false;
                    if (block_break) {
                        block_break = false;
                        break;
                    }
                    if (function_done)
                        return value;
                }
                array_t::const_iterator it = assignments_array.begin();
                const array_t::const_iterator end_it =
                    assignments_array.end();
                while (it != end_it) {
                    array_t::const_iterator assign_it =
                        std::find(it, end_it, assign_token);
                    ++assign_it;
                    exec_statement(array_t(it, assign_it),
                                   local_scope,
                                   stack,
                                   true,
                                   block_continue,
                                   block_break,
                                   function_done);
                    assert(!block_continue && !block_break && !function_done);
                    it = assign_it;
                }
                condition_result = local_scope.inspect(condition);
            }
        } else if (op == continue_k) {
            if (!in_loop)
                throw std::runtime_error("continue statement outside of loop");
            block_continue = true;
        } else if (op == break_k) {
            if (!in_loop)
                throw std::runtime_error("break statement outside of loop");
            block_break = true;
        } else if (op == return_k) {
            any_regular_t value =
                local_scope.inspect(array_t(statement.begin(), statement.end() - 1));
            function_done = true;
            return value;
        }

        return any_regular_t();
    }

    void common_init(
        const adam_function_t::array_function_lookup_t& array_function_lookup,
        const adam_function_t::dictionary_function_lookup_t& dictionary_function_lookup,
        const adam_function_t::adam_function_lookup_t& adam_function_lookup,
        const variable_getter_t& get,
        sheet_t& local_scope
    ) {
        local_scope.machine_m.set_array_function_lookup(array_function_lookup);
        local_scope.machine_m.set_dictionary_function_lookup(dictionary_function_lookup);
        local_scope.machine_m.set_adam_function_lookup(adam_function_lookup);
        local_scope.machine_m.set_variable_lookup(get);
    }

    any_regular_t common_impl(sheet_t& local_scope,
                              std::vector<stack_frame_t>& stack,
                              const std::vector<array_t>& statements)
    {
        for (std::vector<array_t>::const_iterator
                 it = statements.begin(),
                 end_it = statements.end();
             it != end_it;
             ++it) {
            bool function_done = false;
            bool block_continue = false;
            bool block_break = false;
            any_regular_t value = exec_statement(*it,
                                                 local_scope,
                                                 stack,
                                                 false,
                                                 block_continue,
                                                 block_break,
                                                 function_done);
            if (function_done)
                return value;
        }
        return any_regular_t();
    }

}

adam_function_t::adam_function_t()
{}

adam_function_t::adam_function_t(name_t name,
                                 const std::vector<name_t>& parameter_names,
                                 const std::vector<array_t>& statements) :
    m_function_name(name),
    m_parameter_names(parameter_names),
    m_statements(statements)
{
    boost::container::flat_set<name_t> declared_vars(m_parameter_names.begin(),
                                                     m_parameter_names.end());
    for (std::size_t i = 0; i < m_statements.size(); ++i) {
        name_t op_name;
        m_statements[i][m_statements[i].size() - 1].cast(op_name);
        if (op_name == decl_k || op_name == const_decl_k) {
            declared_vars.insert(m_statements[i][0].cast<name_t>());
        } else if (op_name == assign_k) {
            const array_t lvalue = m_statements[i][0].cast<array_t>();
            for (std::size_t j = 0; j < lvalue.size(); ++j) {
                name_t op;
                if (lvalue[j].cast(op) && op == variable_k) {
                    assert(j);
                    name_t var = lvalue[j - 1].cast<name_t>();
                    if (declared_vars.find(var) == declared_vars.end()) {
                        throw_parser_exception(
                            make_string(
                                m_function_name.c_str(),
                                "(): Assignment to unknown variable ",
                                var.c_str()
                            ).c_str(),
                            line_position_t()
                        );
                    }
                }
            }
        }
        for (array_t::const_iterator
                 it = m_statements[i].begin(),
                 end_it = m_statements[i].end();
             it != end_it;
             ++it) {
            if (it->type_info() == boost::typeindex::type_id<name_t>()) {
                array_t::const_iterator next_it = boost::next(it);
                name_t name;
                if (next_it != end_it && next_it->cast(name) && name == variable_k) {
                    name_t var = it->cast<name_t>();
                    if (declared_vars.find(var) == declared_vars.end()) {
                        throw_parser_exception(
                            make_string(
                                m_function_name.c_str(),
                                "(): Use of unknown variable ",
                                var.c_str()
                            ).c_str(),
                            line_position_t()
                        );
                    }
                }
            }
        }
    }
}

name_t adam_function_t::name() const
{ return m_function_name; }

const std::vector<name_t>& adam_function_t::parameter_names() const
{ return m_parameter_names; }

const std::vector<array_t>& adam_function_t::statements() const
{ return m_statements; }

any_regular_t adam_function_t::operator()(
    const array_function_lookup_t& array_function_lookup,
    const dictionary_function_lookup_t& dictionary_function_lookup,
    const adam_function_lookup_t& adam_function_lookup,
    const array_t& parameters
) const
{
    sheet_t local_scope;
    std::vector<stack_frame_t> stack;

    common_init(array_function_lookup,
                dictionary_function_lookup,
                adam_function_lookup,
                variable_getter_t(local_scope, stack),
                local_scope);

    for (std::size_t i = 0; i < m_parameter_names.size(); ++i) {
        if (i < parameters.size()) {
            local_scope.add_interface(m_parameter_names[i],
                                      false,
                                      line_position_t(),
                                      array_t(1, parameters[i]),
                                      line_position_t(),
                                      array_t());
        } else {
            local_scope.add_interface(m_parameter_names[i],
                                      false,
                                      line_position_t(),
                                      array_t(),
                                      line_position_t(),
                                      array_t());
        }
    }

    return common_impl(local_scope, stack, m_statements);
}

any_regular_t adam_function_t::operator()(
    const array_function_lookup_t& array_function_lookup,
    const dictionary_function_lookup_t& dictionary_function_lookup,
    const adam_function_lookup_t& adam_function_lookup,
    const dictionary_t& parameters
) const
{
    sheet_t local_scope;
    std::vector<stack_frame_t> stack;

    common_init(array_function_lookup,
                dictionary_function_lookup,
                adam_function_lookup,
                variable_getter_t(local_scope, stack),
                local_scope);

    for (std::size_t i = 0; i < m_parameter_names.size(); ++i) {
        dictionary_t::const_iterator it =
            parameters.find(m_parameter_names[i]);
        if (it != parameters.end()) {
            local_scope.add_interface(m_parameter_names[i],
                                      false,
                                      line_position_t(),
                                      array_t(1, it->second),
                                      line_position_t(),
                                      array_t());
        } else {
            local_scope.add_interface(m_parameter_names[i],
                                      false,
                                      line_position_t(),
                                      array_t(),
                                      line_position_t(),
                                      array_t());
        }
    }

    return common_impl(local_scope, stack, m_statements);
}

}
