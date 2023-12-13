#ifndef moca_assembler_common_h
#define moca_assembler_common_h
#include <iostream>
#include <stdlib.h> /* exit(), EXIT_FAILURE, EXIT_SUCCESS. */
#include <string.h>
#include <cstring>
#include <memory>
#include <vector>
#include <utility>

#ifdef __cplusplus
extern "C"
{
#endif

typedef char						int8;
typedef const char					c_int8;
typedef char*						p_int8;
typedef const char*					cp_int8;
typedef signed char					sint8;
typedef const signed char			c_sint8;
typedef signed char*				p_sint8;
typedef const signed char*			cp_sint8;
typedef unsigned char				usint8;
typedef const unsigned char			c_usint8;
typedef unsigned char*				p_usint8;
typedef const unsigned char*		cp_usint8;
typedef signed short				sint16;
typedef const signed short			c_sint16;
typedef signed short*				p_sint16;
typedef const signed short*			cp_sint16;
typedef unsigned short				usint16;
typedef const unsigned short		c_usint16;
typedef unsigned short*				p_usint16;
typedef const unsigned short*		cp_usint16;
typedef signed int					sint32;
typedef const signed int			c_sint32;
typedef signed int*					p_sint32;
typedef const signed int*			cp_sint32;
typedef unsigned int				usint32;
typedef const unsigned int			c_usint32;
typedef unsigned int*				p_usint32;
typedef const unsigned int*			cp_usint32;
typedef signed long					slng;
typedef const signed long			c_slng;
typedef signed long*				p_slng;
typedef const signed long*			cp_slng;
typedef unsigned long				uslng;
typedef const unsigned long			c_uslng;
typedef unsigned long*				p_uslng;
typedef const unsigned long*		cp_uslng;
typedef signed long long			slng_lng;
typedef const signed long long		c_slng_lng;
typedef signed long long*			p_slng_lng;
typedef const signed long long*		cp_slng_lng;
typedef unsigned long long			uslng_lng;
typedef const unsigned long long	c_uslng_lng;
typedef unsigned long long*			p_uslng_lng;
typedef const unsigned long long*	cp_uslng_lng;

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus

#ifdef true
#undef true
#endif

#ifdef false
#undef false
#endif

#define true	1
#define false	0

#endif

/* Max integer sizes. */
#define max_byte_size		0xFF
#define max_word_size		0xFFFF
#define max_dword_size		0xFFFFFFFF

/* Error codes for `moca_assembler_error`/`moca_assembler_assert`. */
#define NoSuchFile				0x0002
#define NoDataInFile			0x0003
#define DataReadInError			0x0004
#define InvalidDigit			0x0005
#define InvalidToken			0x0006
#define InvalidNewline			0x0007
#define AllocationError			0x0008
#define InvalidLineForDirective 0x0009
#define OverflowError			0x000A
#define InvalidRegID			0x0010
#define InvalidDataTypeToken	0x0011
#define UnexpectedEOF			0x0012
#define UnexpectedNewline		0x0013
#define VariableNotFound		0x0014
#define UnknownError			0xFFFF

#define moca_assembler_error(exit_code, msg, ...)		\
{														\
	fprintf(stderr, msg, ##__VA_ARGS__);				\
	exit(exit_code);									\
}

#define moca_assembler_assert(cond, err_code, msg, ...)				\
{																	\
	if(!(cond)) moca_assembler_error(err_code, msg, ##__VA_ARGS__)	\
}

#define is_ascii(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z')) ? true : false
#define is_hex(x) ((x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F')) ? true : false
#define is_number(x) (x >= '0' && x <= '9') ? true : false

#endif
