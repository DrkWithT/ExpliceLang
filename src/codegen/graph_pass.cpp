#include "codegen/graph_pass.hpp"

namespace XLang::Codegen {
    constexpr auto dud_offset = -1;
    constexpr Locator dud_locator = {
        .region = Region::none,
        .id = dud_offset
    };


    RegisterAllocator::RegisterAllocator(std::size_t register_n)
    : m_regs (register_n, true) {}

    [[nodiscard]] int RegisterAllocator::allocate() {
        auto reg = 0;

        for (const auto& pos : m_regs) {            
            if (pos) {
                m_regs[reg] = false;
                return reg;
            }

            ++reg;
        }

        return dud_offset;
    }

    [[nodiscard]] bool RegisterAllocator::release(int reg) {
        if (reg >= 0 && reg < static_cast<int>(m_regs.size())) {
            m_regs[reg] = true;
            return true;
        }

        return false;
    }


    /// @todo Implement offset allocator...
    OffsetAllocator::OffsetAllocator(std::size_t stack_capacity)
    : m_slots (stack_capacity, true), m_lru {} {}

    [[nodiscard]] int OffsetAllocator::allocate() {
        auto salvaged_slot = salvage_from_lru();

        if (salvaged_slot != dud_offset) {
            return salvaged_slot;
        }

        auto normal_slot = dud_offset;

        for (int offset_idx = 0; offset_idx < static_cast<int>(m_slots.size()); ++offset_idx) {
            if (m_slots[offset_idx]) {
                m_slots[offset_idx] = false;
                normal_slot = offset_idx;
                m_lru.push_back(normal_slot);

                break;
            }
        }

        if (normal_slot != dud_offset) {
            return normal_slot;
        }

        return dud_offset;
    }

    [[nodiscard]] bool OffsetAllocator::release(int slot) {
        if (slot >= 0 && slot < static_cast<int>(m_slots.size())) {
            return false;
        }

        m_slots[slot] = true;
        m_lru.push_back(slot);

        return true;
    }

    [[nodiscard]] int OffsetAllocator::salvage_from_lru() noexcept {
        if (m_lru.empty()) {
            return dud_offset;
        }

        const auto lru_id = m_lru.front();
        m_lru.erase(m_lru.begin());
        m_slots[lru_id] = false;
        m_lru.push_back(lru_id);

        return lru_id;
    }


    Locator GraphPass::new_location() {
        auto candidate_reg = m_reg_all.allocate();

        if (candidate_reg != dud_offset) {
            return {
                .region = Region::temp_register,
                .id = candidate_reg
            };
        }

        auto candidate_slot = m_off_all.allocate();

        if (candidate_slot != dud_offset) {
            return {
                .region = Region::temp_stack,
                .id = candidate_slot
            };
        }

        return dud_locator;
    }

    bool GraphPass::delete_location(const Locator& loc) {
        const auto& [region, id] = loc;

        if (region == Region::temp_register) {
            return m_reg_all.release(id);
        } else if (region == Region::temp_stack) {
            return m_off_all.release(id);
        } else {
            /// @todo: implement heap allocation support for more codegen...
            ;
        }

        return false;
    }

    GraphPass::GraphPass(std::size_t vm_regs_n, std::size_t vm_stack_n)
    : m_reg_all {vm_regs_n}, m_off_all {vm_stack_n}, m_nodes {}, m_result {new FlowStore {}} {}

    ; // TODO!!
}