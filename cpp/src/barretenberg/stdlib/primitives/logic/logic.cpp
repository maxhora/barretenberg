#include "logic.hpp"

#include "../composers/composers.hpp"
#include "../plookup/plookup.hpp"
#include "barretenberg/common/assert.hpp"
#include "barretenberg/numeric/uint256/uint256.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"
#pragma GCC diagnostic ignored "-Wimplicitly-unsigned-literal"

namespace proof_system::plonk::stdlib {

/**
 * @brief A logical AND or XOR over a variable number of bits.
 *
 * @details Defaults to basic Composer method if not using plookup-compatible composer
 *
 * @tparam Composer
 * @param a
 * @param b
 * @param num_bits
 * @param is_xor_gate
 * @return field_t<Composer>
 */
// underlying assumption that if a or b are not constants than they point to one of the variables in the container of
// potential
// variables that can be indices hence this is not actually adding new witnesses to the new circuit
// i guess these are figured out at the beg
template <typename Composer>
field_t<Composer> logic<Composer>::create_logic_constraint(
    field_pt& a,
    field_pt& b,
    size_t num_bits,
    bool is_xor_gate,
    const std::function<std::pair<field_pt, field_pt>(Composer* ctx, uint256_t, uint256_t)>& get_chunk)
{
    // can't extend past field size!
    ASSERT(num_bits > 0);
    ASSERT(num_bits < 254);
    if (a.is_constant() && b.is_constant()) {
        uint256_t a_native(a.get_value());
        uint256_t b_native(b.get_value());
        uint256_t c_native = is_xor_gate ? (a_native ^ b_native) : (a_native & b_native);
        return field_t<Composer>(c_native);
    }
    if (a.is_constant() && !b.is_constant()) {
        Composer* ctx = b.get_context();
        uint256_t a_native(a.get_value());
        // although this assumes that if a or b are constant they can be new constant, i'm confused
        field_pt a_witness = field_pt::from_witness_index(ctx, ctx->put_constant_variable(a_native));
        return create_logic_constraint(a_witness, b, num_bits, is_xor_gate, get_chunk);
    }
    if (!a.is_constant() && b.is_constant()) {
        Composer* ctx = a.get_context();
        uint256_t b_native(b.get_value());
        field_pt b_witness = field_pt::from_witness_index(ctx, ctx->put_constant_variable(b_native));
        return create_logic_constraint(a, b_witness, num_bits, is_xor_gate, get_chunk);
    }
    if constexpr (Composer::type == ComposerType::PLOOKUP) {
        Composer* ctx = a.get_context();

        const size_t num_chunks = (num_bits / 32) + ((num_bits % 32 == 0) ? 0 : 1);
        uint256_t left((uint256_t)13835058055282163715);
        uint256_t right((uint256_t)3458764513820540940);
        info(left);
        info(right);
        info(b);
        info(a);

        field_pt a_accumulator(barretenberg::fr::zero());
        field_pt b_accumulator(barretenberg::fr::zero());

        field_pt res(ctx, 0);
        for (size_t i = 0; i < num_chunks; ++i) {

            auto [a_chunk, b_chunk] = get_chunk(ctx, left, right);

            field_pt result_chunk = 0;
            if (is_xor_gate) {
                result_chunk =
                    stdlib::plookup_read::read_from_2_to_1_table(plookup::MultiTableId::UINT32_XOR, a_chunk, b_chunk);
            } else {
                result_chunk =
                    stdlib::plookup_read::read_from_2_to_1_table(plookup::MultiTableId::UINT32_AND, a_chunk, b_chunk);
            }

            uint256_t scaling_factor = uint256_t(1) << (32 * i);
            a_accumulator += a_chunk * scaling_factor;
            b_accumulator += b_chunk * scaling_factor;
            res += result_chunk * scaling_factor;

            if (i == num_chunks - 1) {
                const size_t final_num_bits = num_bits - (i * 32);
                if (final_num_bits != 32) {
                    ctx->create_range_constraint(a_chunk.witness_index, final_num_bits, "bad range on a");
                }
            }

            left = left >> 32;
            right = right >> 32;
        }
        info(a_accumulator);
        info(b_accumulator);
        a_accumulator.assert_equal(a, "not equal");
        b_accumulator.assert_equal(b, "not equal");

        return res;
    } else {
        // If the composer doesn't have lookups we call the expensive logic constraint gate
        // this will make constraints for each bit
        Composer* ctx = a.get_context();
        auto accumulator_triple = ctx->create_logic_constraint(
            a.normalize().get_witness_index(), b.normalize().get_witness_index(), num_bits, is_xor_gate);
        auto out_idx = accumulator_triple.out[accumulator_triple.out.size() - 1];
        return field_t<Composer>::from_witness_index(ctx, out_idx);
    }
}
INSTANTIATE_STDLIB_TYPE(logic);
} // namespace proof_system::plonk::stdlib
