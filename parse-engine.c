
#define ERRORLOG_PROC (parser->log_error)

#define ERRORMSG_INVALID_PRIMITIVE "Invalid primitive value" 

enum JSizzleParsePhase {
  ParsePhaseNone,
  ParsePhaseArrayOptValue,
  ParsePhaseArrayReqValue,
  ParsePhaseArrayEndValue,
  ParsePhaseObjectOptKey,
  ParsePhaseObjectReqKey,
  ParsePhaseObjectEndKey,
  ParsePhaseObjectValue,
  ParsePhaseObjectEndValue
};


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

  pnode->value  = value;
  pnode->name   = (void*)atom;
  pnode->type   = type;
  pnode->count  = count;
  pnode->next   = 0;
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
goto error_cleanup;\
}while(0);


static long key_exists(struct jszlnode *value, struct atom *key)
{
  for(value = value->child; value; value = value->next)
    if(value->name == key) return 1;   
  return 0;
}

/*
** Skip whitespace
*/
static unsigned inline skip_ws(const char *loc, unsigned *line)
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

struct value_data {
  int type;
  unsigned length;
  unsigned isNegative;
  unsigned numType;
  unsigned long hash;
};


typedef int (*jszl_string_handler)(struct jszlparser *);
typedef int (*jszl_key_handler)(struct jszlparser *);


/* Macro to advance the current location of the parser
** This moves the pointer forward by 'byte_count' bytes and increments 
** the offset by 'char_count' to keep track of where errors occur in a line 
*/
#define move_loc(p, byte_count, char_count) p.loc += byte_count; p.offset += char_count


/*
** validation function for a JSON value
*/
static enum jszltype validate_value(struct jszlparser *parser, jszl_string_handler string_handler
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
      errcode = JSZLE_SYNTAX;
      goto error_cleanup;
    }
    type = TYPE_STRING;
    value = (long)parser->loc+1;
    move_loc((*parser), length+1, length+1);
    break;

    case '[':
    type = TYPE_ARRAY;
    move_loc((*parser), 1, 1 );
    break;

    case '{':
    type = TYPE_OBJECT;
    move_loc((*parser), 1, 1);
    break;

    case 'f':
    if(memcmp(parser->loc+1, "false", 5)){
      errcode = JSZLE_SYNTAX;
      goto error_cleanup;
    }
    type = TYPE_BOOLEAN;
    move_loc((*parser), 5, 5);
    break;

    case 't':
    if(memcmp(parser->loc+1, "true", 4)){
      errcode = JSZLE_SYNTAX;
      goto error_cleanup;
    }
    type  = TYPE_BOOLEAN;
    value = 1;
    move_loc((*parser), 4, 4);
    break;

    case 'n':
    if(memcmp(parser->loc+1, "null", 4)){
      errcode = JSZLE_SYNTAX;
      goto error_cleanup;
    }
    type = TYPE_NULL;
    move_loc((*parser), 4, 4);
    break;

    default:
    if(isdigit(*parser->loc) || *parser->loc == '-'){
      length = is_valid_number(parser->loc+1, &numType, &isNegative);
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
    errcode = JSZLE_INVALID_VALUE;
    goto error_cleanup;
  }

  pnode = new_node(value, (long)parser->curkey, type, length);
  if(!pnode){
    errcode = JszlE_NoMemory;
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


static unsigned json_parser_engine(
  struct jszlparser *parser,
  struct jszlcontext *ctx,
  const char *str,
  jszl_key_handler key_handler,
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


  if(parser->phase){
    current_namespace = parser->ns_stack[parser->stack_idx].namespace;

    switch(parser->phase){
      case ParsePhaseArrayOptValue  : goto array_phase_opt_value;
      case ParsePhaseArrayReqValue  : goto array_phase_req_value;
      case ParsePhaseArrayEndValue  : goto array_phase_comma;
      case ParsePhaseObjectOptKey   : goto object_phase_opt_key;
      case ParsePhaseObjectReqKey   : goto object_phase_req_key;
      case ParsePhaseObjectEndKey   : goto object_phase_colon;
      case ParsePhaseObjectValue    : goto object_phase_value;
      case ParsePhaseObjectEndValue : goto object_phase_comma;
    }
  }
  else{
    parser->loc = str;
    parser->line = 1;
  }

    parser->loc += skip_ws(parser->loc, &parser->line);
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
    parser->phase = ParsePhaseArrayOptValue;
    n = skip_ws(parser->loc, &parser->line);
    move_loc((*parser), n, n);
    if(*parser->loc == ']'){
        leave_namespace();
    }

  array_phase_req_value:
    parser->phase = ParsePhaseArrayReqValue;
    n = skip_ws(parser->loc, &parser->line);
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
    parser->phase = ParsePhaseArrayEndValue;
    n = skip_ws(parser->loc, &parser->line);
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
    parser->phase = ParsePhaseObjectOptKey;
    n = skip_ws(parser->loc, &parser->line);
    move_loc((*parser), n, n);
    if(*parser->loc == '}'){
      leave_namespace();
    }

  object_phase_req_key:
    parser->phase = ParsePhaseObjectReqKey;
    n = skip_ws(parser->loc, &parser->line);
    move_loc((*parser), n, n);
    len = key_handler(parser);
    move_loc((*parser), len+1, len+1); //TODO account for unicode chars

  object_phase_colon:
    parser->phase = ParsePhaseObjectEndKey;
    n = skip_ws(parser->loc, &parser->line);
    move_loc((*parser), n, n);
    if(*parser->loc != ':')
        LOG_ERROR(JSON_ERROR_SYNTAX, "Error on colon");
    move_loc((*parser), 1, 1);

  object_phase_value:
    parser->phase = ParsePhaseObjectValue;
    n = skip_ws(parser->loc, &parser->line);
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
    parser->phase = ParsePhaseObjectEndValue;
    n = skip_ws(parser->loc, &parser->line);
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
    printf("Error(%u) on line %u\n", 1, parser->line);
    return 1;
 
  exit_root_namespace:
    parser->phase = 0;
    return JszlE_None;

} //END ENGINE
