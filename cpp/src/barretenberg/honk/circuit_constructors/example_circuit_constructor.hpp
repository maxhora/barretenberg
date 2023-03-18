#pragma once
#include "circuit_constructor_base.hpp"
#include "barretenberg/plonk/composer/plookup_tables/plookup_tables.hpp"
#include "barretenberg/plonk/proof_system/constants.hpp"
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/flavor/flavor.hpp"
#include <cstddef>

namespace bonk {

// WORKNOTE: put width into flavor.
class ExampleCircuitConstructor : public CircuitConstructorBase<5> {
  public:
    // WORKTODO: How/why are these coupled and what external relationships exist?
    // WORKTODO: check this
    enum ExampleSelectors { QM, Q1, Q2, Q3, Q4, QC, QLOOKUPTYPE, QSORT, NUM };
    static inline std::vector<std::string> example_selector_names()
    {
        std::vector<std::string> result{ "q_m", "q_1", "q_2", "q_3", "q_4", "q_c", "q_lookup_type", "q_sort" };
        return result;
    }
    // TODO(#216)(Kesha): replace this with Honk enums after we have a verifier and no longer depend on plonk
    // prover/verifier
    static constexpr plonk::ComposerType type = plonk::ComposerType::EXAMPLE_HONK;
    static constexpr size_t UINT_LOG2_BASE = 2;

    static constexpr size_t DEFAULT_PLOOKUP_RANGE_BITNUM = 14;
    static constexpr size_t DEFAULT_PLOOKUP_RANGE_STEP_SIZE = 3;
    static constexpr size_t DEFAULT_PLOOKUP_RANGE_SIZE = (1 << DEFAULT_PLOOKUP_RANGE_BITNUM) - 1;
    // These are variables that we have used a gate on, to enforce that they are
    // equal to a defined value.
    // TODO(#216)(Adrian): Why is this not in CircuitConstructorBase
    std::map<barretenberg::fr, uint32_t> constant_variable_indices;

    ExampleCircuitConstructor(const size_t size_hint = 0)
        : CircuitConstructorBase(example_selector_names(), static_cast<size_t>(ExampleSelectors::NUM), size_hint)
    {
        w_l.reserve(size_hint);
        w_r.reserve(size_hint);
        w_o.reserve(size_hint);
        w_4.reserve(size_hint);
        w_5.reserve(size_hint);
        // To effieciently constrain wires to zero, we set the first value of w_1 to be 0, and use copy constraints for
        // all future zero values.
        // (#216)(Adrian): This should be done in a constant way, maybe by initializing the constant_variable_indices
        // map
        zero_idx = put_constant_variable(barretenberg::fr::zero());
        // TODO(#217)(Cody): Ensure that no polynomial is ever zero. Maybe there's a better way.
        one_idx = put_constant_variable(barretenberg::fr::one());
        // 1 * 1 * 1 + 1 * 1 + 1 * 1 + 1 * 1 + -4
        // m           l       r       o        c
        create_poly_gate({ one_idx, one_idx, one_idx, 1, 1, 1, 1, -4 });
    };

    ExampleCircuitConstructor(const ExampleCircuitConstructor& other) = delete;
    ExampleCircuitConstructor(ExampleCircuitConstructor&& other) = default;
    ExampleCircuitConstructor& operator=(const ExampleCircuitConstructor& other) = delete;
    ExampleCircuitConstructor& operator=(ExampleCircuitConstructor&& other) = default;
    ~ExampleCircuitConstructor() override = default;

    void assert_equal_constant(uint32_t const a_program_widthx,
                               barretenberg::fr const& b,
                               std::string const& msg = "assert equal constant");

    void create_add_gate(const add_triple& in) override;
    void create_mul_gate(const mul_triple& in) override;
    void create_bool_gate(const uint32_t a) override;
    void create_poly_gate(const poly_triple& in) override;
    void create_big_add_gate(const add_quad& in);
    void create_big_add_gate_with_bit_extraction(const add_quad& in);
    void create_big_mul_gate(const mul_quad& in);
    void create_balanced_add_gate(const add_quad& in);

