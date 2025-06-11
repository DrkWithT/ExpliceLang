#include <utility>
#include "syntax/exprs.hpp"
#include "codegen/graph_pass.hpp"
#include "codegen/steps.hpp"
#include "frontend/token.hpp"

namespace XLang::Codegen {
    constexpr auto dud_offset = -1;
    constexpr Locator dud_locator = {
        .region = Region::none,
        .id = dud_offset
    };


    HeapAllocator::HeapAllocator()
    : m_items {}, m_free_list {} {}

    const HeapObjectInfo& HeapAllocator::lookup_info(int id) const noexcept {
        return m_items.at(id);
    }

    int HeapAllocator::allocate(Semantics::ArrayType array_tag) {
        const auto candidate_salvage_id = salvage_free_gap_id();

        if (candidate_salvage_id != dud_offset) {
            m_items[candidate_salvage_id] = std::move(array_tag);
            return candidate_salvage_id;
        }

        const auto candidate_normal_id = next_id();

        m_items[candidate_normal_id] = std::move(array_tag);

        return candidate_normal_id;
    }

    int HeapAllocator::allocate(Semantics::TupleType tuple_tag) {
        const auto candidate_salvage_id = salvage_free_gap_id();

        if (candidate_salvage_id != dud_offset) {
            m_items[candidate_salvage_id] = std::move(tuple_tag);
            return candidate_salvage_id;
        }

        const auto candidate_normal_id = next_id();

        m_items[candidate_normal_id] = std::move(tuple_tag);

        return candidate_normal_id;
    }

    bool HeapAllocator::release(int id) {
        if (m_items.find(id) == m_items.end()) {
            return false;
        }

        m_items[id] = {};
        m_free_list.push_back(id);

        return true;
    }

    int HeapAllocator::next_id() noexcept {
        static auto next = 0;

        const auto temp = next;
        ++next;

        return temp;
    }

    int HeapAllocator::salvage_free_gap_id() {
        if (m_free_list.empty()) {
            return dud_offset;
        }

        const auto result_id = m_free_list.back();
        m_free_list.pop_back();

        return result_id;
    }


    int GraphPass::next_const_id() noexcept {
        return m_const_map.size();
    }

    int GraphPass::next_func_id() noexcept {
        return m_global_func_map.size();
    }

    int GraphPass::next_param_id() noexcept {
        return m_current_params_map.size();
    }

    Locator GraphPass::new_obj_location(Semantics::ArrayType array_tag) {
        return {
            .region = Region::obj_heap,
            .id = m_heap_all.allocate(array_tag)
        };
    }
    
    Locator GraphPass::new_obj_location(Semantics::TupleType tuple_tag) {
        return {
            .region = Region::obj_heap,
            .id = m_heap_all.allocate(tuple_tag)
        };
    }

    bool GraphPass::delete_location(const Locator& loc) {
        const auto& [loc_region, loc_id] = loc;

        switch (loc_region) {
            case Region::obj_heap:
                return m_heap_all.release(loc_id);
            default:
                return false;
        }
    }

    const Locator& GraphPass::lookup_named_location(std::string_view name) const {
        /// @note Check for params. first since they're processed 1st.
        if (auto param_it = m_current_params_map.find(name); param_it != m_current_params_map.end()) {
            return param_it->second;
        }

        return m_current_name_map.at(name);
    }

    Locator GraphPass::lookup_callable_name(std::string_view name) const {
        if (m_native_hints_p->contains(name)) {
            return Locator {
                .region = Region::natives,
                .id = m_native_hints_p->at(name).id
            };
        }

        return m_global_func_map.at(name);
    }

    void GraphPass::commit_current_consts() {
        m_func_consts.emplace_back(m_const_map);
    }

    void GraphPass::update_stack_score_delta(const StepUnion& step) {
        const auto step_op = ([](const StepUnion& step) {
            const auto step_box_tag = step.index();

            if (step_box_tag == 0) {
                return std::get<NonaryStep>(step).op;
            } else if (step_box_tag == 1) {
                return std::get<UnaryStep>(step).op;
            } else if (step_box_tag == 2) {
                return std::get<BinaryStep>(step).op;
            } else if (step_box_tag == 3) {
                return std::get<TernaryStep>(step).op;
            }

            return VM::Opcode::xop_noop;
        })(step);

        const auto opcode_id = static_cast<unsigned int>(step_op);

        if (const auto delta = m_op_stack_deltas[opcode_id]; delta != -100) {
            m_stack_score += delta;
        } else {
            m_stack_score = 0;
        }
    }

