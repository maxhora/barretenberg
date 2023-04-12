#include "barretenberg/honk/flavor/flavor.hpp"
#include "ecc_transcript_relation.hpp"

#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/numeric/random/engine.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"

#include <gtest/gtest.h>
using namespace proof_system::honk::sumcheck;
/**
 * We want to test if all three relations (namely, ArithmeticRelation, GrandProductComputationRelation,
 * GrandProductInitializationRelation) provide correct contributions by manually computing their
 * contributions with deterministic and random inputs. The relations are supposed to work with
 * univariates (edges) of degree one (length 2) and spit out polynomials of corresponding degrees. We have
 * MAX_RELATION_LENGTH = 5, meaning the output of a relation can atmost be a degree 5 polynomial. Hence,
 * we use a method compute_mock_extended_edges() which starts with degree one input polynomial (two evaluation
 points),
 * extends them (using barycentric formula) to six evaluation points, and stores them to an array of polynomials.
 */
// static const size_t input_univariate_length = 2;
// static constexpr size_t FULL_RELATION_LENGTH = 5;
static const size_t NUM_POLYNOMIALS = proof_system::honk::ECCVMArithmetisation::NUM_POLYNOMIALS;

namespace proof_system::honk_relation_tests {

using POLYNOMIAL = proof_system::honk::ECCVMArithmetisation::POLYNOMIAL;

namespace {
auto& engine = numeric::random::get_debug_engine();
}

template <typename, size_t> struct TestWrapper {

    using UnivariateView = barretenberg::fr;

    using Univariate = barretenberg::fr;

    using RelationParameters = barretenberg::fr;
};

struct Opcode {
    bool add;
    bool mul;
    bool eq;
    bool accumulate;
    [[nodiscard]] size_t value() const
    {
        auto res = static_cast<size_t>(add);
        res += res;
        res += static_cast<size_t>(mul);
        res += res;
        res += static_cast<size_t>(eq);
        res += res;
        res += static_cast<size_t>(accumulate);
        return res;
    }
};

struct ExecutionTrace {
    using TranscriptRow = std::array<barretenberg::fr, NUM_POLYNOMIALS>;

    using POLYNOMIAL = proof_system::honk::ECCVMArithmetisation::POLYNOMIAL;

    std::vector<TranscriptRow> rows;
    TranscriptRow next_row = TranscriptRow();

    TranscriptRow get_last_row()
    {
        if (rows.empty()) {
            return {};
        }
        return rows[rows.size() - 1];
    }

    [[nodiscard]] size_t pc() const { return pc_; }

    [[nodiscard]] size_t msm_count() const { return msm_count_; }

    [[nodiscard]] bool ongoing_msm() const { return ongoing_msm_; }

    [[nodiscard]] grumpkin::g1::affine_element accumulator() const { return accumulator_; }

    void add_row(TranscriptRow& row, const grumpkin::g1::affine_element& new_accumulator)
    {
        next_row = TranscriptRow();

        if (!rows.empty()) {
            auto& prev = rows[rows.size() - 1];
            prev[POLYNOMIAL::Q_TRANSCRIPT_MUL_SHIFT] = row[POLYNOMIAL::Q_TRANSCRIPT_MUL];
            prev[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE_SHIFT] = row[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE];
            prev[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_X_SHIFT] = row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_X];
            prev[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_Y_SHIFT] = row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_Y];
        }

        std::cout << "add row" << std::endl;
        if (row[POLYNOMIAL::Q_TRANSCRIPT_MUL] != 0) {
            std::cout << "TEMP1 = " << static_cast<size_t>(row[POLYNOMIAL::TRANSCRIPT_Z1ZERO] != 0) << std::endl;
            std::cout << "TEMP2 = " << static_cast<size_t>(row[POLYNOMIAL::TRANSCRIPT_Z2ZERO] != 0) << std::endl;

            pc_ += static_cast<size_t>(row[POLYNOMIAL::TRANSCRIPT_Z1ZERO] != 0);
            pc_ += static_cast<size_t>(row[POLYNOMIAL::TRANSCRIPT_Z2ZERO] != 0);
            std::cout << "pc_ = " << pc_ << std::endl;
        }
        row[POLYNOMIAL::TRANSCRIPT_PC_SHIFT] = pc();
        next_row[POLYNOMIAL::TRANSCRIPT_PC] = pc();

        bool accumulate = row[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE] != 0;
        bool mul_no_accumulate =
            row[POLYNOMIAL::Q_TRANSCRIPT_MUL] != 0 && row[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE] == 0;
        bool mul_accumulate = row[POLYNOMIAL::Q_TRANSCRIPT_MUL] != 0 && row[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE] != 0;

        bool new_msm = (!ongoing_msm_ && mul_accumulate);
        bool end_msm_this_row = (ongoing_msm_ && !accumulate) || mul_no_accumulate;
        bool continue_msm = ongoing_msm_ && mul_accumulate;

