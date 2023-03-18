# Notes on flavor

Flavor = Arithmetization + Proof system?

Maybe we should define:
  Arithmetization = which paths in the stdlib do we use to construct a circuit

Circuit Constructor just needs to know arithmetization.

Are there places where you need need both arithmetization and proof system?

Stdlib currently could just take `uses_tables` instead of `ComposerType` everywhere excpet in `field_t<Composer>::accumulate`.

When we implement a circuit constructor that uses PLOOKUP path, it needs to have
 - `decompose_non_native_field_double_width_limb` (eg)
 - `create_big_add_gate` with a boolean switch
 - `evaluate_non_native_field_addition`
 - `decompose_into_default_range`
 - create_group_element_rom_tables

High-width circuits means the standard library is going away?

PLOOKUP == use secp256k1 in some places?

PLOOKUP == use special pedersen implementation in stdlib transcript


Uses of POLYNOMIAL enum:
 - Specify order on prover_polynomials when constructing a prover so that they can be consistently extracted in sumcheck and in gemini univariatization round

Flavor vs proving key vs polynomial store


MULTIVARIATE makes sure that the folded polynomials are in the same order as the full polynomials in sumcheck.

using Arithmetization == we want stdlib templated by Azn, want to share Azn acros 
                         different proving systems
using Flavor          == proving system is at least somewhat generic
  - specify width; relations
  - all polynomials used in the protocol

Variable program width stuff only to be implemented by Plonk.

`initialize_proving_key` takes in ComposerType... is only used in plonk to easily select the manifest.

Sharing proving key doesn't really make sense?

How do we modularize enough so that every time you can easily say "I want to use lookup tables" (e.g).

Base needs to have container for wires for sure.

Will any variant of Honk without tables be used? Seems like maybe no.

we have `compute_kzg_round` but sometimes we will use IPA. Is commitment scheme part of flavor? Is that uniform?

# Example

Standard Honk:
 - circuit constructor: 3 wires, 5 selectors "q_m", "q_1", "q_2", "q_3", "q_c", 
 - composer helper: 2 aux polys L_first, L_last
 - prover: full array of prover polys

# Current use of flavor
Contains
 - Alias to Arzn
 - Alias to Enum in the arithmetization (literate estraction of univariates in sumcheck)

Arithmetization is not well specified; everything is Honk-specific.
 - Z_PERM_SHIFT polynomial accesor is not used by PlonK, but this _could_ be done uniformly.
- ENUM_TO_COMM array for extracting commitments from the vk.

# ComposerType current uses
ComposerType Uses:

A fuzzer constants 
X passed to compute_proving_key_base (garbage)
X join_split_example templates by ComposerType (bad form but good interface? use CircuitConstructor in build_circuit)
F Used by benchmark collector
X used to set SYSTEM_COMPOSER
X used to set MAX_NUM_SIMULTANEOUS_FFTS in polynomial cache
`F`/X used to construt polynomial manifest
A specify context for constructing circuits in stdlib
F specify proving system against which to test circuits in stdlib.
A constexpr branches in stdlib (== and !=)
  - generically when we want to use tables
  - specifies curve type in ecdsa
  - different pedersen algs
  - misused in secp256k1_ecdsa_mul(?)

CircuitConstructorBase is templated by program width only.


Todo:
- Strip down join-split example
- 

Probably
Flavor is equal to proving system
Arithmetization specifies circuit constructor type

# Working on `ExampleHonkComposer`

Flavor determines: composer helper; Prover; Verifier (need to implement each, by settings)

from settings, Prover uses: hash_type, num_challenge_bytes (get rid of this now that ther'es no linearization), program width

TODO: figure out what goes in the prover shared lib.

from settings, Verifier uses: field type

Note: verifier settings are in program_settings.hpp; prover settings arezs in prover_settings.hpp

Selector info in circuit constructor is redudant (?) with some of what is specified in flavor. 

Descriptive struct suffers from not generalizing to 900 wires.

Width is shared with both circuit constructor and composer helper. Flavor exists at least to share this data.

Various constructors that all circuit constructors, composers, provers and verifiers will need should be in shared base class? Or will this only copy base class data?

It should be easy to make a new proving system. One way to do this: make all relevant stuff to one copy-and-paste-able directory.

Documentation should contain specs for keys

Need to manually change Selectors alias in every gate.

Need to manually implement every gate

Need to rename lots of tests.

TODO: `composer` used in circuit constructor tests :(

Need to move plookup stuff into shared utility

Updating includes is actually kind of a pain

Need to explicitly instantiate CircuitConstructorBase for new widths. Slightly annoying to track down linker error.

Need to implement check_circuit. Will want to share components.

Depending on what you copy and paste, you need to subset some tests and add other tests.

For some classes we only want to instantiate if plookup module is in use, so can't just check if compsoer type is plookup anymore. Similar with branching

# WORKTODOs

Implement all plookup and 