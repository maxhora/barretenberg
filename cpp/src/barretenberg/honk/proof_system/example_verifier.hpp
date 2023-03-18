#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "./program_settings.hpp"
#include "barretenberg/proof_system/verification_key/verification_key.hpp"
#include "barretenberg/transcript/manifest.hpp"
#include "barretenberg/plonk/proof_system/commitment_scheme/commitment_scheme.hpp"
#include "../sumcheck/sumcheck.hpp"
#include "../sumcheck/relations/arithmetic_relation.hpp"
#include "barretenberg/honk/pcs/commitment_key.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"
#include "barretenberg/honk/pcs/gemini/gemini.hpp"
#include "barretenberg/honk/pcs/shplonk/shplonk_single.hpp"
#include "barretenberg/honk/pcs/kzg/kzg.hpp"

namespace honk {
class ExampleVerifier {

  public:
    // WORKTODO: add a new manifest; template by flavor to share that manifest.
    // WORKTODO: write new manifest.
    ExampleVerifier(std::shared_ptr<bonk::verification_key> verifier_key = nullptr,
                    const transcript::Manifest& manifest = honk::ExampleHonk::create_manifest(0));
    ExampleVerifier(ExampleVerifier&& other);
    ExampleVerifier(const ExampleVerifier& other) = delete;
    ExampleVerifier& operator=(const ExampleVerifier& other) = delete;
    ExampleVerifier& operator=(ExampleVerifier&& other);

    bool verify_proof(const plonk::proof& proof);
    transcript::Manifest manifest;

    static constexpr size_t num_wires = 5;
    std::shared_ptr<bonk::verification_key> key;
    std::map<std::string, barretenberg::g1::affine_element> kate_g1_elements;
    std::map<std::string, barretenberg::fr> kate_fr_elements;
    std::shared_ptr<pcs::kzg::VerificationKey> kate_verification_key;
};

} // namespace honk
