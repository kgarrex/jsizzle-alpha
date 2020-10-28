
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>

#include "npx.h"
#include "jsizzle.h"


#if defined(DEBUG) || defined(_DEBUG)

  #define API_DEFINE(name, ...)\
  _##name(const char *_file, unsigned int _line, const char *_func, __VA_ARGS__)

  #define API_CALL(name, ...)\
  _##name(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)


  #define DEBUG_OUT()\
  printf("Error in file '%s' on line '%u' in function '%s'\n", _file, _line, _func)

#else
  #define API_DEFINE(name, ...) _##name(__VA_ARGS__)
  #define API_CALL(name, ...) _##name(__VA_ARGS__)
  #define DEBUG_OUT()
#endif



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
#ifndef JSZL_MAX_KEY_LENGTH
#define JSZL_MAX_KEY_LENGTH    31 
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

#define JSZL_STRICT_KEYS 0x01

static const union {unsigned char b[4]; uint32_t v;} __host_order__ = {{0,1,2,3}};

/*****
 * 
 */
enum ParsePhase {
  ParsePhase_None,
  ParsePhase_ArrayOptValue,
  ParsePhase_ArrayReqValue,
  ParsePhase_ArrayEndValue,
  ParsePhase_ObjectOptKey,
  ParsePhase_ObjectReqKey,
  ParsePhase_ObjectEndKey,
  ParsePhase_ObjectValue,
  ParsePhase_ObjectEndValue
};


typedef int (*jszl_string_handler)(struct jszlparser *);
typedef int (*jszl_key_handler)(struct jszlparser *);



/*****************************
 * parse_engine
 *
 * @param[in] parser
 * @param[in] ctx  t
 *
 *
 * @return  the error code
 *
 ****************************/

int jsizzle_parser_engine(
  struct jszlparser *parser,
  struct jszlcontext *ctx,
  const char *str,
  jszl_string_handler string_handler
);


/**************************//*
 * query_engine
 *
 * @param[in] pnode JSON used to search in
 * @param[in]
 * @param[out] ppnode returns a JSON node that was found. NULL if not found
 *
 * @return the error code
 *
 ************************/

int query_engine(
  struct jszlnode *pnode,
  const char *path,
  struct jszlnode **ppnode
);
 


#define UTF8_CHAR_LEN(byte) (((0xE5000000 >> ((byte >> 3) & 0x1E)) & 3) +1)
#define IS_UTF8_CONT(byte) ((byte >> 6) & 2)
#define CHAR_UTF8_LEN(dword)

#if defined(_WIN32) || defined (_WIN64)
#include <windows.h>
#include "os\windows.c"
#elif defined(_KERNEL_MODE)
#include "os\winkernel.c"
#elif defined(__linux__)
#include "os/linux.c"
#elif defined(unix) || defined(__unix) || defined(__unix__)
#include "os/unix.c"
#elif defined(__APPLE__) || defined(__MACH__)
#include "os/apple.c"
#elif defined(__FreeBSD__)
#include "os/freebsd.c"
#elif defined(__ANDROID__)
#include "os/android.c"
#endif



/*
** Value structure used to store value info
*/
struct jszlnode{
	short type;
	union{
		unsigned short count;
		unsigned short length;
	};
	struct jszlnode *next;
	struct atom *name;

	union{
		void *data;
		unsigned long value;
		struct jszlnode *child;
		char *string;
		int boolean;
		unsigned unum;
		signed snum;
		unsigned hash;
	};
};


/*
** Atom structure used for hashing strings
*/
struct atom {
	struct atom *next;
	const void *data;
	unsigned long hash;
	//potentially use this field to set flags. Ex: Unicode
	unsigned short refcount;
	unsigned length;
};

struct jszl_header {
	long atom_table;
	short atom_table_size;
};


/* Global Atom Table
**
*/
struct {
	struct atom * table[ATOM_TABLE_SIZE];

#if defined(_WIN32) || defined(_WIN64)
	CRITICAL_SECTION syncobj; 
 
#elif defined(_KERNEL_MODE)
	
#elif defined(__linux__) || defined(__GNUC__)
	pthread_mutex_t syncobj;

#endif

} global_atom_table;

struct atom * g_atomTable[ATOM_TABLE_SIZE];

/*
** TYPE DECLARATIONS
*/
struct jszlcontext {
	struct jszlvtable *vt;

//struct json_node *schema;
	struct jszlnode *RootNS;
	struct jszlnode *CurrentNS;

	char *buffer;
	unsigned int bufsize;

	struct jszlparser *parser;

	struct jszlnode	node_pool[512];
	unsigned int node_pool_size;

	struct atom	atom_pool[512];
	unsigned int atom_pool_size;

	const char *error_file;
	unsigned int error_line;

	void *databuf; // field where all data is copied
	unsigned long databufsize;

	int OpError;
	void *UserContext;
	unsigned int ThreadId;

	char encode;
	//unsigned int ProcessorNumber;
};

/*
** Parser state is a disposable piece of memory that should ideally only be created
** on the stack right before a parse for the highest memory and execution efficiency
*/
struct jszlparser {
	JSON_ERROR_HANDLER log_error;

	struct jszlnode *root_namespace;
	struct jszlnode *current_namespace;

