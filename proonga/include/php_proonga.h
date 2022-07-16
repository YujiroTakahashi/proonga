#ifndef PHP_PROONGA_H
#define PHP_PROONGA_H

#include <groonga.h>

#define PHP_PROONGA_VERSION    "0.2.0"
#define JSON_PARSER_GROONGA_DEPTH   512

extern zend_module_entry niaico_module_entry;
#define phpext_proonga_grn_ptr &niaico_module_entry

#endif  /* PHP_PROONGA_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */