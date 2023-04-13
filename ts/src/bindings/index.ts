// WARNING: FILE CODE GENERATED BY BINDGEN UTILITY. DO NOT EDIT!
/* eslint-disable @typescript-eslint/no-unused-vars */
import { callWasmExport } from '../call_wasm_export/index.js';
import { BufferDeserializer, NumberDeserializer, VectorDeserializer, BoolDeserializer } from '../serialize/index.js';
import { Fr, Fq, Point, Buffer32, Buffer128 } from '../types/index.js';

export function pedersenInit() {
  const result = callWasmExport('pedersen_init', [], []);
  return;
}

export function pedersenCompressFields(left: Fr, right: Fr): Fr {
  const result = callWasmExport('pedersen_compress_fields', [left, right], [Fr]);
  return result[0];
}

export function pedersenPlookupCompressFields(left: Fr, right: Fr): Fr {
  const result = callWasmExport('pedersen_plookup_compress_fields', [left, right], [Fr]);
  return result[0];
}

export function pedersenCompress(inputsBuffer: Fr[]): Fr {
  const result = callWasmExport('pedersen_compress', [inputsBuffer], [Fr]);
  return result[0];
}

export function pedersenPlookupCompress(inputsBuffer: Fr[]): Fr {
  const result = callWasmExport('pedersen_plookup_compress', [inputsBuffer], [Fr]);
  return result[0];
}

export function pedersenCompressWithHashIndex(inputsBuffer: Fr[], hashIndex: number): Fr {
  const result = callWasmExport('pedersen_compress_with_hash_index', [inputsBuffer, hashIndex], [Fr]);
  return result[0];
}

export function pedersenCommit(inputsBuffer: Fr[]): Fr {
  const result = callWasmExport('pedersen_commit', [inputsBuffer], [Fr]);
  return result[0];
}

export function pedersenPlookupCommit(inputsBuffer: Fr[]): Fr {
  const result = callWasmExport('pedersen_plookup_commit', [inputsBuffer], [Fr]);
  return result[0];
}

export function pedersenBufferToField(data: Buffer): Fr {
  const result = callWasmExport('pedersen_buffer_to_field', [data], [Fr]);
  return result[0];
}

export function pedersenHashInit() {
  const result = callWasmExport('pedersen_hash_init', [], []);
  return;
}

export function pedersenHashPair(left: Fr, right: Fr): Fr {
  const result = callWasmExport('pedersen_hash_pair', [left, right], [Fr]);
  return result[0];
}

export function pedersenHashMultiple(inputsBuffer: Fr[]): Fr {
  const result = callWasmExport('pedersen_hash_multiple', [inputsBuffer], [Fr]);
  return result[0];
}

export function pedersenHashMultipleWithHashIndex(inputsBuffer: Fr[], hashIndex: number): Fr {
  const result = callWasmExport('pedersen_hash_multiple_with_hash_index', [inputsBuffer, hashIndex], [Fr]);
  return result[0];
}

export function pedersenHashToTree(data: Fr[]): Fr[] {
  const result = callWasmExport('pedersen_hash_to_tree', [data], [VectorDeserializer(Fr)]);
  return result[0];
}

export function blake2s(data: Buffer): Buffer32 {
  const result = callWasmExport('blake2s', [data], [Buffer32]);
  return result[0];
}

export function blake2sToField(data: Buffer): Fr {
  const result = callWasmExport('blake2s_to_field', [data], [Fr]);
  return result[0];
}

export function schnorrComputePublicKey(privateKey: Fr): Point {
  const result = callWasmExport('schnorr_compute_public_key', [privateKey], [Point]);
  return result[0];
}

export function schnorrNegatePublicKey(publicKeyBuffer: Point): Point {
  const result = callWasmExport('schnorr_negate_public_key', [publicKeyBuffer], [Point]);
  return result[0];
}

export function schnorrConstructSignature(message: Buffer, privateKey: Fr): [Buffer32, Buffer32] {
  const result = callWasmExport('schnorr_construct_signature', [message, privateKey], [Buffer32, Buffer32]);
  return result as any;
}

export function schnorrVerifySignature(message: Buffer, pubKey: Point, sigS: Buffer32, sigE: Buffer32): boolean {
  const result = callWasmExport('schnorr_verify_signature', [message, pubKey, sigS, sigE], [BoolDeserializer()]);
  return result[0];
}

export function schnorrMultisigCreateMultisigPublicKey(privateKey: Fq): Buffer128 {
  const result = callWasmExport('schnorr_multisig_create_multisig_public_key', [privateKey], [Buffer128]);
  return result[0];
}

export function schnorrMultisigValidateAndCombineSignerPubkeys(signerPubkeyBuf: Buffer128[]): [Point, boolean] {
  const result = callWasmExport('schnorr_multisig_validate_and_combine_signer_pubkeys', [signerPubkeyBuf], [Point, BoolDeserializer()]);
  return result as any;
}

export function schnorrMultisigConstructSignatureRound1(): [Buffer128, Buffer128] {
  const result = callWasmExport('schnorr_multisig_construct_signature_round_1', [], [Buffer128, Buffer128]);
  return result as any;
}

export function schnorrMultisigConstructSignatureRound2(message: Buffer, privateKey: Fq, signerRoundOnePrivateBuf: Buffer128, signerPubkeysBuf: Buffer128[], roundOnePublicBuf: Buffer128[]): [Fq, boolean] {
  const result = callWasmExport('schnorr_multisig_construct_signature_round_2', [message, privateKey, signerRoundOnePrivateBuf, signerPubkeysBuf, roundOnePublicBuf], [Fq, BoolDeserializer()]);
  return result as any;
}

export function schnorrMultisigCombineSignatures(message: Buffer, signerPubkeysBuf: Buffer128[], roundOneBuf: Buffer128[], roundTwoBuf: Fr[]): [Buffer32, Buffer32, boolean] {
  const result = callWasmExport('schnorr_multisig_combine_signatures', [message, signerPubkeysBuf, roundOneBuf, roundTwoBuf], [Buffer32, Buffer32, BoolDeserializer()]);
  return result as any;
}
