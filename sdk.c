#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sdk.h"

void static release_context(SCARDCONTEXT ctx)
{
    SCardReleaseContext(ctx);
}

void static disconnect(SCARDCONTEXT ctx)
{
    SCardDisconnect(ctx, SC_DISPOSITION_LEAVE_CARD);
}

void SC_LOAD_AUTH(char LOCATION, char *KEY, char *buffer)
{
    memcpy(buffer, "\xFF" "\x82" "\x00", 3);
    buffer[3] = LOCATION;
    buffer[4] = 0x06;
    memcpy(&buffer[5], KEY, 6);
}

void SC_DO_AUTH(char BLOCK, char KEY_TYPE, char LOCATION, char *buffer)
{
    memcpy(buffer, "\xFF" "\x88" "\x00", 3);
    buffer[3] = BLOCK;
    buffer[4] = KEY_TYPE;
    buffer[5] = LOCATION;
}

void SC_READ_BLOCK(char BLOCK, char BYTES, char *buffer)
{
    memcpy(buffer, "\xFF" "\xB0" "\x00", 3);
    buffer[3] = BLOCK;
    buffer[4] = BYTES;
}

long sc_exec_command(SCARDHANDLE handle, int control_code,
                     char *send_buffer, int send_length,
                     char *recv_buffer, int recv_length,
                     size_t *bytes_read)
{
    printf("SC: sending: ");
    for (int i = 0; i < send_length; i++) {
        printf("%02x ", (int) send_buffer[i] & 0xFF);
    }
    puts("");

    long r = SCardControl(handle, control_code, send_buffer, send_length, recv_buffer, recv_length, bytes_read);

    printf("SC: received: ");
    for (int i = 0; i < recv_length; i++) {
        printf("%02x ", (int) recv_buffer[i] &0xFF);
    }
    puts("");

    printf("SC: r = %lx\n", r);

    return (long) ((recv_buffer[0] << 8) | recv_buffer[1]);
}

int sc_read_card(sc_block_key_t key_info, sc_block_t *output_block)
{
    memset(output_block->data, 0, SC_BLOCK_SIZE);

    SCARDCONTEXT ctx;
    LPCVOID reserved1, reserved2;
    DWORD scope = 2;
    int r = SCardEstablishContext(scope, reserved1, reserved2, &ctx);

    printf("SC: context error: %s %ld\n", pcsc_stringify_error(r & 0xFFFFFFFF), ctx);

    if (r == 0) {
        DWORD bufLen = 0;
        r = SCardListReaders(ctx, NULL, NULL, &bufLen);
        if (r !=0) {
            // Error
            release_context(ctx);
            printf("SC: reader access error: %s\n", pcsc_stringify_error(r & 0xFFFFFFFF));
            return 0;
        }
        printf("SC: found %ld readers\n", bufLen);
        char *readerlist = malloc(sizeof(char)*bufLen);
        r = SCardListReaders(ctx, NULL, readerlist, &bufLen);
        printf("SC: got %s\n", readerlist);

        // Take only the first reader, it should be 0-terminated so the string
        // is longer than necessary if you have more than one reader. Tough life.
        DWORD proto;
        SCARDHANDLE handle;
        r = SCardConnect(ctx,
                         readerlist,
                         SC_SHARE_SHARED,
                         SC_PROTOCOL_T0 | SC_PROTOCOL_T1,
                         &handle,
                         &proto);

        int status = 0;
        if (r != 0) {
            // Error
            release_context(ctx);
            printf("SC: card connection error: %s\n", pcsc_stringify_error(r & 0xFFFFFFFF));
            return 0;
        } else {
            // Read the block
            char ret_buffer[SC_BLOCK_SIZE + 2];
            memset(ret_buffer, 0, sizeof(ret_buffer));

            size_t sc_ret;
            int res;
            printf("SC: load auth\n");
            char load_auth[SC_LOAD_AUTH_BUFFER];
            memset(load_auth, 0, sizeof(load_auth));
            SC_LOAD_AUTH(key_info.location, key_info.key, load_auth);
            if (SC_FAIL == (res = sc_exec_command(handle, CONTROL_CODE,
                                                  load_auth, sizeof(load_auth),
                                                  ret_buffer, sizeof(ret_buffer),
                                                  &sc_ret))) {
                goto error; // Yummy goto
            }
            printf("SC: operation was a success\n");
            memset(ret_buffer, 0, sizeof(ret_buffer));

            printf("SC: do auth\n");
            char do_auth[SC_DO_AUTH_BUFFER];
            memset(do_auth, 0, sizeof(do_auth));
            SC_DO_AUTH(key_info.block_number, key_info.key_type, key_info.location, do_auth);
            if (SC_FAIL == (res = sc_exec_command(handle, CONTROL_CODE,
                                                  do_auth, sizeof(do_auth),
                                                  ret_buffer, sizeof(ret_buffer),
                                                  &sc_ret))) {
                goto error; // Shamless goto
            }

            printf("SC: operation was a success\n");
            memset(ret_buffer, 0, sizeof(ret_buffer));

            printf("SC: do read\n");
            char do_read[SC_READ_BLOCK_BUFFER];
            memset(do_read, 0, sizeof(do_read));
            SC_READ_BLOCK(key_info.block_number, SC_BLOCK_SIZE, do_read);
            if (SC_FAIL == (res = sc_exec_command(handle, CONTROL_CODE,
                                                  do_read, sizeof(do_read),
                                                  ret_buffer, sizeof(ret_buffer),
                                                  &sc_ret))) {
                goto error; // Shamless goto
            }

            printf("SC: operation was a success\n");

            status = 1;
            memcpy(output_block->data, ret_buffer, SC_BLOCK_SIZE);
            goto cleanup; // This goto is rancid
error:
            printf("SC: invalid operation occurred\n");
        }

cleanup:
        disconnect(ctx);
        return status;
    } else {
        // Error
        release_context(ctx);
        return 0;
    }
}

