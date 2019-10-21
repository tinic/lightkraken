#include <string.h>
#include <memory.h>

extern "C" {
#include "gd32f10x.h"
#include "lwip/apps/httpd.h"
}

#include "./bootloader.h"
#include "./multipartparser.h"

//static uintptr_t bufferptr = 0;
//static uint8_t buffer[32768];

static void write_flash() {
}

static void write_flash_finish() {
}

static int on_body_begin(multipartparser *) {
    printf("on_body_begin\n");
    return 0;
}

static int on_part_begin(multipartparser *) {
    printf("on_part_begin\n");
    return 0;
}

static int on_header_field(multipartparser *, const char *data, size_t len) {
    printf("on_header_field %.*s\n", len, data);
    return 0;
}

static int on_header_value(multipartparser *, const char *data, size_t len) {
    printf("on_header_value %.*s\n", len, data);
    return 0;
}

static int on_headers_complete(multipartparser *) {
    printf("on_headers_complete\n");
    return 0;
}

static int on_data(multipartparser *, const char *data, size_t len) {
    printf("on_header_value %.*s\n", len, data);
//	memcpy(&buffer[bufferptr], data, len);
//	bufferptr += len;
    return 0;
}

static int on_part_end(multipartparser *) {
    printf("on_part_end\n");
    return 0;
}

static int on_body_end(multipartparser *) {
    printf("on_body_end\n");
    return 0;
}

static multipartparser_callbacks callbacks;
static multipartparser parser;

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
    
    printf("httpd_post_begin uri: %s\n", uri);
    
    if (strcmp("/upload", uri) != 0) {
    	return ERR_ARG;
    }

	multipartparser_callbacks_init(&callbacks);
    callbacks.on_body_begin = &on_body_begin;
    callbacks.on_part_begin = &on_part_begin;
    callbacks.on_header_field = &on_header_field;
    callbacks.on_header_value = &on_header_value;
    callbacks.on_headers_complete = &on_headers_complete;
    callbacks.on_data = &on_data;
    callbacks.on_part_end = &on_part_end;
    callbacks.on_body_end = &on_body_end;
    multipartparser_init(&parser, "what?");
    
    return ERR_OK;
}

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
    (void)connection;
    err_t res = ERR_OK;
	multipartparser_execute(&parser, &callbacks, (const char *)p->payload, p->len);
	pbuf_free(p);
	write_flash();
    return res;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    (void)connection;
    (void)response_uri_len;
	write_flash_finish();
	strcpy(response_uri, "/done.html");
}