	struct jszlnode *node_pool;
	unsigned int node_pool_size;
	unsigned int pool_idx;

	struct atom *atom_pool;
	unsigned int atom_pool_size;
	unsigned int atom_idx;

	int flags;

	int errcode;
	const char *errmsg;

	struct {
		struct jszlnode *namespace; 
	} ns_stack[MAX_NAMESPACE_LEVEL];
	char stack_idx;

	struct {
		int e;
		const char *msg;
		unsigned int line;
		unsigned int offset; 
	} err_stack[ERROR_STACK_SIZE];

	unsigned line; //current line 
	unsigned offset; //current line offset
	const char *loc; //current location
	struct atom *curkey;
	struct jszlnode * prevnode;
	char phase;

	unsigned int value_pool_idx;
	unsigned int atom_pool_idx;

	//struct node **node_pool;

	union {
		struct json_reader {int NOT_USED;} reader; 
		struct json_writer {int NOT_USED;} writer;
	};

	int (*key_handler)(struct jszlparser *);

};

const char *g_errmsg[MAX_JSZLERR] = {
	"The document root is missing",
};

#include "..\core.h"


#include "hash.c"
#include "shared.c"


/*
** GLOBAL VARIABLES
*/
static struct jszlvtable g_vtable = {0};
const unsigned char g_max_handle = 16;

int g_operror;

struct jszlcontext g_thread[32];
unsigned long global_seed = 5381;
unsigned long handleIndex;
// Atom Table
struct atom * g_AtomTable[ATOM_TABLE_SIZE];

#define GET_VALUE_TYPE(value) (0x07 & value.type)
#define GET_VALUE_SUBTYPE(value) (0x1F & (value.type >> 3)) 

// TODO add function processing to endpoint parsing
// TODO add wildcard processing to endpoint parsing
// TODO use the term 'serialize' and 'deserialize' when writing and reading json objects
// TODO change terminology path to endpoint (ex: char *path -> char *endpoint)
// TODO add JSON schema functionality
// TODO EVENTUALLY (sometime in distant future) add support for custom schemas
// FIXED TODO atom table access should go through a local pointer
//		 this is to allow for flexible atom table memory (global vs. private)
// TODO add reference counting to atoms to allow deletions

/* TODO
** Fix issue where error reporting innacurate offset when looking for comma
** This issue is caused because white space is being trimmed before
** the error is detected
*/


// define keywords in code

#define IS_ARRAY(ns) (GET_VALUE_TYPE(ns) == TYPE_ARRAY)
#define IS_OBJECT(ns) (GET_VALUE_TYPE(ns) == TYPE_OBJECT)


#if !defined(MAX_STRING_LENGTH) || MAX_STRING_LENGTH > 65536
#error Error: Must define MAX_STRING_LENGTH between 0 - 65536
#endif

#if !defined(JSZL_MAX_KEY_LENGTH)	|| JSZL_MAX_KEY_LENGTH > 256
#error Error: Must define JSZL_MAX_KEY_LENGTH between 0 - 256
#endif

//#define ESCAPE '?' '\'

/*
** This structure represents the life of the parsed JSON document
*/


union type_buffer{
		unsigned ipv4;
		union ipv6 ipv6;
};

typedef void (*key_handler_cb)(
	struct json_object *obj
 ,unsigned hash
 ,unsigned len
);

#include "atom.c"
#include "utf8.c"


#define ERRORLOG_PROC (parser->log_error)

#define ERRORMSG_INVALID_PRIMITIVE "Invalid primitive value" 


void * allocmem(int size)
{
	return 0;
}

void deallocmem(void *mem)
{}

//typedef unsigned int dword;
static inline void *
new_node(long value, long atom, short type, short count)
{
	struct jszlnode * pnode;
	pnode = malloc(sizeof(struct jszlnode));
	if(!pnode) return 0;

	pnode->value	= value;
	pnode->name	 = (void*)atom;
	pnode->type	 = type;
	pnode->count	= count;
	pnode->next	 = 0;
	return pnode;
}

static inline void
delete_node(void *node)
{}


/*
** Macro to create a new root namespace and set it as the 
** current namespace. Push the new namespace onto the namespace stack
*/


/*
** Macro to create a new namespace and initialize. The current
** namespace now references the previous namespace. The namespace
** is pushed onto the stack an execution reenters the loop
*/



//goes to cleanup after 1 error
#define LOG_ERROR(e, msg)\
do{\
if(ERRORLOG_PROC && !ERRORLOG_PROC(e, msg, parser->line, 0)) { goto error_cleanup; }\
parser->errcode = e; parser->errmsg = msg;\
goto error_cleanup;\
}while(0);


static long key_exists(struct jszlnode *value, struct atom *key)
{
	for(value = value->child; value; value = value->next)
		if(value->name == key) return 1;	 
	return 0;
}

struct value_data {
	int type;
	unsigned length;
	unsigned isNegative;
	unsigned numType;
	unsigned long hash;
};


/* Macro to advance the current location of the parser
** This moves the pointer forward by 'byte_count' bytes and increments 
** the offset by 'char_count' to keep track of where errors occur in a line 
*/
#define move_loc(p, byte_count, char_count) p.loc += byte_count; p.offset += char_count


