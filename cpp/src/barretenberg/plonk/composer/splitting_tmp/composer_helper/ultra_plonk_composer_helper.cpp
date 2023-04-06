#include "ultra_plonk_composer_helper.hpp"
#include "barretenberg/plonk/proof_system/types/program_settings.hpp"
#include "barretenberg/plonk/proof_system/types/prover_settings.hpp"
#include "barretenberg/plonk/proof_system/verifier/verifier.hpp"
#include "barretenberg/proof_system/circuit_constructors/ultra_circuit_constructor.hpp"
#include "barretenberg/proof_system/composer/permutation_helper.hpp"
#include "barretenberg/plonk/proof_system/commitment_scheme/kate_commitment_scheme.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace proof_system::plonk {

/**
 * @brief Computes `this.witness`, which is basiclly a set of polynomials mapped-to by strings.
 *
 * Note: this doesn't actually compute the _entire_ witness. Things missing: randomness for blinding both the wires and
 * sorted `s` poly, lookup rows of the wire witnesses, the values of `z_lookup`, `z`. These are all calculated
 * elsewhere.
 */

void UltraPlonkComposerHelper::compute_witness(CircuitConstructor& circuit_constructor)
{
    if (computed_witness) {
        return;
    }

    size_t tables_size = 0;
    size_t lookups_size = 0;
    for (const auto& table : circuit_constructor.lookup_tables) {
        tables_size += table.size;
        lookups_size += table.lookup_gates.size();
    }

    const size_t filled_gates = circuit_constructor.num_gates + circuit_constructor.public_inputs.size();
    const size_t total_num_gates = std::max(filled_gates, tables_size + lookups_size);

    const size_t subgroup_size = circuit_constructor.get_circuit_subgroup_size(total_num_gates + NUM_RESERVED_GATES);

    // Pad the wires (pointers to `witness_indices` of the `variables` vector).
    // Note: the remaining NUM_RESERVED_GATES indices are padded with zeros within `compute_witness_base` (called
    // next).
    for (size_t i = filled_gates; i < total_num_gates; ++i) {
        circuit_constructor.w_l.emplace_back(circuit_constructor.zero_idx);
        circuit_constructor.w_r.emplace_back(circuit_constructor.zero_idx);
        circuit_constructor.w_o.emplace_back(circuit_constructor.zero_idx);
        circuit_constructor.w_4.emplace_back(circuit_constructor.zero_idx);
    }

    // TODO(luke): subgroup size was already computed above but compute_witness_base computes it again. If we pass in
    // NUM_RANDOMIZED_GATES (as in the other split composers) the resulting sizes can differ. Reconcile this.
    auto wire_polynomial_evaluations =
        construct_wire_polynomials_base<Flavor>(circuit_constructor, total_num_gates, NUM_RANDOMIZED_GATES);

    for (size_t j = 0; j < program_width; ++j) {
        std::string index = std::to_string(j + 1);
        circuit_proving_key->polynomial_store.put("w_" + index + "_lagrange",
                                                  std::move(wire_polynomial_evaluations[j]));
    }

    polynomial s_1(subgroup_size);
    polynomial s_2(subgroup_size);
    polynomial s_3(subgroup_size);
    polynomial s_4(subgroup_size);
    polynomial z_lookup(subgroup_size + 1); // Only instantiated in this function; nothing assigned.

    // Save space for adding random scalars in the s polynomial later.
    // The subtracted 1 allows us to insert a `1` at the end, to ensure the evaluations (and hence coefficients) aren't
    // all 0.
    // See ComposerBase::compute_proving_key_base for further explanation, as a similar trick is done there.
    size_t count = subgroup_size - tables_size - lookups_size - s_randomness - 1;
    for (size_t i = 0; i < count; ++i) {
        s_1[i] = 0;
        s_2[i] = 0;
        s_3[i] = 0;
        s_4[i] = 0;
    }

    for (auto& table : circuit_constructor.lookup_tables) {
        const fr table_index(table.table_index);
        auto& lookup_gates = table.lookup_gates;
        for (size_t i = 0; i < table.size; ++i) {
            if (table.use_twin_keys) {
                lookup_gates.push_back({
                    {
                        table.column_1[i].from_montgomery_form().data[0],
                        table.column_2[i].from_montgomery_form().data[0],
                    },
                    {
                        table.column_3[i],
                        0,
                    },
                });
            } else {
                lookup_gates.push_back({
                    {
                        table.column_1[i].from_montgomery_form().data[0],
                        0,
                    },
                    {
                        table.column_2[i],
                        table.column_3[i],
                    },
                });
            }
        }

#ifdef NO_TBB
        std::sort(lookup_gates.begin(), lookup_gates.end());
#else
        std::sort(std::execution::par_unseq, lookup_gates.begin(), lookup_gates.end());
#endif

        for (const auto& entry : lookup_gates) {
            const auto components = entry.to_sorted_list_components(table.use_twin_keys);
            s_1[count] = components[0];
            s_2[count] = components[1];
            s_3[count] = components[2];
            s_4[count] = table_index;
            ++count;
        }
    }

    // Initialise the `s_randomness` positions in the s polynomials with 0.
    // These will be the positions where we will be adding random scalars to add zero knowledge
    // to plookup (search for `Blinding` in plonk/proof_system/widgets/random_widgets/plookup_widget_impl.hpp
    // ProverPlookupWidget::compute_sorted_list_polynomial())
    for (size_t i = 0; i < s_randomness; ++i) {
        s_1[count] = 0;
        s_2[count] = 0;
        s_3[count] = 0;
        s_4[count] = 0;
        ++count;
    }

    circuit_proving_key->polynomial_store.put("s_1_lagrange", std::move(s_1));
    circuit_proving_key->polynomial_store.put("s_2_lagrange", std::move(s_2));
    circuit_proving_key->polynomial_store.put("s_3_lagrange", std::move(s_3));
    circuit_proving_key->polynomial_store.put("s_4_lagrange", std::move(s_4));

    computed_witness = true;
}

