#pragma once

#include <utility>
#include <memory>
#include <set>
#include <stack>
#include <vector>
#include "codegen/policies.hpp"
#include "codegen/steps.hpp"
#include "codegen/flow_nodes.hpp"
#include "codegen/graph_pass.hpp"
#include "vm/chunk.hpp"

namespace XLang::Codegen {
    inline constexpr auto dud_num = -1;

    enum class PatchStatus : unsigned char {
        patch_skip_truthy,
        patch_skip_falsy,
        patch_done
    };

    struct Backpatch {
        PatchStatus status;
        int jump_pos;
        int jump_patch;

        friend constexpr bool patch_usable(const Backpatch& patch) noexcept {
            return patch.jump_pos != dud_num && patch.jump_patch != dud_num;
        }
    };

    /**
     * @brief This class contains logic which traverses the control flow graph (basically the IR) and emits bytecode for the stack VM. The CFG is (likely) traversed forward by a BFS algorithm which prunes empty code Units.
     */
    template <typename Result = VM::Chunk, typename Policy = Codegen::DefaultPolicy>
    class EmitCodePass {
    public:
        EmitCodePass()
        : m_result {std::make_unique<VM::XpliceProgram>()}, m_constant_chunks_view {nullptr}, m_ir_unit_idx {0} {}

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

            for (const auto& temp_cfg : cfg_dict) {
                m_result.get()->func_chunks.operator[](m_ir_unit_idx) = process(temp_cfg);
                clear_current_state();
            }

        return {std::move(m_result)};
    }

    private:
        static constexpr auto dud_pos = -1;

        /// @note Stores bytecode program being built: "VM-ProgramStore"
        std::unique_ptr<VM::XpliceProgram> m_result;

        const std::vector<ProtoConstMap>* m_constant_chunks_view;

        int m_ir_unit_idx;

        void clear_current_state() {
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

        [[nodiscard]] std::vector<VM::RuntimeByte> emit_instruction_region(const std::vector<NodeUnion>& ir_steps) {
            std::vector<VM::RuntimeByte> result;
            auto byte_total = 0;

            /// @note Stores next nodes by ID to emit bytecode for, etc.
            std::stack<int> incoming;
            /// @note Stores backpatch info for earlier-encountered dud jumps.
            std::stack<Backpatch> patches;
            /// @note Stores which node IDs were visited.
            std::set<int> visited;

            auto emit_opcode = [&result, &byte_total](VM::Opcode opcode) {
                result.push_back(static_cast<VM::RuntimeByte>(opcode));
                ++byte_total;
            };

            auto emit_arg = [&](const Locator& arg) {
                const auto& [arg_tag, arg_num] = arg;
                const auto tag = static_cast<VM::RuntimeByte>(arg_tag);

                result.push_back(tag);
                ++byte_total;
                result.push_back(arg_num & 0x000000ff);
                result.push_back((arg_num & 0x0000ff00) >> 8);
                result.push_back((arg_num & 0x00ff0000) >> 16);
                result.push_back((arg_num & 0xff000000) >> 24);
                byte_total += 4;
            };

            auto emit_step = [&](const StepUnion& step) {
                const auto step_var_idx = step.index();
                const auto instruction_pos = byte_total;
                VM::Opcode passed_op = VM::Opcode::xop_noop;

                if (step_var_idx == 0) {
                    const auto& [nonary_op] = std::get<NonaryStep>(step);
                    emit_opcode(nonary_op);
                    passed_op = nonary_op;
                } else if (step_var_idx == 1) {
                    const auto& [unary_op, unary_arg0] = std::get<UnaryStep>(step);
                    emit_opcode(unary_op);
                    emit_arg(unary_arg0);
                    passed_op = unary_op;
                } else if (step_var_idx == 2) {
                    const auto& [binary_op, binary_arg0, binary_arg1] = std::get<BinaryStep>(step);
                    emit_opcode(binary_op);
                    emit_arg(binary_arg0);
                    emit_arg(binary_arg1);
                    passed_op = binary_op;
                } else if (step_var_idx == 3) {
                    const auto& [ternary_op, ternary_arg0, ternary_arg1, ternary_arg2] = std::get<TernaryStep>(step);
                    emit_opcode(ternary_op);
                    emit_arg(ternary_arg0);
                    emit_arg(ternary_arg1);
                    emit_arg(ternary_arg2);
                    passed_op = ternary_op;
                } else {}

                if (passed_op == VM::Opcode::xop_jump_not_if) {
                    patches.emplace(Backpatch {
                        .status = PatchStatus::patch_skip_truthy,
                        .jump_pos = instruction_pos,
                        .jump_patch = dud_num
                    });
                } else if (passed_op == VM::Opcode::xop_jump) {
                    patches.top().jump_patch = instruction_pos;
                    patches.emplace(Backpatch {
                        .status = PatchStatus::patch_skip_falsy,
                        .jump_pos = instruction_pos,
                        .jump_patch = dud_num
                    });
                } else if (passed_op == VM::Opcode::xop_noop) {
                    patches.top().jump_patch = instruction_pos;
                }
            };

            auto patch_jump = [&result, &patches]() {
                if (patches.empty()) {
                    return;
                }

                if (!patch_usable(patches.top())) {
                    return;
                }

                const auto& [patch_tag, patch_jump_pos, patch_jump_arg] = patches.top();
                const auto patch_jump_arg_pos = patch_jump_pos + 2;

                result[patch_jump_arg_pos] = (patch_jump_arg & 0x000000ff);
                result[patch_jump_arg_pos + 1] = (patch_jump_arg & 0x0000ff00) >> 8;
                result[patch_jump_arg_pos + 2] = (patch_jump_arg & 0x00ff0000) >> 16;
                result[patch_jump_arg_pos + 3] = (patch_jump_arg & 0xff000000) >> 24;

                patches.pop();
            };

            incoming.push(0);

            while (!incoming.empty()) {
                const auto node_id = incoming.top();
                incoming.pop();

                if (node_id == dud_num || visited.contains(node_id)) {
                    continue;
                }

                const auto& ir_unit = ir_steps[node_id];

                if (std::holds_alternative<Unit>(ir_unit)) {
                    const auto& [steps, next_id] = std::get<Unit>(ir_unit);

                    for (const auto& step : steps) {
                        emit_step(step);
                    }

                    patch_jump();

                    incoming.push(next_id);
                    visited.insert(node_id);
                } else {
                    const auto& [truthy_unit_id, falsy_unit_id] = std::get<Juncture>(ir_unit);

                    incoming.push(falsy_unit_id);
                    incoming.push(truthy_unit_id);
                    visited.insert(node_id);
                }

            }

            patch_jump();

            return result;
        }
    };
}