    void GraphPass::leave_record() {
        m_const_map.clear();
        m_current_name_map.clear();
        m_current_params_map.clear();
        m_stack_score = 0;
    }

    void GraphPass::place_step(StepUnion step) {
        if (m_nodes.empty()) {
            return;
        }

        update_stack_score_delta(step);

        auto& working_node = m_nodes.back();

        if (working_node.index() == 0) {
            std::get<Unit>(working_node).steps.emplace_back(std::move(step));
        }
    }

    void GraphPass::place_node(NodeUnion node_box) {
        m_nodes.emplace_back(std::move(node_box));
    }

    void GraphPass::commit_nodes_to_graph(bool all_decls_done) {
        if (!m_graph) {
            return;
        }

        const int nodes_n = m_nodes.size();

        for (int node_i = 0; node_i < nodes_n; ++node_i) {
            // 1a. Get next node, check if it's a single-child unit / two-child juncture.
            auto& node = m_nodes[node_i];

            if (node.index() == 0) {
                auto& unit_ref = std::get<Unit>(node);

                /// @note 1b. Last node must have no children, so do a conditional child assignment here.
                unit_ref.next = (node_i < nodes_n - 1)
                    ? node_i + 1
                    : dud_offset;
                
                /// @note 2. Connect children as needed.
                auto curr_id = m_graph->add_node(node);
                m_graph->connect_neighbor(curr_id, unit_ref.next);
            } else {
                auto& juncture_ref = std::get<Juncture>(node);

                juncture_ref.left = (node_i < nodes_n - 1)
                    ? node_i + 1
                    : dud_offset;
                juncture_ref.right = (node_i < nodes_n - 2)
                    ? node_i + 2
                    : dud_offset;

                auto curr_id = m_graph->add_node(node);
                m_graph->connect_neighbor(curr_id, juncture_ref.left, juncture_ref.right);
            }
        }

        /// 5. Commit graph to FlowStore.
        m_result->emplace_back(std::move(*m_graph));
        m_nodes.clear();

        if (!all_decls_done) {
            m_graph = std::make_unique<FlowGraph>();
        }
    }

    std::any GraphPass::help_gen_access(const Syntax::Binary& expr) {
        auto source_box = expr.left->accept_visitor(*this);
        auto key_box = expr.right->accept_visitor(*this);
        auto source_locator = std::any_cast<Locator>(source_box);
        auto key_locator = std::any_cast<Locator>(key_box);

        place_step(BinaryStep {
            .op = VM::Opcode::xop_access_field,
            .arg_0 = source_locator,
            .arg_1 = key_locator
        });

        /// @note this result is temporary, no need for any locator!
        return {};
    }

    std::any GraphPass::help_gen_arithmetic(OpLeaning op_lean, const Syntax::Binary& expr) {
        auto get_arith_opcode = [](Semantics::OpTag op) {
            if (op == Semantics::OpTag::add) return VM::Opcode::xop_add;
            else if (op == Semantics::OpTag::subtract) return VM::Opcode::xop_sub;
            else if (op == Semantics::OpTag::multiply) return VM::Opcode::xop_mul;
            else if (op == Semantics::OpTag::divide) return VM::Opcode::xop_div;
            return VM::Opcode::xop_noop;
        };

        const auto opcode = get_arith_opcode(expr.op);

        if (opcode == VM::Opcode::xop_noop) {
            throw std::logic_error {"Invalid operator for arithmetic codegen!\n"};
        }

        if (op_lean == OpLeaning::lean_left) {
            expr.left->accept_visitor(*this);
            expr.right->accept_visitor(*this);
        } else {
            expr.right->accept_visitor(*this);
            expr.left->accept_visitor(*this);
        }

        place_step(NonaryStep {
            .op = opcode
        });

        return {};
    }

    std::any GraphPass::help_gen_compare(OpLeaning op_lean, const Syntax::Binary& expr) {
        auto get_comp_opcode = [](Semantics::OpTag op) {
            if (op == Semantics::OpTag::cmp_equ) return VM::Opcode::xop_cmp_eq;
            else if (op == Semantics::OpTag::cmp_neq) return VM::Opcode::xop_cmp_ne;
            else if (op == Semantics::OpTag::cmp_gt) return VM::Opcode::xop_cmp_gt;
            else if (op == Semantics::OpTag::cmp_lt) return VM::Opcode::xop_cmp_lt;
            return VM::Opcode::xop_noop;
        };

        const auto opcode = get_comp_opcode(expr.op);

        if (op_lean == OpLeaning::lean_left) {
            expr.left->accept_visitor(*this);
            expr.right->accept_visitor(*this);
        } else {
            expr.right->accept_visitor(*this);
            expr.left->accept_visitor(*this);
        }

        place_step(NonaryStep {
            .op = opcode
        });

        return {};
    }