/**************************************************//**
 * jszlparse_validate_value
 *
 * validation function for a JSON value
 *
 **************************************************/
static enum jszltype validate_value(struct jszlparser *parser, jszl_string_handler str_handler
){
	struct jszlnode *pnode;
	unsigned length = 0;
	unsigned type = 0;
	long value = 0;
	int n;
	int numType;
	int isNegative;
	int errcode;

	switch(*parser->loc){
		case '"':
			length = string_handler(parser);

			if(!length){
				parser->errcode = JSZLE_SYNTAX;
				goto error_cleanup;
			}
			type = TYPE_STRING;
			value = (long)parser->loc+1;
			move_loc((*parser), length, length);
			break;

		case '[':
			type = TYPE_ARRAY;
			break;

		case '{':
			type = TYPE_OBJECT;
			break;

		case 'f':
			if(memcmp(parser->loc, "false", 5)){
				printf("ERROR ON FALSE\n");
				parser->errcode = JSZLE_SYNTAX;
				goto error_cleanup;
			}
			type = TYPE_BOOLEAN;
			move_loc((*parser), 5, 5);
			break;

		case 't':
			if(memcmp(parser->loc+1, "true", 4)){
				parser->errcode = JSZLE_SYNTAX;
				goto error_cleanup;
			}
			type	= TYPE_BOOLEAN;
			value = 1;
			move_loc((*parser), 4, 4);
			break;

		case 'n':
			if(memcmp(parser->loc+1, "null", 4)){
				parser->errcode = JSZLE_SYNTAX;
				goto error_cleanup;
			}
			type = TYPE_NULL;
			move_loc((*parser), 4, 4);
			break;

		default:
		if(isdigit(*parser->loc) || *parser->loc == '-'){
			length = is_valid_number(parser->loc, &numType, &isNegative);
			if(length){
				if(numType == _NumberTypeHex){
					value = hextoint(parser->loc+2, length-2);
				}
				else if(numType == _NumberTypeDecimal){
					value = atouint(parser->loc, length);
				}
				type = TYPE_NUMBER;
				move_loc((*parser), length, length);
				break;
			}
		}
		parser->errcode = JSZLE_INVALID_VALUE;
		goto error_cleanup;
	}


	pnode = new_node(value, (long)parser->curkey, type, length);
	if(!pnode){
		parser->errcode = JszlE_NoMemory;
		goto error_cleanup; 
	}
	if(!parser->current_namespace->child){
		parser->current_namespace->child = pnode; 
		parser->current_namespace->count++;
	}
	else{
		parser->prevnode->next = pnode; 
		parser->current_namespace->count++;
	}
	parser->prevnode = pnode;
	return pnode->type;

error_cleanup:
	//log error: invalid value
	return 0;
}


/* leave_namespace()
** This macro is used to remove some code duplication in the main parser engine.
** It checks to see if we are leaving the root namespace, if not, we replace the
** previous node with the current object/array (to link the next value). Next we
** decrement the namespace stack and advance the location pointer.
*/
#define leave_namespace()\
if(parser->current_namespace == parser->root_namespace)\
		goto exit_root_namespace;\
parser->prevnode = parser->current_namespace;\
parser->current_namespace = parser->ns_stack[--parser->stack_idx].namespace;\
move_loc((*parser), 1, 1);\
if(GET_VALUE_TYPE((*parser->current_namespace)) == TYPE_ARRAY)\
		goto array_phase_comma;\
else\
		goto object_phase_comma;\


void parse_error(struct jszlparser *p, int err)
{
		
}



/************************************************//**
 * key_handler
 * @brief Handle a JSON object key
 *
 * @return the size of the key in bytes. On error, the key handler should
 * return 0 and should set the error code.
 *
 ************************************************/

static int key_handler(struct jszlparser *parser)
{
	int len;
	unsigned hash;
	struct atom *atom;

	if(*parser->loc != '"'){
		//set_error();
		return 0;
	}

	len = is_valid_key(parser->loc+1, global_seed, &hash);
	if(!len){
		parse_error(parser, JszlE_BadKey);
		return 0;
	}
	if(parser->loc[len+1] != '"'){
		parse_error(parser, JszlE_BadKey);
		return 0; //len+1 cause ptr starts at start quotes
	}

	if(len > JSZL_MAX_KEY_LENGTH){
		printf("Error: Key too long\n");
		parse_error(parser, JszlE_KeyLength);
		return 0;
	}

	//atom = &parser->atom_pool[parser->atom_pool_idx++];
	atom = malloc(sizeof(struct atom));
	parser->curkey = atom_add(g_atomTable, ATOM_TABLE_SIZE, atom, hash, len, parser->loc);
	if(!parser->curkey){
		printf("Error: No atom\n");
		return 0;
	}
	if(key_exists(parser->current_namespace, parser->curkey)){
		parse_error(parser, JszlE_DupKey);
		return 0;
	}
	return len;
}



/********************************************************//**
 * parser_skip_ws
 *
 * @brief The purpose of this function is to skip the whitespace
 * in a JSON document during the parsing phase 
 *
 * @param loc the 
 * @return the number of characters to skip
 *
 ********************************************************/

