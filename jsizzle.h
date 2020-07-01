/*
** Includes:
** stdint.h, ctype.h
** 
*/

#ifndef JSZLPUBL_H
#define JSZLPUBL_H



#if defined(_WIN32) || defined(_WIN64)
  #define JSZLEXPORT __declspec(dllexport)

#elif defined(__linux__) || defined(__GNUC__)
  #define JSZLEXPORT __attribute__((visibility("default")))

#else
  #define JSZLEXPORT

#endif


#if defined(DEBUG) || defined(_DEBUG)

  #define JSZL_API_DEFINE(name, ...)\
  _##name(const char *_file, unsigned int _line, const char *_func, __VA_ARGS__)

  #define JSZL_API_CALL(name, ...)\
  _##name(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)


  #define DEBUG_OUT()\
  printf("Error in file '%s' on line '%u' in function '%s'\n", _file, _line, _func)

#else
  #define JSZL_API_DEFINE(name, ...) _##name(__VA_ARGS__)
  #define JSZL_API_CALL(name, ...) _##name(__VA_ARGS__)
  #define DEBUG_OUT()
#endif


/*
 * Function header for each function
*/

/********************************************************//**
 * function_name
 *
 * @brief This is the function description
 *
 * @param loc the 
 * @return The description of the return value
 *
 ********************************************************/





// Macros

/* VALUE VALIDATORS
 * CheckFalse  e.g. if(c == 'f')          CheckFalse
 * CheckTrue   e.g. if(c == 't')          CheckTrue
 * CheckNull   e.g. if(c == 'n')          CheckNull
 * CheckObject e.g. if(c == BEGIN_OBJECT) CheckObject
 * CheckArray  e.g. if(c == BEGIN_ARRAY)  CheckArray
 * CheckNumber e.g. if((c > 47 && c < 58) || c == '-')  CheckNumber
 * CheckString e.g. if(c == '"')          CheckString
*/

//http://json-schema.org/draft-07/schema#
//http://json-schema.org/draft-06/schema#
//http://json-schema.org/draft-04/schema#
//http://json-schema.org/draft-03/schema#
//http://json-schema.org/draft-02/schema#
//http://json-schema.org/draft-01/schema#

/*json schema formats
 * https://json-schema.org/understanding-json-schema/reference/string.html
 * date-time
 * time
 * date
 * email
 * idn-email
 * hostname
 * idn-hostname
 * ipv4
 * ipv6
 * uri
 * uri-reference
 * iri
 * iri-reference
 * uri-template
 * json-pointer
 * relative-json-pointer
 * regex
 */

/* "$id" : "https://example.com/person.schema.json"
 * "$schema" : "http://json-schema.org/draft-07/schema#"
 * "title" : "MyInstanceDataModel"
 * "type" : "object"
 * "properties": {}
*/


#define JSZLPARSE_CASE_SENSITIVE_KEYS
#define JSONFLAG_CASE_SENSITIVE_KEYS 0x01  // key match sensitivity. JSON default
#define JSONFLAG_RECURSIVE_SERIALIZATION 0x02

#define CAST(type, var) ((type)(var))

//#define isxdigit(d) 


typedef unsigned long jszlhandle_t;


typedef enum jszlopresult {
  JSZLOP_FAILED
 ,JSZLOP_SUCCESS
} jszlopresult;

// jszl initialization flags
typedef enum jszlinitopt{
  JSZLINIT_MULTITHREADED = 0x01
 ,JSZLINIT_LOGFILE       = 0x02
} jszlinitopt;