UltraProver UltraPlonkComposerHelper::create_prover(CircuitConstructor& circuit_constructor)
{
    finalize_circuit(circuit_constructor);

    compute_proving_key(circuit_constructor);
    compute_witness(circuit_constructor);

    UltraProver output_state(circuit_proving_key, create_manifest(circuit_constructor.public_inputs.size()));

    std::unique_ptr<ProverPermutationWidget<4, true>> permutation_widget =
        std::make_unique<ProverPermutationWidget<4, true>>(circuit_proving_key.get());

    std::unique_ptr<ProverPlookupWidget<>> plookup_widget =
        std::make_unique<ProverPlookupWidget<>>(circuit_proving_key.get());

    std::unique_ptr<ProverPlookupArithmeticWidget<ultra_settings>> arithmetic_widget =
        std::make_unique<ProverPlookupArithmeticWidget<ultra_settings>>(circuit_proving_key.get());

    std::unique_ptr<ProverGenPermSortWidget<ultra_settings>> sort_widget =
        std::make_unique<ProverGenPermSortWidget<ultra_settings>>(circuit_proving_key.get());

    std::unique_ptr<ProverEllipticWidget<ultra_settings>> elliptic_widget =
        std::make_unique<ProverEllipticWidget<ultra_settings>>(circuit_proving_key.get());

    std::unique_ptr<ProverPlookupAuxiliaryWidget<ultra_settings>> auxiliary_widget =
        std::make_unique<ProverPlookupAuxiliaryWidget<ultra_settings>>(circuit_proving_key.get());

    output_state.random_widgets.emplace_back(std::move(permutation_widget));
    output_state.random_widgets.emplace_back(std::move(plookup_widget));

    output_state.transition_widgets.emplace_back(std::move(arithmetic_widget));
    output_state.transition_widgets.emplace_back(std::move(sort_widget));
    output_state.transition_widgets.emplace_back(std::move(elliptic_widget));
    output_state.transition_widgets.emplace_back(std::move(auxiliary_widget));

    std::unique_ptr<KateCommitmentScheme<ultra_settings>> kate_commitment_scheme =
        std::make_unique<KateCommitmentScheme<ultra_settings>>();

    output_state.commitment_scheme = std::move(kate_commitment_scheme);

    return output_state;
}

/**
 * Create verifier: compute verification key,
 * initialize verifier with it and an initial manifest and initialize commitment_scheme.
 *
 * @return The verifier.
 * */
// TODO(Cody): This should go away altogether.

plonk::UltraVerifier UltraPlonkComposerHelper::create_verifier(const CircuitConstructor& circuit_constructor)
{
    auto verification_key = compute_verification_key(circuit_constructor);

    plonk::UltraVerifier output_state(circuit_verification_key,
                                      create_manifest(circuit_constructor.public_inputs.size()));

    std::unique_ptr<plonk::KateCommitmentScheme<plonk::ultra_settings>> kate_commitment_scheme =
        std::make_unique<plonk::KateCommitmentScheme<plonk::ultra_settings>>();

    output_state.commitment_scheme = std::move(kate_commitment_scheme);

    return output_state;
}

