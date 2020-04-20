

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define WORD_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN
#define DWORD_TO_BINARY_PATTERN WORD_TO_BINARY_PATTERN " " WORD_TO_BINARY_PATTERN

#define BYTE_TO_BINARY(byte)\
  (byte & 0x80 ? '1' : '0'),\
  (byte & 0x40 ? '1' : '0'),\
  (byte & 0x20 ? '1' : '0'),\
  (byte & 0x10 ? '1' : '0'),\
  (byte & 0x08 ? '1' : '0'),\
  (byte & 0x04 ? '1' : '0'),\
  (byte & 0x02 ? '1' : '0'),\
  (byte & 0x01 ? '1' : '0')

#define WORD_TO_BINARY(word)\
  (word & 0x8000 ? '1' : '0'),\
  (word & 0x4000 ? '1' : '0'),\
  (word & 0x2000 ? '1' : '0'),\
  (word & 0x1000 ? '1' : '0'),\
  (word & 0x0800 ? '1' : '0'),\
  (word & 0x0400 ? '1' : '0'),\
  (word & 0x0200 ? '1' : '0'),\
  (word & 0x0100 ? '1' : '0'),\
  BYTE_TO_BINARY(word)

#define DWORD_TO_BINARY(dword)\
  (dword & 0x80000000 ? '1' : '0'),\
  (dword & 0x40000000 ? '1' : '0'),\
  (dword & 0x20000000 ? '1' : '0'),\
  (dword & 0x10000000 ? '1' : '0'),\
  (dword & 0x08000000 ? '1' : '0'),\
  (dword & 0x04000000 ? '1' : '0'),\
  (dword & 0x02000000 ? '1' : '0'),\
  (dword & 0x01000000 ? '1' : '0'),\
  (dword & 0x00800000 ? '1' : '0'),\
  (dword & 0x00400000 ? '1' : '0'),\
  (dword & 0x00200000 ? '1' : '0'),\
  (dword & 0x00100000 ? '1' : '0'),\
  (dword & 0x00080000 ? '1' : '0'),\
  (dword & 0x00040000 ? '1' : '0'),\
  (dword & 0x00020000 ? '1' : '0'),\
  (dword & 0x00010000 ? '1' : '0'),\
  WORD_TO_BINARY(dword)

/*
** PUBLIC INTERFACES
**
*/


/*
** checks membind to see if the global environment has already been
** initialized
*/
void JSZL_API_DEFINE(jszl_init,
  struct jszlvtable *vt,
  unsigned long options)
{

  //DEBUG_OUT();
  if(g_vtable.memalloc) return;

  g_vtable.memalloc = vt->memalloc ? vt->memalloc : memalloc;


  if(vt->errorlog) g_vtable.errorlog = vt->errorlog;
  else g_vtable.errorlog = deferrorlog;
}


