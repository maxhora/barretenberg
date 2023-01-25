#pragma once
#include "relation.hpp"
#include <proof_system/flavor/flavor.hpp>
#include "../polynomials/univariate.hpp"

namespace honk::sumcheck {

template <typename FF> class GrandProductComputationRelation : public Relation<FF> {
  public:
    // 1 + polynomial degree of this relation
    static constexpr size_t RELATION_LENGTH = 5;
    using MULTIVARIATE = StandardHonk::MULTIVARIATE;

  public:
    // TODO(luke): make these real challenges once manifest is done
    const FF beta_default = FF::one();
    const FF gamma_default = FF::one();
    const FF public_input_delta_default = FF::one();

    GrandProductComputationRelation() = default;
    explicit GrandProductComputationRelation(auto){}; // TODO(luke): should just be default?

    /**
     * @brief Add contribution of the permutation relation for a given edge
     *
     * @detail There are 2 relations associated with enforcing the wire copy relations
     * This file handles the relation that confirms faithful calculation of the grand
     * product polynomial Z_perm. (Initialization relation Z_perm(0) = 1 is handled elsewhere).
     *
     *      ( z_perm(X) + lagrange_first(X) )*P(X) - ( z_perm_shift(X) + delta * lagrange_last(X) )*Q(X),
     *   where
     *      P(X) = Prod_{i=1:3} w_i(X) + β*(n*(i-1) + idx(X)) + γ
     *      Q(X) = Prod_{i=1:3} w_i(X) + β*σ_i(X) + γ
     *      delta is the public input correction term
     *
     */
    void add_edge_contribution(auto& extended_edges, Univariate<FF, RELATION_LENGTH>& evals)
    {
        add_edge_contribution_internal(extended_edges, evals, beta_default, gamma_default, public_input_delta_default);
    };

    void add_edge_contribution_testing(auto& extended_edges,
                                       Univariate<FF, RELATION_LENGTH>& evals,
                                       std::array<FF, 3> challenges)
    {
        add_edge_contribution_internal(extended_edges, evals, challenges[0], challenges[1], challenges[2]);
    };

    inline void add_edge_contribution_internal(
        auto& extended_edges, Univariate<FF, RELATION_LENGTH>& evals, FF beta, FF gamma, FF public_input_delta)
    {
        auto w_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_L]);
        auto w_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_R]);
        auto w_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::W_O]);
        auto sigma_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::SIGMA_1]);
        auto sigma_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::SIGMA_2]);
        auto sigma_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::SIGMA_3]);
        auto id_1 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::ID_1]);
        auto id_2 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::ID_2]);
        auto id_3 = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::ID_3]);
        auto z_perm = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Z_PERM]);
        auto z_perm_shift = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::Z_PERM_SHIFT]);
        auto lagrange_first = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::LAGRANGE_FIRST]);
        auto lagrange_last = UnivariateView<FF, RELATION_LENGTH>(extended_edges[MULTIVARIATE::LAGRANGE_LAST]);

        // Contribution (1)
        evals += ((z_perm + lagrange_first) * (w_1 + id_1 * beta + gamma) * (w_2 + id_2 * beta + gamma) *
                  (w_3 + id_3 * beta + gamma)) -
                 ((z_perm_shift + lagrange_last * public_input_delta) * (w_1 + sigma_1 * beta + gamma) *
                  (w_2 + sigma_2 * beta + gamma) * (w_3 + sigma_3 * beta + gamma));
    };

    void add_full_relation_value_contribution(auto& purported_evaluations, FF& full_honk_relation_value)
    {
        auto w_1 = purported_evaluations[MULTIVARIATE::W_L];
        auto w_2 = purported_evaluations[MULTIVARIATE::W_R];
        auto w_3 = purported_evaluations[MULTIVARIATE::W_O];
        auto sigma_1 = purported_evaluations[MULTIVARIATE::SIGMA_1];
        auto sigma_2 = purported_evaluations[MULTIVARIATE::SIGMA_2];
        auto sigma_3 = purported_evaluations[MULTIVARIATE::SIGMA_3];
        auto id_1 = purported_evaluations[MULTIVARIATE::ID_1];
        auto id_2 = purported_evaluations[MULTIVARIATE::ID_1];
        auto id_3 = purported_evaluations[MULTIVARIATE::ID_1];
        auto z_perm = purported_evaluations[MULTIVARIATE::Z_PERM];
        auto z_perm_shift = purported_evaluations[MULTIVARIATE::Z_PERM_SHIFT];
        auto lagrange_first = purported_evaluations[MULTIVARIATE::LAGRANGE_FIRST];
        auto lagrange_last = purported_evaluations[MULTIVARIATE::LAGRANGE_LAST];

        // Contribution (1)
        full_honk_relation_value += (z_perm + lagrange_first);
        full_honk_relation_value *= w_1 + beta_default * id_1 + gamma_default;
        full_honk_relation_value *= w_2 + beta_default * id_2 + gamma_default;
        full_honk_relation_value *= w_3 + beta_default * id_3 + gamma_default;
        full_honk_relation_value -= (z_perm_shift + lagrange_last * public_input_delta_default) *
                                    (w_1 + beta_default * sigma_1 + gamma_default) *
                                    (w_2 + beta_default * sigma_2 + gamma_default) *
                                    (w_3 + beta_default * sigma_3 + gamma_default);
    };
};
} // namespace honk::sumcheck