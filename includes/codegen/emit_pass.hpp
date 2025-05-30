#pragma once

#include <utility>
#include <memory>
#include <set>
#include <stack>
#include <queue>
#include "codegen/policies.hpp"
#include "codegen/steps.hpp"
#include "codegen/flow_nodes.hpp"
#include "codegen/graph_pass.hpp"
#include "vm/chunk.hpp"

namespace XLang::Codegen {
    struct Backpatch {
        int pending_units;
        int jump_if_pos;
        int backpatch_iffy;
        int jump_else_pos;
        int backpatch_else;
    };

    /**
     * @brief This class contains logic which traverses the control flow graph (basically the IR) and emits bytecode for the stack VM. The CFG is (likely) traversed forward by a BFS algorithm which prunes empty code Units.
     */
    template <typename Result = VM::Chunk, typename Policy = Codegen::DefaultPolicy>
    class EmitCodePass {
    public:
        EmitCodePass()
        : m_incoming {}, m_backpatches {}, m_visited {}, m_result {std::make_unique<Result>()}, m_constant_chunks_view {nullptr}, m_byte_count {0}, m_ir_unit_idx {0} {}

        [[nodiscard]] Result process(const FlowGraph& control_graph) {
            VM::ConstantStore temp_constants = emit_constant_region(m_constant_chunks_view->at(m_ir_unit_idx));

            std::vector<VM::RuntimeByte> temp_bytecode = emit_instruction_region(control_graph.view_nodes());

            return {
                .constants = std::move(temp_constants),
                .bytecode = std::move(temp_bytecode)
            };
        }

        /// @note Takes in properties of IRStore to create a XpliceProgram structure for the VM.
        std::unique_ptr<VM::XpliceProgram> process_full_ir(const std::vector<ProtoConstMap>& constants, const Codegen::FlowStore& cfg_dict, int entry_point_id) {
            m_constant_chunks_view = &constants;

            m_result.get()->entry_func_id = entry_point_id;

            for (const auto& [temp_cfg_id, temp_cfg] : cfg_dict) {
                m_result.get()->func_chunks.operator[](temp_cfg_id) = process(temp_cfg);
                clear_current_state();
            }

        return {std::move(m_result)};
    }

    private:
        static constexpr auto dud_pos = -1;

        /// @note Stores next nodes by ID to emit bytecode for, etc.
        std::queue<int> m_incoming;

        /// @note Stores backpatch info for earlier-encountered dud jumps.
        std::stack<Backpatch> m_backpatches;

        /// @note Stores which node IDs were visited.
        std::set<int> m_visited;

        /// @note Stores bytecode program being built: "VM-ProgramStore"
        std::unique_ptr<VM::XpliceProgram> m_result;

        const std::vector<ProtoConstMap>* m_constant_chunks_view;

        int m_byte_count;

        /// @note Tracks index of a corresponding pair of constants with their instructions from IRStore.
        int m_ir_unit_idx;

        [[nodiscard]] int current_byte_count() const& noexcept {
            return m_byte_count;
        }

        [[nodiscard]] int current_byte_count() & noexcept {
            return m_byte_count;
        }

        void clear_current_state() {
            m_incoming.swap({});
            m_backpatches.swap({});
            m_visited.clear();
            m_byte_count = 0;
            ++m_ir_unit_idx;
        }

        [[nodiscard]] VM::ConstantStore emit_constant_region(const ProtoConstMap& ir_constant_dict) {
            VM::ConstantStore const_region;

            for (const auto& entry : ir_constant_dict) {
                const auto entry_id = entry.second.id;
                const auto& entry_data = entry.second.data;

                switch (entry_data.index()) {
                    case 0:
                        const_region[entry_id] = VM::Value {std::get<bool>(entry_data)};
                        break;
                    case 1:
                        const_region[entry_id] = VM::Value {std::get<int>(entry_data)};
                        break;
                    case 2:
                        const_region[entry_id] = VM::Value {std::get<float>(entry_data)};
                        break;
                    default:
                        break;
                }
            }

            return const_region;
        }

