#include "lwip/apps/fs.h"
#include "lwip/def.h"


#define file_NULL (struct fsdata_file *) NULL


#ifndef FS_FILE_FLAGS_HEADER_INCLUDED
#define FS_FILE_FLAGS_HEADER_INCLUDED 1
#endif
#ifndef FS_FILE_FLAGS_HEADER_PERSISTENT
#define FS_FILE_FLAGS_HEADER_PERSISTENT 0
#endif
/* FSDATA_FILE_ALIGNMENT: 0=off, 1=by variable, 2=by include */
#ifndef FSDATA_FILE_ALIGNMENT
#define FSDATA_FILE_ALIGNMENT 0
#endif
#ifndef FSDATA_ALIGN_PRE
#define FSDATA_ALIGN_PRE
#endif
#ifndef FSDATA_ALIGN_POST
#define FSDATA_ALIGN_POST
#endif
#if FSDATA_FILE_ALIGNMENT==2
#include "fsdata_alignment.h"
#endif
#if FSDATA_FILE_ALIGNMENT==1
static const unsigned int dummy_align__done_html = 0;
#endif
static const unsigned char FSDATA_ALIGN_PRE data__done_html[] FSDATA_ALIGN_POST = {
/* /done.html (11z chars) */
0x2f,0x64,0x6f,0x6e,0x65,0x2e,0x68,0x74,0x6d,0x6c,0x00,0x00,

/* HTTP header */
/* "HTTP/1.0 200 OK
" (17z bytes) */
0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x30,0x20,0x32,0x30,0x30,0x20,0x4f,0x4b,0x0d,
0x0a,
/* "Server: lwIP/2.2.0d (http://savannah.nongnu.org/projects/lwip)
" (64z bytes) */
0x53,0x65,0x72,0x76,0x65,0x72,0x3a,0x20,0x6c,0x77,0x49,0x50,0x2f,0x32,0x2e,0x32,
0x2e,0x30,0x64,0x20,0x28,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x73,0x61,0x76,0x61,
0x6e,0x6e,0x61,0x68,0x2e,0x6e,0x6f,0x6e,0x67,0x6e,0x75,0x2e,0x6f,0x72,0x67,0x2f,
0x70,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x2f,0x6c,0x77,0x69,0x70,0x29,0x0d,0x0a,

/* "Content-Length: 229
" (18z+ bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4c,0x65,0x6e,0x67,0x74,0x68,0x3a,0x20,
0x32,0x32,0x39,0x0d,0x0a,
/* "Content-Encoding: deflate
" (27 bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x45,0x6e,0x63,0x6f,0x64,0x69,0x6e,0x67,
0x3a,0x20,0x64,0x65,0x66,0x6c,0x61,0x74,0x65,0x0d,0x0a,
/* "Content-Type: text/html

" (27z bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x54,0x79,0x70,0x65,0x3a,0x20,0x74,0x65,
0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x0d,0x0a,0x0d,0x0a,
/* raw file data (229 bytes) */
0x55,0x90,0x4d,0x4b,0xc4,0x30,0x10,0x86,0xef,0xfd,0x15,0xb3,0xb9,0xd7,0x5e,0x05,
0xdb,0x82,0xec,0x2a,0x08,0x8a,0x8b,0x6c,0x0f,0x1e,0xd3,0x74,0x6c,0x02,0xf9,0xb2,
0x3b,0x59,0xec,0xbf,0x37,0x69,0xa2,0xab,0xb9,0x84,0x77,0x3e,0xde,0x79,0x66,0xda,
0xdd,0xe1,0x75,0x7f,0x7a,0x3f,0x3e,0x80,0x24,0xa3,0xfb,0xaa,0xdd,0x3e,0x48,0xaf,
0x02,0x68,0x25,0xf2,0x29,0xab,0x4d,0xc7,0x88,0x41,0xe2,0xb1,0x96,0x7c,0x8d,0x9f,
0x41,0x5d,0x3a,0x26,0x9c,0x25,0xb4,0x54,0xd3,0xea,0x91,0x41,0x51,0x1d,0x23,0xfc,
0xa2,0x26,0x99,0xdd,0x81,0x90,0x7c,0x39,0x23,0x75,0xc3,0xe9,0xb1,0xbe,0x65,0xff,
0xed,0x48,0x91,0xc6,0xfe,0x59,0xcd,0x92,0xe6,0xb0,0xc2,0xd3,0xfd,0xf1,0x9a,0x6b,
0x72,0xf2,0x17,0xa6,0xb9,0xd2,0x24,0x39,0xba,0x69,0xed,0x07,0xaf,0x1d,0x9f,0xe0,
0xe0,0x2c,0xee,0xfe,0xfa,0x8e,0xcb,0xcf,0x9c,0x12,0xf8,0x70,0x8b,0x01,0x2e,0x48,
0x39,0x0b,0x1d,0x6b,0x16,0x8c,0x44,0x46,0x84,0x1b,0x31,0x2b,0x06,0x68,0x45,0xc2,
0x2f,0xd4,0x5e,0x73,0x65,0x19,0xc4,0x4d,0xa5,0x9b,0x3a,0x36,0x23,0x65,0xe8,0x2a,
0xfb,0xb5,0xca,0xfa,0x40,0x90,0x1b,0xce,0x61,0x34,0x8a,0x18,0x5c,0xb8,0x0e,0x51,
0xbe,0x25,0x5b,0x78,0xd9,0x0f,0xac,0x2f,0x3b,0xa4,0xb9,0x7d,0xe1,0xdf,0x88,0xab,
0xb6,0xc9,0xa7,0xfe,0x06,};

