#ifndef PHP_PROONGA_H
#define PHP_PROONGA_H

#include <stdio.h>
#include <groonga.h>

extern zend_module_entry proonga_module_entry;
#define phpext_proonga_ptr &proonga_module_entry

#define PHP_PROONGA_VERSION	"0.1.0"
#define JSON_PARSER_GROONGA_DEPTH   512

#ifdef PHP_WIN32
#	define PHP_PROONGA_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PROONGA_API __attribute__ ((visibility("default")))
#else
#	define PHP_PROONGA_API
#endif

#ifdef ZTS
#   include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(proonga)
    char *dbpath;
    int initialized;
    int gqtpConnected;
ZEND_END_MODULE_GLOBALS(proonga)

/* Always refer to the globals in your function as PROONGA_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define PROONGA_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(proonga, v)

typedef struct {
    zend_object zo;
    grn_ctx ctx;
    grn_obj *db;
} proonga_database_t;

static inline proonga_database_t *proonga_database_from_obj(zend_object *obj) {
	return (proonga_database_t*)((char*)(obj) - XtOffsetOf(proonga_database_t, zo));
}

#define Z_PROONGA_P(zv) proonga_database_from_obj(Z_OBJ_P((zv)))


typedef struct {
    zend_object zo;
    grn_ctx ctx;
} proonga_gqtp_t;

static inline proonga_gqtp_t *proonga_gqtp_from_obj(zend_object *obj) {
	return (proonga_gqtp_t*)((char*)(obj) - XtOffsetOf(proonga_gqtp_t, zo));
}

#define Z_GQTP_P(zv) proonga_gqtp_from_obj(Z_OBJ_P((zv)))


#if defined(ZTS) && defined(COMPILE_DL_PROONGA)
    ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* PHP_PROONGA_H */
