#ifndef PHP_CLASSES_PROONGA_GRN_H
#define PHP_CLASSES_PROONGA_GRN_H

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus

#include "cpproonga.h"

extern "C" {

#include "php.h"
#include "php_ini.h"
#include "main/SAPI.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "ext/json/php_json.h"
#include "SAPI.h"
#include "php_proonga.h"

#endif /* __cplusplus */

typedef void *ProongaHandle;

typedef struct _php_proonga_grn_object {
    ProongaHandle handle;
    zend_object zo;
} php_proonga_grn_object;

static inline php_proonga_grn_object *php_proonga_grn_from_obj(zend_object *obj) {
    return (php_proonga_grn_object*)((char*)(obj) - XtOffsetOf(php_proonga_grn_object, zo));
}

#define Z_PROONGA_GRN_P(zv) php_proonga_grn_from_obj(Z_OBJ_P((zv)))

PHP_METHOD(proonga_grn_class, __construct);
PHP_METHOD(proonga_grn_class, __destruct);
PHP_METHOD(proonga_grn_class, exec);

#ifdef __cplusplus
}   // extern "C"
#endif /* __cplusplus */

#endif /* PHP_CLASSES_PROONGA_GRN_H */