std::shared_ptr<proving_key> UltraPlonkComposerHelper::compute_proving_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_proving_key) {
        return circuit_proving_key;
    }

    size_t tables_size = 0;
    size_t lookups_size = 0;
    for (const auto& table : circuit_constructor.lookup_tables) {
        tables_size += table.size;
        lookups_size += table.lookup_gates.size();
    }

    const size_t minimum_circuit_size = tables_size + lookups_size;
    const size_t num_randomized_gates = NUM_RANDOMIZED_GATES;
    // Initialize circuit_proving_key
    // TODO(#229)(Kesha): replace composer types.
    circuit_proving_key = initialize_proving_key<Flavor>(
        circuit_constructor, crs_factory_.get(), minimum_circuit_size, num_randomized_gates, ComposerType::PLOOKUP);

    construct_selector_polynomials<Flavor>(circuit_constructor, circuit_proving_key.get());

    enforce_nonzero_selector_polynomials<Flavor>(circuit_constructor, circuit_proving_key.get());

    compute_monomial_and_coset_selector_forms(circuit_proving_key.get(), ultra_selector_properties());

    compute_plonk_generalized_sigma_permutations<CircuitConstructor::program_width>(circuit_constructor,
                                                                                    circuit_proving_key.get());

    const size_t subgroup_size = circuit_proving_key->circuit_size;

    polynomial poly_q_table_column_1(subgroup_size);
    polynomial poly_q_table_column_2(subgroup_size);
    polynomial poly_q_table_column_3(subgroup_size);
    polynomial poly_q_table_column_4(subgroup_size);

    size_t offset = subgroup_size - tables_size - s_randomness - 1;

    // Create lookup selector polynomials which interpolate each table column.
    // Our selector polys always need to interpolate the full subgroup size, so here we offset so as to
    // put the table column's values at the end. (The first gates are for non-lookup constraints).
    // [0, ..., 0, ...table, 0, 0, 0, x]
    //  ^^^^^^^^^  ^^^^^^^^  ^^^^^^^  ^nonzero to ensure uniqueness and to avoid infinity commitments
    //  |          table     randomness
    //  ignored, as used for regular constraints and padding to the next power of 2.

    for (size_t i = 0; i < offset; ++i) {
        poly_q_table_column_1[i] = 0;
        poly_q_table_column_2[i] = 0;
        poly_q_table_column_3[i] = 0;
        poly_q_table_column_4[i] = 0;
    }

    for (const auto& table : circuit_constructor.lookup_tables) {
        const fr table_index(table.table_index);

        for (size_t i = 0; i < table.size; ++i) {
            poly_q_table_column_1[offset] = table.column_1[i];
            poly_q_table_column_2[offset] = table.column_2[i];
            poly_q_table_column_3[offset] = table.column_3[i];
            poly_q_table_column_4[offset] = table_index;
            ++offset;
        }
    }

    // Initialise the last `s_randomness` positions in table polynomials with 0. We don't need to actually randomise
    // the table polynomials.
    for (size_t i = 0; i < s_randomness; ++i) {
        poly_q_table_column_1[offset] = 0;
        poly_q_table_column_2[offset] = 0;
        poly_q_table_column_3[offset] = 0;
        poly_q_table_column_4[offset] = 0;
        ++offset;
    }

    // // In the case of using UltraPlonkComposer for a circuit which does _not_ make use of any lookup tables, all four
    // // table columns would be all zeros. This would result in these polys' commitments all being the point at
    // infinity
    // // (which is bad because our point arithmetic assumes we'll never operate on the point at infinity). To avoid
    // this,
    // // we set the last evaluation of each poly to be nonzero. The last `num_roots_cut_out_of_vanishing_poly = 4`
    // // evaluations are ignored by constraint checks; we arbitrarily choose the very-last evaluation to be nonzero.
    // See
    // // ComposerBase::compute_proving_key_base for further explanation, as a similar trick is done there. We could
    // // have chosen `1` for each such evaluation here, but that would have resulted in identical commitments for
    // // all four columns. We don't want to have equal commitments, because biggroup operations assume no points are
    // // equal, so if we tried to verify an ultra proof in a circuit, the biggroup operations would fail. To combat
    // // this, we just choose distinct values:
    ASSERT(offset == subgroup_size - 1);
    auto unique_last_value =
        get_num_selectors() + 1; // Note: in compute_proving_key_base, moments earlier, each selector
                                 // vector was given a unique last value from 1..num_selectors. So we
                                 // avoid those values and continue the count, to ensure uniqueness.
    poly_q_table_column_1[subgroup_size - 1] = unique_last_value;
    poly_q_table_column_2[subgroup_size - 1] = ++unique_last_value;
    poly_q_table_column_3[subgroup_size - 1] = ++unique_last_value;
    poly_q_table_column_4[subgroup_size - 1] = ++unique_last_value;

    add_table_column_selector_poly_to_proving_key(poly_q_table_column_1, "table_value_1");
    add_table_column_selector_poly_to_proving_key(poly_q_table_column_2, "table_value_2");
    add_table_column_selector_poly_to_proving_key(poly_q_table_column_3, "table_value_3");
    add_table_column_selector_poly_to_proving_key(poly_q_table_column_4, "table_value_4");

    // Instantiate z_lookup and s polynomials in the proving key (no values assigned yet).
    // Note: might be better to add these polys to cache only after they've been computed, as is convention
    // TODO(luke): Don't put empty polynomials in the store, just add these where they're computed
    polynomial z_lookup_fft(subgroup_size * 4);
    polynomial s_fft(subgroup_size * 4);
    circuit_proving_key->polynomial_store.put("z_lookup_fft", std::move(z_lookup_fft));
    circuit_proving_key->polynomial_store.put("s_fft", std::move(s_fft));

    // Copy memory read/write record data into proving key. Prover needs to know which gates contain a read/write
    // 'record' witness on the 4th wire. This wire value can only be fully computed once the first 3 wire polynomials
    // have been committed to. The 4th wire on these gates will be a random linear combination of the first 3 wires,
    // using the plookup challenge `eta`
    std::copy(circuit_constructor.memory_read_records.begin(),
              circuit_constructor.memory_read_records.end(),
              std::back_inserter(circuit_proving_key->memory_read_records));
    std::copy(circuit_constructor.memory_write_records.begin(),
              circuit_constructor.memory_write_records.end(),
              std::back_inserter(circuit_proving_key->memory_write_records));

    circuit_proving_key->recursive_proof_public_input_indices =
        std::vector<uint32_t>(recursive_proof_public_input_indices.begin(), recursive_proof_public_input_indices.end());

    circuit_proving_key->contains_recursive_proof = contains_recursive_proof;

    return circuit_proving_key;
}