enum JszlE {
  JszlE_None = -1
 ,JSZLE_SYNTAX
 ,JSON_ERROR_SYNTAX
 ,JSZLE_INVALID_VALUE
 ,JSON_ERROR_TYPE_MISMATCH //need to clean up instances of this error
 ,JszlE_TypeMismatch
 ,JSON_ERROR_SCHEMA_ERROR
 ,JszlE_DupKey
 ,JszlE_KeyUndef
 ,JSZLERR_NO_FILE_EXISTS
 ,JSZLE_NO_FILE
 ,JSON_ERROR_OUT_OF_RANGE
 ,JSON_ERROR_CONTINUE     //fatal
 ,JSZLE_BAD_PARAM
 ,JSON_ERROR_INVALID_STRING
 ,JSON_ERROR_INVALID_NUMBER
 ,JszlE_NoMemory //fatal
 ,JSON_ERROR_MUST_BE_ARRAY_OR_OBJECT
 ,JSON_ERROR_MAX_NESTED_NAMESPACES
 ,JszlE_NoRoot
 ,JSZLE_NOT_EQUAL
 ,JSZLE_BAD_PATH
 ,JszlE_BadHandle
 ,JSZLE_BAD_ERRCODE
 ,MAX_JSZLERR

};

enum JszlProp {
  JszlProp_Encoding = 1,
  JszlProp_ReadBuf,
};

enum JszlEncode {
  JszlEncode_Utf8 = 1,
  JszlEncode_Utf16,
  JszlEncode_Utf32,
  JszlEncode_Utf8Fast,
};

#define SET_TYPE(Type, SubType) ((jszltype)((SubType << 3) | Type))
//definitive type list
#define JSZLTYPE_NUMBER_INT8     SET_TYPE(TYPE_NUMBER, SUBTYPE_INT8)
#define JSZLTYPE_NUMBER_INT16    SET_TYPE(TYPE_NUMBER, SUBTYPE_INT16)
#define JSZLTYPE_NUMBER_INT32    SET_TYPE(TYPE_NUMBER, SUBTYPE_INT32)
#define JSZLTYPE_NUMBER_INT64    SET_TYPE(TYPE_NUMBER, SUBTYPE_INT64)
#define JSZLTYPE_NUMBER_UINT8    SET_TYPE(TYPE_NUMBER, SUBTYPE_UINT8)
#define JSZLTYPE_NUMBER_UINT16   SET_TYPE(TYPE_NUMBER, SUBTYPE_UINT16)
#define JSZLTYPE_NUMBER_UINT32   SET_TYPE(TYPE_NUMBER, SUBTYPE_UINT32)
#define JSZLTYPE_NUMBER_UINT64   SET_TYPE(TYPE_NUMBER, SUBTYPE_UINT64)
#define JSZLTYPE_NUMBER_DOUBLE   SET_TYPE(TYPE_NUMBER, SUBTYPE_DOUBLE)

#define JSZLTYPE_STRING_UTF8     SET_TYPE(TYPE_STRING, SUBTYPE_UTF8)
#define JSZLTYPE_STRING_UTF16    SET_TYPE(TYPE_STRING, SUBTYPE_UTF16)
#define JSZLTYPE_STRING_UUID     SET_TYPE(TYPE_STRING, SUBTYPE_UUID)
#define JSZLTYPE_STRING_IPV4     SET_TYPE(TYPE_STRING, SUBTYPE_IPV4)
#define JSZLTYPE_STRING_IPV6     SET_TYPE(TYPE_STRING, SUBTYPE_IPV6)
#define JSZLTYPE_STRING_EMAIL    SET_TYPE(TYPE_STRING, SUBTYPE_EMAIL)
#define JSZLTYPE_STRING_PHONE    SET_TYPE(TYPE_STRING, SUBTYPE_PHONE) //E.164 formatting
#define JSZLTYPE_STRING_DATE     SET_TYPE(TYPE_STRING, SUBTYPE_DATE)
#define JSZLTYPE_STRING_BASE64   SET_TYPE(TYPE_STRING, SUBTYPE_BASE64)
#define JSZLTYPE_STRING_BASE16   SET_TYPE(TYPE_STRING, SUBTYPE_BASE16)

#define JSZLTYPE_ARRAY_FIXED     SET_TYPE(TYPE_ARRAY, SUBTYPE_FIXED_ARRAY)

typedef unsigned short jszltype;


//accepted number formats: decimal, binary, octal, hexadecimal


#define IS_LITTLE_ENDIAN() (__host_order__.v == 0x03020100ul)
#define IS_BIG_ENDIAN() (__host_order__.v == 0x00010203ul)


enum jszltype {
  TYPE_NULL = 1
 ,TYPE_ARRAY
 ,TYPE_BOOLEAN
 ,TYPE_NUMBER
 ,TYPE_OBJECT
 ,TYPE_STRING
};

#define JSON_DESCRIPTOR_TYPE_POINTER 3


enum value_types{
  SUBTYPE_NONE
 ,SUBTYPE_INT8
 ,SUBTYPE_INT16
 ,SUBTYPE_INT32
 ,SUBTYPE_INT64
 ,SUBTYPE_UINT8
 ,SUBTYPE_UINT16
 ,SUBTYPE_UINT32
 ,SUBTYPE_UINT64
 ,SUBTYPE_UTF8
 ,SUBTYPE_UTF16
 ,SUBTYPE_EMAIL
 ,SUBTYPE_IPV4
 ,SUBTYPE_IPV6
 ,SUBTYPE_UUID
 ,SUBTYPE_FLOAT
 ,SUBTYPE_SCIENTIFIC
 ,SUBTYPE_DATE
};

//Document vs Functional Errors
//Recoverable vs Unrecoverable Errors

/*
** Field descriptor for serializing and deserializing C-style structs/objects.
** A struct must have a descriptor defined representing its memory layout if it is
** to be properly serialized.
*/

//struct JszlFieldDesc
struct field_desc {
  const long offset;   /* field offset */
  const long type;     /* the field type of the object */
  union {
    unsigned long size;      /* total size of each element */
    struct field_desc *desc;
  };
  const long max_elem; /* maximum number number of elements in array */
  const char *name;    /* name of the field */
};

struct field_instance {
  struct field_desc *desc; /* the field desc that describes this field */
  long num_elem;    /* number of elements in array - must not be more than max_elem */
  long length;  /* the length of a string or number of fields in an object */
  long pval;    /* pointer to the data to be serialized */
};

/*

  struct field_desc date_desc[] = {
    {TYPE_CHAR,  1, 1, "month"}
   ,{TYPE_CHAR,  1, 1, "day"}
   ,{TYPE_SHORT, 2, 1, "year"}
  };

  struct field_desc nickname = {"nickname"};

  struct field_desc person_desc[] = {
    {TYPE_CHAR, 1, 64, "name"} 
   ,{TYPE_CHAR, 1, 1, "age"}
   ,{TYPE_OBJECT, sizeof(struct date), 1, "birthday"}
   ,{TYPE_POINTER, sizeof(void *), 4, "nicknames"}
  };

  struct Date{
    char month; 
    char day;
    short year;
  };

  struct Person{
    char name[64];
    char age;
    struct date birthday;
    char * nicknames[4];
  };

  struct Person person;

  struct field_instance birthday_instance[] = {
    {&birthday_desc[0], , , &person.birthday.month}
   ,{&birthday_desc[1], , , &person.birthday.day}
   ,{&birthday_desc[2], , , &person.birthday.year} 
  };

  struct field_instance person_instance[] = {
    {&person_desc[0], 1, 0, person.name}
   ,{&person_desc[1], , , &person.age}
   ,{&person_desc[2], , , &birthday_instance}
   ,{&person_desc[3], , , &
  };

  struct field_desc birthday_desc[] = {
    {TYPE_CHAR ,0 ,sizeof(person.birthday.month) ,"month" ,&person.birthday.month}
   ,{TYPE_CHAR ,0 ,sizeof(person.birthday.day) ,"day" ,&person.birthday.day}
   ,{TYPE_SHORT ,0 ,sizeof(person.birthday.year) ,"year" ,&person.birthday.year}
  };

  {TYPE_STRING ,64 ,"name"}
  struct field_desc person_desc[] = {
    {TYPE_STRING ,1 ,0 ,64 ,"name" ,person.name} 
   ,{TYPE_CHAR   ,1 ,0 ,1 ,"age" ,&person.age}
   ,{TYPE_OBJECT ,1 ,3 ,sizeof(person.birthday) ,"birthday" ,&birthday_desc}
   ,{TYPE_ARRAY  ,4 ,3 ,sizeof(person.nicknames) ,"nicknames" ,person.nicknames}
  };

*/

