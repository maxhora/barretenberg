#pragma once
#include <array>
#include <tuple>

#include "barretenberg/honk/flavor/flavor.hpp"
#include "../../polynomials/univariate.hpp"
#include "../relation.hpp"

template <typename FF, size_t RELATION_LENGTH> struct RelationTypes {
    using UnivariateView = proof_system::honk::sumcheck::UnivariateView<FF, RELATION_LENGTH>;
    using Univariate = proof_system::honk::sumcheck::Univariate<FF, RELATION_LENGTH>;
    using RelationParameters = proof_system::honk::sumcheck::RelationParameters<FF>;
};

namespace proof_system::honk::sumcheck {

template <typename FF, template <typename, size_t> typename TypeMuncher> class ECCVMTranscriptRelation {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 4;
    using MULTIVARIATE = ECCVMHonk::MULTIVARIATE; // could just get from StandardArithmetization

    using UnivariateView = TypeMuncher<FF, RELATION_LENGTH>::UnivariateView;
    using Univariate = TypeMuncher<FF, RELATION_LENGTH>::Univariate;
    using RelationParameters = TypeMuncher<FF, RELATION_LENGTH>::RelationParameters;

    /**
     * @brief Expression for the StandardArithmetic gate.
     * @details The relation is defined as C(extended_edges(X)...) =
     *    (q_m * w_r * w_l) + (q_l * w_l) + (q_r * w_r) + (q_o * w_o) + q_c
     *
     * @param evals transformed to `evals + C(extended_edges(X)...)*scaling_factor`
     * @param extended_edges an std::array containing the fully extended Univariate edges.
     * @param parameters contains beta, gamma, and public_input_delta, ....
     * @param scaling_factor optional term to scale the evaluation before adding to evals.
     */
    void add_edge_contribution(Univariate& evals,
                               const auto& extended_edges,
                               const RelationParameters& /*unused*/,
                               const FF& /*unused*/) const
    {

        auto z1 = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_Z1]);
        auto z2 = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_Z2]);
        auto z1_zero = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_Z1ZERO]);
        auto z2_zero = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_Z2ZERO]);

        auto op = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_OP]);
        auto q_add = UnivariateView(extended_edges[MULTIVARIATE::Q_TRANSCRIPT_ADD]);
        auto q_mul = UnivariateView(extended_edges[MULTIVARIATE::Q_TRANSCRIPT_MUL]);
        auto q_mul_shift = UnivariateView(extended_edges[MULTIVARIATE::Q_TRANSCRIPT_MUL_SHIFT]);
        auto q_eq = UnivariateView(extended_edges[MULTIVARIATE::Q_TRANSCRIPT_EQ]);
        auto q_accumulate = UnivariateView(extended_edges[MULTIVARIATE::Q_TRANSCRIPT_ACCUMULATE]);
        auto q_accumulate_shift = UnivariateView(extended_edges[MULTIVARIATE::Q_TRANSCRIPT_ACCUMULATE_SHIFT]);
        auto q_msm_transition = UnivariateView(extended_edges[MULTIVARIATE::Q_TRANSCRIPT_MSM_TRANSITION]);
        auto msm_count = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_MSM_COUNT]);
        auto msm_count_shift = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_MSM_COUNT_SHIFT]);

        auto pc = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_PC]);
        auto pc_shift = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_PC_SHIFT]);

        auto transcript_accumulator_x_shift =
            UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_ACCUMULATOR_X_SHIFT]);
        auto transcript_accumulator_y_shift =
            UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_ACCUMULATOR_Y_SHIFT]);
        auto transcript_accumulator_x = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_ACCUMULATOR_X]);
        auto transcript_accumulator_y = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_ACCUMULATOR_Y]);
        auto transcript_msm_x = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_MSM_X]);
        auto transcript_msm_y = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_MSM_Y]);
        auto transcript_x = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_X]);
        auto transcript_y = UnivariateView(extended_edges[MULTIVARIATE::TRANSCRIPT_Y]);

        // if z1zero = 0, this does not rule out z1 being zero; this produces unsatisfiable constraints when computing
        // the scalar sum However if z1zero = 1 we must require that z1 = 0. i.e. z1 * z1zero = 0;
        evals += z1 * z1_zero;
        evals += z2 * z2_zero;
        evals += z1_zero * (z1_zero - 1);
        evals += z2_zero * (z2_zero - 1);

        // // set membership components not performed here

        // validate op
        auto tmp = q_add;
        tmp += tmp;
        tmp += q_mul;
        tmp += tmp;
        tmp += q_eq;
        tmp += tmp;
        tmp += q_accumulate;
        evals += (tmp - op);

        // update pc depending on if we are performing msm
        const auto pc_delta = pc_shift - pc;
        std::cout << "pc_shift = " << pc_shift << " pc = " << pc << std::endl;
        std::cout << "pc_delta = " << pc_delta << std::endl;
        std::cout << "qmul = " << q_mul << std::endl;
        std::cout << "z10 = " << z1_zero << std::endl;
        std::cout << "z20 = " << z2_zero << std::endl;

        evals += (pc_delta - q_mul * (z1_zero + z2_zero));

        // if q_mul and q_accumulate

        // determine if we are finishing a MSM on this row
        // MSM end states:
        // |    current row   |    next row      |
        // |    is mul + no accumulate | no accumulate |
        // |    is mul + accumulate | no mul |
        // |    is mul + accumulate | is mul + no accumulate |
        // n.b. can optimize this ?
        auto msm_transition = q_mul * (-q_mul_shift + 1);
        msm_transition += q_mul * (q_mul_shift * (-q_accumulate_shift + 1));
        evals += (q_msm_transition - msm_transition);
        // wtf we do with this?
        // if msm transition...we perform a set membership write (not here)
        // if msm transition we reset msm_count
        evals += (q_msm_transition * msm_count_shift);

        // // if not a msm transition, count updates by number of muls
        // auto msm_count_delta = msm_count_shift - msm_count;
        // evals += (-q_msm_transition + 1) * (msm_count_delta - q_mul * (-z1_zero + 1) * (-z2_zero + 1));
        // // if msm transition and we accumulate, add output into acc.
        // auto x3 = transcript_accumulator_x_shift;
        // auto y3 = transcript_accumulator_y_shift;
        // auto x1 = transcript_accumulator_x;
        // auto y1 = transcript_accumulator_y;
        // auto x2 = transcript_msm_x;
        // auto y2 = transcript_msm_y;
        // auto tmpx = x3 + x2 + x1 * (x2 - x1) * (x2 - x1) - (y2 - y1) * (y2 - y1);
        // auto tmpy = (y3 + y1) * (x2 - x1) - (y2 - y1) * (x1 - x3);
        // evals += tmpx * q_msm_transition * q_accumulate;
        // evals += tmpy * q_msm_transition * q_accumulate;

        // // if msm transition and no accumulate, just transfer
        // evals += q_msm_transition * (-q_accumulate + 1) * (x3 - x2);
        // evals += q_msm_transition * (-q_accumulate + 1) * (y3 - y2);

        // // add-accumulate
        // x2 = transcript_x;
        // y2 = transcript_y;
        // tmpx = x3 + x2 + x1 * (x2 - x1) * (x2 - x1) - (y2 - y1) * (y2 - y1);
        // tmpy = (y3 + y1) * (x2 - x1) - (y2 - y1) * (x1 - x3);
        // evals += tmpx * q_add;
        // evals += tmpy * q_add;

        // // equality check
        // // if msm transition and no accumulate, just transfer
        // evals += q_eq * (x1 - x2);
        // evals += q_eq * (y1 - y2);
    }

    // void add_full_relation_value_contribution(FF& full_honk_relation_value,
    //                                           const auto& purported_evaluations,
    //                                           const RelationParameters<FF>&) const
    // {
    //     auto w_l = purported_evaluations[MULTIVARIATE::W_L];
    //     auto w_r = purported_evaluations[MULTIVARIATE::W_R];
    //     auto w_o = purported_evaluations[MULTIVARIATE::W_O];
    //     auto q_m = purported_evaluations[MULTIVARIATE::Q_M];
    //     auto q_l = purported_evaluations[MULTIVARIATE::Q_L];
    //     auto q_r = purported_evaluations[MULTIVARIATE::Q_R];
    //     auto q_o = purported_evaluations[MULTIVARIATE::Q_O];
    //     auto q_c = purported_evaluations[MULTIVARIATE::Q_C];

    //     full_honk_relation_value += w_l * (q_m * w_r + q_l);
    //     full_honk_relation_value += q_r * w_r;
    //     full_honk_relation_value += q_o * w_o;
    //     full_honk_relation_value += q_c;
    // };
};

} // namespace proof_system::honk::sumcheck
