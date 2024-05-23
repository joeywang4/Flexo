#include <stdint.h>
#include <time.h>
#include <argp.h>

/* Arguments */
static char doc[] = "Test the accuracy and run time of weird machines.";
static char args_doc[] = "";
static struct argp_option options[] = {
    { "retry", 'r', 0, 0, "Retry when an error is detected"},
    { "trial", 't', "TRIAL", 0, "Number of trials"},
    { 0 } 
};

typedef unsigned char byte;

#define NOT(x) ((x) ? 0 : 1)
#define BIT(x, i) (((x) >> (i)) & 0x1)
#define GFTime(x) (((x) << 1) ^ ((((x) >> 7) & 1) * 0x1b))
#define THRESHOLD 2
#define SIZE 128

uint64_t time_elasped(struct timespec* begin, struct timespec* end) {
    int64_t output = end->tv_sec - begin->tv_sec;
    output *= 1000000000;
    output += end->tv_nsec - begin->tv_nsec;
    return output;
}

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

// circuit sbox
inline uint8_t GF_MULS_2(uint8_t A, bool ab, uint8_t B, bool cd)
{
    bool abcd = NOT(ab & cd);
    bool p = NOT(BIT(A, 1) & BIT(B, 1)) ^ abcd;
    bool q = NOT(BIT(A, 0) & BIT(B, 0)) ^ abcd;
    return q | (p << 1);
}

inline uint8_t GF_SQ_2(uint8_t A)
{
    return ((A & 2) >> 1) | ((A & 1) << 1);
}

inline uint8_t GF_MULS_SCL_2(uint8_t A, bool ab, uint8_t B, bool cd)
{
    bool t = NOT(BIT(A, 0) & BIT(B, 0));
    bool p = NOT(ab & cd) ^ t;
    bool q = NOT(BIT(A, 1) & BIT(B, 1)) ^ t;
    return q | (p << 1);
}

inline uint8_t GF_MULS_4(uint8_t A, uint8_t a1, bool Al, bool Ah, bool aa, uint8_t B, uint8_t b1, bool Bl, bool Bh, bool bb)
{
    uint8_t ph = GF_MULS_2(A >> 2, Ah, B >> 2, Bh);
    uint8_t pl = GF_MULS_2(A & 3, Al, B & 3, Bl);
    uint8_t p = GF_MULS_SCL_2(a1, aa, b1, bb);
    uint8_t Q_0 = p ^ pl;
    uint8_t Q_1 = p ^ ph;
    return (Q_0 & 3) | ((Q_1 & 3) << 2);
}

inline uint8_t GF_INV_4(uint8_t X)
{
    uint8_t a, b;
    bool sa, sb, sd;

    a = X >> 2;
    b = X & 0x3;
    sa = BIT(a, 1) ^ BIT(a, 0);
    sb = BIT(b, 1) ^ BIT(b, 0);

    uint8_t c = 0;

    c |= NOT(sa | sb) ^ NOT(BIT(a, 0) & BIT(b, 0));
    c |= (NOT(BIT(a, 1) | BIT(b, 1)) ^ NOT(sa & sb)) << 1;


    uint8_t d = GF_SQ_2(c);

    sd = BIT(d, 1) ^ BIT(d, 0);

    uint8_t p = GF_MULS_2(d, sd, b, sb);
    uint8_t q = GF_MULS_2(d, sd, a, sa);

    return (q & 3) | ((p & 3) << 2);
}

