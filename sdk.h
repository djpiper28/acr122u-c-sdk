#pragma once
#include <reader.h>
#include <winscard.h>

#define SC_BLOCK_SIZE 16

// ACR122U constants
#define CONTROL_CODE SCARD_CTL_CODE(3500)
#define SC_KEY_A 0x60
#define SC_KEY_B 0x61
#define SC_LOAD_AUTH_BUFFER 3 + 1 + 1 + 6
void SC_LOAD_AUTH(char LOCATION, char *KEY, char *buffer);

#define SC_DO_AUTH_BUFFER 3 + 1 + 1 +1
void SC_DO_AUTH(char BLOCK, char KEY_TYPE, char LOCATION, char *buffer);

#define SC_READ_BLOCK_BUFFER 3 + 1 + 1
void SC_READ_BLOCK(char BLOCK, char BYTES, char *buffer);

#define SC_FAIL 0x6300
#define SC_SUCCESS 0x9000

// PCSC constants
#define SC_SHARE_SHARED 2
#define SC_PROTOCOL_T0 1
#define SC_PROTOCOL_T1 2
#define SC_DISPOSITION_LEAVE_CARD 0

typedef struct {
    char *key;
    char location;
    char block_number;
    char key_type;
} sc_block_key_t;

typedef struct {
    char data[SC_BLOCK_SIZE];
} sc_block_t;

int sc_read_card(sc_block_key_t key_info, sc_block_t *output_block);