static unsigned inline parser_skip_ws(const char *loc, unsigned *line)
{
	unsigned c;
	const void *start = loc;
	do{

		c = *loc;
		if(c == ' ' || c == '\t'){
			loc++;
		}
		else if(c == '\r'){
			if(loc[1] == '\n'){
				loc += 2;
				(*line)++;			
				//offset++;
			}
			else{
				printf("Invalid char\n");
			}
		}
		else if(c == '\n'){
			(*line)++; 
			loc++;
			//offset++;
		}
		else break;
	}while(1);
	return loc - start;
}



/******************************//**
 * parse_engine
 *
 * @brief Parse a JSON string
 *
 * @param[in] parser
 * @param[in] ctx	t
 *
 *
 * @return	the error code
 *
 ********************************/

static unsigned parse_engine(
	struct jszlparser *parser,
	struct jszlcontext *ctx,
	const char *str,
	jszl_string_handler string_handler
){
	struct jszlnode *current_namespace = 0;
	const char *loc;
	struct atom * atom;
	struct value_data vd = {0};

	void * l_atomTable = g_AtomTable;

	unsigned type = 0;
	unsigned subtype = 0;
	unsigned numtype = 0;
	unsigned isNegative = 0;
	unsigned len = 0;
	unsigned hash;
	int n;


	if(parser->phase != ParsePhase_None){
		current_namespace = parser->ns_stack[parser->stack_idx].namespace;

		switch(parser->phase){
			case ParsePhase_ArrayOptValue  : goto array_phase_opt_value;
			case ParsePhase_ArrayReqValue  : goto array_phase_req_value;
			case ParsePhase_ArrayEndValue  : goto array_phase_comma;
			case ParsePhase_ObjectOptKey   : goto object_phase_opt_key;
			case ParsePhase_ObjectReqKey   : goto object_phase_req_key;
			case ParsePhase_ObjectEndKey   : goto object_phase_colon;
			case ParsePhase_ObjectValue    : goto object_phase_value;
			case ParsePhase_ObjectEndValue : goto object_phase_comma;
		}
	}
	else{
		parser->loc = str;
		parser->line = 1;
	}

	parser->loc += parser_skip_ws(parser->loc, &parser->line);
	if(*parser->loc == '[') {
		parser->current_namespace = new_node(0, 0, TYPE_ARRAY, 0);
		if(!parser->current_namespace){
			LOG_ERROR(JszlE_NoMemory, 0);
		}
		parser->ns_stack[parser->stack_idx].namespace = parser->current_namespace;
	}
	else if(*parser->loc == '{') { 
		parser->current_namespace = new_node(0, 0, TYPE_OBJECT, 0);
		if(!parser->current_namespace) {
			LOG_ERROR(JszlE_NoMemory, 0);
		}
		parser->ns_stack[parser->stack_idx].namespace = parser->current_namespace;
	}
	else {
		LOG_ERROR(JSON_ERROR_TYPE_MISMATCH, 0);	
	}
	parser->root_namespace = parser->current_namespace;
	ctx->CurrentNS = parser->root_namespace;
	move_loc((*parser), 1, 1);

	enter_namespace:
		if(GET_VALUE_TYPE((*parser->current_namespace)) == TYPE_OBJECT){
			goto object_phase_opt_key;
		}

	array_phase_opt_value:
		parser->phase = ParsePhase_ArrayOptValue;
		n = parser_skip_ws(parser->loc, &parser->line);
		move_loc((*parser), n, n);
		if(*parser->loc == ']'){
			leave_namespace();
		}

	array_phase_req_value:
		parser->phase = ParsePhase_ArrayReqValue;
		n = parser_skip_ws(parser->loc, &parser->line);
		move_loc((*parser), n, n);
		type = validate_value(parser, 0);
		switch(type){
			case TYPE_OBJECT:
				break;
			case TYPE_ARRAY:
				break;
			case 0:
				//value error
			default:
				goto array_phase_comma;
		}
		parser->curkey = 0;
		parser->current_namespace = parser->prevnode; //namespace node is stored here
		if(++parser->stack_idx == MAX_NAMESPACE_LEVEL){
				return 0; //exit, out of namespace memory 
		}
		parser->ns_stack[parser->stack_idx].namespace = parser->current_namespace;
		move_loc((*parser), 1, 1);
		goto enter_namespace;
		
	array_phase_comma:
		parser->phase = ParsePhase_ArrayEndValue;
		n = parser_skip_ws(parser->loc, &parser->line);
		move_loc((*parser), n, n);
		if(*parser->loc == ','){
			move_loc((*parser), 1, 1);
			goto array_phase_req_value;
		}
		else if(*parser->loc == ']'){
			leave_namespace();
		}
		else{
			LOG_ERROR(JSON_ERROR_SYNTAX, "Error on comma in array");
		}

	object_phase_opt_key:
		parser->phase = ParsePhase_ObjectOptKey;
		n = parser_skip_ws(parser->loc, &parser->line);
		move_loc((*parser), n, n);
		if(*parser->loc == '}'){
			leave_namespace();
		}

	object_phase_req_key:
		parser->phase = ParsePhase_ObjectReqKey;
		n = parser_skip_ws(parser->loc, &parser->line);
		move_loc((*parser), n, n);
		len = key_handler(parser);
		assert(len);
		move_loc((*parser), len+2, len+2); // +2 to skip quotes TODO account for unicode chars

	object_phase_colon:
		parser->phase = ParsePhase_ObjectEndKey;
		n = parser_skip_ws(parser->loc, &parser->line);
		move_loc((*parser), n, n);
		if(*parser->loc != ':'){
			LOG_ERROR(JSON_ERROR_SYNTAX, "Error on object colon");
		}
		move_loc((*parser), 1, 1);

	object_phase_value:
		parser->phase = ParsePhase_ObjectValue;
		n = parser_skip_ws(parser->loc, &parser->line);
		move_loc((*parser), n, n);

		type = validate_value(parser, 0);
		switch(type){
			case TYPE_OBJECT:
				break;
			case TYPE_ARRAY:
				break;
			case 0:
				//value error
			default:
				goto object_phase_comma;
		}
		parser->curkey = 0;
		parser->current_namespace = parser->prevnode;
		if(++parser->stack_idx == MAX_NAMESPACE_LEVEL){
			printf("Error: Exceeded max namespace level\n");
			return 0; 
		}
		parser->ns_stack[parser->stack_idx].namespace = parser->current_namespace;
		move_loc((*parser), 1, 1);
		goto enter_namespace;

		parser->curkey = 0;
		type = 0;

	object_phase_comma:
		parser->phase = ParsePhase_ObjectEndValue;
		n = parser_skip_ws(parser->loc, &parser->line);
		move_loc((*parser), n, n);
		if(*parser->loc == ','){
			move_loc((*parser), 1, 1);
			goto object_phase_req_key;
		}
		else
		if(*parser->loc == '}'){
			leave_namespace();
		}
		else {
			LOG_ERROR(JSON_ERROR_SYNTAX, "Error on comma in object");
		}

	error_cleanup:
		printf("Error on line %u: %s (%d)\n",
		parser->line, parser->errmsg, parser->errcode);
		return 1;
 
	exit_root_namespace:
		parser->phase = 0;
		return JszlE_None;

} //END ENGINE



