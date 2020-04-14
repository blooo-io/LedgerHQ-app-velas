#include "message.c"
#include "sol/parser.h"
#include "sol/transaction_summary.h"
#include "util.h"
#include <assert.h>
#include <stdio.h>

void test_process_message_body_ok() {
    Pubkey accounts[] = {
        {{171, 88, 202, 32, 185, 160, 182, 116, 130, 185, 73, 48, 13, 216, 170, 71, 172, 195, 165, 123, 87, 70, 130, 219, 5, 157, 240, 187, 26, 191, 158, 218}},
        {{204, 241, 115, 109, 41, 173, 110, 48, 24, 113, 210, 213, 163, 78, 1, 112, 146, 114, 235, 220, 96, 185, 184, 85, 163, 27, 124, 48, 54, 250, 233, 54}},
        {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    };
    Blockhash blockhash = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    MessageHeader header = {{1, 0, 1, 3}, accounts, &blockhash, 1};
    uint8_t msg_body[] = {2, 2, 0, 1, 12, 2, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0};

    transaction_summary_reset();
    assert(process_message_body(msg_body, ARRAY_LEN(msg_body), &header) == 0);
    enum SummaryItemKind kinds[MAX_TRANSACTION_SUMMARY_ITEMS];
    size_t num_kinds;
    assert(transaction_summary_finalize(kinds, &num_kinds) == 0);
    assert(num_kinds == 4);
}

void test_process_message_body_xfer_w_nonce_ok() {
    Pubkey accounts[] = {
        {{171, 88, 202, 32, 185, 160, 182, 116, 130, 185, 73, 48, 13, 216, 170, 71, 172, 195, 165, 123, 87, 70, 130, 219, 5, 157, 240, 187, 26, 191, 158, 218}},
        {{204, 241, 115, 109, 41, 173, 110, 48, 24, 113, 210, 213, 163, 78, 1, 112, 146, 114, 235, 220, 96, 185, 184, 85, 163, 27, 124, 48, 54, 250, 233, 54}},
        {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    };
    Blockhash blockhash = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    MessageHeader header = {{1, 0, 1, 3}, accounts, &blockhash, 2};
    uint8_t msg_body[] = {
        2, 3, 0, 1, 0, 4, 4, 0, 0, 0, // Nonce
        2, 2, 0, 1, 12, 2, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0
    };
    transaction_summary_reset();
    assert(process_message_body(msg_body, ARRAY_LEN(msg_body), &header) == 0);
    enum SummaryItemKind kinds[MAX_TRANSACTION_SUMMARY_ITEMS];
    size_t num_kinds;
    assert(transaction_summary_finalize(kinds, &num_kinds) == 0);
    assert(num_kinds == 6);
}

void test_process_message_body_too_few_ix_fail() {
    MessageHeader header = {{0, 0, 0, 0}, NULL, NULL, 0};
    assert(process_message_body(NULL, 0, &header) == 1);
}

void test_process_message_body_too_many_ix_fail() {
    Pubkey accounts[] = {
        {{171, 88, 202, 32, 185, 160, 182, 116, 130, 185, 73, 48, 13, 216, 170, 71, 172, 195, 165, 123, 87, 70, 130, 219, 5, 157, 240, 187, 26, 191, 158, 218}},
        {{204, 241, 115, 109, 41, 173, 110, 48, 24, 113, 210, 213, 163, 78, 1, 112, 146, 114, 235, 220, 96, 185, 184, 85, 163, 27, 124, 48, 54, 250, 233, 54}},
        {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    };
    Blockhash blockhash = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    uint8_t xfer_ix[] = {2, 2, 0, 1, 12, 2, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0};

#define TOO_MANY_IX (MAX_INSTRUCTIONS + 1)
#define XFER_IX_LEN ARRAY_LEN(xfer_ix)

    uint8_t msg_body[TOO_MANY_IX * XFER_IX_LEN];
    for (size_t i = 0; i < TOO_MANY_IX; i++) {
        uint8_t* start = msg_body + (i * XFER_IX_LEN);
        memcpy(start, xfer_ix, XFER_IX_LEN);
    }
    MessageHeader header = {{1, 0, 1, 3}, accounts, &blockhash, TOO_MANY_IX};
    assert(process_message_body(msg_body, ARRAY_LEN(msg_body), &header) == 1);
}

void test_process_message_body_data_too_short_fail() {
    MessageHeader header = {{0, 0, 0, 0}, NULL, NULL, 1};
    assert(process_message_body(NULL, 0, &header) == 1);
}

void test_process_message_body_data_too_long_fail() {
    Pubkey accounts[] = {
        {{171, 88, 202, 32, 185, 160, 182, 116, 130, 185, 73, 48, 13, 216, 170, 71, 172, 195, 165, 123, 87, 70, 130, 219, 5, 157, 240, 187, 26, 191, 158, 218}},
        {{204, 241, 115, 109, 41, 173, 110, 48, 24, 113, 210, 213, 163, 78, 1, 112, 146, 114, 235, 220, 96, 185, 184, 85, 163, 27, 124, 48, 54, 250, 233, 54}},
        {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    };
    Blockhash blockhash = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    MessageHeader header = {{1, 0, 1, 3}, accounts, &blockhash, 1};
    uint8_t msg_body[] = {
        2, 2, 0, 1, 12, 2, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0,
        0
    };
    assert(process_message_body(msg_body, ARRAY_LEN(msg_body), &header) == 1);
}

void test_process_message_body_bad_ix_account_index_fail() {
    MessageHeader header = {{0, 0, 0, 1}, NULL, NULL, 1};
    uint8_t msg_body[] = {1, 0, 0};
    assert(process_message_body(msg_body, ARRAY_LEN(msg_body), &header) == 1);
}

void test_process_message_body_unknown_ix_enum_fail() {
    Pubkey accounts[] = {
        {{171, 88, 202, 32, 185, 160, 182, 116, 130, 185, 73, 48, 13, 216, 170, 71, 172, 195, 165, 123, 87, 70, 130, 219, 5, 157, 240, 187, 26, 191, 158, 218}},
        {{204, 241, 115, 109, 41, 173, 110, 48, 24, 113, 210, 213, 163, 78, 1, 112, 146, 114, 235, 220, 96, 185, 184, 85, 163, 27, 124, 48, 54, 250, 233, 54}},
        {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    };
    Blockhash blockhash = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    MessageHeader header = {{1, 0, 1, 3}, accounts, &blockhash, 1};
    uint8_t msg_body[] = {
        2, 2, 0, 1, 12, 255, 255, 255, 255, 42, 0, 0, 0, 0, 0, 0, 0,
    };
    assert(process_message_body(msg_body, ARRAY_LEN(msg_body), &header) == 1);
}

void test_process_message_body_ix_with_unknown_program_id_fail() {
    Pubkey accounts[] = {
        {{171, 88, 202, 32, 185, 160, 182, 116, 130, 185, 73, 48, 13, 216, 170, 71, 172, 195, 165, 123, 87, 70, 130, 219, 5, 157, 240, 187, 26, 191, 158, 218}},
        {{204, 241, 115, 109, 41, 173, 110, 48, 24, 113, 210, 213, 163, 78, 1, 112, 146, 114, 235, 220, 96, 185, 184, 85, 163, 27, 124, 48, 54, 250, 233, 54}},
        {{255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}},
    };
    Blockhash blockhash = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    MessageHeader header = {{1, 0, 1, 3}, accounts, &blockhash, 1};
    uint8_t msg_body[] = {
        2, 2, 0, 1, 12, 2, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0,
    };
    assert(process_message_body(msg_body, ARRAY_LEN(msg_body), &header) == 1);
}

static void process_message_body_and_sanity_check(const uint8_t* message, size_t message_length, size_t expected_fields) {
    MessageHeader header;
    Parser parser = { message, message_length };
    assert(parse_message_header(&parser, &header) == 0);
    transaction_summary_reset();
    assert(process_message_body(parser.buffer, parser.buffer_length, &header) == 0);
    transaction_summary_set_fee_payer_pubkey(&header.pubkeys[0]);

    enum SummaryItemKind kinds[MAX_TRANSACTION_SUMMARY_ITEMS];
    size_t num_kinds;
    assert(transaction_summary_finalize(kinds, &num_kinds) == 0);
    assert(num_kinds == expected_fields);
    for (size_t i = 0; i < num_kinds; i++) {
        assert(transaction_summary_display_item(i) == 0);
    }
}

void test_process_message_body_nonced_stake_create_with_seed() {
    uint8_t message[] = {
        2, 1, 4, 8,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            247, 157, 35, 131, 179, 105, 135, 105, 0, 178, 6, 62, 22, 251, 47, 102, 208, 237, 66, 72, 149, 5, 127, 149, 253, 28, 66, 250, 52, 173, 30, 105,
            6, 167, 213, 23, 25, 44, 86, 142, 224, 138, 132, 95, 115, 210, 151, 136, 207, 3, 92, 49, 69, 178, 26, 179, 68, 216, 6, 46, 169, 64, 0, 0,
            6, 167, 213, 23, 25, 44, 92, 81, 33, 140, 201, 76, 61, 74, 241, 127, 88, 218, 238, 8, 155, 161, 253, 68, 227, 219, 217, 138, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            6, 161, 216, 23, 145, 55, 84, 42, 152, 52, 55, 189, 254, 42, 122, 178, 85, 127, 83, 92, 138, 120, 114, 43, 104, 164, 157, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3,
            // system - advance nonce
            6,
            3,
                2, 4, 1,
            4,
                4, 0, 0, 0,
            // system - create account with seed
            6,
            2,
                0, 3,
            124,
                3, 0, 0, 0,
                18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
                32, 0, 0, 0, 0, 0, 0, 0,
                    115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100,
                42, 0, 0, 0, 0, 0, 0, 0,
                200, 0, 0, 0, 0, 0, 0, 0,
                6, 161, 216, 23, 145, 55, 84, 42, 152, 52, 55, 189, 254, 42, 122, 178, 85, 127, 83, 92, 138, 120, 114, 43, 104, 164, 157, 192, 0, 0, 0, 0,
            // stake - initialize
            7,
            2,
                3, 5,
            116,
                0, 0, 0, 0,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    process_message_body_and_sanity_check(message, sizeof(message), 12);
}

void test_process_message_body_create_stake_account() {
    uint8_t message[] = {
        2, 0, 3,
        5,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 25, 44, 92, 81, 33, 140, 201, 76, 61, 74, 241, 127, 88, 218, 238, 8, 155, 161, 253, 68, 227, 219, 217, 138, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            6, 161, 216, 23, 145, 55, 84, 42, 152, 52, 55, 189, 254, 42, 122, 178, 85, 127, 83, 92, 138, 120, 114, 43, 104, 164, 157, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2,
            // system - create account
            3,
            2,
                0, 1,
            52,
                0, 0, 0, 0,
                42, 0, 0, 0, 0, 0, 0, 0,
                200, 0, 0, 0, 0, 0, 0, 0,
                6, 161, 216, 23, 145, 55, 84, 42, 152, 52, 55, 189, 254, 42, 122, 178, 85, 127, 83, 92, 138, 120, 114, 43, 104, 164, 157, 192, 0, 0, 0, 0,
            // stake - initialize
            4,
            2,
                1, 2,
            116,
                0, 0, 0, 0,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    process_message_body_and_sanity_check(message, sizeof(message), 8);
}

void test_process_message_body_create_nonce_account() {
    uint8_t message[] = {
        2, 0, 3,
        5,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 25, 44, 86, 142, 224, 138, 132, 95, 115, 210, 151, 136, 207, 3, 92, 49, 69, 178, 26, 179, 68, 216, 6, 46, 169, 64, 0, 0,
            6, 167, 213, 23, 25, 44, 92, 81, 33, 140, 201, 76, 61, 74, 241, 127, 88, 218, 238, 8, 155, 161, 253, 68, 227, 219, 217, 138, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2,
            // system - create account
            4,
            2,
                0, 1,
            52,
                0, 0, 0, 0,
                42, 0, 0, 0, 0, 0, 0, 0,
                80, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // system - initialize nonce
            4,
            3,
                1, 2, 3,
            36,
                6, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    process_message_body_and_sanity_check(message, sizeof(message), 5);
}

void test_process_message_body_create_nonce_account_with_seed() {
    uint8_t message[] = {
        1, 0, 3,
        5,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            133, 66, 139, 176, 93, 124, 142, 23, 153, 82, 31, 46, 236, 244, 156, 121, 7, 225, 187, 61, 33, 34, 179, 138, 134, 108, 157, 56, 213, 162, 32, 68,
            6, 167, 213, 23, 25, 44, 86, 142, 224, 138, 132, 95, 115, 210, 151, 136, 207, 3, 92, 49, 69, 178, 26, 179, 68, 216, 6, 46, 169, 64, 0, 0,
            6, 167, 213, 23, 25, 44, 92, 81, 33, 140, 201, 76, 61, 74, 241, 127, 88, 218, 238, 8, 155, 161, 253, 68, 227, 219, 217, 138, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2,
            // system - create account with seed
            4,
            2,
                0, 1,
            124,
                3, 0, 0, 0,
                18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
                32, 0, 0, 0, 0, 0, 0, 0,
                    115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100,
                42, 0, 0, 0, 0, 0, 0, 0,
                80, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // system - initialize nonce
            4,
            3,
                1, 2, 3,
            36,
                6, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    process_message_body_and_sanity_check(message, sizeof(message), 7);
}

void test_process_message_body_create_vote_account() {
    uint8_t message[] = {
        2, 0, 4,
        6,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 25, 44, 92, 81, 33, 140, 201, 76, 61, 74, 241, 127, 88, 218, 238, 8, 155, 161, 253, 68, 227, 219, 217, 138, 0, 0, 0, 0,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2,
            // system - create account
            4,
            2,
                0, 1,
            52,
                0, 0, 0, 0,
                42, 0, 0, 0, 0, 0, 0, 0,
                147, 14, 0, 0, 0, 0, 0, 0,
                7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
            // vote - initialize
            5,
            3,
                1, 2, 3,
            101,
                0, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                50
    };

    process_message_body_and_sanity_check(message, sizeof(message), 8);
}

void test_process_message_body_create_vote_account_with_seed() {
    uint8_t message[] = {
        1, 0, 4,
        6,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            228, 28, 89, 247, 92, 128, 175, 120, 101, 30, 55, 24, 60, 143, 49, 55, 57, 67, 79, 63, 90, 198, 149, 232, 85, 165, 148, 141, 164, 223, 110, 211,
            6, 167, 213, 23, 25, 44, 92, 81, 33, 140, 201, 76, 61, 74, 241, 127, 88, 218, 238, 8, 155, 161, 253, 68, 227, 219, 217, 138, 0, 0, 0, 0,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2,
            // system - create account with seed
            4,
            2,
                0, 1,
            124,
                3, 0, 0, 0,
                18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
                32, 0, 0, 0, 0, 0, 0, 0,
                    115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100, 115, 101, 101, 100,
                42, 0, 0, 0, 0, 0, 0, 0,
                147, 14, 0, 0, 0, 0, 0, 0,
                7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
            // vote - initialize
            5,
            3,
                1, 2, 3,
            101,
                0, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                50
    };

    process_message_body_and_sanity_check(message, sizeof(message), 10);
}

void test_process_message_body_nonce_withdraw() {
    uint8_t message[] = {
        1, 1, 3,
        6,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            6, 167, 213, 23, 25, 44, 86, 142, 224, 138, 132, 95, 115, 210, 151, 136, 207, 3, 92, 49, 69, 178, 26, 179, 68, 216, 6, 46, 169, 64, 0, 0,
            6, 167, 213, 23, 25, 44, 92, 81, 33, 140, 201, 76, 61, 74, 241, 127, 88, 218, 238, 8, 155, 161, 253, 68, 227, 219, 217, 138, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // system - nonce withdraw
            5,
            5,
                1, 2, 3, 4, 0,
            12,
                5, 0, 0, 0,
                42, 0, 0, 0, 0, 0, 0, 0
    };

    process_message_body_and_sanity_check(message, sizeof(message), 5);
}

void test_process_message_body_stake_withdraw() {
    uint8_t message[] = {
        1, 1, 3,
        6,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            6, 167, 213, 23, 25, 53, 132, 208, 254, 237, 155, 179, 67, 29, 19, 32, 107, 229, 68, 40, 27, 87, 184, 86, 108, 197, 55, 95, 244, 0, 0, 0,
            6, 161, 216, 23, 145, 55, 84, 42, 152, 52, 55, 189, 254, 42, 122, 178, 85, 127, 83, 92, 138, 120, 114, 43, 104, 164, 157, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // stake - withdraw
            5,
            5,
                1, 2, 3, 4, 0,
            12,
                4, 0, 0, 0,
                42, 0, 0, 0, 0, 0, 0, 0
    };

    process_message_body_and_sanity_check(message, sizeof(message), 5);
}

void test_process_message_body_vote_withdraw() {
    uint8_t message[] = {
        1, 1, 1,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // vote - withdraw
            3,
            3,
                1, 2, 0,
            12,
                3, 0, 0, 0,
                42, 0, 0, 0, 0, 0, 0, 0
    };

    process_message_body_and_sanity_check(message, sizeof(message), 5);
}

void test_process_message_body_system_nonce_authorize() {
    uint8_t message[] = {
        1, 1, 1,
        3,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // system - authorize nonce
            2,
            2,
                1, 0,
            36,
                7, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    process_message_body_and_sanity_check(message, sizeof(message), 4);
}

void test_process_message_body_stake_authorize_staker() {
    uint8_t message[] = {
        1, 1, 2,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // stake - authorize
            3,
            3,
                1, 2, 0,
            40,
                1, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                0, 0, 0, 0 // staker
    };

    process_message_body_and_sanity_check(message, sizeof(message), 4);
}

void test_process_message_body_stake_authorize_withdrawer() {
    uint8_t message[] = {
        1, 1, 2,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            6, 161, 216, 23, 145, 55, 84, 42, 152, 52, 55, 189, 254, 42, 122, 178, 85, 127, 83, 92, 138, 120, 114, 43, 104, 164, 157, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // stake - authorize
            3,
            3,
                1, 2, 0,
            40,
                1, 0, 0, 0,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                1, 0, 0, 0 // withdrawer
    };

    process_message_body_and_sanity_check(message, sizeof(message), 4);
}

void test_process_message_body_stake_authorize_both() {
    uint8_t message[] = {
        1, 1, 2,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            6, 161, 216, 23, 145, 55, 84, 42, 152, 52, 55, 189, 254, 42, 122, 178, 85, 127, 83, 92, 138, 120, 114, 43, 104, 164, 157, 192, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2,
            // stake - authorize
            3,
            3,
                1, 2, 0,
            40,
                1, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                0, 0, 0, 0, // staker
            3,
            3,
                1, 2, 0,
            40,
                1, 0, 0, 0,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                1, 0, 0, 0 // withdrawer
    };

    process_message_body_and_sanity_check(message, sizeof(message), 5);
}

void test_process_message_body_vote_authorize_voter() {
    uint8_t message[] = {
        1, 1, 2,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // vote - authorize
            3,
            3,
                1, 2, 0,
            40,
                1, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                0, 0, 0, 0 // voter
    };

    process_message_body_and_sanity_check(message, sizeof(message), 4);
}

void test_process_message_body_vote_authorize_withdrawer() {
    uint8_t message[] = {
        1, 1, 2,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // vote - authorize
            3,
            3,
                1, 2, 0,
            40,
                1, 0, 0, 0,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                1, 0, 0, 0 // withdrawer
    };

    process_message_body_and_sanity_check(message, sizeof(message), 4);
}

void test_process_message_body_vote_authorize_both() {
    uint8_t message[] = {
        1, 1, 2,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2,
            // vote - authorize
            3,
            3,
                1, 2, 0,
            40,
                1, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                0, 0, 0, 0, // voter
            // vote - authorize
            3,
            3,
                1, 2, 0,
            40,
                1, 0, 0, 0,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                1, 0, 0, 0 // withdrawer
    };

    process_message_body_and_sanity_check(message, sizeof(message), 5);
}

void test_process_message_body_vote_update_node_v1_0_7() {
    uint8_t message[] = {
        1, 1, 2,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            6, 167, 213, 23, 24, 199, 116, 201, 40, 86, 99, 152, 105, 29, 94, 182, 139, 94, 184, 163, 155, 75, 109, 92, 115, 85, 91, 33, 0, 0, 0, 0,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // vote - update node
            3,
            3,
                1, 2, 0,
            36,
                4, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    process_message_body_and_sanity_check(message, sizeof(message), 4);

    Pubkey expected_node = {{
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    }};
    char bs58_expected[BASE58_PUBKEY_LENGTH];
    encode_base58(&expected_node, sizeof(expected_node), bs58_expected, sizeof(bs58_expected));
    char expected[SUMMARY_LENGTH + 2 + SUMMARY_LENGTH + 1];
    print_summary(bs58_expected, expected, sizeof(expected), SUMMARY_LENGTH, SUMMARY_LENGTH);

    transaction_summary_display_item(1); // node id
    assert_string_equal(G_transaction_summary_text, expected);
}

void test_process_message_body_vote_update_node_v1_0_8() {
    uint8_t message[] = {
        1, 1, 2,
        4,
            18, 67, 85, 168, 124, 173, 88, 142, 77, 171, 80, 178, 8, 218, 230, 68, 85, 231, 39, 54, 184, 42, 162, 85, 172, 139, 54, 173, 194, 7, 64, 250,
            112, 173, 25, 161, 89, 143, 220, 223, 128, 33, 149, 41, 12, 152, 202, 202, 203, 163, 182, 246, 158, 15, 22, 77, 171, 71, 63, 249, 10, 117, 172, 52,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            7, 97, 72, 29, 53, 116, 116, 187, 124, 77, 118, 36, 235, 211, 189, 179, 216, 53, 94, 115, 209, 16, 67, 252, 13, 163, 83, 128, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1,
            // vote - update node
            3,
            3,
                1, 2, 0,
            4,
                4, 0, 0, 0
    };

    process_message_body_and_sanity_check(message, sizeof(message), 4);

    Pubkey expected_node = {{
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    }};
    char bs58_expected[BASE58_PUBKEY_LENGTH];
    encode_base58(&expected_node, sizeof(expected_node), bs58_expected, sizeof(bs58_expected));
    char expected[SUMMARY_LENGTH + 2 + SUMMARY_LENGTH + 1];
    print_summary(bs58_expected, expected, sizeof(expected), SUMMARY_LENGTH, SUMMARY_LENGTH);

    transaction_summary_display_item(1); // node id
    assert_string_equal(G_transaction_summary_text, expected);
}

int main() {
    test_process_message_body_ok();
    test_process_message_body_too_few_ix_fail();
    test_process_message_body_too_many_ix_fail();
    test_process_message_body_data_too_short_fail();
    test_process_message_body_data_too_long_fail();
    test_process_message_body_bad_ix_account_index_fail();
    test_process_message_body_unknown_ix_enum_fail();
    test_process_message_body_ix_with_unknown_program_id_fail();
    test_process_message_body_xfer_w_nonce_ok();
    test_process_message_body_nonced_stake_create_with_seed();
    test_process_message_body_create_stake_account();
    test_process_message_body_create_nonce_account_with_seed();
    test_process_message_body_create_nonce_account();
    test_process_message_body_create_vote_account_with_seed();
    test_process_message_body_create_vote_account();
    test_process_message_body_nonce_withdraw();
    test_process_message_body_stake_withdraw();
    test_process_message_body_vote_withdraw();
    test_process_message_body_system_nonce_authorize();
    test_process_message_body_stake_authorize_staker();
    test_process_message_body_stake_authorize_withdrawer();
    test_process_message_body_stake_authorize_both();
    test_process_message_body_vote_authorize_voter();
    test_process_message_body_vote_authorize_withdrawer();
    test_process_message_body_vote_authorize_both();
    test_process_message_body_vote_update_node_v1_0_7();
    test_process_message_body_vote_update_node_v1_0_8();

    printf("passed\n");
    return 0;
}
