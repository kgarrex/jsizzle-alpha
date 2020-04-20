
#include "jszlpriv.h"

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
      parser->errcode = JSZLE_SYNTAX;
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
    if(parser->loc[1] != 'a' && 
       parser->loc[2] != 'l' && 
       parser->loc[3] != 's' && 
       parser->loc[4] != 'e'){
    //if(memcmp(parser->loc+1, "false", 5)){
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
    type  = TYPE_BOOLEAN;
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