/********************************************************//**
 * get_value_byidx
 *
 * @brief The purpose of this function is to skip the whitespace
 * in a JSON document during the parsing phase 
 *
 * @param loc the 
 * @return the number of characters to skip
 *
 ********************************************************/

static struct jszlnode *get_value_byidx
(struct jszlnode *value, unsigned idx)
{
	if(idx >= value->count) return 0; //err: Index too high
	if( !(value = value->child) ) return 0; //err: no values
	while(idx--){
		assert(value);
		value = value->next;
	}
	return value;
}

static struct jszlnode *get_value_byname(
	struct jszlcontext *handle
 ,struct jszlnode *value
 ,unsigned long hash
 ,unsigned len
){
	struct atom *atom = 0;
	void * l_atomTable = g_AtomTable;
	//hash = djb2(name, len);
	atom = atom_find(l_atomTable, ATOM_TABLE_SIZE, hash, len);
	assert(atom);
	if(!atom) return 0;
	for(value = value->child; value; value = value->next)
		if(value->name == atom) break;
	return value; 
}



/******************************
 * resolve_root
 *
 * @brief get the first character of a JSON path and resolve
 * the root node
 *
 * @param pctx point to the jsizzle context struct
 * @param ppnode a pointer to a node object that holds the returned node
 * @param symbol the root character of the json path
 *
 *
 * $ = absolute root
 * # = relative root
 *
 *******************************/

static int resolve_root(
	struct jszlcontext *pctx
	,struct jszlnode **ppnode
	,const symbol
){
	*ppnode = 0;
	if(!pctx || !ppnode || !symbol) return JszlE_BadParam;
	if(!pctx->RootNS) return JszlE_NoRoot;

	if(symbol == '$') *ppnode = pctx->RootNS;
	else if(symbol == '#') *ppnode = pctx->CurrentNS;
	else return JszlE_BadPath;

	return JszlE_None; 
}



int get_node_byname(
	struct jszlnode *pnode
 ,struct jszlnode **ppnode
 ,const char *name
){
	unsigned n;
	unsigned long hash;
	struct atom *atom = 0;
	void * l_atomTable = g_AtomTable;

	n = is_valid_key(name, global_seed, &hash);
	if(!n)		return 0;
	atom = atom_find(l_atomTable, ATOM_TABLE_SIZE, hash, n);
	if(!atom) return 0;

	for(pnode = pnode->child; pnode; pnode = pnode->next)
		if(pnode->name == atom) break;
	if(!pnode) return 0;
	*ppnode = pnode;
	return n;
}



/**************************//*
 * query_engine
 * @brief Query through JSON nodes to find a node
 *
 * @param[in] pnode JSON used to search in
 * @param[in]
 * @param[out] ppnode returns a JSON node that was found. NULL if not found
 *
 * @return the error code
 *
 ************************/