    std::any GraphPass::help_gen_logical(const Syntax::Binary& expr) {
        auto get_logical_opcode = [](Semantics::OpTag op) {
            if (op == Semantics::OpTag::logic_and) return VM::Opcode::xop_log_and;
            else if (op == Semantics::OpTag::logic_or) return VM::Opcode::xop_log_or;
            return VM::Opcode::xop_noop;
        };

        const auto opcode = get_logical_opcode(expr.op);

        expr.left->accept_visitor(*this);
        expr.right->accept_visitor(*this);

        place_step(NonaryStep {
            .op = opcode
        });

        return {};
    }

    std::any GraphPass::help_gen_assign(const Syntax::Binary& expr) {
        /// @note Assignment to a variable resolves to the init-expr's result anyways... Also check the left (which is guaranteed to be a name by the grammar and semantic checks that will only allow assignments to names).
        /// @note LHS may be a Literal or Access expr.
        std::string_view lhs_name = ([&](const Syntax::Expr* expr) {
            const auto expr_arity_hint = expr->arity();
            Frontend::Token name_token = {
                .tag = Frontend::LexTag::eof,
                -1,
                0,
                -1,
                -1
            };

            if (expr_arity_hint == Syntax::ExprArity::one) {
                /// 1. LHS name literal case
                name_token = dynamic_cast<const Syntax::Literal*>(expr)->token;
            } else if (expr_arity_hint == Syntax::ExprArity::two) {
                // 2. LHS Access case (Yes, it's cursed, but see the notes above.)
                name_token = dynamic_cast<const Syntax::Literal*>(
                    dynamic_cast<const Syntax::Binary*>(expr)->left.get()
                )->token;
            }

            if (name_token.tag == Frontend::LexTag::eof) {
                throw std::logic_error {"Could not deduce LHS name in an assignment expr."};
            }

            return Frontend::peek_lexeme(name_token, m_old_src);
        })(expr.left.get());

        auto initializer_locator = expr.right->accept_visitor(*this);

        m_current_name_map[lhs_name] = Locator {
            .region = Region::temp_stack,
            .id = m_stack_score
        };

        return Locator {
            .region = Region::temp_stack,
            .id = m_stack_score
        };
    }


    GraphPass::GraphPass(std::string_view old_source, const Semantics::NativeHints* native_hints_p_) noexcept
    : m_heap_all {}, m_current_name_map {}, m_current_params_map {}, m_global_func_map {}, m_const_map {}, m_func_consts {}, m_nodes {}, m_graph {std::make_unique<FlowGraph>()}, m_result {new FlowStore {}}, m_old_src {old_source}, m_native_hints_p {native_hints_p_}, m_stack_score {0}, m_main_func_idx {dud_offset} {}

    std::any GraphPass::visit_literal(const Syntax::Literal& expr) {
        auto record_const_primitive = [this](Semantics::TypeTag tag, const Frontend::Token& primitive_token) {
            auto literal_lexeme = Frontend::peek_lexeme(primitive_token, m_old_src);
            auto literal_text = Frontend::get_lexeme(primitive_token, m_old_src);
            auto const_primitive_id = dud_offset;

            if (m_const_map.find(literal_lexeme) != m_const_map.end()) {
                const_primitive_id = m_const_map.at(literal_lexeme).id;

                place_step(UnaryStep {
                    .op = VM::Opcode::xop_load_const,
                    .arg_0 = Locator {
                        .region = Region::consts,
                        .id = const_primitive_id
                    }
                });

                return Locator {
                    .region = Region::consts,
                    .id = const_primitive_id
                };
            }


            if (tag == Semantics::TypeTag::x_type_bool) {
                const_primitive_id = next_const_id();
                m_const_map[literal_lexeme] = {
                    .data = literal_text == "true",
                    .id = const_primitive_id
                };
            } else if (tag == Semantics::TypeTag::x_type_int) {
                const_primitive_id = next_const_id();
                m_const_map[literal_lexeme] = {
                    .data = std::stoi(literal_text),
                    .id = const_primitive_id
                };
            } else if (tag == Semantics::TypeTag::x_type_float) {
                const_primitive_id = next_const_id();
                m_const_map[literal_lexeme] = {
                    .data = std::stof(literal_text),
                    .id = const_primitive_id
                };
            }

            place_step(UnaryStep {
                .op = VM::Opcode::xop_load_const,
                .arg_0 = Locator {
                    .region = Region::consts,
                    .id = const_primitive_id
                }
            });

            return Locator {
                .region = Region::consts,
                .id = const_primitive_id
            };
        };

        const auto primitive_tag = std::any_cast<Semantics::TypeTag>(expr.type_tagging());
        const auto& expr_token = expr.token;
        auto literal_lexeme = Frontend::peek_lexeme(expr_token, m_old_src);

        if (primitive_tag == Semantics::TypeTag::x_type_bool || primitive_tag == Semantics::TypeTag::x_type_int || primitive_tag == Semantics::TypeTag::x_type_float) {
            return record_const_primitive(primitive_tag, expr_token);
        } else if (primitive_tag == Semantics::TypeTag::x_type_unknown) {
            /// @note Handle identifiers here...
            auto name_loc = lookup_named_location(literal_lexeme);

            place_step(UnaryStep {
                .op = VM::Opcode::xop_push,
                .arg_0 = name_loc
            });

            return name_loc;
        }

        throw std::logic_error {"String codegen unsupported!\n"};
    }