        if (!continue_msm) {
            msm_count_ = 0;
        }
        if (end_msm_this_row) {
            row[POLYNOMIAL::TRANSCRIPT_MSM_X] = msm_accumulator_.x;
            row[POLYNOMIAL::TRANSCRIPT_MSM_Y] = msm_accumulator_.y;
            row[POLYNOMIAL::Q_TRANSCRIPT_MSM_TRANSITION] = 1;
            msm_accumulator_ = grumpkin::g1::affine_point_at_infinity;
        }
        if (ongoing_msm_ || new_msm) {
            msm_count_ += static_cast<size_t>(row[POLYNOMIAL::TRANSCRIPT_Z1ZERO] != 0);
            msm_count_ += static_cast<size_t>(row[POLYNOMIAL::TRANSCRIPT_Z2ZERO] != 0);
        }

        if (ongoing_msm_ && !accumulate) {
            // add msm + accumulator into new accumulator
            grumpkin::g1::affine_element tmp = grumpkin::g1::element(accumulator_) + msm_accumulator_;
            row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_X_SHIFT] = tmp.x;
            row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_Y_SHIFT] = tmp.y;
            next_row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_X] = tmp.x;
            next_row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_Y] = tmp.y;
            accumulator_ = tmp;
        } else {
            row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_X_SHIFT] = new_accumulator.x;
            row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_Y_SHIFT] = new_accumulator.y;
            next_row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_X] = new_accumulator.x;
            next_row[POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_Y] = new_accumulator.y;
            accumulator_ = new_accumulator;
        }

        row[POLYNOMIAL::TRANSCRIPT_MSM_COUNT_SHIFT] = msm_count_;
        next_row[POLYNOMIAL::TRANSCRIPT_MSM_COUNT] = msm_count_;
        ongoing_msm_ = mul_accumulate || mul_no_accumulate;

        rows.push_back(row);
    }

    void add_accumulate(const grumpkin::g1::affine_element& to_add)
    {

        grumpkin::g1::element old_acc(accumulator_);
        grumpkin::g1::affine_element new_acc = old_acc + to_add;

        TranscriptRow new_row = next_row;

        new_row[POLYNOMIAL::Q_TRANSCRIPT_ADD] = 1;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_MUL] = 0;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_EQ] = 0;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE] = 1;
        new_row[POLYNOMIAL::TRANSCRIPT_X] = to_add.x;
        new_row[POLYNOMIAL::TRANSCRIPT_Y] = to_add.y;
        new_row[POLYNOMIAL::TRANSCRIPT_Z1] = 0;
        new_row[POLYNOMIAL::TRANSCRIPT_Z2] = 0;
        new_row[POLYNOMIAL::TRANSCRIPT_Z1ZERO] = 1;
        new_row[POLYNOMIAL::TRANSCRIPT_Z2ZERO] = 1;
        new_row[POLYNOMIAL::TRANSCRIPT_OP] =
            Opcode{ .add = true, .mul = false, .eq = false, .accumulate = true }.value();
        add_row(new_row, new_acc);
    }

    void mul_accumulate(const grumpkin::g1::affine_element& to_mul, const grumpkin::fr& scalar)
    {
        const auto old_msm_acc = msm_accumulator_;
        grumpkin::g1::affine_element mul_value = grumpkin::g1::element(to_mul) * scalar;

        if (ongoing_msm_) {
            msm_accumulator_ = grumpkin::g1::element(msm_accumulator_) + mul_value;
        } else {
            msm_accumulator_ = mul_value;
        }

        TranscriptRow new_row = next_row;

        grumpkin::fr z1 = 0;
        grumpkin::fr z2 = 0;
        grumpkin::fr::split_into_endomorphism_scalars(scalar, z1, z1);

        std::cout << "z1" << z1 << std::endl;
        std::cout << "z2" << z2 << std::endl;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_ADD] = 0;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_MUL] = 1;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_EQ] = 0;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE] = 1;
        new_row[POLYNOMIAL::TRANSCRIPT_X] = to_mul.x;
        new_row[POLYNOMIAL::TRANSCRIPT_Y] = to_mul.y;
        new_row[POLYNOMIAL::TRANSCRIPT_Z1] = static_cast<uint256_t>(z1);
        new_row[POLYNOMIAL::TRANSCRIPT_Z2] = static_cast<uint256_t>(z2);
        new_row[POLYNOMIAL::TRANSCRIPT_Z1ZERO] = static_cast<const int>(z1.is_zero());
        new_row[POLYNOMIAL::TRANSCRIPT_Z2ZERO] = static_cast<const int>(z2.is_zero());
        new_row[POLYNOMIAL::TRANSCRIPT_OP] =
            Opcode{ .add = false, .mul = true, .eq = false, .accumulate = true }.value();
        add_row(new_row, accumulator_);
    }

    void mul(const grumpkin::g1::affine_element& to_mul, const grumpkin::fr& scalar)
    {
        grumpkin::g1::affine_element mul_value = grumpkin::g1::element(to_mul) * scalar;

        msm_accumulator_ = mul_value;
        accumulator_ = mul_value;

        TranscriptRow new_row = next_row;

        grumpkin::fr z1 = 0;
        grumpkin::fr z2 = 0;
        grumpkin::fr::split_into_endomorphism_scalars(scalar, z1, z1);

        new_row[POLYNOMIAL::Q_TRANSCRIPT_ADD] = 0;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_MUL] = 1;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_EQ] = 0;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE] = 0;
        new_row[POLYNOMIAL::TRANSCRIPT_X] = to_mul.x;
        new_row[POLYNOMIAL::TRANSCRIPT_Y] = to_mul.y;
        new_row[POLYNOMIAL::TRANSCRIPT_Z1] = static_cast<uint256_t>(z1);
        new_row[POLYNOMIAL::TRANSCRIPT_Z2] = static_cast<uint256_t>(z2);
        new_row[POLYNOMIAL::TRANSCRIPT_Z1ZERO] = static_cast<const int>(z1.is_zero());
        new_row[POLYNOMIAL::TRANSCRIPT_Z2ZERO] = static_cast<const int>(z2.is_zero());
        new_row[POLYNOMIAL::TRANSCRIPT_OP] =
            Opcode{ .add = false, .mul = true, .eq = false, .accumulate = false }.value();
        add_row(new_row, mul_value);
    }

    void eq(const grumpkin::g1::affine_element& expected)
    {
        TranscriptRow new_row = next_row;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_ADD] = 0;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_MUL] = 0;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_EQ] = 1;
        new_row[POLYNOMIAL::Q_TRANSCRIPT_ACCUMULATE] = 0;
        new_row[POLYNOMIAL::TRANSCRIPT_X] = expected.x;
        new_row[POLYNOMIAL::TRANSCRIPT_Y] = expected.y;
        new_row[POLYNOMIAL::TRANSCRIPT_Z1] = 0;
        new_row[POLYNOMIAL::TRANSCRIPT_Z2] = 0;
        new_row[POLYNOMIAL::TRANSCRIPT_Z1ZERO] = 1;
        new_row[POLYNOMIAL::TRANSCRIPT_Z2ZERO] = 1;
        new_row[POLYNOMIAL::TRANSCRIPT_OP] =
            Opcode{ .add = false, .mul = false, .eq = true, .accumulate = false }.value();
        add_row(new_row, accumulator_);
    }

    std::vector<TranscriptRow> export_rows()
    {
        // TODO(@zac-williamson): make this const!
        add_row(next_row, accumulator_);
        return rows;
    }

    size_t pc_ = 0;
    size_t msm_count_ = 0;
    bool ongoing_msm_ = false;

    grumpkin::g1::affine_element accumulator_;
    grumpkin::g1::affine_element msm_accumulator_;

    // grumpkin::g1::element get_accumulator()
    // {
    //     ASSERT(rows.size() > 0);
    //     const auto x = rows[rows.size() - 1][POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_X];
    //     const auto y = rows[rows.size() - 1][POLYNOMIAL::TRANSCRIPT_ACCUMULATOR_Y];

    // }
};

