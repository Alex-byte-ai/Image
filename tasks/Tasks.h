#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" DLL_API void Task0( const void *arguments, void *result );
extern "C" DLL_API void Task1( const void *arguments, void *result );
extern "C" DLL_API void Task2( const void *arguments, void *result );