    std::any GraphPass::visit_unary(const Syntax::Unary& expr) {
        auto inner_box = expr.inner->accept_visitor(*this);
        auto expr_op = expr.op;

        if (expr_op == Semantics::OpTag::negate) {
            if (inner_box.type() == typeid(Locator)) {
                const auto inner_locator = std::any_cast<Locator>(inner_box);

                place_step(UnaryStep {
                    .op = VM::Opcode::xop_push,
                    .arg_0 = inner_locator
                }); // push x
            }

            place_step(NonaryStep {
                .op = VM::Opcode::xop_negate
            }); // negate <top>

            return {};
        }

        throw std::logic_error {"Unsupported codegen for non-negation operation of unary <anonymous>!\n"};
    }

    std::any GraphPass::visit_binary(const Syntax::Binary& expr) {
        switch (expr.op) {
        case Semantics::OpTag::access:
            return help_gen_access(expr);
        case Semantics::OpTag::add:
        case Semantics::OpTag::multiply:
            return help_gen_arithmetic(OpLeaning::lean_left, expr);
        case Semantics::OpTag::subtract:
        case Semantics::OpTag::divide:
            return help_gen_arithmetic(OpLeaning::lean_right, expr);
        case Semantics::OpTag::cmp_equ:
        case Semantics::OpTag::cmp_neq:
            return help_gen_compare(OpLeaning::lean_left, expr);
        case Semantics::OpTag::cmp_lt:
        case Semantics::OpTag::cmp_gt:
            return help_gen_compare(OpLeaning::lean_right, expr);
        case Semantics::OpTag::logic_and:
        case Semantics::OpTag::logic_or:
            return help_gen_logical(expr);
        case Semantics::OpTag::assign:
            return help_gen_assign(expr);
        default:
            break;
        }

        throw std::logic_error {"Invalid / unsupported binary operation for codegen!\n"};
    }

    std::any GraphPass::visit_call(const Syntax::Call& expr) {
        const auto func_locator = lookup_callable_name(expr.func_name);
        auto args_n = static_cast<int>(expr.args.size());

        for (auto arg_iter = args_n - 1; arg_iter >= 0; --arg_iter) {
            expr.args[arg_iter]->accept_visitor(*this);
        }

        if (func_locator.region == Region::routines) {
            place_step(BinaryStep {
                .op = VM::Opcode::xop_call,
                .arg_0 = func_locator,
                .arg_1 = Locator {
                    .region = Region::none,
                    .id = args_n
                }
            });
        } else if (func_locator.region == Region::natives) {
            /// @todo When modules are added, add logic in semantics and codegen to resolve arg_0! (0 is for the entry TU.)
            place_step(TernaryStep {
                .op = VM::Opcode::xop_call_native,
                .arg_0 = Locator {
                    .region = Region::none,
                    .id = 0,
                },
                .arg_1 = func_locator,
                .arg_2 = Locator {
                    .region = Region::none,
                    .id = args_n
                }
            });
        } else {
            throw std::logic_error {"Invalid IR locator passed for a call expression. The name must title a used native function or defined procedure."};
        }

        return {};
    }

    std::any visit_native_use([[maybe_unused]] const Syntax::NativeUse& stmt) {
        return {};
    }

