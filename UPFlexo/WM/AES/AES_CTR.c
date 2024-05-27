#include <stdbool.h>
#include "aes.h"
#include "chksum.h"

#define CHKSUMLEN 80
#define KEYLEN 16

unsigned f_weird_decrypt(byte* input, byte* output, unsigned len) {
    byte key[KEYLEN];
    byte ctr[16];
    byte key_stream[16];
    ((unsigned*)ctr)[0] = 0;
    ((unsigned*)ctr)[1] = 0;
    ((unsigned*)ctr)[2] = 0;
    len -= KEYLEN;
    memcpy(input + len, key, KEYLEN);

    unsigned main_part = (len / (CHKSUMLEN + 4)) * CHKSUMLEN;
    unsigned tail = (len % (CHKSUMLEN + 4)) ? ((len % (CHKSUMLEN + 4)) - 4) : 0;
    len = main_part + tail;

    // decrypt one chunk at a time
    for (unsigned i = 0; i < len; i += CHKSUMLEN) {
        unsigned chunk_len = ((i + CHKSUMLEN) <= len) ? CHKSUMLEN : (len - i);
        unsigned chksum =  *(unsigned*)(&input[len + 4 * (i / CHKSUMLEN)]);
        do {
            // decrypt blocks of a chunk
            for (unsigned j = 0; j < chunk_len; j += 16) {
                ((unsigned*)ctr)[3] = (i + j) / 16;
                aes_encrypt(ctr, key, key_stream);

                // decrypt one block
                for (unsigned k = 0; k < 16 && (j + k) < chunk_len; ++k) {
                    output[i + j + k] = input[i + j + k] ^ key_stream[k];
                }
            }
        } while (chksum != adler32(output + i, chunk_len));
    }

    return len;
}
