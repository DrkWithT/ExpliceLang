#include <array>
#include <format>
#include <sstream>
#include <utility>
#include <variant>
#include "semantics/analysis.hpp"

namespace XLang::Semantics {
    static constexpr std::array<std::string_view, static_cast<std::size_t>(OpTag::last)> op_tag_names = {
        "(none)",
        "assignment",
        "field access",
        "negation",
        "multiplication",
        "division",
        "addition",
        "subtraction",
        "equality",
        "inequality",
        "less than",
        "greater than",
        "logical and",
        "logical or",
    };

    static constexpr std::array<std::string_view, static_cast<std::size_t>(TypeTag::last)> type_tag_names = {
        "bool",
        "int",
        "float",
        "string",
        "(unknown)"
    };

    TypeTag unpack_underlying_type_tag(const TypeInfo& info) {
        if (std::holds_alternative<PrimitiveType>(info)) {
            return std::get<PrimitiveType>(info).item_tag;
        } else if (std::holds_alternative<ArrayType>(info)) {
            return std::get<ArrayType>(info).item_tag;
        } else if (std::holds_alternative<CallableType>(info)) {
            return unpack_underlying_type_tag(
                std::any_cast<TypeInfo>(
                    std::get<CallableType>(info).result_tag
                )
            );
        }

        return TypeTag::x_type_unknown;
    };

    std::string_view op_tag_to_name(OpTag tag) noexcept {
        return op_tag_names[static_cast<unsigned int>(tag)];
    }

    std::string type_info_to_str(const TypeInfo& typing) {
        static std::ostringstream sout;
        sout.str("");

        if (std::holds_alternative<NullType>(typing)) {
            return "(null)";
        } else if (std::holds_alternative<PrimitiveType>(typing)) {
            const auto& primitive_info = std::get<PrimitiveType>(typing);

            std::string_view qualifier = (primitive_info.readonly)
                ? "readonly"
                : "mutable";
            std::string_view item_info = type_tag_names[static_cast<unsigned int>(primitive_info.item_tag)];

            sout << qualifier << ' ' << item_info;
        } else if (std::holds_alternative<ArrayType>(typing)) {
            const auto& array_info = std::get<ArrayType>(typing);

            sout << "array[" << type_tag_names[static_cast<unsigned int>(array_info.item_tag)] << ", " << array_info.n << ']';
        } else if (std::holds_alternative<TupleType>(typing)) {
            const auto& tuple_info = std::get<TupleType>(typing);

            sout << "tuple[";

            for (const auto& item : tuple_info.item_tags) {
                sout << type_tag_names[static_cast<unsigned int>(item)] << ", ";
            }

            sout << ']';
        } else if (std::holds_alternative<CallableType>(typing)) {
            const auto& callable_info = std::get<CallableType>(typing);

            sout << type_info_to_str(typing) << '(';

            for (const auto& item : callable_info.item_tags) {
                sout << type_info_to_str(item) << ", ";
            }

            sout << ')';
        } else {
            sout << "(unknown)";
        }

        return sout.str();
    }

    bool compare_type_info(const TypeInfo& lhs_type, const TypeInfo& rhs_type) {
        if (lhs_type.index() != rhs_type.index()) {
            return false;
        }

        if (std::holds_alternative<PrimitiveType>(lhs_type)) {
            return std::get<PrimitiveType>(lhs_type).item_tag == std::get<PrimitiveType>(rhs_type).item_tag;
        } else if (std::holds_alternative<ArrayType>(lhs_type)) {
            const auto& lhs_array_type = std::get<ArrayType>(lhs_type);
            const auto& rhs_array_type = std::get<ArrayType>(rhs_type);

            return lhs_array_type.item_tag == rhs_array_type.item_tag && lhs_array_type.n == rhs_array_type.n;
        } else if (std::holds_alternative<TupleType>(lhs_type)) {
            const auto& [lhs_item_tags] = std::get<TupleType>(lhs_type);
            const auto& [rhs_item_tags] = std::get<TupleType>(rhs_type);

            if (lhs_item_tags.size() != rhs_item_tags.size()) {
                return false;
            }

            for (auto item_pos = 0UL; item_pos < lhs_item_tags.size(); ++item_pos) {
                if (lhs_item_tags[item_pos] != rhs_item_tags[item_pos]) {
                    return false;
                }
            }

            return true;
        } else if (std::holds_alternative<CallableType>(lhs_type)) {
            const auto& [lhs_callable_inputs, lhs_callable_out] = std::get<CallableType>(lhs_type);
            const auto& [rhs_callable_inputs, rhs_callable_out] = std::get<CallableType>(rhs_type);

            /// Pre-Check: arity and return type must match...
            if (lhs_callable_out.type() != rhs_callable_out.type() || lhs_callable_inputs.size() != rhs_callable_inputs.size()) {
                return false;
            }

            for (auto arg_pos = 0UL; arg_pos < lhs_callable_inputs.size(); ++arg_pos) {
                if (!compare_type_info(lhs_callable_inputs[arg_pos], rhs_callable_inputs[arg_pos])) {
                    return false;
                }
            }

            return true;
        }

        return true;
    }

