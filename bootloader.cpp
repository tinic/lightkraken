#include <string.h>
#include <memory.h>

extern "C" {
#include "gd32f10x.h"
#include "lwip/apps/httpd.h"
}

#include "./main.h"
#include "./systick.h"
#include "./bootloader.h"
#include "./multipartparser.h"

constexpr static size_t page_num = (FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR) / FMC_PAGE_SIZE;
constexpr static size_t word_num = ((FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR) >> 2);
constexpr static size_t bytes_num = ((FMC_WRITE_END_ADDR - FMC_WRITE_START_ADDR));

static uint8_t buffer[8192];

static uintptr_t bufferptr = 0;
static size_t word_index = 0;

static void write_flash() {
	if (word_index >= word_num) {
		return;
	}
	DEBUG_PRINTF(("write_flash %d bytes\n", bufferptr));
	size_t to_write = bufferptr;
	size_t off_write = 0;
	while(to_write >= sizeof(uint32_t)) {
        fmc_word_program(word_index, (buffer[off_write+0] << 24) |
        					   	     (buffer[off_write+1] << 16) |
        						     (buffer[off_write+2] <<  8) |
        						     (buffer[off_write+3] <<  0) );
		to_write -= sizeof(uint32_t);
		off_write += sizeof(uint32_t);
		word_index += sizeof(uint32_t);
	}
	memcpy(&buffer[0], &buffer[off_write], to_write);
	bufferptr = to_write;
}

static void write_flash_finish() {
	write_flash();
	if (word_index >= word_num) {
		return;
	}
	DEBUG_PRINTF(("write_flash_finish %d bytes\n", bufferptr));
	fmc_word_program(word_index, (buffer[0] << 24) |
						  	     (buffer[1] << 16) |
							     (buffer[2] <<  8) |
							     (buffer[3] <<  0) );
	bufferptr = 0;
	word_index += sizeof(uint32_t);
}

enum base64_decodestep {
	step_a, step_b, step_c, step_d
};

struct base64_decodestate {
	base64_decodestep step;
	char plainchar;
} ;

static int base64_decode_value(signed char value_in) {
	static const signed char decoding[] = {
		62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,
		-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
		24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,
		39,40,41,42,43,44,45,46,47,48,49,50,51};
	static const char decoding_size = sizeof(decoding);
	value_in -= 43;
	if (value_in < 0 || value_in >= decoding_size) return -1;
	return decoding[(int)value_in];
}

static void base64_init_decodestate(base64_decodestate* state_in) {
	state_in->step = step_a;
	state_in->plainchar = 0;
}

static int base64_decode_block(const char* code_in, const int length_in, 
                               char *plaintext_out, base64_decodestate *state_in)
{ 
	const char* codechar = code_in;
	char* plainchar = plaintext_out;
	signed char fragment;
	
	*plainchar = state_in->plainchar;
	
	while (1) {
	switch (state_in->step) {
		case step_a:
			do {
				if (codechar == code_in+length_in) {
					state_in->step = step_a;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar    = (fragment & 0x03f) << 2;
			[[fallthrough]];
		case step_b:
			do {
				if (codechar == code_in+length_in) {
					state_in->step = step_b;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++ |= (fragment & 0x030) >> 4;
			*plainchar    = (fragment & 0x00f) << 4;
			[[fallthrough]];
		case step_c:
			do {
				if (codechar == code_in+length_in) {
					state_in->step = step_c;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++ |= (fragment & 0x03c) >> 2;
			*plainchar    = (fragment & 0x003) << 6;
			[[fallthrough]];
		case step_d:
			do {
				if (codechar == code_in+length_in) {
					state_in->step = step_d;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++   |= (fragment & 0x03f);
			[[fallthrough]];
	}
	}
	/* control should not reach here */
	return plainchar - plaintext_out;
}

static base64_decodestate decodestate;

static int on_body_begin(multipartparser *) {
    DEBUG_PRINTF(("on_body_begin\n"));
    return 0;
}

static int on_part_begin(multipartparser *) {
    DEBUG_PRINTF(("on_part_begin\n"));
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
    
    fmc_lock();

	base64_init_decodestate(&decodestate);

    fmc_unlock();

	bufferptr = 0;
	word_index = 0;
    return 0;
}

static int on_header_field(multipartparser *, const char *data, size_t len) {
	(void) data;
	(void) len;
    DEBUG_PRINTF(("on_header_field %.*s\n", len, data));
    return 0;
}

static int on_header_value(multipartparser *, const char *data, size_t len) {
	(void) data;
	(void) len;
    DEBUG_PRINTF(("on_header_value %.*s\n", len, data));
    return 0;
}

static int on_headers_complete(multipartparser *) {
    DEBUG_PRINTF(("on_headers_complete\n"));
    return 0;
}

static int on_data(multipartparser *, const char *data, size_t len) {
	(void) data;
	(void) len;
    DEBUG_PRINTF(("on_header_value %.*s\n", len, data));
    bufferptr += base64_decode_block(data, len, (char *)&buffer[bufferptr], &decodestate);
    write_flash();
    return 0;
}

static int on_part_end(multipartparser *) {
    DEBUG_PRINTF(("on_part_end\n"));
    write_flash_finish();
    fmc_lock();
    return 0;
}

static int on_body_end(multipartparser *) {
    DEBUG_PRINTF(("on_body_end\n"));
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
		multipartparser_init(&parser, "what?");
    	return ERR_OK;
    }

  	return ERR_ARG;
}

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
	(void)connection;
	if (uploading) {
		multipartparser_execute(&parser, &callbacks, (const char *)p->payload, p->len);
		write_flash();
	}
	pbuf_free(p);
	return ERR_OK;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    (void)connection;
    (void)response_uri_len;
	if (uploading) {
		write_flash_finish();
		strcpy(response_uri, "/done.html");
	}
}

