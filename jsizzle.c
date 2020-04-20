
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>

#include "jsizzle.h"
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
 * jsizzle_parse_engine
 *
 * @brief Parse a JSON string
 *
 * @param[in] parser
 * @param[in] ctx  t
 *
 *
 * @return  the error code
 *
 ********************************/

static unsigned jsizzle_parse_engine(
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
    move_loc((*parser), len+1, len+1); //TODO account for unicode chars
    printf("Key: %.*s\n", 4, parser->loc);

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


#include "query-engine.c"



/**************************//*
 * jsizzle_query_engine
 * @brief Query through JSON nodes to find a node
 *
 * @param[in] pnode JSON used to search in
 * @param[in]
 * @param[out] ppnode returns a JSON node that was found. NULL if not found
 *
 * @return the error code
 *
 ************************/

int jsizzle_query_engine(
  struct jszlnode *pnode
 ,const char *path
 ,struct jszlnode **ppnode)
{
  const char *loc;
  unsigned n, type, subtype;
  unsigned short idx;


  loc = path;

begin_loop:

  if(*loc == '\0'){
    *ppnode =  pnode;
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