#define SBOX_IMPL(input) \
    bool R1, R2, R3, R4, R5, R6, R7, R8, R9; \
    bool T1, T2, T3, T4, T5, T6, T7, T8, T9; \
    uint8_t B = 0; \
    R1 = BIT(input, 7) ^ BIT(input, 5); \
    R2 = BIT(input, 7) ^ BIT(input, 4); \
    R3 = BIT(input, 6) ^ BIT(input, 0); \
    R4 = BIT(input, 5) ^ R3; \
    R5 = BIT(input, 4) ^ R4; \
    R6 = BIT(input, 3) ^ BIT(input, 0); \
    R7 = BIT(input, 2) ^ R1; \
    R8 = BIT(input, 1) ^ R3; \
    R9 = BIT(input, 3) ^ R8; \
    B |= (R7 ^ R8) << 7; \
    B |= R5 << 6; \
    B |= (BIT(input, 1) ^ R4) << 5; \
    B |= (R1 ^ R3) << 4; \
    B |= (BIT(input, 1) ^ R2 ^ R6) << 3; \
    B |= BIT(input, 0) << 2; \
    B |= R4 << 1; \
    B |= BIT(input, 2) ^ R9; \
    uint8_t C; \
    { \
        uint8_t a, b, sa, sb, c = 0, sd; \
        bool al, ah, aa, bl, bh, bb, c1, c2, c3, dl, dh, dd; \
        a = B >> 4; \
        b = B & 0xF; \
        sa = (a >> 2) ^ (a & 0x3); \
        sb = (b >> 2) ^ (b & 0x3); \
        al = BIT(a, 1) ^ BIT(a, 0); \
        ah = BIT(a, 3) ^ BIT(a, 2); \
        aa = BIT(sa, 1) ^ BIT(sa, 0); \
        bl = BIT(b, 1) ^ BIT(b, 0); \
        bh = BIT(b, 3) ^ BIT(b, 2); \
        bb = BIT(sb, 1) ^ BIT(sb, 0); \
        c1 = NOT(ah & bh); \
        c2 = NOT(BIT(sa, 0) & BIT(sb, 0)); \
        c3 = NOT(aa & bb); \
        c |= NOT(BIT(a, 0) | BIT(b, 0)) ^ NOT(al & bl) ^ NOT(BIT(sa, 1) & BIT(sb, 1)) ^ c2; \
        c |= (NOT(al | bl) ^ NOT(BIT(a, 1) & BIT(b, 1)) ^ c2 ^ c3) << 1; \
        c |= (NOT(BIT(sa, 1) | BIT(sb, 1)) ^ NOT(BIT(a, 2) & BIT(b, 2)) ^ c1 ^ c2) << 2; \
        c |= (NOT(BIT(sa, 0) | BIT(sb, 0)) ^ NOT(BIT(a, 3) & BIT(b, 3)) ^ c1 ^ c3) << 3; \
        uint8_t d = GF_INV_4(c); \
        sd = (d >> 2) ^ (d & 0x3); \
        dl = BIT(d, 1) ^ BIT(d, 0); \
        dh = BIT(d, 3) ^ BIT(d, 2); \
        dd = BIT(sd, 1) ^ BIT(sd, 0); \
        uint8_t p = GF_MULS_4(d, sd, dl, dh, dd, b, sb, bl, bh, bb); \
        uint8_t q = GF_MULS_4(d, sd, dl, dh, dd, a, sa, al, ah, aa); \
        C = (q & 0xF) | ((p & 0xF) << 4); \
    } \
    T1 = BIT(C, 7) ^ BIT(C, 3); \
    T2 = BIT(C, 6) ^ BIT(C, 4); \
    T3 = BIT(C, 6) ^ BIT(C, 0); \
    T4 = BIT(C, 5) ^ BIT(C, 3); \
    T5 = BIT(C, 5) ^ T1; \
    T6 = BIT(C, 5) ^ BIT(C, 1); \
    T7 = BIT(C, 4) ^ T6; \
    T8 = BIT(C, 2) ^ T4; \
    T9 = BIT(C, 1) ^ T2; \
    uint8_t out = 0; \
    out |= T4 << 7; \
    out |= NOT(T1) << 6; \
    out |= NOT(T3) << 5; \
    out |= T5 << 4; \
    out |= (T2 ^ T5) << 3; \
    out |= (T3 ^ T8) << 2; \
    out |= NOT(T7) << 1; \
    out |= NOT(T9);

// AES round: shift rows
#define ShiftRows(input) \
    /* ShiftRows */ \
    byte shifted_0 = input[0]; \
    byte shifted_1 = input[5]; \
    byte shifted_2 = input[10]; \
    byte shifted_3 = input[15]; \
    byte shifted_4 = input[4]; \
    byte shifted_5 = input[9]; \
    byte shifted_6 = input[14]; \
    byte shifted_7 = input[3]; \
    byte shifted_8 = input[8]; \
    byte shifted_9 = input[13]; \
    byte shifted_A = input[2]; \
    byte shifted_B = input[7]; \
    byte shifted_C = input[12]; \
    byte shifted_D = input[1]; \
    byte shifted_E = input[6]; \
    byte shifted_F = input[11];

