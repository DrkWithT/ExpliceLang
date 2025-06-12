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

    /// @todo Fix unexpected unknown type tag result when visit_return is done.
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
            sout << "(null)";
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

            sout << type_info_to_str(
                std::any_cast<TypeInfo>(callable_info.result_tag)
            ) << '(';

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
    : m_native_hints {}, m_scopes {}, m_location {}, m_source {source_} {}

    SemanticResult SemanticsPass::operator()(const std::vector<Syntax::StmtPtr>& ast_decls) {
        enter_scope(); // begin processing global scope

        for (const auto& decl : ast_decls) {
            try {
                decl->accept_visitor(*this);
            } catch (const std::logic_error& error) {
                ;
            }
        }

        leave_scope(); // end processing of global scope

        return {
            .errors = std::move(m_result),
            .native_hints = std::move(m_native_hints)
        };
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
                .line = m_location.line,
                .column = -1
            });

            throw std::logic_error {""};
        }

        const auto real_info = std::any_cast<TypeInfo>(inner_info);
        const auto& [checked_info, check_ok] = check_type_operation(expr_op, real_info);

        if (!check_ok) {
            record_diagnosis(
                std::format(
                    "Invalid operation on value of type {} in {} expression, results in {} value.",
                    type_info_to_str(real_info),
                    op_tag_to_name(expr_op),
                    type_info_to_str(checked_info)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_location.line,
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
                    .line = m_location.line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        auto lhs_info = std::any_cast<TypeInfo>(lhs_box);
        auto rhs_info = std::any_cast<TypeInfo>(rhs_box);

        const auto [checked_info, check_ok] = check_type_operation(expr_op, lhs_info, rhs_info);

        if (!check_ok) {
            record_diagnosis(
                std::format(
                    "Invalid types {} and {} for {} expression, results in {} value.",
                    type_info_to_str(lhs_info),
                    type_info_to_str(rhs_info),
                    op_tag_to_name(expr_op),
                    type_info_to_str(checked_info)
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_location.line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        /// 1. Handle access expr. case...
        if (const auto lhs_type_num = lhs_info.index(); lhs_type_num >= 2) {
            return checked_info;
        } else if (expr_op >= OpTag::cmp_equ && expr_op <= OpTag::logic_or) {
            // 2. Handle logical exprs.
            return TypeInfo {
                PrimitiveType {
                    .item_tag = TypeTag::x_type_bool,
                    .readonly = true
                }
            };
        }

        /// 2. Handle plain-old arithmetic with primitives...
        return lhs_info;
    }

    std::any SemanticsPass::visit_call(const Syntax::Call& expr) {
        const auto& callee_name = expr.func_name;
        const auto& callee_box = resolve_type_from(callee_name);

        if (std::holds_alternative<NullType>(callee_box)) {
            record_diagnosis(
                std::format(
                    "Undeclared name '{}' in a call expression.",
                    callee_name
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_location.line,
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
                    .line = m_location.line,
                    .column = -1
                }
            );

            throw std::logic_error {""};
        }

        auto callee_info = std::get<CallableType>(callee_box);
        const auto argc = expr.args.size();

        if (const auto real_argc = callee_info.item_tags.size(); argc != real_argc) {
            record_diagnosis(
                std::format(
                    "Invalid argument count of {} passed to procedure '{}'.",
                    argc,
                    callee_name
                ),
                Frontend::Token {
                    .tag = Frontend::LexTag::unknown,
                    .start = -1,
                    .length = 0,
                    .line = m_location.line,
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
                    std::format(
                        "Wrong type for argument #{} of procedure '{}'.",
                        arg_pos,
                        expr.func_name
                    ),
                    Frontend::Token {
                        .tag = Frontend::LexTag::unknown,
                        .start = -1,
                        .length = 0,
                        .line = m_location.line,
                        .column = -1
                    }
                );

                throw std::logic_error {""};
            }
        }

        return callee_info.result_tag;
    }

    std::any SemanticsPass::visit_native_use(const Syntax::NativeUse& stmt) {
        auto native_name = Frontend::peek_lexeme(stmt.native_name, m_source);
        std::vector<TypeInfo> params_info;

        for (const auto& temp : stmt.args) {
            params_info.emplace_back(temp.type);
        }

        record_native_name(native_name, TypeInfo {
            CallableType {
                .item_tags = std::move(params_info),
                .result_tag = stmt.possible_result_type()
            }
        });

        return {};
    }

    std::any SemanticsPass::visit_import([[maybe_unused]] const Syntax::Import& stmt) {
        return {}; /// @todo After module support, implement cross module name resolution??
    }
    
    std::any SemanticsPass::visit_variable_decl(const Syntax::VariableDecl& stmt) {
        m_location.line = stmt.name.line;

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
                    .line = m_location.line,
                    .column = -1
                }
            );

            return {};
        }

        /// @note Only record a variable name once it's fully processed... This avoids semantic weirdness like const a: int = a;
        record_name(var_name, var_type);

        return {};
    }

    std::any SemanticsPass::visit_function_decl(const Syntax::FunctionDecl& stmt) {
        enter_scope();
        std::string_view proc_name = Frontend::peek_lexeme(stmt.name, m_source);

        m_location = {
            .name = proc_name,
            .line = stmt.name.line
        };

        const auto ret_type = stmt.typing;
        std::vector<TypeInfo> arg_types;

        for (const auto& [arg_type, arg_token] : stmt.args) {
            /// @todo Add re-definition check for arg names vs. other names...
            record_name(Frontend::peek_lexeme(arg_token, m_source), arg_type);
            arg_types.push_back(arg_type);
        }

        record_proc_name(
            proc_name,
            CallableType {
                .item_tags = std::move(arg_types),
                .result_tag = ret_type
            }
        );

        stmt.body->accept_visitor(*this);

        leave_scope();

        return {};
    }

    std::any SemanticsPass::visit_expr_stmt(const Syntax::ExprStmt& stmt) {
        stmt.inner->accept_visitor(*this);

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
        std::string_view parent_proc_name = m_location.name;
        TypeInfo parent_return_type = PrimitiveType {
            .item_tag = unpack_underlying_type_tag(resolve_type_from(parent_proc_name)),
            .readonly = true
        };

        auto returned_type = std::any_cast<TypeInfo>(
            stmt.result_expr->accept_visitor(*this)
        );

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
                    .line = m_location.line,
                    .column = -1
                }
            );
        }

        return {};
    }

    std::any SemanticsPass::visit_if(const Syntax::If& stmt) {
        auto test_type = std::any_cast<TypeInfo>(
            stmt.test->accept_visitor(*this)
        );

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
                    .line = m_location.line,
                    .column = -1
                }
            );

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
                    .line = m_location.line,
                    .column = -1
                }
            );
        }

        stmt.truthy_body->accept_visitor(*this);

        if (stmt.falsy_body) {
            stmt.falsy_body->accept_visitor(*this);
        }

        return {};
    }


    void SemanticsPass::enter_scope() {
        m_scopes.emplace_back(Scope {});
    }

    void SemanticsPass::leave_scope() {
        m_scopes.pop_back();
    }

    void SemanticsPass::record_proc_name(std::string_view name, TypeInfo info) {
        m_scopes.front()[name] = SemanticEntry {
            .type = std::move(info),
            .value_group = ValuingTag::x_unknown_value
        };
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
        if (m_native_hints.contains(name)) {
            return true;
        }

        /// @note Static scoping is intended, so lookups walk up the scope stack.
        for (const auto& scope : m_scopes) {
            if (scope.contains(name)) {
                return true;
            }
        }

        return false;
    }

    void SemanticsPass::record_native_name(std::string_view name, TypeInfo info) {
        /// @todo Handle cases of native name re-declaration?
        if (m_native_hints.contains(name)) {
            return;
        }

        const auto next_native_id = static_cast<int>(m_native_hints.size());

        m_native_hints[name] = SemanticNativeEntry {
            .signature_type = std::move(info),
            .id = next_native_id
        };
    }

    TypeInfo SemanticsPass::resolve_type_from(std::string_view name) {
        if (m_native_hints.contains(name)) {
            return m_native_hints.at(name).signature_type;
        }

        for (const auto& scope : m_scopes) {
            if (scope.contains(name)) {
                return scope.at(name).type;
            }
        }

        return NullType {};
    }

    SemanticsPass::OpTypeCheckResult SemanticsPass::check_type_operation(OpTag op, const TypeInfo& arg_typing) {
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

        return OpTypeCheckResult {
            .result_data_type = arg_typing,
            .ok = cm_basic_type_ops[op_id][arg_type_num]
        };
    }

    SemanticsPass::OpTypeCheckResult SemanticsPass::check_type_operation(OpTag op, const TypeInfo& lhs_typing, const TypeInfo& rhs_typing) {
        const auto op_id = static_cast<unsigned int>(op);

        const auto lhs_is_sequential = lhs_typing.index() == 2 || lhs_typing.index() == 3;
        const auto rhs_is_sequential = rhs_typing.index() == 2 || rhs_typing.index() == 3;

        TypeTag lhs_type = unpack_underlying_type_tag(lhs_typing);
        TypeTag rhs_type = unpack_underlying_type_tag(rhs_typing);

        if (const auto access_op_id = static_cast<unsigned int>(OpTag::access); op_id == access_op_id) {
            /// @note Check: sequential containers cannot be access keys but only integers can be for now.
            if (rhs_is_sequential || (lhs_is_sequential && (rhs_type != TypeTag::x_type_int))) {
                return {
                    .result_data_type = NullType {},
                    .ok = false
                };
            }

            return {
                .result_data_type = PrimitiveType {
                    .item_tag = lhs_type,
                    .readonly = true
                },
                .ok = false
            };
        } else if (lhs_type != TypeTag::x_type_unknown && rhs_type != TypeTag::x_type_unknown) {
            const auto ops_ok = cm_basic_type_ops[op_id][static_cast<unsigned int>(lhs_type)] == cm_basic_type_ops[op_id][static_cast<unsigned int>(rhs_type)];
            return {
                .result_data_type = PrimitiveType {
                    .item_tag = lhs_type,
                    .readonly = true
                },
                .ok = ops_ok
            };
        }

        return {
            .result_data_type = PrimitiveType {
                .item_tag = TypeTag::x_type_unknown,
                .readonly = true
            },
            .ok = true
        };
    }

    void SemanticsPass::record_diagnosis(std::string message, Frontend::Token culprit) {
        m_result.emplace_back(SemanticDump {
            .message = std::move(message),
            .culprit = culprit
        });
    }
}
