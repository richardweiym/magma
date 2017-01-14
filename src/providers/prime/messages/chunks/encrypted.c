
/**
 * @file /magma/src/providers/prime/messages/chunks/encrypted.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void encrypted_chunk_free(prime_encrypted_chunk_t *chunk) {

	if (chunk) {

		if (chunk->signature) st_free(chunk->signature);
		if (chunk->data) st_free(chunk->data);
		if (chunk->trailing) st_free(chunk->trailing);
		if (chunk->encrypted) st_free(chunk->encrypted);

		if (chunk->slots.author) st_free(chunk->slots.author);
		if (chunk->slots.origin) st_free(chunk->slots.origin);
		if (chunk->slots.destination) st_free(chunk->slots.destination);
		if (chunk->slots.recipient) st_free(chunk->slots.recipient);

		mm_free(chunk);
	}
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME encrypted chunk pointer was passed to the free function.");
	}
#endif

	return;
}

void encrypted_chunk_cleanup(prime_encrypted_chunk_t *chunk) {
	if (chunk) {
		encrypted_chunk_free(chunk);
	}
	return;
}

prime_encrypted_chunk_t * encrypted_chunk_alloc(void) {

	prime_encrypted_chunk_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_encrypted_chunk_t)))) {
		log_pedantic("PRIME encrypted chunk allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_encrypted_chunk_t));

	return result;
}

stringer_t * encrypted_chunk_buffer(prime_encrypted_chunk_t *chunk) {

	stringer_t *buffer = NULL;

	if (chunk) {
		buffer = chunk->encrypted;
	}

	return buffer;
}

prime_encrypted_chunk_t * encrypted_chunk_get(ed25519_key_t *signing, secp256k1_key_t *encryption, secp256k1_key_t *author, secp256k1_key_t *origin,
	secp256k1_key_t *destination, secp256k1_key_t *recipient, stringer_t *data) {

	uint32_t big_endian_length = 0;
	prime_encrypted_chunk_t *result = NULL;

	// We need a signing key, encryption key, and at least one actor.
	if (!signing || signing->type != ED25519_PRIV || !encryption || !data || (!author && !origin && !destination && !recipient)) {
		log_pedantic("Invalid parameters passed to the encrypted chunk generator.");
		return NULL;
	}
	else if (!(result = encrypted_chunk_alloc())) {
		return NULL;
	}


	// The entire buffer must be evenly divisible by 16. divisible by
	// 64 signature + 3 data length + 1 flags + 1 padding length = 69
	result->pad = ((st_length_get(data) + 69) % 16);
	result->length = st_length_get(data);
	big_endian_length = htobe32(result->length);
	result->flags = 0;

	// Allocate a place holder and a buffer to store the padding bytes.
	if (!(result->data = st_alloc(result->length + result->pad + 69)) || !(result->trailing = st_alloc(result->pad))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// Create an appropriately sized padding buffer, and initialize the bytes to match the padding amount.
	mm_set(st_data_get(result->trailing), result->pad, result->pad);
	st_length_set(result->trailing, result->pad);

	// Copy the big endian length into the buffer.
	mm_copy(st_data_get(result->data) + 64, ((uchr_t *)&big_endian_length) + 1, 3);

	// Copy the flags into the buffer.
	mm_copy(st_data_get(result->data) + 67, ((uchr_t *)&result->flags), 1);

	// Copy in the padding length.
	mm_copy(st_data_get(result->data) + 68, ((uchr_t *)&result->pad), 1);

	// Copy in the payload.
	mm_copy(st_data_get(result->data) + 69, st_data_get(data), result->length);

	// Copy in the trailing bytes.
	mm_copy(st_data_get(result->data) + result->length + 69, st_data_get(result->trailing), result->pad);

	// Generate the signature.
	ed25519_sign(signing, PLACER(st_data_get(result->data) + 64, result->length + result->pad + 5), MANAGED(st_data_get(result->data), 64, 64));
#error unfinished
	aes_object_encrypt(key, object, output)
	return NULL;
}