// AES round: sbox + shift rows
#define Sbox_ShiftRows(input, sbox) \
    /* SubBytes + ShiftRows */ \
    byte shifted_0 = sbox(input[0]); \
    byte shifted_1 = sbox(input[5]); \
    byte shifted_2 = sbox(input[10]); \
    byte shifted_3 = sbox(input[15]); \
    byte shifted_4 = sbox(input[4]); \
    byte shifted_5 = sbox(input[9]); \
    byte shifted_6 = sbox(input[14]); \
    byte shifted_7 = sbox(input[3]); \
    byte shifted_8 = sbox(input[8]); \
    byte shifted_9 = sbox(input[13]); \
    byte shifted_A = sbox(input[2]); \
    byte shifted_B = sbox(input[7]); \
    byte shifted_C = sbox(input[12]); \
    byte shifted_D = sbox(input[1]); \
    byte shifted_E = sbox(input[6]); \
    byte shifted_F = sbox(input[11]);

// AES round: mix columns + add round key
#define MixColumns_RoundKey(key, output) \
    /* MixColumns + AddRoundKey */ \
    byte t_0 = shifted_0 ^ shifted_1 ^ shifted_2 ^ shifted_3; \
    output[0] = t_0 ^ GFTime(shifted_0 ^ shifted_1) ^ shifted_0 ^ key[0]; \
    output[1] = t_0 ^ GFTime(shifted_1 ^ shifted_2) ^ shifted_1 ^ key[1]; \
    output[2] = t_0 ^ GFTime(shifted_2 ^ shifted_3) ^ shifted_2 ^ key[2]; \
    output[3] = t_0 ^ GFTime(shifted_3 ^ shifted_0) ^ shifted_3 ^ key[3]; \
    byte t_1 = shifted_4 ^ shifted_5 ^ shifted_6 ^ shifted_7; \
    output[4] = t_1 ^ GFTime(shifted_4 ^ shifted_5) ^ shifted_4 ^ key[4]; \
    output[5] = t_1 ^ GFTime(shifted_5 ^ shifted_6) ^ shifted_5 ^ key[5]; \
    output[6] = t_1 ^ GFTime(shifted_6 ^ shifted_7) ^ shifted_6 ^ key[6]; \
    output[7] = t_1 ^ GFTime(shifted_7 ^ shifted_4) ^ shifted_7 ^ key[7]; \
    byte t_2 = shifted_8 ^ shifted_9 ^ shifted_A ^ shifted_B; \
    output[8] = t_2 ^ GFTime(shifted_8 ^ shifted_9) ^ shifted_8 ^ key[8]; \
    output[9] = t_2 ^ GFTime(shifted_9 ^ shifted_A) ^ shifted_9 ^ key[9]; \
    output[10] = t_2 ^ GFTime(shifted_A ^ shifted_B) ^ shifted_A ^ key[10]; \
    output[11] = t_2 ^ GFTime(shifted_B ^ shifted_8) ^ shifted_B ^ key[11]; \
    byte t_3 = shifted_C ^ shifted_D ^ shifted_E ^ shifted_F; \
    output[12] = t_3 ^ GFTime(shifted_C ^ shifted_D) ^ shifted_C ^ key[12]; \
    output[13] = t_3 ^ GFTime(shifted_D ^ shifted_E) ^ shifted_D ^ key[13]; \
    output[14] = t_3 ^ GFTime(shifted_E ^ shifted_F) ^ shifted_E ^ key[14]; \
    output[15] = t_3 ^ GFTime(shifted_F ^ shifted_C) ^ shifted_F ^ key[15]; \
    return output[0];

// AES key schedule: round constants
const byte rcon[] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
    0x80, 0x1B, 0x36
};

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

bool update_votes(byte* output, byte* error_output, char* votes) {
    bool all_done = true;
    for (int j = 0; j < SIZE; ++j) {
        bool out_bit = (output[j / 8] >> (j % 8)) & 1;
        bool err_bit = (error_output[j / 8] >> (j % 8)) & 1;

        if (!err_bit) {
            if (out_bit && votes[j] < 126) ++votes[j];
            else if (!out_bit && votes[j] > -126) --votes[j];
        }
        if (votes[j] < THRESHOLD && votes[j] > -THRESHOLD) all_done = false;
    }
    return all_done;
}

void merge_output(byte* output, char* votes) {
    for (unsigned j = 0; j < SIZE; ++j) {
        if (votes[j] >= THRESHOLD) output[j / 8] |= 1 << (j % 8);
        else if (votes[j] <= -THRESHOLD) output[j / 8] &= ~(1 << (j % 8));
        votes[j] = 0;
    }
}