/**
 * Compute verification key consisting of selector precommitments.
 *
 * @return Pointer to created circuit verification key.
 * */

std::shared_ptr<plonk::verification_key> UltraPlonkComposerHelper::compute_verification_key(
    const CircuitConstructor& circuit_constructor)
{
    if (circuit_verification_key) {
        return circuit_verification_key;
    }

    if (!circuit_proving_key) {
        compute_proving_key(circuit_constructor);
    }
    circuit_verification_key = compute_verification_key_common(circuit_proving_key, crs_factory_->get_verifier_crs());

    circuit_verification_key->composer_type = type; // Invariably plookup for this class.

    // See `add_recusrive_proof()` for how this recursive data is assigned.
    circuit_verification_key->recursive_proof_public_input_indices =
        std::vector<uint32_t>(recursive_proof_public_input_indices.begin(), recursive_proof_public_input_indices.end());

    circuit_verification_key->contains_recursive_proof = contains_recursive_proof;

    return circuit_verification_key;
}

void UltraPlonkComposerHelper::add_table_column_selector_poly_to_proving_key(polynomial& selector_poly_lagrange_form,
                                                                             const std::string& tag)
{
    polynomial selector_poly_lagrange_form_copy(selector_poly_lagrange_form, circuit_proving_key->small_domain.size);

    selector_poly_lagrange_form.ifft(circuit_proving_key->small_domain);
    auto& selector_poly_coeff_form = selector_poly_lagrange_form;

    polynomial selector_poly_coset_form(selector_poly_coeff_form, circuit_proving_key->circuit_size * 4);
    selector_poly_coset_form.coset_fft(circuit_proving_key->large_domain);

    circuit_proving_key->polynomial_store.put(tag, std::move(selector_poly_coeff_form));
    circuit_proving_key->polynomial_store.put(tag + "_lagrange", std::move(selector_poly_lagrange_form_copy));
    circuit_proving_key->polynomial_store.put(tag + "_fft", std::move(selector_poly_coset_form));
}

} // namespace proof_system::plonk
