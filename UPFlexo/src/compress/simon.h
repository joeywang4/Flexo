#ifndef ROUNDS
    #define ROUNDS 32
#endif
#define ROR(x, r) ((x >> r) | (x << (16 - r)))
#define ROL(x, r) ((x << r) | (x >> (16 - r)))
#define SIMON_ROUND(x, y, k) \
    tmp = (ROL(x, 1) & ROL(x, 8)) ^ y ^ ROL(x, 2); \
    y = x; \
    x = tmp ^ k;

void ref_simon_encrypt(byte* input, const byte* key, byte* output);

void ref_simon_encrypt(byte* input, const byte* key, byte* output) {
    uint16_t x = input[1] | (input[0] << 8);
    uint16_t y = input[3] | (input[2] << 8);
    uint16_t keys[ROUNDS];
    uint16_t tmp;

    for (int i = 0; i < 4; ++i) {
        keys[3 - i] = key[i * 2 + 1] | (key[i * 2] << 8);
    }
    uint32_t z0 = 0b10110011100001101010010001011111;
    for (int i = 4; i < ROUNDS; ++i) {
        tmp = ROR(keys[i - 1], 3);
        tmp ^= keys[i - 3];
        tmp ^= ROR(tmp, 1);
        keys[i] = ~keys[i - 4] ^ tmp ^ 3 ^ ((z0 >> (i - 4)) & 1);
    }
    for (int i = 0; i < ROUNDS; ++i) {
        SIMON_ROUND(x, y, keys[i]);
    }
    output[0] = x >> 8;
    output[1] = x;
    output[2] = y >> 8;
    output[3] = y;
}

/*************************************************************************
// Encrypted data: [data][checksum][key]
**************************************************************************/

// Add an 32-bit Adler checksum every 80 bytes of data (5% overhead)
#define CHKSUMLEN 80
#define KEYLEN 8
#define BLKSIZE 4

unsigned encrypt(byte* data, unsigned len);

unsigned encrypt(byte* data, unsigned len) {
    unsigned main_part = (len / CHKSUMLEN) * (CHKSUMLEN + 4);
    unsigned tail = (len % CHKSUMLEN) ? ((len % CHKSUMLEN) + 4) : 0;
    unsigned new_len = main_part + tail;

    for (unsigned i = 0; i < len; i += CHKSUMLEN) {
        unsigned chunk_len = ((i + CHKSUMLEN) <= len) ? CHKSUMLEN : (len - i);
        unsigned chksum = upx_adler32(data + i, chunk_len, 1);
        *(unsigned*)(&data[len + 4 * (i / CHKSUMLEN)]) = chksum;
    }

    for (unsigned i = 0; i < KEYLEN; i += 4) {
        *(unsigned*)(&data[new_len + i]) = rand();
    }

    byte* key = data + new_len;
    new_len += KEYLEN;
    byte key_stream[BLKSIZE];

    #ifdef CTR_MODE
    for (unsigned i = 0; i < len; i += BLKSIZE) {
        byte ctr[BLKSIZE];
        ((unsigned*)ctr)[0] = i / BLKSIZE;
        ref_simon_encrypt(ctr, key, key_stream);
        for (unsigned j = 0; j < BLKSIZE && (i + j) < len; ++j) {
            data[i + j] ^= key_stream[j];
        }
    }
    #else // single block
    byte msg[BLKSIZE] = {0x00, 0x11, 0x22, 0x33};
    ref_aes_encrypt(msg, key, key_stream);
    for (unsigned i = 0; i < len; ++i) {
        data[i] ^= key_stream[i % BLKSIZE];
    }
    #endif

    // return new length after encryption (with checksum)
    return new_len;
}

unsigned decrypt(const byte* input, byte* output, unsigned len);

unsigned decrypt(const byte* input, byte* output, unsigned len) {
    len -= KEYLEN;
    const byte* key = input + len;
    unsigned main_part = (len / (CHKSUMLEN + 4)) * CHKSUMLEN;
    unsigned tail = (len % (CHKSUMLEN + 4)) ? ((len % (CHKSUMLEN + 4)) - 4) : 0;
    len = main_part + tail;
    byte key_stream[BLKSIZE];

    #ifdef CTR_MODE
    for (unsigned i = 0; i < len; i += BLKSIZE) {
        byte ctr[BLKSIZE];
        ((unsigned*)ctr)[0] = i / BLKSIZE;
        ref_simon_encrypt(ctr, key, key_stream);
        for (unsigned j = 0; j < BLKSIZE && (i + j) < len; ++j) {
            output[i + j] = input[i + j] ^ key_stream[j];
        }
    }
    #else // single block
    byte msg[BLKSIZE] = {0x00, 0x11, 0x22, 0x33};
    ref_aes_encrypt(msg, key, key_stream);
    for (unsigned i = 0; i < len; ++i) {
        output[i] = input[i] ^ key_stream[i % BLKSIZE];
    }
    #endif

    // verify checksum
    for (unsigned i = 0; i < len; i += CHKSUMLEN) {
        unsigned chunk_len = ((i + CHKSUMLEN) <= len) ? CHKSUMLEN : (len - i);
        unsigned chksum = upx_adler32(output + i, chunk_len, 1);
        unsigned expected =  *(const unsigned*)(&input[len + 4 * (i / CHKSUMLEN)]);
        assert(chksum == expected);
    }

    // return new length after decryption (without checksum)
    return len;
}
