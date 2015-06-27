#include <asc.h>
#include <openssl/sha.h>

/**
 * @brief Compute a machine-independent cryptographic hash of state vector @p x.
 *
 * Store a machine-independent representation of the input state vector @p x
 * to a buffer.  If @p raw is not null, use it as the buffer, otherwise
 * allocate a private buffer and free it before returning.
 * Compute cryptographic hash of buffer, placing the result in @p hash.
 * For convenience, also return the low 8 bytes of @p hash as an integer.
 *
 * @param hash Output cryptographic hash.
 * @param raw Temporary buffer to be used if not null.
 * @param x Input state vector.
 * @return A 64-bit integer representation of the low 8 bytes of @p hash.
 */
unsigned long sha(unsigned char *hash, unsigned char *raw, mpz_t const x)
{
    unsigned char buf[SHA256_DIGEST_LENGTH];
    size_t c, size, count;
    unsigned long h;
    FILE *stream;
    char *ptr;

    /* Open memory stream to buffer for portable respresentation.  */
    if (raw) {
        /* Calculate size of preallocated memory.  */
        size = 4 + sizeof(long) * x->_mp_size;

        /* Open stream.  */
        if ((stream = fmemopen(raw, size, "w")) == 0) {
            diagnostic("fmemopen");
            __builtin_trap();
        }

        /* Swing pointer to newly-allocated buffer.  */
        ptr = (char *) raw;
    } else {
        /* If no preallocated memory, allocate memory and open stream.  */
        if ((stream = open_memstream(&ptr, &size)) == 0) {
            diagnostic("open_memstream");
            __builtin_trap();
        }
    }

    /* Write portable representation of state vector to memory stream.  */
    if ((count = mpz_out_raw(stream, x)) == 0) {
        diagnostic("mpz_out_raw");
        __builtin_trap();
    }

    /* Bail out if anything went wrong.  */
    if (count > size) {
        diagnostic("mpz_out_raw: count = %ld but size = %ld", count, size);
        __builtin_trap();
    }

    /* Flush memory stream.  */
    fflush(stream);

    /* Close memory stream.  */
    if ((c = fclose(stream)) != 0) {
        diagnostic("fclose: %ld count: %ld size: %ld", c, count, size);
        __builtin_trap();
    }

    /* Use stack allocated buffer if none passed.  */
    if (hash == 0)
        hash = buf;

    /* Compute SHA-256.  */
    SHA256((unsigned char *) ptr, size, hash);

    /* Release memory.  */
    if (raw == 0)
        free(ptr);

    /* Obtain 64-bit integer from the low 8 bytes of the SHA-256 hash.  */
    h = 0;
    h |= hash[0];
    h <<= 8;
    h |= hash[1];
    h <<= 8;
    h |= hash[2];
    h <<= 8;
    h |= hash[3];
    h <<= 8;
    h |= hash[4];
    h <<= 8;
    h |= hash[5];
    h <<= 8;
    h |= hash[6];
    h <<= 8;
    h |= hash[7];

    /* Return 64-bit integer representation of the low 8 bytes of hash.  */
    return h;
}
