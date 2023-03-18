#include "example_circuit_constructor.hpp"
#include <gtest/gtest.h>
#include "barretenberg/crypto/pedersen/pedersen.hpp"
#include "barretenberg/crypto/pedersen/generator_data.hpp"

using namespace barretenberg;
using namespace bonk;

namespace {
auto& engine = numeric::random::get_debug_engine();
}

namespace example_circuit_constructor_tests {
// WORKTODO: add tests of new functionality
TEST(example_circuit_constructor, base_case)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();
    fr a = fr::one();
    constructor.add_public_variable(a);

    bool result = constructor.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(example_circuit_constructor, test_add_gate)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();
    fr a = fr::one();
    uint32_t a_idx = constructor.add_public_variable(a);
    fr b = fr::one();
    fr c = a + b;
    fr d = a + c;
    // uint32_t a_idx = constructor.add_variable(a);
    uint32_t b_idx = constructor.add_variable(b);
    uint32_t c_idx = constructor.add_variable(c);
    uint32_t d_idx = constructor.add_variable(d);
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    constructor.create_add_gate({ d_idx, c_idx, a_idx, fr::one(), fr::neg_one(), fr::neg_one(), fr::zero() });

    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ b_idx, a_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    bool result = constructor.check_circuit(); // instance, prover.reference_string.SRS_T2);
    EXPECT_EQ(result, true);
}

TEST(example_circuit_constructor, test_mul_gate_proofs)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();
    fr q[7]{ fr::random_element(), fr::random_element(), fr::random_element(), fr::random_element(),
             fr::random_element(), fr::random_element(), fr::random_element() };
    fr q_inv[7]{
        q[0].invert(), q[1].invert(), q[2].invert(), q[3].invert(), q[4].invert(), q[5].invert(), q[6].invert(),
    };

    fr a = fr::random_element();
    fr b = fr::random_element();
    fr c = -((((q[0] * a) + (q[1] * b)) + q[3]) * q_inv[2]);
    fr d = -((((q[4] * (a * b)) + q[6]) * q_inv[5]));

    uint32_t a_idx = constructor.add_variable(a);
    uint32_t b_idx = constructor.add_variable(b);
    uint32_t c_idx = constructor.add_variable(c);
    uint32_t d_idx = constructor.add_variable(d);

    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });

    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });
    constructor.create_add_gate({ a_idx, b_idx, c_idx, q[0], q[1], q[2], q[3] });
    constructor.create_mul_gate({ a_idx, b_idx, d_idx, q[4], q[5], q[6] });

    bool result = constructor.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(example_circuit_constructor, range_constraint)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();

    for (size_t i = 0; i < 10; ++i) {

        uint32_t value = engine.get_random_uint32();
        fr witness_value = fr{ value, 0, 0, 0 }.to_montgomery_form();
        uint32_t witness_index = constructor.add_variable(witness_value);

        // include non-nice numbers of bits, that will bleed over gate boundaries
        size_t extra_bits = 2 * (i % 4);

        constructor.create_range_constraint(witness_index, 32 + extra_bits, "failure in range constraint test");

        // for (uint32_t j = 0; j < 16; ++j) {
        //     uint32_t result = (value >> (30U - (2 * j)));
        //     fr source = constructor.get_variable(accumulators[j + (extra_bits >> 1)]).from_montgomery_form();
        //     uint32_t expected = static_cast<uint32_t>(source.data[0]);
        //     EXPECT_EQ(result, expected);
        // }
        // for (uint32_t j = 1; j < 16; ++j) {
        //     uint32_t left = (value >> (30U - (2 * j)));
        //     uint32_t right = (value >> (30U - (2 * (j - 1))));
        //     EXPECT_EQ(left - 4 * right < 4, true);
        // }
    }

    uint32_t zero_idx = constructor.add_variable(fr::zero());
    uint32_t one_idx = constructor.add_variable(fr::one());
    constructor.create_big_add_gate(
        { zero_idx, zero_idx, zero_idx, one_idx, fr::one(), fr::one(), fr::one(), fr::one(), fr::neg_one() });

    bool result = constructor.check_circuit();

    EXPECT_EQ(result, true);
}

TEST(example_circuit_constructor, range_constraint_fail)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();

    uint64_t value = 0xffffff;
    uint32_t witness_index = constructor.add_variable(fr(value));

    constructor.create_range_constraint(witness_index, 23, "expected failure in range constraint test");

    bool result = constructor.check_circuit();

    EXPECT_EQ(result, false);
}

TEST(example_circuit_constructor, big_add_gate_with_bit_extract)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();

    const auto generate_constraints = [&constructor](uint32_t quad_value) {
        uint32_t quad_accumulator_left =
            (engine.get_random_uint32() & 0x3fffffff) - quad_value; // make sure this won't overflow
        uint32_t quad_accumulator_right = (4 * quad_accumulator_left) + quad_value;

        uint32_t left_idx = constructor.add_variable(uint256_t(quad_accumulator_left));
        uint32_t right_idx = constructor.add_variable(uint256_t(quad_accumulator_right));

        uint32_t input = engine.get_random_uint32();
        uint32_t output = input + (quad_value > 1 ? 1 : 0);

        add_quad gate{ constructor.add_variable(uint256_t(input)),
                       constructor.add_variable(uint256_t(output)),
                       right_idx,
                       left_idx,
                       fr(6),
                       -fr(6),
                       fr::zero(),
                       fr::zero(),
                       fr::zero() };

        constructor.create_big_add_gate_with_bit_extraction(gate);
    };

    generate_constraints(0);
    generate_constraints(1);
    generate_constraints(2);
    generate_constraints(3);

    bool result = constructor.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(example_circuit_constructor, test_range_constraint_fail)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();
    uint32_t witness_index = constructor.add_variable(fr::neg_one());
    constructor.create_range_constraint(witness_index, 32, "another expected failure in range constraint test");

    bool result = constructor.check_circuit();

    EXPECT_EQ(result, false);
}

TEST(example_circuit_constructor, test_check_circuit_correct)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();
    fr a = fr::one();
    uint32_t a_idx = constructor.add_public_variable(a);
    fr b = fr::one();
    fr c = a + b;
    fr d = a + c;
    // uint32_t a_idx = constructor.add_variable(a);
    uint32_t b_idx = constructor.add_variable(b);
    uint32_t c_idx = constructor.add_variable(c);
    uint32_t d_idx = constructor.add_variable(d);
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    constructor.create_add_gate({ d_idx, c_idx, a_idx, fr::one(), fr::neg_one(), fr::neg_one(), fr::zero() });

    bool result = constructor.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(example_circuit_constructor, test_check_circuit_broken)
{
    ExampleCircuitConstructor constructor = ExampleCircuitConstructor();
    fr a = fr::one();
    uint32_t a_idx = constructor.add_public_variable(a);
    fr b = fr::one();
    fr c = a + b;
    fr d = a + c + 1;
    // uint32_t a_idx = constructor.add_variable(a);
    uint32_t b_idx = constructor.add_variable(b);
    uint32_t c_idx = constructor.add_variable(c);
    uint32_t d_idx = constructor.add_variable(d);
    constructor.create_add_gate({ a_idx, b_idx, c_idx, fr::one(), fr::one(), fr::neg_one(), fr::zero() });

    constructor.create_add_gate({ d_idx, c_idx, a_idx, fr::one(), fr::neg_one(), fr::neg_one(), fr::zero() });

    bool result = constructor.check_circuit();
    EXPECT_EQ(result, false);
}

} // namespace example_circuit_constructor_tests
