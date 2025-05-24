#include <utility>
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

    const PassTypeInfo& HeapAllocator::lookup_info(int id) const noexcept {
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

    int GraphPass::next_local_id() noexcept {
        return m_current_name_map.size();
    }

    int GraphPass::next_func_id() noexcept {
        return m_global_func_map.size();
    }

    int GraphPass::curr_const_id() noexcept {
        return next_const_id() - 1;
    }

    int GraphPass::curr_local_id() noexcept {
        return next_local_id() - 1;
    }

    int GraphPass::curr_func_id() noexcept {
        return next_func_id() - 1;
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
        return m_current_name_map.at(name);
    }

    const Locator& GraphPass::lookup_callable_name(std::string_view name) const {
        return m_global_func_map.at(name);
    }

    void GraphPass::leave_record() {
        m_current_name_map.clear();
    }

    void GraphPass::place_step(StepUnion step) {
        if (m_nodes.empty()) {
            return;
        }

        auto& todo_it = m_nodes.back();

        if (todo_it.index() == 0) {
            std::get<Unit>(todo_it).steps.emplace_back(std::move(step));
        }
    }

    void GraphPass::place_node(NodeUnion node_box) {
        m_nodes.emplace_back(std::move(node_box));
    }

    void GraphPass::commit_nodes_to_graph(int current_func_id, bool all_decls_done) {
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
        m_result->operator[](current_func_id) = std::move(*m_graph);
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
        /// @note Assignment to a variable resolves to the init-expr's result anyways I think.
        auto initializer_locator = expr.right->accept_visitor(*this);

        return initializer_locator;
    }


    GraphPass::GraphPass(std::string_view old_source)
    : m_heap_all {}, m_current_name_map {}, m_global_func_map {}, m_const_map {}, m_nodes {}, m_graph {std::make_unique<FlowGraph>()}, m_result {new FlowStore {}}, m_old_src {old_source} {}

    std::any GraphPass::visit_literal(const Syntax::Literal& expr) {
        const auto primitive_tag = std::any_cast<Semantics::TypeTag>(expr.type_tagging());
        const auto& expr_token = expr.token;
        auto literal_lexeme = Frontend::peek_lexeme(expr_token, m_old_src);
        auto literal_const_id = dud_offset;

        switch (primitive_tag) {
        case Semantics::TypeTag::x_type_bool:
        case Semantics::TypeTag::x_type_int:
        case Semantics::TypeTag::x_type_float:
            literal_const_id = next_const_id();
            m_const_map[literal_lexeme] = literal_const_id;

            place_step(UnaryStep {
                .op = VM::Opcode::xop_load_const,
                .arg_0 = Locator {
                    .region = Region::consts,
                    .id = literal_const_id
                }
            });

            return Locator {
                .region = Region::consts,
                .id = literal_const_id
            };
        case Semantics::TypeTag::x_type_unknown:
            /// @note Handle identifiers here...
            return lookup_named_location(literal_lexeme);
        default:
            break;
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

        for (int arg_it = static_cast<int>(expr.args.size()); arg_it >= 0; --arg_it) {
            expr.args[arg_it]->accept_visitor(*this);
        }

        place_step(UnaryStep {
            .op = VM::Opcode::xop_call,
            .arg_0 = func_locator
        });

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
                .id = curr_local_id()
            };
        }

        m_current_name_map[var_name] = var_init_locator;

        return {}; // TODO
    }

    std::any GraphPass::visit_function_decl(const Syntax::FunctionDecl& stmt) {
        auto func_name = Frontend::peek_lexeme(stmt.name, m_old_src);
        place_node(Unit {
            .steps = {},
            .next = dud_offset
        });

        /// 1. Enter call frame of function
        place_step(NonaryStep {
            .op = VM::Opcode::xop_enter
        });

        for (const auto& arg_param : stmt.args) {
            auto param_name = Frontend::peek_lexeme(arg_param.name, m_old_src);

            m_current_name_map[param_name] = {
                .region = Region::temp_stack,
                .id = next_local_id()
            };
        }

        stmt.body->accept_visitor(*this);

        const auto func_id = next_func_id();

        m_global_func_map[func_name] = {
            .region = Region::routines,
            .id = func_id
        };
        leave_record();

        return func_id;
    }

    std::any GraphPass::visit_expr_stmt(const Syntax::ExprStmt& stmt) {
        auto inner_result = stmt.inner->accept_visitor(*this);

        // place_step(NonaryStep {
        //     .op = VM::Opcode::xop_pop
        // });

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
            .op = VM::Opcode::xop_jump_if,
            .arg_0 = dud_locator
        });

        place_step(UnaryStep {
            .op = VM::Opcode::xop_jump,
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

        place_node(Unit {
            .steps = {},
            .next = dud_offset
        });

        if (stmt.falsy_body) {
            stmt.falsy_body->accept_visitor(*this);
        }

        place_node(Unit {
            .steps = {},
            .next = dud_offset
        });

        return {};
    }

    std::unique_ptr<FlowStore> GraphPass::process(const std::vector<Syntax::StmtPtr>& ast) {
        const auto decls_n = static_cast<int>(ast.size());

        for (auto func_decl_idx = 0; func_decl_idx < decls_n; ++func_decl_idx) {
            auto decl_id = ast[func_decl_idx]->accept_visitor(*this);

            const auto generated_func_id = std::any_cast<int>(decl_id);

            commit_nodes_to_graph(generated_func_id, func_decl_idx == decls_n - 1);
        }

        return {std::move(m_result)};
    }
}