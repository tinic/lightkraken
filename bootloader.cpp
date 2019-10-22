#include <string.h>
#include <memory.h>

extern "C" {
#include "gd32f10x.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
}

#include "./main.h"
#include "./systick.h"
#include "./bootloader.h"
#include "./multipartparser.h"

#define DEBUG_DO_ACTUALLY_FLASH 1

constexpr static size_t page_num = (FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR) / FMC_PAGE_SIZE;
constexpr static size_t word_num = ((FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR) >> 2);
constexpr static size_t bytes_num = ((FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR));

static uint8_t buffer[8192];

static uintptr_t bufferptr = 0;
static size_t word_index = 0;
static size_t bytes_in = 0;
static bool ok_to_assemble = false;

static void write_flash(const char *data, size_t len) {
    if (word_index >= word_num) {
        return;
    }
    size_t to_write = bufferptr + len;
    size_t off_write = bufferptr;
    memcpy(&buffer[bufferptr], data, len);
    bufferptr += len;
    while(to_write >= sizeof(uint32_t)) {
#if DEBUG_DO_ACTUALLY_FLASH
        fmc_word_program(word_index,(buffer[off_write + 3] << 24) |
                                    (buffer[off_write + 2] << 16) |
                                    (buffer[off_write + 1] <<  8) |
                                    (buffer[off_write + 0] <<  0) );
#endif  // #if DEBUG_DO_ACTUALLY_FLASH
        to_write -= sizeof(uint32_t);
        off_write += sizeof(uint32_t);
        word_index ++;
    }
    memcpy(&buffer[0], &buffer[off_write], to_write);
    bufferptr = to_write;
}

static int on_body_begin(multipartparser *) {
    return 0;
}

static int on_part_begin(multipartparser *) {
    return 0;
}

static int on_header_field(multipartparser *, const char *, size_t ) {
    return 0;
}

static int on_header_value(multipartparser *, const char *data, size_t len) {
    if (strncmp(data, "application/octet-stream", len) == 0) {

#if DEBUG_DO_ACTUALLY_FLASH
        fmc_unlock();

        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

        for(size_t c = 0; c < page_num; c++){
            fmc_page_erase(FMC_WRITE_START_ADDR + (FMC_PAGE_SIZE * c));
            fmc_flag_clear(FMC_FLAG_BANK0_END);
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        }
#endif  // #if DEBUG_DO_ACTUALLY_FLASH

        bufferptr = 0;
        word_index = 0;
        bytes_in = 0;
        ok_to_assemble = true;
    }
    return 0;
}

static int on_headers_complete(multipartparser *) {
    return 0;
}

static int on_data(multipartparser *, const char *data, size_t len) {
    
    if (ok_to_assemble) {
        write_flash(data, len);
    }
    return 0;
}

static int on_part_end(multipartparser *) {
    DEBUG_PRINTF(("Flashed %d bytes\n", word_index*4));
    bufferptr = 0;
    word_index = 0;
    bytes_in = 0;
    ok_to_assemble = false;
    
#if DEBUG_DO_ACTUALLY_FLASH
    fmc_lock();
#endif  // #if DEBUG_DO_ACTUALLY_FLASH
    
    return 0;
}

static int on_body_end(multipartparser *) {
    return 0;
}

static multipartparser_callbacks callbacks;
static multipartparser parser;
static bool uploading = false;

err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                    u16_t http_request_len, int content_len, char *response_uri,
                    u16_t response_uri_len, u8_t *post_auto_wnd) {
    (void)connection;
    (void)uri;
    (void)http_request;
    (void)http_request_len;
    (void)content_len;
    (void)response_uri;
    (void)response_uri_len;
    (void)post_auto_wnd;
    
    DEBUG_PRINTF(("httpd_post_begin uri: %s\n", uri));

    if (strcmp("/reset", uri) == 0) {
        strcpy(response_uri, "/reset.html");
        lightguy::Systick::instance().scheduleReset();
        return ERR_OK;
    }
    
    if (strcmp("/upload", uri) == 0) {
    
        char *boundaryStr = lwip_strnstr(http_request,"boundary", http_request_len);
        if (boundaryStr == 0) {
            return ERR_ARG;
        }

        char *boundaryEnd = lwip_strnstr(boundaryStr,"\r\n", http_request_len - (boundaryStr - http_request));
        if (boundaryEnd == 0) {
            return ERR_ARG;
        }

        boundaryStr += strlen("boundary");

        for (;;) {
            // stop right after quotes
            if (*boundaryStr == '"') {
                boundaryStr++;
                break;
            }
            // skip starting spaces
            if (*boundaryStr == ' ' ||
                *boundaryStr == '=') {
                boundaryStr++;
                continue;
            }
            break;
        }

        for (;;) {
            // stop right after quotes
            if (*boundaryEnd == '"') {
                boundaryEnd--;
                break;
            }
            // skip trailing spaces
            if (*boundaryEnd == ' ' ||
                *boundaryEnd == '=') {
                boundaryEnd--;
                continue;
            }
            break;
        }
        
        char boundary[71];
        memset(boundary, 0, sizeof(boundaryStr));
        strncpy(boundary, boundaryStr, boundaryEnd - boundaryStr);

        uploading = true;
        multipartparser_callbacks_init(&callbacks);
        callbacks.on_body_begin = &on_body_begin;
        callbacks.on_part_begin = &on_part_begin;
        callbacks.on_header_field = &on_header_field;
        callbacks.on_header_value = &on_header_value;
        callbacks.on_headers_complete = &on_headers_complete;
        callbacks.on_data = &on_data;
        callbacks.on_part_end = &on_part_end;
        callbacks.on_body_end = &on_body_end;
        multipartparser_init(&parser, boundary);
        return ERR_OK;
    }

    return ERR_ARG;
}

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
    (void)connection;
    if (uploading) {
        multipartparser_execute(&parser, &callbacks, (const char *)p->payload, p->len);
    }
    pbuf_free(p);
    return ERR_OK;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    (void)connection;
    (void)response_uri_len;
    if (uploading) {
        strcpy(response_uri, "/done.html");
    }
}

