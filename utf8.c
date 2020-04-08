#include <stdint.h>

#define INVERTED_EXCLAMATION_MARK           0x00A1
#define CENT_SIGN                           0x00A2
#define POUND_SIGN                          0x00A3
#define CURRENCY_SIGN                       0x00A4
#define YEN_SIGN                            0x00A5
#define BROKEN_BAR                          0x00A6
#define SECTION_SIGN                        0x00A7
#define DIAERESIS                           0x00A8
#define COPYRIGHT_SIGN                      0x00A9
#define FEMININE_ORDINAL_INDICATOR          0x00AA
#define LEFT_POINTING_DOUBLE_QUOTATION_MARK 0x00AB
#define NOT_SIGN                            0x00AC
#define SOFT_SIGN                           0x00AD
#define REGISTERED_SIGN                     0x00AE
#define MACRON                              0x00AF
#define DEGREE_SYMBOL                       0x00B0
#define PLUS_MINUS_SIGN                     0x00B1
#define SUPERSCRIPT_TWO                     0x00B2
#define SUPERSCRIPT_THREE                   0x00B3
#define ACUTE_ACCENT                        0x00B4
#define MICRO_SIGN                          0x00B5
#define PILCRO_SIGN                         0x00B6
#define MIDDLE_DOT                          0x00B7
#define CEDILLA                             0x00B8
#define SUPERSCRIPT_ONE                     0x00B9

// Double Arrows
#define LEFTWARDS_DOUBLE_ARROW_WITH_STROKE  0x21CD
#define LEFT_RIGHT_DOUBLE_ARROW_WITH_STROKE 0x21CE
#define RIGHTWARDS_DOUBLE_ARROW_WITH_STROKE 0x21CF
#define LEFTWARDS_DOUBLE_ARROW              0x21D0
#define UPWARDS_DOUBLE_ARROW                0x21D1
#define RIGHTWARDS_DOUBLE_ARROW             0x21D2
#define DOWNWARDS_DOUBLE_ARROW              0x21D3
#define LEFT_RIGHT_DOUBLE_ARROW             0x21D4
#define UP_DOWN_DOUBLE_ARROW                0x21D5
#define NORTH_WEST_DOUBLE_ARROW             0x21D6 
#define NORTH_EAST_DOUBLE_ARROW             0x21D7
#define SOUTH_EAST_DOUBLE_ARROW             0x21D8
#define SOUTH_WEST_DOUBLE_ARROW             0x21D9


// Chess Symbols
#define WHITE_CHESS_KING                    0x2654
#define WHITE_CHESS_QUEEN                   0x2655
#define WHITE_CHESS_ROOK                    0x2656
#define WHITE_CHESS_BISHOP                  0x2657
#define WHITE_CHESS_KNIGHT                  0x2658
#define WHITE_CHESS_PAWN                    0x2659
#define BLACK_CHESS_KING                    0x265A
#define BLACK_CHESS_QUEEN                   0x265B
#define BLACK_CHESS_ROOK                    0x265C
#define BLACK_CHESS_BISHOP                  0x265D
#define BLACK_CHESS_KNIGHT                  0x265E
#define BLACK_CHESS_PAWN                    0x265F

// Playing Card Symbols
#define BLACK_SPADE_SUIT 0x2660
#define WHITE_HEART_SUIT 0x2661
#define WHITE_DIAMOND_SUIT 0x2662
#define BLACK_CLUB_SUIT 0x2663
#define WHITE_SPADE_SUIT 0x2664
#define BLACK_HEART_SUIT 0x2665
#define BLACK_DIAMOND_SUIT 0x2666
#define WHITE_CLUB_SUIT 0x2667


static void *

utf8_encode(void *buf, long c)
{
    unsigned char *s = buf;

    if (c >= (1L << 16)) {
        s[0] = 0xf0 |  (c >> 18);
        s[1] = 0x80 | ((c >> 12) & 0x3f);
        s[2] = 0x80 | ((c >>  6) & 0x3f);
        s[3] = 0x80 | ((c >>  0) & 0x3f);
        return s + 4;
    } else if (c >= (1L << 11)) {
        s[0] = 0xe0 |  (c >> 12);
        s[1] = 0x80 | ((c >>  6) & 0x3f);
        s[2] = 0x80 | ((c >>  0) & 0x3f);
        return s + 3;
    } else if (c >= (1L << 7)) {
        s[0] = 0xc0 |  (c >>  6);
        s[1] = 0x80 | ((c >>  0) & 0x3f);
        return s + 2;
    } else {
        s[0] = c;
        return s + 1;
    }
}

/* Decode the next character, C, from BUF, reporting errors in E.
 *
 * Since this is a branchless decoder, four bytes will be read from the
 * buffer regardless of the actual length of the next character. This
 * means the buffer _must_ have at least three bytes of zero padding
 * following the end of the data stream.
 *
 * Errors are reported in E, which will be non-zero if the parsed
 * character was somehow invalid: invalid byte sequence, non-canonical
 * encoding, or a surrogate half.
 *
 * The function returns a pointer to the next character. When an error
 * occurs, this pointer will be a guess that depends on the particular
 * error, but it will always advance at least one byte.
 */

static void * utf8_decode_fast(void *buf, uint32_t *c, int *e)
{
    static const char lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };
    static const int masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
    static const uint32_t mins[] = {4194304, 0, 128, 2048, 65536};
    static const int shiftc[] = {0, 18, 12, 6, 0};
    static const int shifte[] = {0, 6, 4, 2, 0};

    unsigned char *s = buf;
    int len = lengths[s[0] >> 3];

    /* Compute the pointer to the next character early so that the next
     * iteration can start working on the next character. Neither Clang
     * nor GCC figure out this reordering on their own.
     */
    unsigned char *next = s + len + !len;

    /* Assume a four-byte character and load four bytes. Unused bits are
     * shifted out.
     */
    *c  = (uint32_t)(s[0] & masks[len]) << 18;
    *c |= (uint32_t)(s[1] & 0x3f) << 12;
    *c |= (uint32_t)(s[2] & 0x3f) <<  6;
    *c |= (uint32_t)(s[3] & 0x3f) <<  0;
    *c >>= shiftc[len];

    /* Accumulate the various error conditions. */
    *e  = (*c < mins[len]) << 6; // non-canonical encoding
    *e |= ((*c >> 11) == 0x1b) << 7;  // surrogate half?
    *e |= (*c > 0x10FFFF) << 8;  // out of range?
    *e |= (s[1] & 0xc0) >> 2;
    *e |= (s[2] & 0xc0) >> 4;
    *e |= (s[3]       ) >> 6;
    *e ^= 0x2a; // top two bits of each tail byte correct?
    *e >>= shifte[len];

    return next;
}