int JSZL_API_DEFINE(jszl_property,
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
jszlhandle_t JSZL_API_DEFINE(jszl_thread_init)
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
int JSZL_API_DEFINE(jszlParseString, jszlhandle_t handle, const char *json){
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


static int key_handler(struct jszlparser *parser)
{
  int len;
  unsigned hash;
  struct atom *atom;

  if(*parser->loc != '"'){
    return 0;
  }

  len = is_valid_key(parser->loc+1, global_seed, &hash);
  if(parser->loc[len+1] != '"') return 0; //len+1 cause ptr starts at start quotes

  if(len > JSZL_MAX_KEY_LENGTH){
    printf("Error: Key too long\n");
  }

  atom = &parser->atom_pool[parser->atom_pool_idx++];
  parser->curkey = atom_add(g_atomTable, ATOM_TABLE_SIZE, atom, hash, len, parser->loc);
  if(key_exists(parser->current_namespace, parser->curkey)){
    return JszlE_DupKey; 
  }
  return len+1;
}

void * new_parser()
{
  void *parser = malloc(sizeof(struct jszlparser));

  memset(parser, 0, sizeof(struct jszlparser));
  return parser;
}


//jszlParseLocalFile

int JSZL_API_DEFINE(jszl_parse_local_file, jszlhandle_t handle, const char *path){
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

  openFile(&fs, path);
  json = mapFile(&fs);


  //invoke the parsing engine on the mapped pointer
  rslt = jsizzle_parse_engine(&parser, ctx, json, string_handler);
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

int JSZL_API_DEFINE(jszl_load, jszlhandle_t handle, const char *filename)
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
    err = jsizzle_parse_engine(&parser, pctx, filename, string_handler);

    //delete_parser(

    return JszlE_None;
}


int JSZL_API_DEFINE(json_read,
 struct jszlparser *pstate, struct jszlcontext *handle, const char *str
){
int rslt;
    rslt = jsizzle_parse_engine(pstate, handle, str, string_handler);
    if(rslt != JszlE_None) return rslt;
    return JszlE_None;
}

/**
* Set the namespace scope of the current document
* @param handle handle to a JSizzle context
* @param path JSON path to a valid object or array
*/
jszlopresult JSZL_API_DEFINE(jszl_set_document_scope,
 jszlhandle_t handle, const char *path
){
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
    if(JszlE_None != jsizzle_query_engine(pnode, path+1, &pnode)){
        return 0;
    }

    if(!IS_ARRAY( (*pnode) ) && !IS_OBJECT( (*pnode) ) ){
        return 0; 
    }

    pctx->CurrentNS = pnode;
    return 1;    
}


int JSZL_API_DEFINE(jszl_document_is_root,
 jszlhandle_t handle, const char *path
){
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
    if(JszlE_None != jsizzle_query_engine(pnode, path+1, &pnode)){
        return 0; 
    }

    return pctx->CurrentNS == pnode;

    return JszlE_None;
}


/*
** Set the user context pointer to be passed to the user defined virtual table
*/
int JSZL_API_DEFINE(jszlSetUserContext,
 jszlhandle_t handle, void *userctx
){
    struct jszlcontext *ctx;

    ctx = get_context(handle);

    ctx->UserContext = userctx;
    return JszlE_None;
}


int JSZL_API_DEFINE(jszl_geterror,
 jszlhandle_t handle, const char **errmsg
){
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



int JSZL_API_DEFINE(jszlIterate,
 jszlhandle_t handle, int (*callback)(void *, int), void *passback, const char *path
){
    struct jszlcontext *pctx;
    struct jszlnode *pnode;

    pctx = get_context(handle);

    resolve_root(pctx, &pnode, *path);
    jsizzle_query_engine(pnode, path+1, &pnode);

    if(GET_VALUE_TYPE((*pnode)) != TYPE_ARRAY) return JszlE_TypeMismatch;

    //for each node, use callback to process record
    return 1;
}


int JSZL_API_DEFINE(jszl_deserialize_object,
 jszlhandle_t handle,
 void *buffer,
 struct field_desc table[],
 int count, const char *path)
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

    if(!handle || !path || !count || !table || !buffer) return JSZLE_BAD_PARAM;

    pctx = get_context(handle);

    resolve_root(pctx, &rootnode, *path);
    msg = jsizzle_query_engine(rootnode, path+1, &rootnode);
    if(msg != JszlE_None) return msg;
    if(GET_VALUE_TYPE((*rootnode)) != TYPE_OBJECT) return JszlE_TypeMismatch;

RESTART:


    // 1. Look for field name in json object, if a field name is not
    // in the object move to next field as fields aren't required to exist

    length = get_node_byname(rootnode, &pnode, table[i].name);
    if(!length) goto NEXT_DESCRIPTOR;

    field = (char*)buffer + table[i].offset;

    switch(table[i].type){

      case TYPE_STRING:
      case JSZLTYPE_STRING_UTF8:
      case JSZLTYPE_STRING_UTF16:

	// 2. Check the size of the field against the size of the value returned
	// if the size of the returned data is too large, bailout!
        if(pnode->length > table[i].size){
            return JszlE_NoMemory;
        }

	// 3. copy data and set null term
        size = copy_json_string((char*)field, pnode->data, pnode->length);

	// 4. increment to next instance
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

int JSZL_API_DEFINE(jszl_key_exists, jszlhandle_t handle, const char *path)
{
    get_context(handle);
    return JszlE_None;
}


/**
* Get a string representation of a given error code
* @param errcode an error code 
* @param errmsg  the string message
*
*/
int JSZL_API_DEFINE(jszl_op_error, jszlhandle_t handle, char **errmsg)
{
    get_context(handle);
    return JszlE_None;
}



int JSZL_API_DEFINE(jszl_count_items,
 jszlhandle_t handle, const char *path)
{
int msg;
struct jszlnode *pnode;
struct jszlcontext *pctx;

    pctx = get_context(handle);
    resolve_root(pctx, &pnode, *path);
    msg = jsizzle_query_engine(pnode, path+1, &pnode);
    if(msg != JszlE_None) return -1;
    if( GET_VALUE_TYPE((*pnode)) != TYPE_ARRAY) return JszlE_TypeMismatch; 
    return pnode->count;
}
