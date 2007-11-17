#pragma once

//2007.08.30 kobake ’Ç‰Á
#ifdef assert
#undef assert
#endif

#ifdef _DEBUG
	void debug_output(const char* str, ...);
	void debug_exit();
	void warning_point();

	#define assert(exp) \
	do{ \
		if(!(exp)){ \
			debug_output("!assert: %hs(line%d): %hs\n", __FILE__, __LINE__, #exp); \
			debug_exit(); \
		} \
	}while(false)

	#define assert_warning(exp) \
	do{ \
		if(!(exp)){ \
			debug_output("!warning: %hs(line%d): %hs\n", __FILE__, __LINE__, #exp); \
			warning_point(); \
		} \
	}while(false)

#else
	#define assert(exp)
	#define assert_warning(exp)
#endif
