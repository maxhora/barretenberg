#pragma once

#include "composer_helper/example_honk_composer_helper.hpp"
#include "barretenberg/honk/proof_system/example_prover.hpp"
#include "barretenberg/honk/circuit_constructors/example_circuit_constructor.hpp"
#include "barretenberg/srs/reference_string/file_reference_string.hpp"
#include "barretenberg/transcript/manifest.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"

namespace honk {
/**
 * @brief I'm only impelmenting this so I can see about instantiating the uint tests.
 *
 */
class ExampleHonkComposer {
  public:
    static constexpr plonk::ComposerType type = plonk::ComposerType::EXAMPLE_HONK;

    static constexpr size_t UINT_LOG2_BASE = 2;
    // An instantiation of the circuit constructor that only depends on arithmetization, not  on the proof system
    ExampleCircuitConstructor circuit_constructor;
    // Composer helper contains all proof-related material that is separate from circuit creation such as:
    // 1) Proving and verification keys
    // 2) CRS
    // 3) Converting variables to witness vectors/polynomials
    ExampleHonkComposerHelper composer_helper;

    // Leaving it in for now just in case
    bool contains_recursive_proof = false;
    static constexpr size_t program_width = 5;

    /**Standard methods*/

    ExampleHonkComposer(const size_t size_hint = 0)
        : circuit_constructor(size_hint)
        , num_gates(circuit_constructor.num_gates)
        , variables(circuit_constructor.variables){};

    ExampleHonkComposer(std::string const& crs_path, const size_t size_hint = 0)
        : ExampleHonkComposer(
              std::unique_ptr<bonk::ReferenceStringFactory>(new bonk::FileReferenceStringFactory(crs_path)),
              size_hint){};

    ExampleHonkComposer(std::shared_ptr<bonk::ReferenceStringFactory> const& crs_factory, const size_t size_hint = 0)
        : circuit_constructor(size_hint)
        , composer_helper(crs_factory)
        , num_gates(circuit_constructor.num_gates)
        , variables(circuit_constructor.variables)

    {}
    ExampleHonkComposer(std::unique_ptr<bonk::ReferenceStringFactory>&& crs_factory, const size_t size_hint = 0)
        : circuit_constructor(size_hint)
        , composer_helper(std::move(crs_factory))
        , num_gates(circuit_constructor.num_gates)
        , variables(circuit_constructor.variables)

    {}

    ExampleHonkComposer(std::shared_ptr<bonk::proving_key> const& p_key,
                        std::shared_ptr<bonk::verification_key> const& v_key,
                        size_t size_hint = 0)
        : circuit_constructor(size_hint)
        , composer_helper(p_key, v_key)
        , num_gates(circuit_constructor.num_gates)
        , variables(circuit_constructor.variables)
    {}

    ExampleHonkComposer(const ExampleHonkComposer& other) = delete;
    ExampleHonkComposer(ExampleHonkComposer&& other) = default;
    ExampleHonkComposer& operator=(const ExampleHonkComposer& other) = delete;
    // TODO(#230)(Cody): This constructor started to be implicitly deleted when I added `n` and `variables` members.
    // This is a temporary measure until we can rewrite Plonk and all tests using circuit builder methods in place of
    // composer methods, where appropriate. ExampleHonkComposer& operator=(ExampleHonkComposer&& other) = default;
    ~ExampleHonkComposer() = default;

    size_t get_num_gates() const { return circuit_constructor.get_num_gates(); }

    /**Methods related to circuit construction
     * They simply get proxied to the circuit constructor
     */
    void assert_equal(const uint32_t a_variable_idx, const uint32_t b_variable_idx, std::string const& msg)
    {
        circuit_constructor.assert_equal(a_variable_idx, b_variable_idx, msg);
    }
    void assert_equal_constant(uint32_t const a_idx,
                               barretenberg::fr const& b,
                               std::string const& msg = "assert equal constant")
    {
        circuit_constructor.assert_equal_constant(a_idx, b, msg);
    }

    void create_add_gate(const add_triple& in) { circuit_constructor.create_add_gate(in); }
    void create_mul_gate(const mul_triple& in) { circuit_constructor.create_mul_gate(in); }
    void create_bool_gate(const uint32_t a) { circuit_constructor.create_bool_gate(a); }
    void create_poly_gate(const poly_triple& in) { circuit_constructor.create_poly_gate(in); }
    void create_big_add_gate(const add_quad& in) { circuit_constructor.create_big_add_gate(in); }
    void create_big_add_gate_with_bit_extraction(const add_quad& in)
    {
        circuit_constructor.create_big_add_gate_with_bit_extraction(in);
    }
    void create_big_mul_gate(const mul_quad& in) { circuit_constructor.create_big_mul_gate(in); }
    void create_balanced_add_gate(const add_quad& in) { circuit_constructor.create_balanced_add_gate(in); }

    void fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value)
    {
        circuit_constructor.fix_witness(witness_index, witness_value);
    }

    void create_range_constraint(const uint32_t variable_index,
                                 const size_t num_bits,
                                 std::string const& msg = "create_range_constraint")
    {
        circuit_constructor.create_range_constraint(variable_index, num_bits, msg);
    }

    uint32_t add_variable(const barretenberg::fr& in) { return circuit_constructor.add_variable(in); }

    uint32_t add_public_variable(const barretenberg::fr& in) { return circuit_constructor.add_public_variable(in); }

    virtual void set_public_input(const uint32_t witness_index)
    {
        return circuit_constructor.set_public_input(witness_index);
    }

    uint32_t put_constant_variable(const barretenberg::fr& variable)
    {
        return circuit_constructor.put_constant_variable(variable);
    }

    size_t get_num_constant_gates() const { return circuit_constructor.get_num_constant_gates(); }

    bool check_circuit() { return circuit_constructor.check_circuit(); }

    barretenberg::fr get_variable(const uint32_t index) const { return circuit_constructor.get_variable(index); }

    /**Proof and verification-related methods*/

    std::shared_ptr<bonk::proving_key> compute_proving_key()
    {
        return composer_helper.compute_proving_key(circuit_constructor);
    }

    std::shared_ptr<bonk::verification_key> compute_verification_key()
    {
        return composer_helper.compute_verification_key(circuit_constructor);
    }

    uint32_t zero_idx = 0;

    void compute_witness() { composer_helper.compute_witness(circuit_constructor); };

    ExampleVerifier create_verifier() { return composer_helper.create_verifier(circuit_constructor); }
    ExampleProver create_prover() { return composer_helper.create_prover(circuit_constructor); };

    size_t& num_gates;
    std::vector<barretenberg::fr>& variables;
    bool failed() const { return circuit_constructor.failed(); };
    const std::string& err() const { return circuit_constructor.err(); };
    void failure(std::string msg) { circuit_constructor.failure(msg); }
};
} // namespace honk
