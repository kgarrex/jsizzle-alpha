
#ifndef JSZLPRIV_H
#define JSZLPRIV_H


/*
** The size of the atom table array stored in the json object
** The atom table is an array of pointers to atom structures
** This value should be a prime number to reduce collisions and its best to
** use a prime number that is closest to (but less than) a power of 2
*/
#ifndef ATOM_TABLE_SIZE
#define ATOM_TABLE_SIZE     127 
#endif


/* 
** The max number of namespaces (arrays/objects) that can be nested
** inside of a namespace. This is the size of a array in the json object
*/
#ifndef MAX_NAMESPACE_LEVEL
#define MAX_NAMESPACE_LEVEL 16 
#endif
#if MAX_NAMESPACE_LEVEL > 256
#error Error: Must define MAX_NAMESPACE_LEVEL between 0 - 256
#endif


/*
** The max length enforced for object keys. NOT IMPLEMENTED
*/
#ifndef MAX_KEY_LENGTH
#define MAX_KEY_LENGTH      32
#endif


/*
** The max length enforced for string values. NOT IMPLEMENTED
*/
#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH   4096 
#endif


#ifndef MAX_VALUE_LIMIT
#define MAX_VALUE_LIMIT     65536
#endif


/*
** max number of errors to accumulate in a buffer. NOT IMPLEMENTED
*/
#ifndef ERROR_STACK_SIZE
#define ERROR_STACK_SIZE    16
#endif



/*
** The maximum number of documents that can be parsed and stored
** per thread context. This value sets a static field
*/
#ifndef MAX_CONCURRENT_DOCUMENTS_PER_THREAD
#define MAX_CONCURRENT_DOCUMENTS_PER_THREAD 2
#endif
#if MAX_CONCURRENT_DOCUMENTS_PER_THREAD > 16
#error *** Max concurrent documents cannot exceed 16 per thread ***
#endif


#ifndef HASH_VALUE_BIT_SIZE
#define HASH_VALUE_BIT_SIZE 32
#endif


#define MAX_ITEM_COUNT 4096 


#define tolower(c) ((char)(c > 96 && c < 123 ? c - 32 : c))
#define toupper(c) ((char)(c > 64 && c < 91 ? c + 32 : c))

#define isipv6digit(d) (isdigit(d) || (d >= 'a' && d <= 'f'))


static const union {unsigned char b[4]; uint32_t v;} __host_order__ = {{0,1,2,3}};
 

#endif