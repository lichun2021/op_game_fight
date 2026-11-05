#ifndef LUA_CJSON_CONF_H
#define LUA_CJSON_CONF_H

#ifdef _MSC_VER

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#if _MSC_VCR < 1900

#ifndef snprintf
#define snprintf _snprintf
#endif

#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

#ifndef LUACJSON_INLINE
#define LUACJSON_INLINE __inline
#endif

#endif /* _MSC_VCR < 1900 */
#endif /* _MSC_VCR */


#ifndef LUACJSON_INLINE
#define LUACJSON_INLINE inline
#endif

#endif /* LUA_CJSON_CONF_H */