#define GFTime(x) (((x) << 1) ^ ((((x) >> 7) & 1) * 0x1b))

// reference sbox
const unsigned char sbox_table[256] = {
    // 0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

// AES key schedule: round constants
const byte rcon[] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
    0x80, 0x1B, 0x36
};

void ref_key_schedule(const byte* key, byte* round_keys);

// reference AES key schedule
void ref_key_schedule(const byte* key, byte* round_keys) {
    for (int i = 0; i < 16; ++i) {
        round_keys[i] = key[i];
    }
    for (int i = 1; i < 11; ++i) {
        byte t[4] = {0};
        t[0] = round_keys[(i - 1) * 16 + 12];
        t[1] = round_keys[(i - 1) * 16 + 13];
        t[2] = round_keys[(i - 1) * 16 + 14];
        t[3] = round_keys[(i - 1) * 16 + 15];

        byte temp = t[0];
        t[0] = sbox_table[t[1]];
        t[1] = sbox_table[t[2]];
        t[2] = sbox_table[t[3]];
        t[3] = sbox_table[temp];

        t[0] ^= rcon[i];

        for (int j = 0; j < 4; ++j) {
            round_keys[i * 16 + j] = round_keys[(i - 1) * 16 + j] ^ t[j];
        }
        for (int j = 4; j < 16; ++j) {
            byte right = round_keys[i * 16 + j - 4];
            round_keys[i * 16 + j] = round_keys[(i - 1) * 16 + j] ^ right;
        }
    }
}

void ref_aes_round(byte* input, const byte* key, byte* output);

// reference AES round
void ref_aes_round(byte* input, const byte* key, byte* output) {
    /* SubBytes + ShiftRows */
    byte shifted[16] = {
        sbox_table[input[0]], sbox_table[input[5]], sbox_table[input[10]], sbox_table[input[15]],
        sbox_table[input[4]], sbox_table[input[9]], sbox_table[input[14]], sbox_table[input[3]],
        sbox_table[input[8]], sbox_table[input[13]], sbox_table[input[2]], sbox_table[input[7]],
        sbox_table[input[12]], sbox_table[input[1]], sbox_table[input[6]], sbox_table[input[11]],
    };

    /* MixColumns + AddRoundKey */
    for (int i = 0; i < 4; ++i) {
        byte t = shifted[i * 4 + 0] ^ shifted[i * 4 + 1] ^ shifted[i * 4 + 2] ^ shifted[i * 4 + 3];
        output[i * 4 + 0] = t ^ GFTime(shifted[i * 4 + 0] ^ shifted[i * 4 + 1]) ^ shifted[i * 4 + 0] ^ key[i * 4 + 0];
        output[i * 4 + 1] = t ^ GFTime(shifted[i * 4 + 1] ^ shifted[i * 4 + 2]) ^ shifted[i * 4 + 1] ^ key[i * 4 + 1];
        output[i * 4 + 2] = t ^ GFTime(shifted[i * 4 + 2] ^ shifted[i * 4 + 3]) ^ shifted[i * 4 + 2] ^ key[i * 4 + 2];
        output[i * 4 + 3] = t ^ GFTime(shifted[i * 4 + 3] ^ shifted[i * 4 + 0]) ^ shifted[i * 4 + 3] ^ key[i * 4 + 3];
    }
}

void ref_aes_last_round(byte* input, byte* key, byte* output);

// reference AES last round
void ref_aes_last_round(byte* input, byte* key, byte* output) {
    /* SubBytes + ShiftRows */
    byte shifted[16] = {
        sbox_table[input[0]], sbox_table[input[5]], sbox_table[input[10]], sbox_table[input[15]],
        sbox_table[input[4]], sbox_table[input[9]], sbox_table[input[14]], sbox_table[input[3]],
        sbox_table[input[8]], sbox_table[input[13]], sbox_table[input[2]], sbox_table[input[7]],
        sbox_table[input[12]], sbox_table[input[1]], sbox_table[input[6]], sbox_table[input[11]],
    };

    /* AddRoundKey */
    for (int i = 0; i < 16; ++i) {
        output[i] = shifted[i] ^ key[i];
    }
}

void ref_aes_encrypt(const byte* input, const byte* key, byte* output);

// reference AES encryption (128-bit block)
void ref_aes_encrypt(const byte* input, const byte* key, byte* output) {
    byte round_keys[11 * 16] = {0};
    ref_key_schedule(key, round_keys);
    for (int i = 0; i < 16; ++i) {
        output[i] = input[i] ^ round_keys[i];
    }
    for (int i = 1; i < 10; ++i) {
        ref_aes_round(output, round_keys + i * 16, output);
    }
    ref_aes_last_round(output, round_keys + 10 * 16, output);
}

// Add an 32-bit Adler checksum every 80 bytes of data (5% overhead)
#define CHKSUMLEN 80
#define KEYLEN 16

/*************************************************************************
// Encrypted data: [data][checksum][key]
**************************************************************************/

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
    byte key_stream[16];

    #ifdef CTR_MODE
    for (unsigned i = 0; i < len; i += 16) {
        byte ctr[16] = {0};
        ((unsigned*)ctr)[3] = i / 16;
        ref_aes_encrypt(ctr, key, key_stream);
        for (unsigned j = 0; j < 16 && (i + j) < len; ++j) {
            data[i + j] ^= key_stream[j];
        }
    }
    #else // single block
    byte msg[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    ref_aes_encrypt(msg, key, key_stream);
    for (unsigned i = 0; i < len; ++i) {
        data[i] ^= key_stream[i % 16];
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
    byte key_stream[16];

    #ifdef CTR_MODE
    for (unsigned i = 0; i < len; i += 16) {
        byte ctr[16] = {0};
        ((unsigned*)ctr)[3] = i / 16;
        ref_aes_encrypt(ctr, key, key_stream);
        for (unsigned j = 0; j < 16 && (i + j) < len; ++j) {
            output[i + j] = input[i + j] ^ key_stream[j];
        }
    }
    #else // single block
    byte msg[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    ref_aes_encrypt(msg, key, key_stream);
    for (unsigned i = 0; i < len; ++i) {
        output[i] = input[i] ^ key_stream[i % 16];
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