typedef struct {
  union {
    const char * ascii;
    //const wchar_t * unicode;
  };
  short length;
  short type;
} JSON_STRING;

typedef struct {
  const char *string;
  short length;
  char type;
  char subtype;
} JSON_QUERY, JSON_RESPONSE;

typedef int (*JSON_QUERY_FOREACH_CALLBACK)(
  struct jszlValue *value 
 ,void * pb
);

typedef int (*JSON_ERROR_HANDLER)(
  unsigned err
 ,const char *msg
 ,unsigned line
 ,void *pb
);


/*
** *Note* an app is not allowed to set membind without
** setting memfree (or vice versa)
*/
struct jszlvtable {
  void * (*memalloc)(void *ctx, unsigned long size, int doalloc);
  void (*errorlog)(void *ctx, int err, const char *msg);
};

 

/*
An atom is a pointer to a unique, immutable sequence of zero or more arbitrary bytes. There is only one occurence of any atom.

The atom table should exists in memory as long as the atom pool.
*/

#define jszl_init(vtblref, options) JSZL_API_CALL(jszl_init, vtblref, options)
void JSZL_API_DEFINE(jszl_init,
  struct jszlvtable *vtblref, unsigned long options
);



#define jszl_property(handle, property, ...)\
  JSZL_API_CALL(jszl_property, handle, property, __VA_ARGS__) 
int JSZL_API_DEFINE(jszl_property,
  jszlhandle_t handle, enum JszlProperty property, ...
);


/*
 get the number of items in an object/array 

 handle: the context handle to perform the operation
 path: null-terminated dot-notated json path of the object/array 

 return: 0 - MAX_ITEM_COUNT on success. -1 on failure
 use jszl_op_error to retrieve error info

 the count may not be larger than MAX_ITEM_COUNT(default: 4096) 
*/

#define jszl_count(handle, path)\
JSZL_API_CALL(jszl_count, handle, path)
JSZLEXPORT int JSZL_API_DEFINE(jszl_count,
  jszlhandle_t handle, const char *path
);


#define jszl_thread_init() JSZL_API_CALL(jszl_thread_init)
JSZLEXPORT jszlhandle_t JSZL_API_DEFINE(jszl_thread_init);


#define jszl_parse_local_file(Handle, File)\
  JSZL_API_CALL(jszl_parse_local_file, Handle, File)
JSZLEXPORT int JSZL_API_DEFINE(jszl_parse_local_file,
  jszlhandle_t handle ,const char *file
);


#define jszlIterate(Handle, Callback, Passback, Path)\
JSZL_API_CALL(jszlIterate, Callback, Passback, Path)
JSZLEXPORT int JSZL_API_DEFINE(jszlIterate,
  jszlhandle_t handle,
  int (*callback)(void *, int),
  void *passback,
  const char *path
);

/*
** Parse and cache a JSON document
*/
#define jszl_load(HANDLE, JSONSTR) JSZL_API_CALL(jszl_load, HANDLE, JSONSTR)
int JSZL_API_DEFINE(jszl_load, jszlhandle_t hdl, const char *jsonstr);



#define json_read(PSTATE, POBJECT, JSON_STR)\
JSZL_API_CALL(json_read, PSTATE, POBJECT, JSON_STR)
int JSZL_API_DEFINE(json_read,
  struct jszlparser *pstate,
  struct jszlcontext *handle,
  const char *str
);



/*
** Set the document scope on a handle
*/

#define jszl_set_document_scope(handle, path)\
  JSZL_API_CALL(jszl_set_document_scope, handle, path)
JSZLEXPORT int JSZL_API_DEFINE(jszl_set_document_scope,
  jszlhandle_t handle, const char *pathstr);



/*
** Check to see if an object or array is set as the root
** '#' should always equal the current root
*/
#define jszl_is_root(Handle, Path)\
  JSZL_API_CALL(jszl_is_root, Handle, Path)
JSZLEXPORT int JSZL_API_DEFINE(jszl_is_root,
  jszlhandle_t handle, const char *pathstr);