        void emit_instruction_region(const std::vector<NodeUnion>& ir_steps) {
            std::vector<VM::RuntimeByte> result;

            auto emit_opcode = [&result, this](VM::Opcode opcode) {
                result.push_back(static_cast<VM::RuntimeByte>(opcode));
                ++m_byte_count;
            };

            auto emit_locator = [&result, this](const Locator& locator) {
                const auto& [loc_region, loc_id] = locator;

                result.push_back(static_cast<VM::RuntimeByte>(loc_region));
                ++m_byte_count;

                /// @note Store integers as big endian for easier decoding from LSB -> MSB with a "decoding forward" cursor in the VM.
                result.push_back(loc_id & 0x000000ff);
                result.push_back((loc_id & 0x0000ff00) >> 8);
                result.push_back((loc_id & 0x00ff0000) >> 16);
                result.push_back((loc_id & 0xff000000) >> 24);
                m_byte_count += 4;
            };

            auto emit_step = [&result, this](const StepUnion& step) {
                const auto step_variant_idx = step.index();
                VM::Opcode temp_op;
                auto saved_instr_pos = 0;

                if (step_variant_idx == 0) {
                    // Nonary: 1 byte
                    temp_op = std::get<NonaryStep>(step).op;
                    saved_instr_pos = m_byte_count;
                    emit_opcode(temp_op);
                } else if (step_variant_idx == 1) {
                    // Unary: 6 bytes
                    const auto& [un_op, un_arg0] = std::get<UnaryStep>(step);
                    temp_op = un_op;
                    saved_instr_pos = m_byte_count;
                    emit_opcode(un_op);
                    emit_locator(un_arg0);
                } else if (step_variant_idx == 2) {
                    // Binary: 11 bytes
                    const auto& [bi_op, bi_arg0, bi_arg1] = std::get<BinaryStep>(step);
                    temp_op = bi_op;
                    saved_instr_pos = m_byte_count;
                    emit_opcode(bi_op);
                    emit_locator(bi_arg0);
                    emit_locator(bi_arg1);
                } else if (step_variant_idx == 3) {
                    // Ternary: 16 bytes
                    const auto& [tri_op, tri_arg0, tri_arg1, tri_arg2] = std::get<TernaryStep>(step);
                    temp_op = tri_op;
                    saved_instr_pos = m_byte_count;
                    emit_opcode(tri_op);
                    emit_locator(tri_arg0);
                    emit_locator(tri_arg1);
                    emit_locator(tri_arg2);
                } else {
                    /// @todo Add codegen error: ill-formed StepUnions have a wrong arity outside [0:3]!
                    saved_instr_pos = m_byte_count;
                    emit_opcode(VM::Opcode::xop_noop);
                }

                if (temp_op == VM::Opcode::xop_jump_if) {
                    /// @note any jump to "else code" always follows a jump_if per branch.
                    m_backpatches.push(Backpatch {
                        .pending_units = 0,
                        .jump_if_pos = saved_instr_pos,
                        .backpatch_iffy = 0,
                        .jump_else_pos = dud_pos,
                        .backpatch_else = 0
                    });
                } else if (temp_op == VM::Opcode::xop_jump) {
                    m_backpatches.top().jump_else_pos = saved_instr_pos;
                }
            };

            auto patch_jump = [&result, this]() {
                if (m_backpatches.empty()) {
                    return;
                }

                const auto& [pending_units, if_jmp_idx, if_jmp_target, else_jmp_idx, else_jmp_target] = m_backpatches.top();

                if (pending_units == 2) {
                    --m_backpatches.top().pending_units;
                    return;
                } else if (pending_units == 1) {
                    /// @note After emitting the truthy block, place a `jmp` to skip execution of the else branch for correctness. This jump will be patched too on any fallthrough through these guard clauses.
                    --m_backpatches.top().pending_units;
                    m_backpatches.push(Backpatch {
                        .pending_units = 0,
                        .jump_if_pos = dud_pos,
                        .backpatch_iffy = dud_pos,
                        .jump_else_pos = m_byte_count,
                        .backpatch_else = dud_pos,
                    });
                    emit_step(UnaryStep {
                        .op = VM::Opcode::xop_jump,
                        .arg_0 = Locator {
                            .region = Region::none,
                            .id = dud_pos,
                        }
                    });

                    return;
                }

                if (if_jmp_idx != dud_pos) {
                    /// @note Uses +2 to pass the opcode and argument tag (Region)
                    const auto jmp_if_arg_pos = if_jmp_idx + 2;
                    
                    result[jmp_if_arg_pos] = if_jmp_target & 0x000000ff;
                    result[jmp_if_arg_pos + 1] = (if_jmp_target & 0x0000ff00) >> 8;
                    result[jmp_if_arg_pos + 2] = (if_jmp_target & 0x00ff0000) >> 16;
                    result[jmp_if_arg_pos + 3] = (if_jmp_target & 0xff000000) >> 24;
                }

                if (else_jmp_idx != dud_pos) {
                    /// @note Uses +2 to pass the opcode and argument tag (Region)
                    const auto jmp_else_arg_pos = else_jmp_idx + 2;
                    
                    result[jmp_else_arg_pos] = else_jmp_target & 0x000000ff;
                    result[jmp_else_arg_pos + 1] = (else_jmp_target & 0x0000ff00) >> 8;
                    result[jmp_else_arg_pos + 2] = (else_jmp_target & 0x00ff0000) >> 16;
                    result[jmp_else_arg_pos + 3] = (else_jmp_target & 0xff000000) >> 24;
                }
   
                m_backpatches.pop();
            };

            m_incoming.push(0);

            while (!m_incoming.empty()) {
                // 1. get next node.
                const int node_id = m_incoming.front();
                m_incoming.pop();

                // 2. emit each step in the node while updating byte count.
                // 2b. If the step is a jump operation, push a pending Backpatch...
                // 2c. Update any previously pending backpatch if it exists after a L/R unit is emitted.
                if (ir_steps[node_id].index() == 0) {
                    const auto& [unit_steps, unit_next] = std::get<Unit>(ir_steps[node_id]);

                    for (const auto& step : unit_steps) {
                        emit_step(step);
                    }

                    patch_jump();

                    // 3. Queue next children for processing...
                    if (unit_next != dud_pos) {
                        m_incoming.push(unit_next);
                    }
                } else {
                    const auto& [truthy_next, falsy_next] = std::get<Juncture>(ir_steps[node_id]);

                    // 3. Queue next children for processing...
                    if (truthy_next != dud_pos) {
                        m_incoming.push(truthy_next);
                        ++m_backpatches.top().pending_units;
                    }

                    if (falsy_next != dud_pos) {
                        m_incoming.push(falsy_next);
                        ++m_backpatches.top().pending_units;
                    }
                }
            }
        }
    };
}