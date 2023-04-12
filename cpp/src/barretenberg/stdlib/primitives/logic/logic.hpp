#pragma once
#include "barretenberg/numeric/uint256/uint256.hpp"
#include "barretenberg/stdlib/primitives/composers/composers_fwd.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"
#include "barretenberg/stdlib/primitives/witness/witness.hpp"
#include <cstdint>
#include <functional>
#include <utility>

namespace proof_system::plonk::stdlib {

template <typename Composer> class logic {
  public:
    using field_pt = field_t<Composer>;
    using witness_pt = witness_t<Composer>;

  public:
    static field_pt create_logic_constraint(
        field_pt& a,
        field_pt& b,
        size_t num_bits,
        bool is_xor_gate,
        const std::function<std::pair<field_pt, field_pt>(Composer* ctx, uint256_t, uint256_t)>& get_chunk =
            [](Composer* ctx, uint256_t left, uint256_t right) {
                uint256_t left_chunk = left & ((uint256_t(1) << 32) - 1);
                uint256_t right_chunk = right & ((uint256_t(1) << 32) - 1);

                const field_pt a_chunk = witness_pt(ctx, left_chunk);
                const field_pt b_chunk = witness_pt(ctx, right_chunk);
                return std::make_pair(a_chunk, b_chunk);
            });
};

EXTERN_STDLIB_TYPE(logic);

} // namespace proof_system::plonk::stdlib