/*
** Query a JSON document to check is a value exist
*/
#define jszl_value_exists(Handle, Path)\
JSZL_API_CALL(jszl_values_exists, Handle, Path)
JSZLEXPORT int JSZL_API_DEFINE(jszl_key_exists,
  jszlhandle_t handle,
  const char *path
);


/*
** Deserialize a string value
*/
#define jszl_deserialize_string(HANDLE, ENDPOINT, STRBUF, BUFSIZE, PSIZE)\
JSZL_API_CALL(jszl_deserialize_string, HANDLE, ENDPOINT, STRBUF, BUFSIZE, PSIZE)
JSZLEXPORT int JSZL_API_DEFINE(jszl_deserialize_string,
  jszlhandle_t handle,
  const char *endpoint,
  char strbuf[],
  int bufsize,
  int *psize
);



#define json_query_get_string(POBJ, PATH, STRBUF, BUFSIZE, PSIZE)\
JSZL_API_CALL(json_query_get_string, POBJ, PATH, STRBUF, BUFSIZE, PSIZE)
int JSZL_API_DEFINE(json_query_get_string,
  struct jszlcontext *handle,
  const char * path,
  char strbuf[],
  int bufsize,
  int *psize
);



/*
** Deserialize a number value 
*/
#define jszl_deserialize_number(HANDLE, ENDPOINT, PNUM)\
JSZL_API_CALL(jszl_deserialize_number, HANDLE, ENDPOINT, PNUM)
int JSZL_API_DEFINE(jszl_deserialize_number,
  jszlhandle_t handle,
  const char *endpoint,
  long *pnum
);


#define jszl_deserialize_object(Handle, Buffer, Table, Count, Path)\
JSZL_API_CALL(jszl_deserialize_object, Handle, Buffer, Table, Count, Path)
int JSZL_API_DEFINE(jszl_deserialize_object,
  jszlhandle_t handle,
  void *buffer,
  struct field_desc table[],
  int count,
  const char *path
);


/*
** Returns: JSON_SUCCESS JSON_ERROR_TYPE_MISMATCH
*/
#define jszl_deserialize_boolean(HANDLE, ENDPOINT, PBOOL)\
JSZL_API_CALL(jszl_deserialize_boolean, HANDLE, ENDPOINT, PBOOL)
int JSZL_API_DEFINE(jszl_deserialize_boolean,
  jszlhandle_t handle,
  const char *endpoint,
  long *boolean
);


/*
 * jszl_value_exists()
 * jszl_get_type(handle, 
 * jszl_deserialize(handle, type, buf, size, psize) 
 * jszl_deserialize_string()
 * jszl_deserialize_number()
 * jszlDeserializeBoolean
 * jszlValueExists
 * jszlQueryType
 * jszlQueryArrayCount
*/



/*
** Returns:
** JSON_INVALID_NODE - node pointer out of range
*/
/*
#define jszl_query_type(HANDLE, ENDPOINT, PTYPE)\
_jszl_query_type(JSZLDBG_CALL_WITH_PARAMS, HANDLE, ENDPOINT, PTYPE)
jszle _jszl_query_type(JSZLDBG_DEF_WITH_PARAMS
  jszlhandle_t hdl
 ,const char *endpoint
 ,jszltype *ptype
);
*/



/*
** get the string error message from a given error code
**
*/
#define jszl_op_error(handle, errmsg) JSZL_API_CALL(jszl_op_error, handle, errmsg)
int JSZL_API_DEFINE(jszl_op_error, jszlhandle_t handle, char **errmsg);


/*
** Returns:
** JSON_TYPE_MISMATCH - if the node is not a structural node
*/
/*
#define json_query_foreach(PNODE, CALLBACK, PASSBACK)\
_json_foreach(__FILE__, __LINE__, PNODE, CALLBACK, PASSBACK)
int _json_foreach(
  const char *__file__, unsigned __line__
 ,JSON_OBJECT * pobj 
 ,JSON_QUERY_FOREACH_CALLBACK cb
 ,void *pb
);
*/


//#undef JSZLDBG_DEF_WITH_PARAMS

#endif
