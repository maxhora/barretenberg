#pragma once // just adding these willy-nilly
#include "numeric/bitop/get_msb.hpp"
#include "proof_system/proving_key/proving_key.hpp"
#include "transcript/transcript_wrappers.hpp"
#include <array>
#include <algorithm>
#include <span>
#include <common/log.hpp>
#include <common/serialize.hpp>

namespace honk {
namespace sumcheck {

// std::span has no comparison operator, so this is a quick-and-dirty workaround for testing
// IMPROVEMENT(Cody): Move and/or implement as == in some class
bool span_arrays_equal(auto& lhs, auto& rhs)
{
    bool result(true);
    result = lhs.size() == rhs.size();
    if (result) {
        for (size_t i = 0; i < lhs.size(); i++) {
            result &= std::equal(lhs[i].begin(), lhs[i].end(), rhs[i].begin(), rhs[i].end());
        };
    }
    return result;
}

/**
 *
 * @brief A container for all of the Honk  polynomials (wire and selector polynomials, grand product, and much more).
 * These polynomials all low-degree extensions over H^d with H = {0, 1} (where d = ceil(log(number of gates))), hence
 * they are multilinear polynomials in d variables. As such, it is efficient to store these polynomials in terms of
 * univariate degree-1 polynomials.

 * Suppose now the Honk polynomials (multilinear in d variables) are called P_1, ..., P_N. At initialization,
 * we think of these as lying in a two-dimensional array, where each column records the value of one P_i on H^d. After
 * the first round, the array will be updated ('folded'), so that the first n/2 rows will represent the evaluations
 * P_i(X1, ..., X_{d-1}, u_d) as a low-degree extension on H^{d-1}. In reality, we elide copying all of the polynomial-
 * defining data by only populating folded_multivariates after the first round. I.e.:

       We imagine all of the defining polynomial data in a matrix like this:
                   | P_1 | P_2 | P_3 | P_4 | ... | P_N | N = number of multivariatesk
                   |-----------------------------------|
         group 0 --|  *  |  *  |  *  |  *  | ... |  *  | vertex 0
                 \-|  *  |  *  |  *  |  *  | ... |  *  | vertex 1
         group 1 --|  *  |  *  |  *  |  *  | ... |  *  | vertex 2
                 \-|  *  |  *  |  *  |  *  | ... |  *  | vertex 3
                   |  *  |  *  |  *  |  *  | ... |  *  |
       group m-1 --|  *  |  *  |  *  |  *  | ... |  *  | vertex n-2
                 \-|  *  |  *  |  *  |  *  | ... |  *  | vertex n-1
        m = n/2
                                       *
        Each group consists of N edges |, and our construction of univariates and folding
                                       *
        operations naturally operate on these groups of edges

 *
 * NOTE: With ~40 columns, prob only want to allocate 256 EdgeGroup's at once to keep stack under 1MB?
 * TODO(Cody): might want to just do C-style multidimensional array? for guaranteed adjacency?
 */
template <class FF_, size_t num_polys> class Multivariates {
  public:
    using FF = FF_;
    const size_t multivariate_n;
    const size_t multivariate_d;
    static constexpr size_t num = num_polys;

    std::array<std::span<FF>, num_polys> full_polynomials;
    // TODO(Cody): adjacency issues with std::array of std::vectors?
    // IMPROVEMENT(Cody): for each round after the first, we could release half of the memory reserved by
    // folded_polynomials.
    std::array<std::vector<FF>, num_polys> folded_polynomials;

    Multivariates() = default;

    // TODO(Cody): static span extent below more efficient
    explicit Multivariates(std::array<std::span<FF>, num_polys> full_polynomials)
        : multivariate_n(full_polynomials[0].size())
        , multivariate_d(numeric::get_msb(multivariate_n))
        , full_polynomials(full_polynomials)
    {
        for (auto& polynomial : folded_polynomials) {
            polynomial.resize(multivariate_n >> 1);
        }
    };

    explicit Multivariates(const std::shared_ptr<waffle::proving_key>& proving_key)
        : multivariate_n(proving_key->n)
        , multivariate_d(proving_key->log_n)
    {
        for (size_t i = 0; i < waffle::STANDARD_HONK_MANIFEST_SIZE; i++) {
            auto label = proving_key->polynomial_manifest[i].polynomial_label;
            info("i, label = ", i, ", ", label);
            full_polynomials[i] = proving_key->polynomial_cache.get(std::string(label));
        }

        info("full_polynomials[0][0]: ", full_polynomials[0][0]);
        info("full_polynomials[1][0]: ", full_polynomials[1][0]);
        info("full_polynomials[2][0]: ", full_polynomials[2][0]);
        info("full_polynomials[3][0]: ", full_polynomials[3][0]);
        info("full_polynomials[4][0]: ", full_polynomials[4][0]);
        info("full_polynomials[5][0]: ", full_polynomials[5][0]);
        info("full_polynomials[6][0]: ", full_polynomials[6][0]);
        info("full_polynomials[7][0]: ", full_polynomials[7][0]);
        info("full_polynomials[8][0]: ", full_polynomials[8][0]);
        info("full_polynomials[9][0]: ", full_polynomials[9][0]);
        info("full_polynomials[10][0]: ", full_polynomials[10][0]);
        info("full_polynomials[11][0]: ", full_polynomials[11][0]);
        info("full_polynomials[12][0]: ", full_polynomials[12][0]);
        info("full_polynomials[13][0]: ", full_polynomials[13][0]);
        info("full_polynomials[14][0]: ", full_polynomials[14][0]);
        info("full_polynomials[15][0]: ", full_polynomials[15][0]);
        info("full_polynomials[16][0]: ", full_polynomials[16][0]);
        for (auto& polynomial : folded_polynomials) {
            polynomial.reserve(multivariate_n >> 1);
            polynomial.resize(multivariate_n >> 1);
        }
    }

    explicit Multivariates(transcript::StandardTranscript transcript)
        : multivariate_n([](std::vector<uint8_t> buffer) {
            return static_cast<size_t>(buffer[3]) + (static_cast<size_t>(buffer[2]) << 8) +
                   (static_cast<size_t>(buffer[1]) << 16) + (static_cast<size_t>(buffer[0]) << 24);
        }(transcript.get_element("circuit_size")))
        , multivariate_d(numeric::get_msb(multivariate_n))
    {
        // std::vector<uint8_t> buffer = transcript.get_element("circuit_size");
        // multivariate_n = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24);
        // multivariate_d = numeric::get_msb(multivariate_n);
    }

    // TODO(Cody): Rename. fold is not descriptive, and it's already in use in the Gemini context.
    //             Probably just call it partial_evaluation?
    /**
     * @brief Evaluate at the round challenge and prepare class for next round.
     * Illustration of layout in example of first round when d==3 (showing just one Honk polynomial,
     * i.e., what happens in just one column of our two-dimensional array):
     *
     * groups    vertex terms              collected vertex terms               groups after folding
     *     g0 -- v0 (1-X1)(1-X2)(1-X3) --- (v0(1-X3) + v1 X3) (1-X1)(1-X2) ---- (v0(1-u3) + v1 u3) (1-X1)(1-X2)
     *        \- v1 (1-X1)(1-X2)  X3   --/                                  --- (v2(1-u3) + v3 u3) (1-X1)  X2
     *     g1 -- v2 (1-X1)  X2  (1-X3) --- (v1(1-X3) + v2 X3) (1-X1)  X2  -/ -- (v4(1-u3) + v5 u3)   X1  (1-X2)
     *        \- v3 (1-X1)  X2    X3   --/                                  / - (v6(1-u3) + v7 u3)   X1    X2
     *     g2 -- v4   X1  (1-X2)(1-X3) --- (v3(1-X3) + v4 X3)   X1  (1-X2)-/ /
     *        \- v5   X1  (1-X2)  X3   --/                                  /
     *     g3 -- v6   X1    X2  (1-X3) --- (v5(1-X3) + v6 X3)   X1    X2  -/
     *        \- v7   X1    X2    X3   --/
     *
     * @param challenge
     */
    void fold(auto& polynomials, size_t round_size, const FF& round_challenge)
    {
        // after the first round, operate in place on folded_polynomials
        for (size_t j = 0; j < num_polys; ++j) {
            for (size_t i = 0; i < round_size; i += 2) {
                FF new_value = polynomials[j][i] + round_challenge * (polynomials[j][i + 1] - polynomials[j][i]);
                // info(new_value);
                folded_polynomials[j][i >> 1] = new_value;
            }
        }

        info("Folding from size ", round_size);
        info("folded_polynomials[0][0]: ", folded_polynomials[0][0]);
        info("folded_polynomials[1][0]: ", folded_polynomials[1][0]);
        info("folded_polynomials[2][0]: ", folded_polynomials[2][0]);
        info("folded_polynomials[3][0]: ", folded_polynomials[3][0]);
        info("folded_polynomials[4][0]: ", folded_polynomials[4][0]);
        info("folded_polynomials[5][0]: ", folded_polynomials[5][0]);
        info("folded_polynomials[6][0]: ", folded_polynomials[6][0]);
        info("folded_polynomials[7][0]: ", folded_polynomials[7][0]);
        info("folded_polynomials[8][0]: ", folded_polynomials[8][0]);
        info("folded_polynomials[9][0]: ", folded_polynomials[9][0]);
        info("folded_polynomials[10][0]: ", folded_polynomials[10][0]);
        info("folded_polynomials[11][0]: ", folded_polynomials[11][0]);
        info("folded_polynomials[12][0]: ", folded_polynomials[12][0]);
        info("folded_polynomials[13][0]: ", folded_polynomials[13][0]);
        info("folded_polynomials[14][0]: ", folded_polynomials[14][0]);
        info("folded_polynomials[15][0]: ", folded_polynomials[15][0]);
        info("folded_polynomials[16][0]: ", folded_polynomials[16][0]);
    };

    std::array<FF, num_polys> batch_evaluate()
    {
        // TODO(Cody): these just get extracted from the folded multivariates
        // For now, at least initialize properly.
        std::array<FF, num_polys> result;
        for (auto& entry : result) {
            entry = 1;
        }
        return result;
    };
};
} // namespace sumcheck
} // namespace honk