    fixed_group_add_quad previous_add_quad;

    // TODO(#216)(Adrian): This should be a virtual overridable method in the base class.
    void fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value);

    void create_new_range_constraint(const uint32_t variable_index, const uint64_t target_range)
    { /* WORKTODO */
        (void)variable_index;
        (void)target_range;
    };
    void create_range_constraint(const uint32_t variable_index, const size_t num_bits, std::string const&)
    {
        if (num_bits <= DEFAULT_PLOOKUP_RANGE_BITNUM) {
            /**
             * N.B. if `variable_index` is not used in any arithmetic constraints, this will create an unsatisfiable
             *      circuit!
             *      this range constraint will increase the size of the 'sorted set' of range-constrained integers by 1.
             *      The 'non-sorted set' of range-constrained integers is a subset of the wire indices of all arithmetic
             *      gates. No arithemtic gate => size imbalance between sorted and non-sorted sets. Checking for this
             *      and throwing an error would require a refactor of the Composer to catelog all 'orphan' variables not
             *      assigned to gates.
             **/
            create_new_range_constraint(variable_index, 1ULL << num_bits);
        } else {
            decompose_into_default_range(variable_index, num_bits);
        }
    }

    // TODO(#216)(Adrian): The 2 following methods should be virtual in the base class
    uint32_t put_constant_variable(const barretenberg::fr& variable);

    size_t get_num_constant_gates() const override { return 0; }

    /**
     * Plookup Methods
     **/
    void add_table_column_selector_poly_to_proving_key(barretenberg::polynomial& small, const std::string& tag);
    void initialize_precomputed_table(
        const plookup::BasicTableId id,
        bool (*generator)(std::vector<barretenberg::fr>&,
                          std::vector<barretenberg::fr>&,
                          std::vector<barretenberg::fr>&),
        std::array<barretenberg::fr, 2> (*get_values_from_key)(const std::array<uint64_t, 2>));

    plookup::BasicTable& get_table(const plookup::BasicTableId id);
    plookup::MultiTable& create_table(const plookup::MultiTableId id);

    plookup::ReadData<uint32_t> create_gates_from_plookup_accumulators(
        const plookup::MultiTableId& id,
        const plookup::ReadData<barretenberg::fr>& read_values,
        const uint32_t key_a_index,
        std::optional<uint32_t> key_b_index = std::nullopt);

    /**
     * Generalized Permutation Methods
     **/
    std::vector<uint32_t> decompose_into_default_range(
        const uint32_t variable_index,
        const uint64_t num_bits,
        const uint64_t target_range_bitnum = DEFAULT_PLOOKUP_RANGE_BITNUM,
        std::string const& msg = "decompose_into_default_range")
    {
        (void)variable_index;
        (void)num_bits;
        (void)target_range_bitnum;
        (void)msg;
        return { 0, 1, 2, 3 }; /* WORKTODO */
    };
    void create_dummy_constraints(const std::vector<uint32_t>& variable_index);
    void create_sort_constraint(const std::vector<uint32_t>& variable_index);
    void create_sort_constraint_with_edges(const std::vector<uint32_t>& variable_index,
                                           const barretenberg::fr&,
                                           const barretenberg::fr&);
    void assign_tag(const uint32_t variable_index, const uint32_t tag)
    {
        ASSERT(tag <= current_tag);
        ASSERT(real_variable_tags[real_variable_index[variable_index]] == DUMMY_TAG);
        real_variable_tags[real_variable_index[variable_index]] = tag;
    }

    uint32_t create_tag(const uint32_t tag_index, const uint32_t tau_index)
    {
        tau.insert({ tag_index, tau_index });
        current_tag++; // Why exactly?
        return current_tag;
    }

    uint32_t get_new_tag()
    {
        current_tag++;
        return current_tag;
    }

    struct RangeList {
        uint64_t target_range;
        uint32_t range_tag;
        uint32_t tau_tag;
        std::vector<uint32_t> variable_indices;
    };

    RangeList create_range_list(const uint64_t target_range);
    void process_range_list(const RangeList& list);
    void process_range_lists();

    // /**
    //  * @brief Get the final number of gates in a circuit, which consists of the sum of:
    //  * 1) Current number number of actual gates
    //  * 2) Number of public inputs, as we'll need to add a gate for each of them
    //  * 3) Number of Rom array-associated gates
    //  * 4) NUmber of range-list associated gates
    //  *
    //  *
    //  * @param count return arument, number of existing gates
    //  * @param rangecount return argument, extra gates due to range checks
    //  * @param romcount return argument, extra gates due to rom reads
    //  * @param ramcount return argument, extra gates due to ram read/writes
    //  */
    // void get_num_gates_split_into_components(size_t& count,
    //                                          size_t& rangecount,
    //                                          size_t& romcount,
    //                                          size_t& ramcount) const
    // {
    //     count = num_gates;
    //     rangecount = 0;
    //     romcount = 0;
    //     ramcount = 0;
    //     // each ROM gate adds +1 extra gate due to the rom reads being copied to a sorted list set
    //     for (size_t i = 0; i < rom_arrays.size(); ++i) {
    //         for (size_t j = 0; j < rom_arrays[i].state.size(); ++j) {
    //             if (rom_arrays[i].state[j][0] == UNINITIALIZED_MEMORY_RECORD) {
    //                 romcount += 2;
    //             }
    //         }
    //         romcount += (rom_arrays[i].records.size());
    //         romcount += 1; // we add an addition gate after procesing a rom array
    //     }

    //     constexpr size_t gate_width = ultra_settings::program_width;
    //     // each RAM gate adds +2 extra gates due to the ram reads being copied to a sorted list set,
    //     // as well as an extra gate to validate timestamps
    //     for (size_t i = 0; i < ram_arrays.size(); ++i) {
    //         for (size_t j = 0; j < ram_arrays[i].state.size(); ++j) {
    //             if (ram_arrays[i].state[j] == UNINITIALIZED_MEMORY_RECORD) {
    //                 ramcount += 2;
    //             }
    //         }
    //         ramcount += (ram_arrays[i].records.size() * 2);
    //         ramcount += 1; // we add an addition gate after procesing a ram array

    //         // there will be 'max_timestamp' number of range checks, need to calculate.
    //         const auto max_timestamp = ram_arrays[i].access_count - 1;

    //         // TODO: if a range check of length `max_timestamp` already exists, this will be innacurate!
    //         // TODO: fix this
    //         size_t padding = (gate_width - (max_timestamp % gate_width)) % gate_width;
    //         if (max_timestamp == gate_width)
    //             padding += gate_width;
    //         const size_t ram_range_check_list_size = max_timestamp + padding;

    //         size_t ram_range_check_gate_count = (ram_range_check_list_size / gate_width);
    //         ram_range_check_gate_count += 1; // we need to add 1 extra addition gates for every distinct range list

    //         ramcount += ram_range_check_gate_count;
    //     }
    //     for (const auto& list : range_lists) {
    //         auto list_size = list.second.variable_indices.size();
    //         size_t padding = (gate_width - (list.second.variable_indices.size() % gate_width)) % gate_width;
    //         if (list.second.variable_indices.size() == gate_width)
    //             padding += gate_width;
    //         list_size += padding;
    //         rangecount += (list_size / gate_width);
    //         rangecount += 1; // we need to add 1 extra addition gates for every distinct range list
    //     }
    // }

    /**
     * Member Variables
     **/
    bool circuit_finalised = false;

    // This variable controls the amount with which the lookup table and witness values need to be shifted
    // above to make room for adding randomness into the permutation and witness polynomials in the plookup widget.
    // This must be (num_roots_cut_out_of_the_vanishing_polynomial - 1), since the variable num_roots_cut_out_of_
    // vanishing_polynomial cannot be trivially fetched here, I am directly setting this to 4 - 1 = 3.
    static constexpr size_t s_randomness = 3;

    std::vector<plookup::BasicTable> lookup_tables;
    std::vector<plookup::MultiTable> lookup_multi_tables;
    std::map<uint64_t, RangeList> range_lists; // DOCTODO: explain this.

    bool check_circuit();
};
} // namespace bonk
