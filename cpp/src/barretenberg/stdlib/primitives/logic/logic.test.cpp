#include "../bool/bool.hpp"
#include "logic.hpp"
#include "barretenberg/plonk/proof_system/constants.hpp"
#include <gtest/gtest.h>
#include "barretenberg/honk/composer/standard_honk_composer.hpp"
#include "barretenberg/plonk/composer/standard_composer.hpp"
#include "barretenberg/plonk/composer/ultra_composer.hpp"
#include "barretenberg/plonk/composer/turbo_composer.hpp"
#include "barretenberg/numeric/random/engine.hpp"

#pragma GCC diagnostic ignored "-Wimplicitly-unsigned-literal"

#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

#define STDLIB_TYPE_ALIASES                                                                                            \
    using Composer = TypeParam;                                                                                        \
    using witness_ct = stdlib::witness_t<Composer>;                                                                    \
    using field_ct = stdlib::field_t<Composer>;                                                                        \
    using bool_ct = stdlib::bool_t<Composer>;                                                                          \
    using public_witness_ct = stdlib::public_witness_t<Composer>;

namespace test_stdlib_logic {

namespace {
auto& engine = numeric::random::get_debug_engine();
}

template <class T> void ignore_unused(T&) {} // use to ignore unused variables in lambdas

using namespace barretenberg;
using namespace proof_system::plonk;

template <class Composer> class LogicTest : public testing::Test {};

using ComposerTypes =
    ::testing::Types<honk::StandardHonkComposer, plonk::StandardComposer, plonk::TurboComposer, plonk::UltraComposer>;

TYPED_TEST_SUITE(LogicTest, ComposerTypes);

TYPED_TEST(LogicTest, TestCorrectLogic)
{
    STDLIB_TYPE_ALIASES
    auto composer = Composer();

    auto run_test = [](size_t num_bits, Composer& composer) {
        uint256_t mask = (uint256_t(1) << num_bits) - 1;

        uint256_t a = engine.get_random_uint256() & mask;
        uint256_t b = engine.get_random_uint256() & mask;

        uint256_t and_expected = a & b;
        // uint256_t xor_expected = a ^ b;

        field_ct x = witness_ct(&composer, a);
        field_ct y = witness_ct(&composer, b);

        field_ct x_const(&composer, a);
        field_ct y_const(&composer, b);
        field_ct and_result = stdlib::logic<Composer>::create_logic_constraint(x, y, num_bits, false);
        field_ct and_result_1 = stdlib::logic<Composer>::create_logic_constraint(x, y, num_bits, false);
        (void)and_result_1;
        // field_ct xor_result = stdlib::logic<Composer>::create_logic_constraint(x, y, num_bits, true);
        // field_ct xor_result_1 = stdlib::logic<Composer>::create_logic_constraint(x, y, num_bits, true);
        // (void)xor_result_1;
        // field_ct and_result_left_constant =
        // stdlib::logic<Composer>::create_logic_constraint(x_const, y, num_bits, false);
        // field_ct xor_result_left_constant =
        //     stdlib::logic<Composer>::create_logic_constraint(x_const, y, num_bits, true);

        // field_ct and_result_right_constant =
        //     stdlib::logic<Composer>::create_logic_constraint(x, y_const, num_bits, false);
        // field_ct xor_result_right_constant =
        //     stdlib::logic<Composer>::create_logic_constraint(x, y_const, num_bits, true);

        // field_ct and_result_both_constant =
        //     stdlib::logic<Composer>::create_logic_constraint(x_const, y_const, num_bits, false);
        // field_ct xor_result_both_constant =
        //     stdlib::logic<Composer>::create_logic_constraint(x_const, y_const, num_bits, true);

        EXPECT_EQ(uint256_t(and_result.get_value()), and_expected);
        // EXPECT_EQ(uint256_t(and_result_left_constant.get_value()), and_expected);
        // EXPECT_EQ(uint256_t(and_result_right_constant.get_value()), and_expected);
        // EXPECT_EQ(uint256_t(and_result_both_constant.get_value()), and_expected);

        // EXPECT_EQ(uint256_t(xor_result.get_value()), xor_expected);
        // EXPECT_EQ(uint256_t(xor_result_left_constant.get_value()), xor_expected);
        // EXPECT_EQ(uint256_t(xor_result_right_constant.get_value()), xor_expected);
        // EXPECT_EQ(uint256_t(xor_result_both_constant.get_value()), xor_expected);
    };

    // for (size_t i = 8; i < 248; i += 8) {
    run_test(8, composer);
    // run_test(8, composer);
    // }
    auto prover = composer.create_prover();
    plonk::proof proof = prover.construct_proof();
    auto verifier = composer.create_verifier();
    bool result = verifier.verify_proof(proof);
    EXPECT_EQ(result, true);
}
} // namespace test_stdlib_logic