ExecutionTrace generate_transcript_native()
{
    /**
     * Create following transcript:
     *
     * 1. MUL x1 * [A]
     * 2. MADD x2 * [B]
     * 3. EQ [C]
     */
    ExecutionTrace result;

    grumpkin::g1::element a = grumpkin::get_generator(0);
    grumpkin::g1::element b = grumpkin::get_generator(1);
    grumpkin::fr x = grumpkin::fr::random_element();
    grumpkin::g1::element expected = a + b * x;

    result.add_accumulate(a);
    result.mul_accumulate(b, x);
    result.eq(expected);

    return result;
}

TEST(SumcheckRelation, ECCVMTranscriptRelationAlgebra)
{
    auto relation = ECCVMTranscriptRelation<barretenberg::fr, TestWrapper>();
    // T::typename Univariate<FF, RELATION_LENGTH>& evals,
    //                                const auto& extended_edges,
    //                                const T::typename RelationParameters<FF>&,
    //                                const FF& scaling_factor

    barretenberg::fr result = 0;
    barretenberg::fr relation_parameter = 0;
    barretenberg::fr scaling_factor = 0;

    ExecutionTrace trace = generate_transcript_native();

    auto rows = trace.export_rows();

    for (size_t i = 0; i < rows.size(); ++i) {
        relation.add_edge_contribution(result, rows[i], relation_parameter, scaling_factor);
        std::cout << "i = " << i << std::endl;
        EXPECT_EQ(result, 0);
    }
};

} // namespace proof_system::honk_relation_tests
