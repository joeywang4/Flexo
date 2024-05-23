#define TO_LONG(ARR) \
    ARR[0] + \
    ((u_int64_t)(ARR[1]) << 8) + \
    ((u_int64_t)(ARR[2]) << 16) + \
    ((u_int64_t)(ARR[3]) << 24) + \
    ((u_int64_t)(ARR[4]) << 32) + \
    ((u_int64_t)(ARR[5]) << 40) + \
    ((u_int64_t)(ARR[6]) << 48) + \
    ((u_int64_t)(ARR[7]) << 56)

#define MASK(NUM, BITS) \
    NUM <<= 64 - BITS; \
    NUM >>= 64 - BITS

#define TO_BYTES1(ARR, NUM) \
    ARR[0] = NUM & 0xFF

#define TO_BYTES2(ARR, NUM) \
    TO_BYTES1(ARR, NUM); \
    ARR[1] = (NUM >> 8) & 0xFF

#define TO_BYTES3(ARR, NUM) \
    TO_BYTES2(ARR, NUM); \
    ARR[2] = (NUM >> 16) & 0xFF

#define TO_BYTES4(ARR, NUM) \
    TO_BYTES3(ARR, NUM); \
    ARR[3] = (NUM >> 24) & 0xFF

#define TO_BYTES5(ARR, NUM) \
    TO_BYTES4(ARR, NUM); \
    ARR[4] = (NUM >> 32) & 0xFF

#define TO_BYTES6(ARR, NUM) \
    TO_BYTES5(ARR, NUM); \
    ARR[5] = (NUM >> 40) & 0xFF

#define TO_BYTES7(ARR, NUM) \
    TO_BYTES6(ARR, NUM); \
    ARR[6] = (NUM >> 48) & 0xFF

#define TO_BYTES8(ARR, NUM) \
    TO_BYTES7(ARR, NUM); \
    ARR[7] = (NUM >> 56) & 0xFF

#define RAND64() ((u_int64_t)rand() << 32) | rand()

# define EXPR_IF(bit, expr) BOOST_PP_EXPR_IIF_I(bit, expr)
# define BOOST_PP_EXPR_IIF_I(bit, expr) BOOST_PP_EXPR_IIF_ ## bit(expr)
# define BOOST_PP_EXPR_IIF_0(expr)
# define BOOST_PP_EXPR_IIF_1(expr) expr
