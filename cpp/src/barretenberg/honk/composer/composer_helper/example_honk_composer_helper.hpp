#pragma once

#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/srs/reference_string/file_reference_string.hpp"
#include "barretenberg/proof_system/proving_key/proving_key.hpp"
#include "barretenberg/honk/proof_system/example_prover.hpp"
#include "barretenberg/honk/proof_system/example_verifier.hpp"
#include "barretenberg/honk/circuit_constructors/standard_circuit_constructor.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/proof_system/verification_key/verification_key.hpp"
#include "barretenberg/plonk/proof_system/verifier/verifier.hpp"
#include "barretenberg/proof_system/composer/composer_base.hpp"
#include "composer_helper_lib.hpp"
#include "permutation_helper.hpp"

#include <utility>

namespace honk {
// TODO(Kesha): change initializations to specify this parameter
// Cody: What does this mean?

/**
 * @brief A composer with a subset of the functionality of UltraComposer having extra wires.
 *
 */
class ExampleHonkComposerHelper {
  public:
    static constexpr size_t NUM_RANDOMIZED_GATES = 2; // equal to the number of multilinear evaluations leaked
    static constexpr size_t program_width = 5;        // WORKNOTE: Flavor?
    std::shared_ptr<bonk::proving_key> circuit_proving_key;
    std::vector<barretenberg::polynomial> wire_polynomials; // WORKNOTE: Allocate everything here?
    std::shared_ptr<bonk::verification_key> circuit_verification_key;
    // TODO(#218)(kesha): we need to put this into the commitment key, so that the composer doesn't have to handle srs
    // at all
    std::shared_ptr<bonk::ReferenceStringFactory> crs_factory_;
    bool computed_witness = false; // WORKNOTE: Really necessary?
    // WORKNOTE: This could go in a base class.
    ExampleHonkComposerHelper()
        : ExampleHonkComposerHelper(
              std::shared_ptr<bonk::ReferenceStringFactory>(new bonk::FileReferenceStringFactory("../srs_db/ignition")))
    {}
    ExampleHonkComposerHelper(std::shared_ptr<bonk::ReferenceStringFactory> crs_factory)
        : crs_factory_(std::move(crs_factory))
    {}

    ExampleHonkComposerHelper(std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory)
        : crs_factory_(std::move(crs_factory))
    {}
    ExampleHonkComposerHelper(std::shared_ptr<bonk::proving_key> p_key, std::shared_ptr<bonk::verification_key> v_key)
        : circuit_proving_key(std::move(p_key))
        , circuit_verification_key(std::move(v_key))
    {}
    ExampleHonkComposerHelper(ExampleHonkComposerHelper&& other) noexcept = default;
    ExampleHonkComposerHelper(const ExampleHonkComposerHelper& other) = delete;
    ExampleHonkComposerHelper& operator=(ExampleHonkComposerHelper&& other) noexcept = default;
    ExampleHonkComposerHelper& operator=(const ExampleHonkComposerHelper& other) = delete;
    ~ExampleHonkComposerHelper() = default;

    // WORKNOTE: auto should possibly be a particular circuit constructor type.
    std::shared_ptr<bonk::proving_key> compute_proving_key(const auto& circuit_constructor);
    std::shared_ptr<bonk::verification_key> compute_verification_key(const auto& circuit_constructor);

    ExampleVerifier create_verifier(const auto& circuit_constructor);

    // WORKNOTE: Flavor is for creating manifest.
    ExampleProver create_prover(const auto& circuit_constructor);

    // TODO(#216)(Adrian): Seems error prone to provide the number of randomized gates
    // Cody: Where should this go? In the flavor (or whatever that becomes)?
    std::shared_ptr<bonk::proving_key> compute_proving_key_base(
        const auto& circuit_constructor,
        const size_t minimum_ciricut_size = 0,
        const size_t num_randomized_gates = NUM_RANDOMIZED_GATES);
    // This needs to be static as it may be used only to compute the selector commitments.

    // WORKNOTE: Move this function to shared lib.
    static std::shared_ptr<bonk::verification_key> compute_verification_key_base(
        std::shared_ptr<bonk::proving_key> const& proving_key,
        std::shared_ptr<bonk::VerifierReferenceString> const& vrs);

    void compute_witness(const auto& circuit_constructor, const size_t minimum_circuit_size = 0);
};

} // namespace honk