#if FSDATA_FILE_ALIGNMENT==1
static const unsigned int dummy_align__index_html = 1;
#endif
static const unsigned char FSDATA_ALIGN_PRE data__index_html[] FSDATA_ALIGN_POST = {
/* /index.html (12z chars) */
0x2f,0x69,0x6e,0x64,0x65,0x78,0x2e,0x68,0x74,0x6d,0x6c,0x00,

/* HTTP header */
/* "HTTP/1.0 200 OK
" (17z bytes) */
0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x30,0x20,0x32,0x30,0x30,0x20,0x4f,0x4b,0x0d,
0x0a,
/* "Server: lwIP/2.2.0d (http://savannah.nongnu.org/projects/lwip)
" (64z bytes) */
0x53,0x65,0x72,0x76,0x65,0x72,0x3a,0x20,0x6c,0x77,0x49,0x50,0x2f,0x32,0x2e,0x32,
0x2e,0x30,0x64,0x20,0x28,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x73,0x61,0x76,0x61,
0x6e,0x6e,0x61,0x68,0x2e,0x6e,0x6f,0x6e,0x67,0x6e,0x75,0x2e,0x6f,0x72,0x67,0x2f,
0x70,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x2f,0x6c,0x77,0x69,0x70,0x29,0x0d,0x0a,

/* "Content-Length: 258
" (18z+ bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4c,0x65,0x6e,0x67,0x74,0x68,0x3a,0x20,
0x32,0x35,0x38,0x0d,0x0a,
/* "Content-Encoding: deflate
" (27 bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x45,0x6e,0x63,0x6f,0x64,0x69,0x6e,0x67,
0x3a,0x20,0x64,0x65,0x66,0x6c,0x61,0x74,0x65,0x0d,0x0a,
/* "Content-Type: text/html

" (27z bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x54,0x79,0x70,0x65,0x3a,0x20,0x74,0x65,
0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x0d,0x0a,0x0d,0x0a,
/* raw file data (258 bytes) */
0x6d,0x91,0x4d,0x4b,0xc4,0x30,0x10,0x86,0xef,0xfe,0x8a,0x31,0xf7,0x12,0x0f,0x1e,
0x44,0xdb,0x05,0xf1,0x03,0x04,0x61,0x0b,0x76,0x0f,0x1e,0xa7,0xed,0x74,0x1b,0x48,
0x93,0xd8,0x4e,0x8a,0xf5,0xd7,0x9b,0x74,0x16,0x4f,0x9e,0xf2,0xbe,0xf3,0xf5,0x64,
0x92,0xf2,0xfa,0xf9,0xf8,0xd4,0x7c,0xd6,0x2f,0x30,0xf2,0x64,0x0f,0x57,0xa5,0x1c,
0x00,0xe5,0x48,0xd8,0xef,0x62,0x22,0xc6,0x94,0xe5,0x50,0xd0,0x57,0x34,0x6b,0xa5,
0x3a,0xef,0x98,0x1c,0x17,0xbc,0x05,0x52,0x70,0x71,0x95,0x62,0xfa,0x66,0x9d,0xdb,
0x1f,0xa0,0x1b,0x71,0x5e,0x88,0xab,0x53,0xf3,0x5a,0xdc,0xa9,0x7d,0x0a,0x1b,0xb6,
0x74,0x78,0x37,0xe7,0x91,0xcf,0x71,0x83,0xb7,0xc7,0xba,0xd4,0x12,0xcb,0x59,0xfd,
0x47,0x6b,0x7d,0xbf,0x65,0x91,0xe4,0xe0,0xe7,0x09,0xb0,0x63,0xe3,0x1d,0x54,0x4a,
0xc7,0x60,0x3d,0xf6,0x0a,0xc8,0x75,0x99,0x5c,0xa9,0x29,0x5a,0x36,0x01,0x67,0xd6,
0xb9,0xb2,0xe8,0x91,0x51,0x41,0xba,0xed,0xe8,0xfb,0x4a,0xd5,0xc7,0x8f,0x46,0xc8,
0xe1,0x50,0x5b,0xc2,0x85,0x60,0x09,0xd4,0x99,0x61,0x03,0x84,0xd6,0x38,0x9c,0x37,
0x18,0x8c,0x25,0x60,0x0f,0x32,0xf9,0x5e,0xa8,0xed,0x7c,0xc1,0x1b,0x17,0x22,0x83,
0xa0,0x72,0xa5,0x02,0x87,0x53,0xd2,0x99,0x23,0x7e,0x31,0x3f,0xc9,0xdf,0xde,0x08,
0x47,0x87,0xfd,0xe8,0xcd,0xfa,0xcf,0x80,0x25,0xb6,0x93,0x61,0x05,0x2b,0xda,0x98,
0xec,0x49,0x76,0x91,0xbe,0xbd,0xa3,0xdc,0x97,0x90,0x80,0xbc,0x41,0xa9,0xe5,0x2f,
0x7e,0x01,};

