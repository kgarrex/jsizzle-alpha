
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>

#include "jszlpubl.h"
#include "jszlpriv.h"



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

  struct jszlnode  node_pool[512];
  unsigned int node_pool_size;

  struct atom  atom_pool[512];
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
  "The document root is missing"
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
//     this is to allow for flexible atom table memory (global vs. private)
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

#if !defined(JSZL_MAX_KEY_LENGTH)  || JSZL_MAX_KEY_LENGTH > 256
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
#include "parse-engine.c"
#include "query-engine.c"

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
//2048    0x7FF
//65536   0xFFFF
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
    c = (0x80 | (char)(c & 0x3F));         //0011 1111
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



#include "jszlapi.c"

