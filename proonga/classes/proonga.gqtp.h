#ifndef PHP_CLASSES_PROONGA_GQTP_H
#define PHP_CLASSES_PROONGA_GQTP_H

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus

extern "C" {

#include "php.h"
#include "php_ini.h"
#include "main/SAPI.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "Zend/zend_smart_str.h"
#include "ext/json/php_json.h"
#include "ext/standard/php_http.h"
#include "ext/standard/url.h"
#include "SAPI.h"
#include "php_proonga.h"

#endif /* __cplusplus */

typedef struct _php_proonga_gqtp_object {
    grn_ctx ctx;
    grn_obj *db;
    zend_object zo;
} php_proonga_gqtp_object;

static inline php_proonga_gqtp_object *php_proonga_gqtp_from_obj(zend_object *obj) {
    return (php_proonga_gqtp_object*)((char*)(obj) - XtOffsetOf(php_proonga_gqtp_object, zo));
}

#define Z_PROONGA_GQTP_P(zv) php_proonga_gqtp_from_obj(Z_OBJ_P((zv)))

PHP_METHOD(proonga_gqtp_class, __construct);
PHP_METHOD(proonga_gqtp_class, __destruct);
PHP_METHOD(proonga_gqtp_class, exec);

#ifdef __cplusplus
}   // extern "C"
#endif /* __cplusplus */

#endif /* PHP_CLASSES_PROONGA_GQTP_H */