#if FSDATA_FILE_ALIGNMENT==1
static const unsigned int dummy_align__reset_html = 2;
#endif
static const unsigned char FSDATA_ALIGN_PRE data__reset_html[] FSDATA_ALIGN_POST = {
/* /reset.html (12z chars) */
0x2f,0x72,0x65,0x73,0x65,0x74,0x2e,0x68,0x74,0x6d,0x6c,0x00,

/* HTTP header */
/* "HTTP/1.0 200 OK
" (17z bytes) */
0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x30,0x20,0x32,0x30,0x30,0x20,0x4f,0x4b,0x0d,
0x0a,
/* "Server: lwIP/2.2.0d (http://savannah.nongnu.org/projects/lwip)
" (64z bytes) */
0x53,0x65,0x72,0x76,0x65,0x72,0x3a,0x20,0x6c,0x77,0x49,0x50,0x2f,0x32,0x2e,0x32,
0x2e,0x30,0x64,0x20,0x28,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x73,0x61,0x76,0x61,
0x6e,0x6e,0x61,0x68,0x2e,0x6e,0x6f,0x6e,0x67,0x6e,0x75,0x2e,0x6f,0x72,0x67,0x2f,
0x70,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x2f,0x6c,0x77,0x69,0x70,0x29,0x0d,0x0a,

/* "Content-Length: 147
" (18z+ bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4c,0x65,0x6e,0x67,0x74,0x68,0x3a,0x20,
0x31,0x34,0x37,0x0d,0x0a,
/* "Content-Encoding: deflate
" (27 bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x45,0x6e,0x63,0x6f,0x64,0x69,0x6e,0x67,
0x3a,0x20,0x64,0x65,0x66,0x6c,0x61,0x74,0x65,0x0d,0x0a,
/* "Content-Type: text/html

" (27z bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x54,0x79,0x70,0x65,0x3a,0x20,0x74,0x65,
0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x0d,0x0a,0x0d,0x0a,
/* raw file data (147 bytes) */
0x55,0x8f,0x4d,0x0b,0x82,0x40,0x10,0x86,0xef,0xfe,0x8a,0x71,0xef,0xb2,0xd7,0xa0,
0x55,0x88,0x2c,0x08,0x82,0x24,0xec,0xd0,0xd1,0x74,0x70,0x17,0x74,0xd7,0x6a,0x8c,
0xf6,0xdf,0xb7,0x1f,0x51,0x38,0x97,0xe1,0x79,0x19,0x1e,0xde,0x11,0x69,0x79,0xda,
0xd6,0xd7,0x6a,0x07,0x92,0xc6,0xa1,0x48,0x44,0x58,0xe0,0x27,0x01,0x10,0x12,0x9b,
0x2e,0x52,0x60,0x97,0x8c,0x48,0x8d,0xbb,0xa5,0x29,0xc3,0xfb,0xac,0x5e,0x39,0x6b,
0x8d,0x26,0xd4,0x94,0x91,0x9d,0x90,0xc1,0x97,0x72,0x46,0xf8,0x26,0xee,0x65,0x6b,
0x68,0x65,0xf3,0x78,0x22,0xe5,0x97,0x7a,0x9f,0xad,0xd8,0x52,0x47,0x8a,0x06,0x2c,
0x8e,0xaa,0x97,0xd4,0xcf,0x16,0x0e,0x9b,0x4a,0xf0,0x98,0xfd,0x3a,0xf0,0x7f,0x09,
0x8f,0x37,0xd3,0xd9,0xe2,0x8c,0x4e,0x08,0xa5,0xd1,0x98,0x0a,0x1e,0x92,0x44,0xf0,
0xf8,0xc1,0x07,};



const struct fsdata_file file__done_html[] = { {
file_NULL,
data__done_html,
data__done_html + 12,
sizeof(data__done_html) - 12,
FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT,
}};

const struct fsdata_file file__index_html[] = { {
file__done_html,
data__index_html,
data__index_html + 12,
sizeof(data__index_html) - 12,
FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT,
}};

const struct fsdata_file file__reset_html[] = { {
file__index_html,
data__reset_html,
data__reset_html + 12,
sizeof(data__reset_html) - 12,
FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT,
}};

#define FS_ROOT file__reset_html
#define FS_NUMFILES 3