static int query_engine(struct jszlnode *pnode,
	const char *path, struct jszlnode **ppnode)
{
	const char *loc;
	unsigned n, type, subtype;
	unsigned short idx;


	loc = path;

begin_loop:

	if(*loc == '\0'){
		*ppnode = pnode;
		return JszlE_None;
	}
	else if(*loc == '['){ 
		if(!IS_ARRAY((*pnode)) && !IS_OBJECT((*pnode)))
		{
			printf("Error: Must be structural node\n");
			*ppnode = 0;
			return JSON_ERROR_MUST_BE_ARRAY_OR_OBJECT;
		}
		loc++;
		n = is_valid_number(loc, &type, &subtype);
		idx = atouint(loc, n);
		pnode = get_value_byidx(pnode, idx);
		loc += n;
		if(!pnode) return JszlE_KeyUndef;
		if(*loc++ != ']'){
			printf("Error: Syntax\n");	
			*ppnode = 0;
			return JSON_ERROR_SYNTAX;
		}
	}
	else if(*loc == '.' || *loc == '/'){ //object
		loc++;
		if(!IS_OBJECT((*pnode))){
			printf("Error: Must be an object\n");
			*ppnode = 0;
			return JSON_ERROR_TYPE_MISMATCH;
		}
		n = get_node_byname(pnode, &pnode, loc);
		if(!n) return JszlE_KeyUndef;
		loc += n;
	}
	else { //err
	//should check if 'loc' is an object and a valid key name
		if(IS_ARRAY( (*pnode) ) || IS_OBJECT((*pnode)))
		{
			printf("Error: No namespaces\n");
			*ppnode = 0;
			return JSON_ERROR_SYNTAX;
		}
	}
	//do an atom find on the key
	//ns = json_find_ns(node);
	goto begin_loop;
}



#define NODE_BY_IDX(pnode, idx)\
		pnode = pnode->value;\
		if(idx > (node).count) { printf("Error: Index too high\n"); }\
		while(idx--) pnode = pnode->next;


static int copy_json_string(void *dest, void *src, unsigned int len)
{
	char *_dest = dest, *_src = src;
	int count;
	int buf;
	while(len--) {
		if(*_src == '\\'){
			count = get_escaped_char(++_src, &buf);
			len -= count;
			_src += count;
			*_dest++ = buf;
		}
		else *_dest++ = *_src++;
	}
	return _dest - dest;
}


//128
//2048		0x7FF
//65536	 0xFFFF
//2097152 0x1FFFFF


void utf8_encode_char(unsigned long c)
{
		if(c < 128); //1 byte
		else if(c < 2048); //2 byte
		else if(c < 65536); //3 byte
}

static int uft8_string(char *str, char *buf)
{
		unsigned int c;
		c = (0xF0 | ((char)(c >> 18) & 0x07)); //0000 0111
		c = (0x80 | ((char)(c >> 12) & 0x0F)); //0000 1111
		c = (0xC0 | ((char)(c >> 6) & 0x1F)); //0001 1111 
		c = (0x80 | (char)(c & 0x3F));				 //0011 1111
}

/*
** Validate the descriptor table of an object against a binary json object value
** Descriptor table stack is the stack that holds the nested descriptors of objects
*/

/*
#define enter_public_api(ptr)\
ptr = jszl_handle_to_resource(handle);\
ptr->error_file = __file__;\
ptr->error_line = __line__
*/


static void *get_context(jszlhandle_t handle)
{
		if(handle > g_max_handle){
				return 0; 
		}
		return &g_thread[handle-1]; //decrement by 1
}

//0x80
//0xF0

void utf8()
{

}



/*
** PUBLIC INTERFACES
**
*/


/*
** checks membind to see if the global environment has already been
** initialized
*/
NPX_PUBLIC_API_DEFINE(
	jszl_init,
	struct jszlvtable *vt,
	unsigned long options)
{
	//DEBUG_OUT();
	if(g_vtable.memalloc) return 0;

	g_vtable.memalloc = vt->memalloc ? vt->memalloc : memalloc;


	if(vt->errorlog) g_vtable.errorlog = vt->errorlog;
	else g_vtable.errorlog = deferrorlog;
}


NPX_PUBLIC_API_DEFINE(
	jszl_property,
	jszlhandle_t handle,
	enum JszlProp prop, ...)
{
	struct jszlcontext *pctx;
	va_list args;
		
	pctx = get_context(handle); 

	va_start(args, prop);
	switch(prop){

		case JszlProp_Encoding:
		{
			int encode = va_arg(args, char);
			pctx->encode = (char)encode;
			break;
		}

		case JszlProp_ReadBuf:
		{ 
			unsigned int bufsize;
			char *buffer;
			buffer = va_arg(args, char *);
			bufsize = va_arg(args, unsigned int);
			pctx->buffer = buffer;
			pctx->bufsize = bufsize;
			break;
		}
	} //end switch
	va_end(args);

	return JszlE_None;
}



/*
** jszl_thread_init
*/
NPX_PUBLIC_API_DEFINE(jszl_thread_init, jszlhandle_t *handle)
{
	if(!g_vtable.memalloc) return 0; //failure to init global

	struct jszlcontext *pctx;
	int curcpu;
		
	curcpu = get_current_cpu();

	g_thread[curcpu].ThreadId = get_thread_id();

	//assign the handle to a valid resource

	//initialize the multithreaded environment 
	init_multithreaded_environment();

	return (jszlhandle_t)curcpu+1; //+1 for 0 array index
}



