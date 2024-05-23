#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define THRESHOLD 2
#define SIZE 5
#define UNIT_SIZE 32

#define F1 (b & c) | ((~b) & d)
#define F2 b ^ c ^ d
#define F3 (b & c) | (b & d) | (c & d)
#define F4 b ^ c ^ d
#define C1 0x5A827999
#define C2 0x6ED9EBA1
#define C3 0x8F1BBCDC
#define C4 0xCA62C1D6

#define SHA1_ROUND(inputs, w, outputs, r) \
    unsigned a = inputs[0]; \
    unsigned b = inputs[1]; \
    unsigned c = inputs[2]; \
    unsigned d = inputs[3]; \
    unsigned e = inputs[4]; \
    unsigned f = F##r; \
    unsigned temp = ROL(a, 5) + f + e + w + C##r; \
    outputs[0] = temp; \
    outputs[1] = a; \
    outputs[2] = ROL(b, 30); \
    outputs[3] = c; \
    outputs[4] = d; \
    error_output[0] = f; \
    return f & 1;

#define RUN_SHA1_ROUND(r, w) \
    do { \
        has_error = __weird__sha1_round##r(states, w, new_states, err_out); \
        if (update_votes(new_states, err_out, votes)) has_error = false; \
    } while (retry && has_error); \
    merge_output(new_states, votes);

uint64_t time_elasped(struct timespec* begin, struct timespec* end) {
    int64_t output = end->tv_sec - begin->tv_sec;
    output *= 1000000000;
    output += end->tv_nsec - begin->tv_nsec;
    return output;
}

void ref_sha1_round(unsigned* inputs, unsigned w, unsigned* outputs, unsigned round) {
    unsigned a = inputs[0];
    unsigned b = inputs[1];
    unsigned c = inputs[2];
    unsigned d = inputs[3];
    unsigned e = inputs[4];
    const unsigned constants[4] = {C1, C2, C3, C4};

    unsigned f;
    switch(round) {
        case 0:
            f = (b & c) | ((~b) & d);
            break;
        case 1:
            f = b ^ c ^ d;
            break;
        case 2:
            f = (b & c) | (b & d) | (c & d);
            break;
        case 3:
        default:
            f = b ^ c ^ d;
            break;
    }
    unsigned temp = ROL(a, 5) + f + e + w + constants[round];

    outputs[0] = temp;
    outputs[1] = a;
    outputs[2] = ROL(b, 30);
    outputs[3] = c;
    outputs[4] = d;
}

bool update_votes(unsigned* output, unsigned* error_output, char* votes) {
    bool all_done = true;
    for (int j = 0; j < SIZE * UNIT_SIZE; ++j) {
        bool out_bit = (output[j / UNIT_SIZE] >> (j % UNIT_SIZE)) & 1;
        bool err_bit = (error_output[j / UNIT_SIZE] >> (j % UNIT_SIZE)) & 1;

        if (!err_bit) {
            if (out_bit && votes[j] < 126) ++votes[j];
            else if (!out_bit && votes[j] > -126) --votes[j];
        }
        if (votes[j] < THRESHOLD && votes[j] > -THRESHOLD) all_done = false;
    }
    return all_done;
}

void merge_output(unsigned* output, char* votes) {
    for (unsigned j = 0; j < SIZE * UNIT_SIZE; ++j) {
        if (votes[j] >= THRESHOLD) {
            output[j / UNIT_SIZE] |= 1 << (j % UNIT_SIZE);
        }
        else if (votes[j] <= -THRESHOLD) {
            output[j / UNIT_SIZE] &= ~(1 << (j % UNIT_SIZE));
        }
        votes[j] = 0;
    }
}