    std::any GraphPass::visit_import([[maybe_unused]] const Syntax::Import& stmt) {
        /// @todo implement module item inclusion...
        return {};
    }

    std::any GraphPass::visit_variable_decl(const Syntax::VariableDecl& stmt) {
        const auto var_init_box = stmt.init_expr->accept_visitor(*this);
        auto var_name = Frontend::peek_lexeme(stmt.name, m_old_src);
        auto var_init_locator = dud_locator;

        if (var_init_box.has_value()) {
            var_init_locator = std::any_cast<Locator>(var_init_box);
        } else {
            var_init_locator = Locator {
                .region = Region::temp_stack,
                .id = m_stack_score,
            };
        }

        m_current_name_map[var_name] = var_init_locator;

        return {}; // TODO
    }

    std::any GraphPass::visit_function_decl(const Syntax::FunctionDecl& stmt) {
        auto func_name = Frontend::peek_lexeme(stmt.name, m_old_src);

        const auto func_id = next_func_id();

        /// @note Mark which compiled routine is the main one, as it's invoked by the VM.
        if (func_name == "main") {
            m_main_func_idx = func_id;
        }

        m_global_func_map[func_name] = {
            .region = Region::routines,
            .id = func_id
        };

        place_node(Unit {
            .steps = {},
            .next = dud_offset
        });

        /// 1. Enter param. list of function
        for (const auto& arg_param : stmt.args) {
            auto param_name = Frontend::peek_lexeme(arg_param.name, m_old_src);

            m_current_params_map[param_name] = {
                .region = Region::frame_slot,
                .id = next_param_id()
            };
        }

        stmt.body->accept_visitor(*this);

        commit_current_consts();
        leave_record();

        return {};
    }

    std::any GraphPass::visit_expr_stmt(const Syntax::ExprStmt& stmt) {
        auto inner_result = stmt.inner->accept_visitor(*this);

        return {}; // TODO
    }

    std::any GraphPass::visit_block(const Syntax::Block& stmt) {
        // From an IF statement, the condition will still branch the control flow into 2 children, but 2 children have the same continuation of its nested block, but that afterward-fragment is a new node too.
        for (const auto& inner_stmt : stmt.stmts) {
            inner_stmt->accept_visitor(*this);
        }

        return {};
    }

    std::any GraphPass::visit_return(const Syntax::Return& stmt) {
        auto result_box = stmt.result_expr->accept_visitor(*this);

        if (!result_box.has_value()) {
            /// @todo: Add some logic to trace back the return value on the stack?
            place_step(UnaryStep {
                .op = VM::Opcode::xop_ret,
                .arg_0 = dud_locator,
            });
        } else if (result_box.type() == typeid(Locator)) {
            place_step(UnaryStep {
                .op = VM::Opcode::xop_ret,
                .arg_0 = std::any_cast<Locator>(result_box) 
            });
        }

        return {};
    }

    std::any GraphPass::visit_if(const Syntax::If& stmt) {
        stmt.test->accept_visitor(*this);

        place_step(UnaryStep {
            .op = VM::Opcode::xop_jump_not_if,
            .arg_0 = dud_locator
        });

        /// @note If stmt. forks control flow into T/F branches, so place a Juncture into the current control flow graph.
        place_node(Juncture {
            .left = dud_offset,
            .right = dud_offset
        });

        place_node(Unit {
            .steps = {},
            .next = dud_offset
        });
        stmt.truthy_body->accept_visitor(*this);
        place_step(UnaryStep {
            .op = VM::Opcode::xop_jump,
            .arg_0 = dud_locator
        });

        if (stmt.falsy_body) {
            place_node(Unit {
                .steps = {},
                .next = dud_offset
            });
            stmt.falsy_body->accept_visitor(*this);
        }
        place_step(NonaryStep {
            .op = VM::Opcode::xop_noop
        });

        place_node(Unit {
            .steps = {},
            .next = dud_offset
        });

        return {};
    }

    IRStore GraphPass::process(const std::vector<Syntax::StmtPtr>& ast) {
        const auto decls_n = static_cast<int>(ast.size());

        for (auto func_decl_idx = 0; func_decl_idx < decls_n; ++func_decl_idx) {
            ast[func_decl_idx]->accept_visitor(*this);

            commit_nodes_to_graph(func_decl_idx == decls_n - 1);
        }

        return {
            .const_chunks = std::move(m_func_consts),
            .func_cfgs = std::move(m_result),
            .main_func_id = m_main_func_idx
        };
    }
}