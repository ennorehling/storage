#ifdef USE_AUTOCONF
# include <autoconf.h>
#elif !defined(_MSC_VER)
# define _snprintf snprintf
#endif
