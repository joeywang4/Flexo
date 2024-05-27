#include <stdint.h>

typedef unsigned char byte;

#define NOT(x) ((x) ? 0 : 1)
#define BIT(x, i) (((x) >> (i)) & 0x1)
#define GFTime(x) (((x) << 1) ^ ((((x) >> 7) & 1) * 0x1b))
#define memcpy(a, b, c) \
    for (int i = 0; i < c; ++i) (b)[i] = (a)[i];
#define THRESHOLD 2
#define SIZE 128

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

inline __attribute__((always_inline)) byte sbox(byte input) {
    SBOX_IMPL(input);
    return out;
}

bool __weird__aes_round(byte* input, byte* key, byte* output, byte* error_output) {
    // prevent clang's optimization on error_output and the return value
    error_output[0] = input[0];

    Sbox_ShiftRows(input, sbox);
    MixColumns_RoundKey(key, output);
}

bool __weird__aes_first_round(
    byte* input, byte* key, byte* round_key, byte* output, byte* error_output
) {
    // prevent clang's optimization on error_output and the return value
    error_output[0] = input[0];

    /* SubBytes + ShiftRows */
    byte shifted_0 = sbox(key[0] ^ input[0]);
    byte shifted_1 = sbox(key[5] ^ input[5]);
    byte shifted_2 = sbox(key[10] ^ input[10]);
    byte shifted_3 = sbox(key[15] ^ input[15]);
    byte shifted_4 = sbox(key[4] ^ input[4]);
    byte shifted_5 = sbox(key[9] ^ input[9]);
    byte shifted_6 = sbox(key[14] ^ input[14]);
    byte shifted_7 = sbox(key[3] ^ input[3]);
    byte shifted_8 = sbox(key[8] ^ input[8]);
    byte shifted_9 = sbox(key[13] ^ input[13]);
    byte shifted_A = sbox(key[2] ^ input[2]);
    byte shifted_B = sbox(key[7] ^ input[7]);
    byte shifted_C = sbox(key[12] ^ input[12]);
    byte shifted_D = sbox(key[1] ^ input[1]);
    byte shifted_E = sbox(key[6] ^ input[6]);
    byte shifted_F = sbox(key[11] ^ input[11]);

    MixColumns_RoundKey(round_key, output);
}

bool __weird__aes_last_round(
    byte* input, byte* key, byte rcon, byte* output, byte* error_output
) {
    // last round key
    byte t_0 = sbox(key[13]) ^ rcon;
    byte t_1 = sbox(key[14]);
    byte t_2 = sbox(key[15]);
    byte t_3 = sbox(key[12]);

    byte next_key_0 = key[0] ^ t_0;
    byte next_key_1 = key[1] ^ t_1;
    byte next_key_2 = key[2] ^ t_2;
    byte next_key_3 = key[3] ^ t_3;

    /* SubBytes + ShiftRows + AddRoundKey */
    output[0] = next_key_0 ^ sbox(input[0]);
    output[1] = next_key_1 ^ sbox(input[5]);
    output[2] = next_key_2 ^ sbox(input[10]);
    output[3] = next_key_3 ^ sbox(input[15]);

    byte next_key_4 = key[4] ^ next_key_0;
    byte next_key_5 = key[5] ^ next_key_1;
    byte next_key_6 = key[6] ^ next_key_2;
    byte next_key_7 = key[7] ^ next_key_3;

    output[4] = next_key_4 ^ sbox(input[4]);
    output[5] = next_key_5 ^ sbox(input[9]);
    output[6] = next_key_6 ^ sbox(input[14]);
    output[7] = next_key_7 ^ sbox(input[3]);

    byte next_key_8 = key[8] ^ next_key_4;
    byte next_key_9 = key[9] ^ next_key_5;
    byte next_key_10 = key[10] ^ next_key_6;
    byte next_key_11 = key[11] ^ next_key_7;

    output[8] = next_key_8 ^ sbox(input[8]);
    output[9] = next_key_9 ^ sbox(input[13]);
    output[10] = next_key_10 ^ sbox(input[2]);
    output[11] = next_key_11 ^ sbox(input[7]);

    output[12] = next_key_8 ^ key[12] ^ sbox(input[12]);
    output[13] = next_key_9 ^ key[13] ^ sbox(input[1]);
    output[14] = next_key_10 ^ key[14] ^ sbox(input[6]);
    output[15] = next_key_11 ^ key[15] ^ sbox(input[11]);

    // prevent clang's optimization on error_output and the return value
    error_output[0] = input[0];
    return error_output[0] & 1;
}

bool __weird__round_key(
    byte* prev_key, byte* next_key, byte rcon, byte* error_next_key
) {
    // prevent clang's optimization on error_output and the return value
    error_next_key[0] = prev_key[0];

    byte t_0 = sbox(prev_key[13]) ^ rcon;
    byte t_1 = sbox(prev_key[14]);
    byte t_2 = sbox(prev_key[15]);
    byte t_3 = sbox(prev_key[12]);

    next_key[0] = prev_key[0] ^ t_0;
    next_key[1] = prev_key[1] ^ t_1;
    next_key[2] = prev_key[2] ^ t_2;
    next_key[3] = prev_key[3] ^ t_3;

    next_key[4] = prev_key[4] ^ next_key[0];
    next_key[5] = prev_key[5] ^ next_key[1];
    next_key[6] = prev_key[6] ^ next_key[2];
    next_key[7] = prev_key[7] ^ next_key[3];

    next_key[8] = prev_key[8] ^ next_key[4];
    next_key[9] = prev_key[9] ^ next_key[5];
    next_key[10] = prev_key[10] ^ next_key[6];
    next_key[11] = prev_key[11] ^ next_key[7];

    next_key[12] = prev_key[12] ^ next_key[8];
    next_key[13] = prev_key[13] ^ next_key[9];
    next_key[14] = prev_key[14] ^ next_key[10];
    next_key[15] = prev_key[15] ^ next_key[11];
    
    return next_key[0];
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

bool aes_encrypt(byte* input, byte* key, byte* output) {
    byte round_key[16], err_out[16], round_input[16], prev_round_key[16];
    char votes[SIZE];
    bool has_error;
    #define rcon(i) ((i == 9) ? 0x1B : (1 << (i - 1)))
    merge_output(round_key, votes);

    // first round
    do {
        has_error = __weird__round_key(key, round_key, 1, err_out);
        if (update_votes(round_key, err_out, votes)) has_error = false;
    } while (has_error);
    merge_output(round_key, votes);

    do {
        has_error = __weird__aes_first_round(input, key, round_key, output, err_out);
        if (update_votes(output, err_out, votes)) has_error = false;
    } while (has_error);
    merge_output(output, votes);

    memcpy(round_key, prev_round_key, 16);
    memcpy(output, round_input, 16);

    for (unsigned i = 2; i < 10; ++i) {
        do {
            has_error = __weird__round_key(
                prev_round_key, round_key, rcon(i), err_out
            );
            if (update_votes(round_key, err_out, votes)) has_error = false;
        } while (has_error);
        merge_output(round_key, votes);
        memcpy(round_key, prev_round_key, 16);

        do {
            has_error = __weird__aes_round(round_input, round_key, output, err_out);
            if (update_votes(output, err_out, votes)) has_error = false;
        } while (has_error);
        merge_output(output, votes);
        memcpy(output, round_input, 16);
    }

    do {
        has_error = __weird__aes_last_round(
            round_input, prev_round_key, 0x36, output, err_out
        );
        if (update_votes(output, err_out, votes)) has_error = false;
    } while (has_error);
    merge_output(output, votes);

    return has_error;
}