//jszlParseString
NPX_PUBLIC_API_DEFINE(
	jszlParseString,
	jszlhandle_t handle,
	const char *json)
{
	return JszlE_None;
}


int string_handler(struct jszlparser *parser){
	int len;
	struct jszlnode *pnode = 0;
	int hash;

	struct jszlnode *current_namespace;

	len = is_valid_string(parser->loc+1, global_seed, &hash);
	if(!len) return 0;

	//current_namespace = parser->current_namespace;

	return len+1;
}


void * new_parser()
{
	void *parser = malloc(sizeof(struct jszlparser));

	memset(parser, 0, sizeof(struct jszlparser));
	return parser;
}


/********************************************************//**
 * function_name
 *
 * @brief Parse a json file from the local file system
 *
 * @return Return a jszl return code
 *
 ********************************************************/

NPX_PUBLIC_API_DEFINE(
	jszl_parse_local_file,
	jszlhandle_t handle,
	const char *path)
{
	struct jszlfile fs = {0};
	//struct jszlfile *pfs = &fs;
	const char *json;

	struct jszlcontext *ctx;
	struct jszlparser parser = {0};
	unsigned rslt;

	ctx = get_context(handle);
	if(!ctx){
		printf("Error: No context found\n"); 
		return 0;
	}

	ctx->parser = new_parser();
	ctx->parser->atom_pool = ctx->atom_pool;

	//jszlpriv_open_file();
	openFile(&fs, path);
	json = mapFile(&fs);


	//invoke the parsing engine on the mapped pointer
	rslt = parse_engine(&parser, ctx, json, string_handler);
	if(rslt != JszlE_None){
		printf("Error(%u): Failed to parse JSON file\n", rslt); 
		return 0;
	}
	printf("Parse Success!\n");

	char buf[8], *s = buf;

	//printf("UTF8 0x%x\n", utf8_encode(&buf, 1));
	s = utf8_encode(s, WHITE_CHESS_KING);
	s = utf8_encode(s, BLACK_SPADE_SUIT);
	printf("UC %s\n", buf);

	//unmapFile(&fs);
	return JszlE_None;
}


//jszl_load

NPX_PUBLIC_API_DEFINE(
	jszl_load,
	jszlhandle_t handle,
	const char *filename)
{

	struct jszlfile file;
	int err;
	struct jszlcontext *pctx;
	struct jszlparser parser;

	pctx = get_context(handle);
	//pctx->error_file = __file__;
	//pctx->error_line = __line__;

	// open the file and load it into memory
	openFile(&file, filename);
	//new_parser(
	//set_parser_key_handler(&parser, key_handler);
	err = parse_engine(&parser, pctx, filename, string_handler);

	//delete_parser(

	return JszlE_None;
}


NPX_PUBLIC_API_DEFINE(
	json_read,
	struct jszlparser *pstate,
	struct jszlcontext *handle,
	const char *str)
{
	int rslt;
	rslt = parse_engine(pstate, handle, str, string_handler);
	if(rslt != JszlE_None) return rslt;
	return JszlE_None;
}

/**
* Set the namespace scope of the current document
* @param handle handle to a JSizzle context
* @param path JSON path to a valid object or array
*/
NPX_PUBLIC_API_DEFINE(
	jszl_set_document_scope,
	jszlhandle_t handle,
	const char *path,
	jszlopresult *res)
{
	struct jszlcontext *pctx;
	struct jszlnode *pnode;

	pctx = get_context(handle);
	if(!pctx){
		g_operror = JszlE_BadHandle;
		return 0;
	}

	if(!pctx->RootNS) {//error: no document loaded
		g_operror = JszlE_NoRoot;
		return 0;
	}

	//search the node tree and set the node returned
	resolve_root(pctx, &pnode, *path+1);
	if(JszlE_None != query_engine(pnode, path+1, &pnode)){
		return 0;
	}

	if(!IS_ARRAY( (*pnode) ) && !IS_OBJECT( (*pnode) ) ){
		return 0; 
	}

	pctx->CurrentNS = pnode;
	return 1;		
}



/********************************************************//**
 * jszl_is_root 
 *
 * @brief This is the function description
 *
 * @param loc the 
 * @return The description of the return value
 *
 ********************************************************/

NPX_PUBLIC_API_DEFINE(
	jszl_is_root,
	jszlhandle_t handle,
	const char *path)
{
	struct jszlcontext *pctx;
	struct jszlnode *pnode;

	pctx = get_context(handle);
	if(!pctx){
		return 0; 
	}

	if(!pctx->RootNS){
		g_operror = JszlE_NoRoot; 
		return 0;
	}

	resolve_root(pctx, &pnode, *path);
	if(JszlE_None != query_engine(pnode, path+1, &pnode)){
		return 0; 
	}

	return pctx->CurrentNS == pnode;

	return JszlE_None;
}


/*
** Set the user context pointer to be passed to the user defined virtual table
*/
NPX_PUBLIC_API_DEFINE(
	jszlSetUserContext,
	jszlhandle_t handle,
	void *userctx)
{
	struct jszlcontext *ctx;

	ctx = get_context(handle);

	ctx->UserContext = userctx;
	return JszlE_None;
}