    SemanticsPass::SemanticsPass(std::string_view source_)
    : m_scopes {}, m_locations {}, m_source {source_} {}

    SemanticDiagnoses SemanticsPass::operator()(const std::vector<Syntax::StmtPtr>& ast_decls) {
        enter_scope(); // begin processing global scope

        for (const auto& decl : ast_decls) {
            decl->accept_visitor(*this);
        }

        leave_scope(); // end processing of global scope

        return {std::move(m_result)};
    }


    std::any SemanticsPass::visit_literal(const Syntax::Literal& expr) {
        if (expr.type == TypeTag::x_type_unknown) {
            return resolve_type_from(Frontend::peek_lexeme(expr.token, m_source));
        }

        return TypeInfo {
            PrimitiveType {
                .item_tag = expr.type,
                .readonly = true
            }
        };
    }

    std::any SemanticsPass::visit_unary(const Syntax::Unary& expr) {
        const auto expr_op = expr.op;
        auto inner_info = expr.inner->accept_visitor(*this);

        if (!inner_info.has_value()) {
            record_diagnosis(std::format("Unknown / invalid type of value found in an unary {} expression.", op_tag_to_name(expr_op)), Frontend::Token {
                .tag = Frontend::LexTag::unknown,
                .start = -1,
                .length = 0,
                .line = m_locations.back().line,
                .column = -1
            });

            throw std::logic_error {""};
        }

        if (const auto real_info = std::any_cast<TypeInfo>(inner_info); !check_type_operation(expr_op, real_info)) {
            record_diagnosis(
                std::format(
                    "Invalid operation on value of type {} in {} expression.",
                    type_info_to_str(real_info),
                    op_tag_to_name(expr_op)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        return inner_info;
    }

    std::any SemanticsPass::visit_binary(const Syntax::Binary& expr) {
        const auto expr_op = expr.op;
        auto lhs_box = expr.left->accept_visitor(*this);
        auto rhs_box = expr.right->accept_visitor(*this);

        if (!lhs_box.has_value() || !rhs_box.has_value()) {
            record_diagnosis(
                std::format(
                    "Invalid / unknown type found in a binary {} expression.",
                    op_tag_to_name(expr_op)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        auto lhs_info = std::any_cast<TypeInfo>(lhs_box);
        auto rhs_info = std::any_cast<TypeInfo>(rhs_box);

        if (!check_type_operation(expr_op, lhs_info, rhs_info)) {
            record_diagnosis(
                std::format(
                    "Invalid types {} and {} for {} expression.",
                    type_info_to_str(lhs_info),
                    type_info_to_str(rhs_info),
                    op_tag_to_name(expr_op)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        return {}; // todo
    }

    std::any SemanticsPass::visit_call(const Syntax::Call& expr) {
        const auto& callee_box = resolve_type_from(expr.func_name);

        if (std::holds_alternative<NullType>(callee_box)) {
            record_diagnosis(
                std::format(
                    "Undeclared name '{}' in a call expression.",
                    expr.func_name
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        if (!std::holds_alternative<CallableType>(callee_box)) {
            record_diagnosis(
                std::format(
                    "Invalid call on '{}' which names a non-callable type.",
                    expr.func_name
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        auto callee_info = std::get<CallableType>(callee_box);
        const auto argc = expr.args.size();

        if (const auto real_argc = callee_info.item_tags.size(); argc != real_argc) {
            record_diagnosis(
                std::format(""),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        for (auto arg_pos = 0UL; arg_pos < argc; ++arg_pos) {
            auto arg_info = std::any_cast<TypeInfo>(
                expr.args[arg_pos]->accept_visitor(*this)
            );

            if (!compare_type_info(arg_info, callee_info.item_tags[arg_pos])) {
                record_diagnosis(
                    std::format(""),
                    Frontend::Token {
                        .tag = Frontend::LexTag::unknown,
                        .start = -1,
                        .length = 0,
                        .line = m_locations.back().line,
                        .column = -1
                    }
                );

                throw std::logic_error {""};
            }
        }

        return callee_info.result_tag;
    }

    std::any SemanticsPass::visit_import([[maybe_unused]] const Syntax::Import& stmt) {
        return {}; /// @todo After module support, implement cross module name resolution??
    }
    
    std::any SemanticsPass::visit_variable_decl(const Syntax::VariableDecl& stmt) {
        enter_location(m_locations.back().name, stmt.name.line);

        const auto& var_type = stmt.typing;
        auto var_init_type = std::any_cast<TypeInfo>(
            stmt.init_expr->accept_visitor(*this)
        );
        std::string_view var_name = Frontend::peek_lexeme(stmt.name, m_source);

        if (!compare_type_info(var_type, var_init_type)) {
            record_diagnosis(
                std::format(
                    "Variable '{}' of type {} cannot initialize with a value of type {}.",
                    var_name,
                    type_info_to_str(var_type),
                    type_info_to_str(var_init_type)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );

            leave_location();
            return {};
        }

        /// @note Only record a variable name once it's fully processed... This avoids semantic weirdness like const a: int = a;
        record_name(var_name, var_type);
        leave_location();

        return {};
    }

    std::any SemanticsPass::visit_function_decl(const Syntax::FunctionDecl& stmt) {
        enter_location(m_locations.back().name, stmt.name.line);
        enter_scope();

        const auto ret_type = stmt.typing;
        std::vector<TypeInfo> arg_types;

        for (const auto& [arg_type, arg_token] : stmt.args) {
            /// @todo Add re-definition check for arg names vs. other names...
            record_name(Frontend::peek_lexeme(arg_token, m_source), arg_type);
            arg_types.push_back(arg_type);
        }

        record_name(Frontend::peek_lexeme(stmt.name, m_source), CallableType {
            .item_tags = std::move(arg_types),
            .result_tag = ret_type
        });

        stmt.body->accept_visitor(*this);

        leave_scope();
        leave_location();

        return {};
    }

    std::any SemanticsPass::visit_expr_stmt(const Syntax::ExprStmt& stmt) {
        enter_location("<anonymous-expr>", m_locations.back().line + 1);

        stmt.inner->accept_visitor(*this);

        leave_location();

        return {};
    }

    std::any SemanticsPass::visit_block(const Syntax::Block& stmt) {
        for (const auto& temp_stmt : stmt.stmts) {
            temp_stmt->accept_visitor(*this);
        }

        return {};
    }

    std::any SemanticsPass::visit_return(const Syntax::Return& stmt) {
        
        // 1. lookup semantic location of the parent procedure 
        std::string_view parent_proc_name = m_locations.back().name;
        auto parent_return_type = resolve_type_from(parent_proc_name);
        
        auto returned_type = std::any_cast<TypeInfo>(
            stmt.result_expr->accept_visitor(*this)
        );
        
        enter_location("<anonymous-expr>", m_locations.back().line);

        // 2. compare type-info from the procedure with the returned type
        if (!compare_type_info(returned_type, parent_return_type)) {
            record_diagnosis(
                std::format(
                    "Invalid return of {} from a procedure returning {}.",
                    type_info_to_str(returned_type),
                    type_info_to_str(parent_return_type)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );
        }

        leave_location();

        return {};
    }

    std::any SemanticsPass::visit_if(const Syntax::If& stmt) {
        auto test_type = std::any_cast<TypeInfo>(
            stmt.test->accept_visitor(*this)
        );
        
        enter_location("<if-stmt>", m_locations.back().line + 1);

        if (!std::holds_alternative<PrimitiveType>(test_type)) {
            record_diagnosis(
                std::format(
                    "Cannot have conditionals testing non-primitive values of {}.",
                    type_info_to_str(test_type)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );

            leave_location();

            return {};
        }

        const auto& primitive_info = std::get<PrimitiveType>(test_type);

        if (primitive_info.item_tag != TypeTag::x_type_bool) {
            record_diagnosis(
                std::format(
                    "Invalid conditional expression, expected a bool but found {}.",
                    type_info_to_str(test_type)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_locations.back().line,
                    .column = -1
                }
            );
        }

        stmt.truthy_body->accept_visitor(*this);

        if (stmt.falsy_body) {
            stmt.falsy_body->accept_visitor(*this);
        }

        leave_location();

        return {};
    }


    void SemanticsPass::enter_scope() {
        m_scopes.emplace_back(Scope {});
    }

    void SemanticsPass::leave_scope() {
        m_scopes.pop_back();
    }

    void SemanticsPass::enter_location(std::string_view name, int line) {
        m_locations.emplace_back(name, line);
    }

    void SemanticsPass::leave_location() {
        m_locations.pop_back();
    }

    void SemanticsPass::record_name(std::string_view name, TypeInfo info) {
        /// @todo Handle cases of name re-declaration with some return value?
        if (m_scopes.back().contains(name)) {
            return;
        }

        m_scopes.back()[name] = SemanticEntry {
            .type = std::move(info),
            /// @todo Add value group checking for constructs like variable assignments.
            .value_group = ValuingTag::x_unknown_value
        };
    }

    bool SemanticsPass::resolve_name_existence(std::string_view name) {
        /// @note Static scoping is intended, so lookups walk up the scope stack.
        for (const auto& scope : m_scopes) {
            if (scope.contains(name)) {
                return true;
            }
        }

        return false;
    }

    TypeInfo SemanticsPass::resolve_type_from(std::string_view name) {
        for (const auto& scope : m_scopes) {
            if (scope.contains(name)) {
                return scope.at(name).type;
            }
        }

        return NullType {};
    }

    bool SemanticsPass::check_type_operation(OpTag op, const TypeInfo& arg_typing) {
        const auto op_id = static_cast<unsigned int>(op);

        const auto unary_arg_type = ([](const TypeInfo& info) {
            if (std::holds_alternative<PrimitiveType>(info)) {
                return std::get<PrimitiveType>(info).item_tag;
            } else if (std::holds_alternative<CallableType>(info)) {
                auto result_tag = std::get<CallableType>(info).result_tag;

                return (result_tag.type() == typeid(PrimitiveType))
                    ? std::any_cast<PrimitiveType>(result_tag).item_tag
                    : TypeTag::x_type_unknown;
            }

            /// @note Only primitive types or results are OK in unary expressions.
            return TypeTag::x_type_unknown;
        })(arg_typing);

        const auto arg_type_num = static_cast<unsigned int>(unary_arg_type);

        return cm_basic_type_ops[op_id][arg_type_num];
    }

    bool SemanticsPass::check_type_operation(OpTag op, const TypeInfo& lhs_typing, const TypeInfo& rhs_typing) {
        const auto op_id = static_cast<unsigned int>(op);

        const auto lhs_is_sequential = lhs_typing.index() == 2 || lhs_typing.index() == 3;
        const auto rhs_is_sequential = rhs_typing.index() == 2 || rhs_typing.index() == 3;

        TypeTag lhs_type = unpack_underlying_type_tag(lhs_typing);
        TypeTag rhs_type = unpack_underlying_type_tag(rhs_typing);

        if (const auto access_op_id = static_cast<unsigned int>(OpTag::access); op_id == access_op_id) {
            /// @note Check: sequential containers cannot be access keys but only integers can be for now.
            if (rhs_is_sequential || (lhs_is_sequential && (rhs_type != TypeTag::x_type_int))) {
                return false;
            }

            return true;
        } else if (lhs_type != TypeTag::x_type_unknown && rhs_type != TypeTag::x_type_unknown) {
            return cm_basic_type_ops[op_id][static_cast<unsigned int>(lhs_type)] == cm_basic_type_ops[op_id][static_cast<unsigned int>(rhs_type)];
        }

        return true;
    }

    void SemanticsPass::record_diagnosis(std::string message, Frontend::Token culprit) {
        m_result.emplace_back(SemanticDump {
            .message = std::move(message),
            .culprit = culprit
        });
    }
}
