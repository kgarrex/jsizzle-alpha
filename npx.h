

#if defined(_WIN32) || defined(_WIN64)
  #define JSZLEXPORT __declspec(dllexport)

#elif defined(__linux__) || defined(__GNUC__)
  #define JSZLEXPORT __attribute__((visibility("default")))

#else
  #define JSZLEXPORT
#endif


#if defined(NPX_DEBUG)

#define NPX_PUBLIC_API_DEFINE(name, ...)\
JSZLEXPORT int \
_##name(const char *_file, unsigned int _line, \
	const char *_func, __VA_ARGS__)

#define NPX_PRIVATE_API_DEFINE(name, ...)\
int _##name(const char *_file, unsigned int _line, \
	const char *_func, __VA_ARGS__)

#define NPX_PUBLIC_API_CALL(name, ...)\
_##name(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define NPX_PRIVATE_API_CALL(name, ...)\
	NPX_PUBLIC_API_CALL(name, __VA_ARGS__)


#define NPX_DEBUG_OUT()\
printf("Error in file '%s' on line '%u' in function '%s'\n", _file, _line, _func)

#else
#define NPX_PUBLIC_API_DEFINE(name, ...) JSZLEXPORT int _##name(__VA_ARGS__)
#define NPX_PUBLIC_API_CALL(name, ...) _##name(__VA_ARGS__)
#define NPX_PRIVATE_API_DEFINE(name, ...) NPX_PRIVATE_API_DEFINE(name, __VA_ARGS__)
#define NPX_PRIVATE_API_CALL(name, ...) NPX_PUBLIC_API_CALL(name, __VA_ARGS__)

#define NPX_DEBUG_OUT()
#endif