NPX_PUBLIC_API_DEFINE(
	jszl_geterror,
	jszlhandle_t handle,
	const char **errmsg)
{
	struct jszlcontext *ctx;
	ctx = get_context(handle);

	if(g_operror == JszlE_None) *errmsg = 0;
	else *errmsg = g_errmsg[0];

	return g_operror;
}



struct descriptor_table {
	struct field_instance * inst;
	int idx;
	int inst_count;
};



NPX_PUBLIC_API_DEFINE(
	jszlIterate,
	jszlhandle_t handle,
	int (*callback)(void *, int),
	void *passback,
	const char *path)
{
	struct jszlcontext *pctx;
	struct jszlnode *pnode;

	pctx = get_context(handle);

	resolve_root(pctx, &pnode, *path);
	query_engine(pnode, path+1, &pnode);

	if(GET_VALUE_TYPE((*pnode)) != TYPE_ARRAY) return JszlE_TypeMismatch;

	//for each node, use callback to process record
	return 1;
}



// 1. Look for field name in json object, if a field name is not
// in the object move to next field as fields aren't required to exist
// 2. Check the size of the field against the size of the value returned
// if the size of the returned data is too large, bailout!
// 3. copy data and set null term
// 4. increment to next instance

/*********************************************//**
 * jszl_deserialize_object
 *
 * @return 
 *
 **********************************************/

NPX_PUBLIC_API_DEFINE(
	jszl_deserialize_object,
	jszlhandle_t handle,
	void *buffer,
	struct field_desc table[],
	int count,
	const char *path)
{
	struct jszlcontext *pctx;
	int msg;
	struct jszlnode *rootnode;
	struct jszlnode *pnode;
	struct field_desc *curdesc;
	char jsonPath[64];
	int size;
	int length;

	int stack_idx = 0;
	struct descriptor_table stack[8] = {0};
	struct descriptor_table *stack_item;

	struct field_desc *fd;
	int i = 0;

	char *field;

	if(!handle || !path || !count || !table || !buffer) return JszlE_BadParam;

	pctx = get_context(handle);

	resolve_root(pctx, &rootnode, *path);
	msg = query_engine(rootnode, path+1, &rootnode);
	if(msg != JszlE_None) return msg;
	if(GET_VALUE_TYPE((*rootnode)) != TYPE_OBJECT) return JszlE_TypeMismatch;

RESTART:


	length = get_node_byname(rootnode, &pnode, table[i].name);
	if(!length) goto NEXT_DESCRIPTOR;

	field = (char*)buffer + table[i].offset;

	switch(table[i].type){

		case TYPE_STRING:
		case JSZLTYPE_STRING_UTF8:
		case JSZLTYPE_STRING_UTF16:

			if(pnode->length > table[i].size){
				return JszlE_NoMemory;
			}
			size = copy_json_string((char*)field, pnode->data, pnode->length);

	/*
	stack_item->idx++;
	if(stack_item->idx > stack_item->inst_count) {
		// gone through table, decrement stack
		stack_idx--;
		//goto ENTER_INSTANCE;
				}
	*/
	//goto ENTER_INSTANCE; 
	
			break;

		case JSZLTYPE_NUMBER_INT8:
		case JSZLTYPE_NUMBER_UINT8:

		case JSZLTYPE_NUMBER_INT16:
		case JSZLTYPE_NUMBER_UINT16:

		case TYPE_NUMBER:
		case JSZLTYPE_NUMBER_INT32:
		case JSZLTYPE_NUMBER_UINT32:
			*(int*)field = pnode->value;
			break;

		case JSZLTYPE_NUMBER_INT64:
		case JSZLTYPE_NUMBER_UINT64:

		case TYPE_OBJECT:
			break;
					
		case TYPE_ARRAY:
	// go into field instance and get the pointer to nested object instance
			stack_idx++;

		default: ; // unsupported type, bailout!

	}

NEXT_DESCRIPTOR:
	if(++i < count) goto RESTART;

	return 0;
}


/*
** check if key/value pair exists in document
*/

NPX_PUBLIC_API_DEFINE(
	jszl_key_exists,
	jszlhandle_t handle,
	const char *path)
{
	get_context(handle);
	return JszlE_None;
}


/**
* Get a string representation of a given error code
* @param errcode an error code 
* @param errmsg	the string message
*
*/
NPX_PUBLIC_API_DEFINE(
	jszl_op_error,
	jszlhandle_t handle,
	char **errmsg)
{
	get_context(handle);
	return JszlE_None;
}



NPX_PUBLIC_API_DEFINE(
	jszl_count,
	jszlhandle_t handle,
	const char *path)
{
	int err;
	struct jszlnode *pnode;
	struct jszlcontext *pctx;

	pctx = get_context(handle);
	err = resolve_root(pctx, &pnode, *path);
	if(err != JszlE_None){
		printf("Error: could not resolve JSON root path\n");	
		return -1;
	}
	err = query_engine(pnode, path+1, &pnode);
	if(err != JszlE_None){
		printf("Error: Could not query node\n");
		return -1;
	}
	if( GET_VALUE_TYPE((*pnode)) != TYPE_ARRAY) return JszlE_TypeMismatch; 
	return pnode->count;
}

#undef API_DEFINE
#undef API_CALL

