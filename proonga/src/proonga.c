#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/json/php_json.h"
#include "main/SAPI.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "SAPI.h"

#include "php_proonga.h"
#include "classes/proonga.grn.h"
#include "classes/proonga.gqtp.h"

/* Handlers */
static zend_object_handlers proonga_grn_object_handlers;
static zend_object_handlers proonga_gqtp_object_handlers;

/* Class entries */
zend_class_entry *proonga_grn_ce;
zend_class_entry *proonga_gqtp_ce;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_proonga_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_proonga_grn_ctor, 0, 0, 1)
    ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_proonga_gqtp_ctor, 0, 0, 2)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_proonga_exec, 0, 0, 1)
    ZEND_ARG_INFO(0, command)
    ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ php_grn_class_methods */
static zend_function_entry php_grn_class_methods[] = {
    PHP_ME(proonga_grn_class, __construct, arginfo_proonga_grn_ctor, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(proonga_grn_class, __destruct,  arginfo_proonga_void,     ZEND_ACC_PUBLIC)
    PHP_ME(proonga_grn_class, exec,        arginfo_proonga_exec,     ZEND_ACC_PUBLIC)

    PHP_FE_END
};
/* }}} */

/* {{{ php_gqtp_class_methods */
static zend_function_entry php_gqtp_class_methods[] = {
    PHP_ME(proonga_gqtp_class, __construct, arginfo_proonga_gqtp_ctor, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(proonga_gqtp_class, __destruct,  arginfo_proonga_void,      ZEND_ACC_PUBLIC)
    PHP_ME(proonga_gqtp_class, exec,        arginfo_proonga_exec,      ZEND_ACC_PUBLIC)

    PHP_FE_END
};
/* }}} */

static void php_grn_object_free_storage(zend_object *object) /* {{{ */
{
    php_proonga_grn_object *intern = php_proonga_grn_from_obj(object);
    if (!intern) {
        return;
    }
    zend_object_std_dtor(&intern->zo);
}
/* }}} */

static zend_object *php_grn_object_new(zend_class_entry *class_type) /* {{{ */
{
    php_proonga_grn_object *intern;

    intern = ecalloc(1, sizeof(php_proonga_grn_object) + zend_object_properties_size(class_type));
    zend_object_std_init(&intern->zo, class_type);
    object_properties_init(&intern->zo, class_type);
    intern->zo.handlers = &proonga_grn_object_handlers;

    return &intern->zo;
}
/* }}} */

static void php_gqtp_object_free_storage(zend_object *object) /* {{{ */
{
    php_proonga_gqtp_object *intern = php_proonga_gqtp_from_obj(object);
    if (!intern) {
        return;
    }
    zend_object_std_dtor(&intern->zo);
}
/* }}} */

static zend_object *php_gqtp_object_new(zend_class_entry *class_type) /* {{{ */
{
    php_proonga_gqtp_object *intern;

    intern = ecalloc(1, sizeof(php_proonga_gqtp_object) + zend_object_properties_size(class_type));
    zend_object_std_init(&intern->zo, class_type);
    object_properties_init(&intern->zo, class_type);
    intern->zo.handlers = &proonga_gqtp_object_handlers;

    return &intern->zo;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(proonga)
{
    zend_class_entry ce;

    /* Register Proonga Class */
    memcpy(&proonga_grn_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    INIT_CLASS_ENTRY(ce, "Proonga", php_grn_class_methods);
    ce.create_object = php_grn_object_new;
    proonga_grn_object_handlers.offset = XtOffsetOf(php_proonga_grn_object, zo);
    proonga_grn_object_handlers.clone_obj = NULL;
    proonga_grn_object_handlers.free_obj = php_grn_object_free_storage;
    proonga_grn_ce = zend_register_internal_class(&ce);

    /* Register GQTP Class */
    memcpy(&proonga_gqtp_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    INIT_CLASS_ENTRY(ce, "GQTP", php_gqtp_class_methods);
    ce.create_object = php_gqtp_object_new;
    proonga_gqtp_object_handlers.offset = XtOffsetOf(php_proonga_gqtp_object, zo);
    proonga_gqtp_object_handlers.clone_obj = NULL;
    proonga_gqtp_object_handlers.free_obj = php_gqtp_object_free_storage;
    proonga_gqtp_ce = zend_register_internal_class(&ce);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(proonga)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(proonga)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Proonga support", "enabled");
    php_info_print_table_row(2, "Proonga module version", PHP_PROONGA_VERSION);
    php_info_print_table_row(2, "Groonga Library", grn_get_version());
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proonga_module_entry
*/
zend_module_entry proonga_module_entry = {
    STANDARD_MODULE_HEADER,
    "proonga",
    NULL,
    PHP_MINIT(proonga),
    PHP_MSHUTDOWN(proonga),
    NULL,
    NULL,
    PHP_MINFO(proonga),
    PHP_PROONGA_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */


#ifdef COMPILE_DL_PROONGA
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(proonga)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */