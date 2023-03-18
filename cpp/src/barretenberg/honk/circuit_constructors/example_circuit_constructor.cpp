#include "example_circuit_constructor.hpp"
#include <unordered_set>
#include <unordered_map>

using namespace barretenberg;

#pragma GCC diagnostic ignored "-Wunused-variable"

namespace bonk {
#define EXAMPLE_SELECTOR_REFS                                                                                          \
    auto& q_m = selectors[ExampleSelectors::QM];                                                                       \
    auto& q_c = selectors[ExampleSelectors::QC];                                                                       \
    auto& q_1 = selectors[ExampleSelectors::Q1];                                                                       \
    auto& q_2 = selectors[ExampleSelectors::Q2];                                                                       \
    auto& q_3 = selectors[ExampleSelectors::Q3];                                                                       \
    auto& q_4 = selectors[ExampleSelectors::Q4];                                                                       \
    auto& q_lookup_type = selectors[ExampleSelectors::QLOOKUPTYPE];                                                    \
    auto& q_sort = selectors[ExampleSelectors::QSORT];

/**
 * Create an addition gate.
 *
 * @param in An add_triple containing the indexes of variables to be placed into the
 * wires w_l, w_r, w_o and addition coefficients to be placed into q_1, q_2, q_3, q_c.
 */
void ExampleCircuitConstructor::create_add_gate(const add_triple& in)
{
    EXAMPLE_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c });

    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(in.c_scaling);
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(in.const_scaling);
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());

    ++num_gates;
}

/**
 * Create a big addition gate.
 * (a*a_c + b*b_c + c*c_c + d*d_c + q_c = 0)
 *
 * @param in An add quad containing the indexes of variables a, b, c, d and
 * the scaling factors.
 * */
void ExampleCircuitConstructor::create_big_add_gate(const add_quad& in)
{
    // (a terms + b terms = temp)
    // (c terms + d  terms + temp = 0 )
    fr t0 = get_variable(in.a) * in.a_scaling;
    fr t1 = get_variable(in.b) * in.b_scaling;
    fr temp = t0 + t1;
    uint32_t temp_idx = add_variable(temp);

    create_add_gate(add_triple{ in.a, in.b, temp_idx, in.a_scaling, in.b_scaling, fr::neg_one(), fr::zero() });

    create_add_gate(add_triple{ in.c, in.d, temp_idx, in.c_scaling, in.d_scaling, fr::one(), in.const_scaling });
}

/**
 * Create a balanced addition gate.
 * (a*a_c + b*b_c + c*c_c + d*d_c + q_c = 0, where d is in [0,3])
 *
 * @param in An add quad containing the indexes of variables a, b, c, d and
 * the scaling factors.
 * */
void ExampleCircuitConstructor::create_balanced_add_gate(const add_quad& in)
{

    EXAMPLE_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c, in.d });

    // (a terms + b terms = temp)
    // (c terms + d  terms + temp = 0 )
    fr t0 = get_variable(in.a) * in.a_scaling;
    fr t1 = get_variable(in.b) * in.b_scaling;
    fr temp = t0 + t1;
    uint32_t temp_idx = add_variable(temp);

    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(temp_idx);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(in.a_scaling);
    q_2.emplace_back(in.b_scaling);
    q_3.emplace_back(fr::neg_one());
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(fr::zero());
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());

    ++num_gates;

    w_l.emplace_back(temp_idx);
    w_r.emplace_back(in.c);
    w_o.emplace_back(in.d);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(fr::one());
    q_2.emplace_back(in.c_scaling);
    q_3.emplace_back(in.d_scaling);
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(in.const_scaling);
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());

    ++num_gates;

    // in.d must be between 0 and 3
    // i.e. in.d * (in.d - 1) * (in.d - 2) = 0
    fr temp_2 = get_variable(in.d).sqr() - get_variable(in.d);
    uint32_t temp_2_idx = add_variable(temp_2);
    w_l.emplace_back(in.d);
    w_r.emplace_back(in.d);
    w_o.emplace_back(temp_2_idx);
    q_m.emplace_back(fr::one());
    q_1.emplace_back(fr::neg_one());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::neg_one());
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(fr::zero());
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());

    ++num_gates;

    constexpr fr neg_two = -fr(2);
    w_l.emplace_back(temp_2_idx);
    w_r.emplace_back(in.d);
    w_o.emplace_back(zero_idx);
    q_m.emplace_back(fr::one());
    q_1.emplace_back(neg_two);
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::zero());
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(fr::zero());
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());

    ++num_gates;
}

void ExampleCircuitConstructor::create_big_add_gate_with_bit_extraction(const add_quad& in)
{
    // blah.
    // delta = (c - 4d)
    // delta^2 = c^2 + 16d^2 - 8dc
    // r = (-2*delta*delta + 9*delta - 7)*delta
    // r =

    fr delta = get_variable(in.d);
    delta += delta;
    delta += delta;
    delta = get_variable(in.c) - delta;

    uint32_t delta_idx = add_variable(delta);
    constexpr fr neg_four = -(fr(4));
    create_add_gate(add_triple{ in.c, in.d, delta_idx, fr::one(), neg_four, fr::neg_one(), fr::zero() });

    constexpr fr two = fr(2);
    constexpr fr seven = fr(7);
    constexpr fr nine = fr(9);
    const fr r_0 = (delta * nine) - ((delta.sqr() * two) + seven);
    uint32_t r_0_idx = add_variable(r_0);
    create_poly_gate(poly_triple{ delta_idx, delta_idx, r_0_idx, -two, nine, fr::zero(), fr::neg_one(), -seven });

    fr r_1 = r_0 * delta;
    uint32_t r_1_idx = add_variable(r_1);
    create_mul_gate(mul_triple{
        r_0_idx,
        delta_idx,
        r_1_idx,
        fr::one(),
        fr::neg_one(),
        fr::zero(),
    });

    // ain.a1 + bin.b2 + cin.c3 + din.c4 + r_1 = 0

    fr r_2 = (r_1 + (get_variable(in.d) * in.d_scaling));
    uint32_t r_2_idx = add_variable(r_2);
    create_add_gate(add_triple{ in.d, r_1_idx, r_2_idx, in.d_scaling, fr::one(), fr::neg_one(), fr::zero() });

    create_big_add_gate(
        add_quad{ in.a, in.b, in.c, r_2_idx, in.a_scaling, in.b_scaling, in.c_scaling, fr::one(), in.const_scaling });
}

void ExampleCircuitConstructor::create_big_mul_gate(const mul_quad& in)
{
    fr temp = ((get_variable(in.c) * in.c_scaling) + (get_variable(in.d) * in.d_scaling));
    uint32_t temp_idx = add_variable(temp);
    create_add_gate(add_triple{ in.c, in.d, temp_idx, in.c_scaling, in.d_scaling, fr::neg_one(), fr::zero() });

    create_poly_gate(
        poly_triple{ in.a, in.b, temp_idx, in.mul_scaling, in.a_scaling, in.b_scaling, fr::one(), in.const_scaling });
}

/**
 * Create a multiplication gate.
 *
 * @param in A mul_tripple containing the indexes of variables to be placed into the
 * wires w_l, w_r, w_o and scaling coefficients to be placed into q_m, q_3, q_c.
 */
void ExampleCircuitConstructor::create_mul_gate(const mul_triple& in)
{
    EXAMPLE_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c });

    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    q_m.emplace_back(in.mul_scaling);
    q_1.emplace_back(fr::zero());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(in.c_scaling);
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(in.const_scaling);
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());

    ++num_gates;
}

/**
 * Create a bool gate.
 * This gate constrains a variable to two possible values: 0 or 1.
 *
 * @param variable_index The index of the variable.
 */
void ExampleCircuitConstructor::create_bool_gate(const uint32_t variable_index)
{
    EXAMPLE_SELECTOR_REFS
    assert_valid_variables({ variable_index });

    w_l.emplace_back(variable_index);
    w_r.emplace_back(variable_index);
    w_o.emplace_back(variable_index);

    q_m.emplace_back(fr::one());
    q_1.emplace_back(fr::zero());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::neg_one());
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(fr::zero());
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());

    ++num_gates;
}

/**
 * Create a gate where you set all the indexes and coefficients yourself.
 *
 * @param in A poly_triple containing all the information.
 */
void ExampleCircuitConstructor::create_poly_gate(const poly_triple& in)
{
    EXAMPLE_SELECTOR_REFS
    assert_valid_variables({ in.a, in.b, in.c });

    w_l.emplace_back(in.a);
    w_r.emplace_back(in.b);
    w_o.emplace_back(in.c);
    q_m.emplace_back(in.q_m);
    q_1.emplace_back(in.q_l);
    q_2.emplace_back(in.q_r);
    q_3.emplace_back(in.q_o);
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(in.q_c);
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());

    ++num_gates;
}

void ExampleCircuitConstructor::fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value)
{
    EXAMPLE_SELECTOR_REFS
    assert_valid_variables({ witness_index });

    w_l.emplace_back(witness_index);
    w_r.emplace_back(zero_idx);
    w_o.emplace_back(zero_idx);
    q_m.emplace_back(fr::zero());
    q_1.emplace_back(fr::one());
    q_2.emplace_back(fr::zero());
    q_3.emplace_back(fr::zero());
    q_4.emplace_back(fr::zero());
    q_c.emplace_back(-witness_value);
    q_lookup_type.emplace_back(fr::zero());
    q_sort.emplace_back(fr::zero());
    ++num_gates;
}

uint32_t ExampleCircuitConstructor::put_constant_variable(const barretenberg::fr& variable)
{
    if (constant_variable_indices.contains(variable)) {
        return constant_variable_indices.at(variable);
    } else {

        uint32_t variable_index = add_variable(variable);
        fix_witness(variable_index, variable);
        constant_variable_indices.insert({ variable, variable_index });
        return variable_index;
    }
}

void ExampleCircuitConstructor::assert_equal_constant(uint32_t const a_idx, fr const& b, std::string const& msg)
{
    if (variables[a_idx] != b && !failed()) {
        failure(msg);
    }
    auto b_idx = put_constant_variable(b);
    assert_equal(a_idx, b_idx, msg);
}

// WORKTODO: implement plookup and genperm stuff

/**
 * Check if all the circuit gates are correct given the witnesses.
 * Goes through each gates and checks if the identity holds.
 *
 * @return true if the circuit is correct.
 * WORKTODO: implement this
 * */
bool ExampleCircuitConstructor::check_circuit()
{
    EXAMPLE_SELECTOR_REFS

    fr gate_sum;
    fr left, right, output;
    for (size_t i = 0; i < num_gates; i++) {
        gate_sum = fr::zero();
        left = get_variable(w_l[i]);
        right = get_variable(w_r[i]);
        output = get_variable(w_o[i]);
        gate_sum = q_m[i] * left * right + q_1[i] * left + q_2[i] * right + q_3[i] * output + q_c[i];
        if (!gate_sum.is_zero())
            return false;
    }
    return true;
}
} // namespace bonk