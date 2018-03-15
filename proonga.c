#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_http.h"
#include "ext/standard/url.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/json/php_json.h"
#include "php_proonga.h"
#include "main/SAPI.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "SAPI.h"

ZEND_DECLARE_MODULE_GLOBALS(proonga)

/* True global resources - no need for thread safety here */
//static int le_proonga;

/* Handlers */
static zend_object_handlers proonga_object_handlers;
static zend_object_handlers gqtp_object_handlers;

/* Class entries */
zend_class_entry *proonga_ce;
zend_class_entry *gqtp_ce;
zend_class_entry *proonga_exception_ce;

/* {{{ proto int plain_exec(grn_ctx *ctx, char *command, size_t command_len, zval *return_value)
 */
static void plain_exec(grn_ctx *ctx, char *command, size_t command_len, zval *return_value)
{
	grn_obj *cmd = grn_ctx_get(ctx, command, command_len);
    if (NULL == cmd) {
		zend_throw_exception(proonga_exception_ce, "Command is not found.", 0);
    }

    grn_expr_exec(ctx, cmd, 0);
    if (ctx->rc == GRN_SUCCESS) {
		char* recv;
	    uint32_t recv_len;
	    int32_t recv_flg;
	    grn_ctx_recv(ctx, &recv, &recv_len, &recv_flg);

		php_json_decode_ex(
	        return_value,
	        recv,
	        recv_len,
	        PHP_JSON_OBJECT_AS_ARRAY,
	        JSON_PARSER_GROONGA_DEPTH
	    );
	} else {
		ZVAL_FALSE(return_value);
	}
	grn_expr_clear_vars(ctx, cmd);
	grn_obj_unlink(ctx, cmd);
}
/* }}} */

/* {{{ proto int param_exec(grn_ctx *ctx, char *command, size_t command_len, HashTable *ht, zval *return_value)
 */
static void param_exec(grn_ctx *ctx, char *command, size_t command_len, HashTable *ht, zval *return_value)
{
	zend_string *attribute;
	zend_ulong index;
	int i, num_params = zend_hash_num_elements(ht);
	grn_obj **attrs, *cmd = grn_ctx_get(ctx, command, command_len);

    if (NULL == cmd) {
		zend_throw_exception(proonga_exception_ce, "Command is not found.", 0);
    }

	attrs = emalloc(sizeof(grn_obj *) * num_params);

	for (i=0; i<num_params; i++) {
		if (zend_hash_get_current_key(ht, &attribute, &index) == HASH_KEY_IS_STRING) {
			grn_obj *attr = grn_expr_get_var(ctx, cmd, ZSTR_VAL(attribute), ZSTR_LEN(attribute));
			if (NULL == attr) {
				zend_throw_exception(proonga_exception_ce, "Attribute is not found.", 0);
			} else {
				zend_string *buf;
				zval *value = zend_hash_get_current_data(ht);
				buf = zval_get_string(value);

				grn_obj_reinit(ctx, attr, GRN_DB_TEXT, 0);
				GRN_TEXT_PUTS(ctx, attr, ZSTR_VAL(buf));

				zend_string_release(buf);
			}
			attrs[i] = attr;
		}
		zend_hash_move_forward(ht);
	}
	ZVAL_FALSE(return_value);

    grn_expr_exec(ctx, cmd, 0);
    if (ctx->rc == GRN_SUCCESS) {
		char* recv;
	    uint32_t recv_len;
	    int32_t recv_flg;
	    grn_ctx_recv(ctx, &recv, &recv_len, &recv_flg);

		php_json_decode_ex(
	        return_value,
	        recv,
	        recv_len,
	        PHP_JSON_OBJECT_AS_ARRAY,
	        JSON_PARSER_GROONGA_DEPTH
	    );
	} else {
		ZVAL_FALSE(return_value);
	}
	grn_expr_clear_vars(ctx, cmd);

	for (--i; i >= 0; i--) {
		grn_obj_unlink(ctx, attrs[i]);
	}
	efree(attrs);

	grn_obj_unlink(ctx, cmd);
}
/* }}} */

/* {{{ proto void Proonga::__construct(string dbpath)
 */
PHP_METHOD(Proonga, __construct)
{
	proonga_database_t *self;
    char *dbpath;
    size_t dbpath_len;
	zval *object = getThis();

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(dbpath, dbpath_len)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    if (!dbpath_len) {
        zend_throw_exception(proonga_exception_ce, "Database name is empty.", 0);
    }
	self = Z_PROONGA_P(object);
    grn_ctx_init(&self->ctx, 0);

    GRN_DB_OPEN_OR_CREATE(&self->ctx, dbpath, 0, self->db);
    if (NULL == self->db) {
        zend_throw_exception(proonga_exception_ce, "Could not connect to database.", 0);
    }

    if (GRN_SUCCESS != grn_ctx_set_output_type(&self->ctx, GRN_CONTENT_JSON)) {
        zend_throw_exception(proonga_exception_ce, "Unable to set the output type.", 0);
    }
}
/* }}} */

/* {{{ proto void Proonga::exec(string command, array params)
 */
PHP_METHOD(Proonga, exec)
{
    proonga_database_t *self;
    char *command, *recv;
    size_t command_len;
    zval *object = getThis(), *params = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(command, command_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    self = Z_PROONGA_P(object);

    if (NULL == params) {
		plain_exec(&self->ctx, command, command_len, return_value);
	} else {
		param_exec(&self->ctx, command, command_len, Z_ARRVAL_P(params), return_value);
    }
}
/* }}} */


/* {{{ proto void GQTP::__construct(string host, number port)
 */
PHP_METHOD(GQTP, __construct)
{
	proonga_gqtp_t *self;
    char *host;
    size_t host_len;
	zend_long port, flags = 0;
	zval *object = getThis();

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(host, host_len)
		Z_PARAM_LONG(port)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    if (!host_len) {
        zend_throw_exception(proonga_exception_ce, "Database name is empty.", 0);
        RETURN_FALSE;
    }
	self = Z_GQTP_P(object);
    grn_ctx_init(&self->ctx, 0);

    if (grn_ctx_connect(&self->ctx, host, port, flags) != GRN_SUCCESS) {
		zend_throw_exception(proonga_exception_ce, "Unable to connect to groonga server.", 0);
        RETURN_FALSE;
    }

	/* 出力タイプの設定 */
    if (GRN_SUCCESS != grn_ctx_set_output_type(&self->ctx, GRN_CONTENT_JSON)) {
        zend_throw_exception(proonga_exception_ce, "Unable to set the output type.", 0);
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto void GQTP::exec(string command, array params)
 */
PHP_METHOD(GQTP, exec)
{
    proonga_gqtp_t *self;
    char *command;
    size_t command_len;
	smart_str query = {0};
    zval *object = getThis(), *params = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(command, command_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    self = Z_GQTP_P(object);

	smart_str_appendl(&query, "/d/", strlen("/d/"));
    smart_str_appendl(&query, command, command_len);
    smart_str_appendl(&query, ".json", strlen(".json"));

    if (NULL != params) {
		smart_str param = {0};
		long enc_type = PHP_QUERY_RFC1738;

		if (php_url_encode_hash_ex(HASH_OF(params), &param, NULL, 0, NULL, 0, NULL, 0, (Z_TYPE_P(params) == IS_OBJECT ? params : NULL), NULL, enc_type) == SUCCESS) {
	        smart_str_appendc(&query, '?');
	        smart_str_append_smart_str(&query, &param);
	    }
		smart_str_free(&param);
	}

	grn_ctx_send(&self->ctx, ZSTR_VAL(query.s), ZSTR_LEN(query.s), 0);
	smart_str_free(&query);

	if (GRN_SUCCESS == self->ctx.rc) {
		char *recv = NULL;
		uint32_t recv_len = 0;
	    int32_t recv_flg;

		grn_ctx_recv(&self->ctx, &recv, &recv_len, &recv_flg);
	    php_json_decode_ex(
	        return_value,
	        recv,
	        recv_len,
	        PHP_JSON_OBJECT_AS_ARRAY,
	        JSON_PARSER_GROONGA_DEPTH
	    );
		return ;
    }
	RETURN_FALSE;
}
/* }}} */


/* {{{ ARG Info
 */
 ZEND_BEGIN_ARG_INFO_EX(arginfo_arg_once, 0, ZEND_RETURN_VALUE, 1)
     ZEND_ARG_INFO(0, arg1)
 ZEND_END_ARG_INFO()

 ZEND_BEGIN_ARG_INFO_EX(arginfo_arg_one_opt, 0, ZEND_RETURN_VALUE, 0)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
 ZEND_END_ARG_INFO()

 ZEND_BEGIN_ARG_INFO_EX(arginfo_arg_two, 0, ZEND_RETURN_VALUE, 0)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
 ZEND_END_ARG_INFO()

/* }}} */


/* {{{ proonga_class_methods
 */
static zend_function_entry proonga_class_methods[] = {
	PHP_ME(Proonga, __construct, arginfo_arg_once, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(Proonga, exec, arginfo_arg_one_opt, ZEND_ACC_PUBLIC)

	PHP_FE_END
};
/* }}} */

/* {{{ proonga_class_methods
 */
static zend_function_entry gqtp_class_methods[] = {
	PHP_ME(Proonga, __construct, arginfo_arg_two, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
    PHP_ME(Proonga, exec, arginfo_arg_one_opt, ZEND_ACC_PUBLIC)

	PHP_FE_END
};
/* }}} */


/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("groonga.dbpath", "/tmp", PHP_INI_ALL, OnUpdateString, dbpath, zend_proonga_globals, proonga_globals)
PHP_INI_END()
/* }}} */

/* {{{ proonga_init_globals
 */
static void proonga_init_globals(zend_proonga_globals *proonga_globals)
{
	proonga_globals->initialized = 0;
	proonga_globals->gqtpConnected = 0;
}
/* }}} */

/* {{{ php_proonga_object_free_storage
 */
static void proonga_object_free_storage(zend_object *object)
{
	proonga_database_t *intern = proonga_database_from_obj(object);

	if (!intern) {
		return;
	}

    if (NULL != intern->db) {
        if (GRN_SUCCESS != grn_obj_close(&intern->ctx, intern->db)) {
            zend_throw_exception(proonga_exception_ce, "Unable to close database.", 0);
        }
    }
    grn_ctx_fin(&intern->ctx);

	zend_object_std_dtor(&intern->zo);
}
/* }}} */

/* {{{ php_proonga_object_new
 */
static zend_object *proonga_object_new(zend_class_entry *class_type)
{
	proonga_database_t *intern;

	/* Allocate memory for it */
	intern = ecalloc(1, sizeof(proonga_database_t) + zend_object_properties_size(class_type));

	zend_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	intern->zo.handlers = &proonga_object_handlers;

	return &intern->zo;
}
/* }}} */


/* {{{ php_gqtp_object_free_storage
 */
static void gqtp_object_free_storage(zend_object *object)
{
	proonga_gqtp_t *intern = proonga_gqtp_from_obj(object);

	if (!intern) {
		return;
	}
    grn_ctx_fin(&intern->ctx);

	zend_object_std_dtor(&intern->zo);
}
/* }}} */

/* {{{ php_gqtp_object_new
 */
static zend_object *gqtp_object_new(zend_class_entry *class_type)
{
	proonga_gqtp_t *intern;

	/* Allocate memory for it */
	intern = ecalloc(1, sizeof(proonga_gqtp_t) + zend_object_properties_size(class_type));

	zend_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	intern->zo.handlers = &gqtp_object_handlers;

	return &intern->zo;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(proonga)
{
	zend_class_entry ce;

    REGISTER_INI_ENTRIES();
	ZEND_INIT_MODULE_GLOBALS(proonga, proonga_init_globals, NULL);

    /* groongaライブラリを初期化 */
    if (0 == PROONGA_G(initialized)) {
        if (GRN_SUCCESS != grn_init()) {
            return FAILURE;
        }
        PROONGA_G(initialized) = 1;
    }

    memcpy(&proonga_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	memcpy(&gqtp_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* Register Proonga Class */
	INIT_CLASS_ENTRY(ce, "Proonga", proonga_class_methods);
	ce.create_object = proonga_object_new;
	proonga_object_handlers.offset = XtOffsetOf(proonga_database_t, zo);
	proonga_object_handlers.clone_obj = NULL;
	proonga_object_handlers.free_obj = proonga_object_free_storage;
	proonga_ce = zend_register_internal_class(&ce);

	/* Register GQTP Class */
	INIT_CLASS_ENTRY(ce, "GQTP", gqtp_class_methods);
	ce.create_object = gqtp_object_new;
	gqtp_object_handlers.offset = XtOffsetOf(proonga_gqtp_t, zo);
	gqtp_object_handlers.clone_obj = NULL;
	gqtp_object_handlers.free_obj = gqtp_object_free_storage;
	gqtp_ce = zend_register_internal_class(&ce);

    /* Register Proonga\\Exception Class */
    INIT_CLASS_ENTRY(ce, "Proonga\\Exception", NULL);
    proonga_exception_ce = zend_register_internal_class_ex(&ce, spl_ce_RuntimeException);

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(proonga)
{
    if (1 == PROONGA_G(initialized)) {
        grn_fin();
        PROONGA_G(initialized) = 0;
    }

	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(proonga)
{
#if defined(COMPILE_DL_PROONGA) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(proonga)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(proonga)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Groonga support", "enabled");
	php_info_print_table_row(2, "Proonga module version", PHP_PROONGA_VERSION);
	php_info_print_table_row(2, "Groonga Library", grn_get_version());
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(proonga)
{
	memset(proonga_globals, 0, sizeof(*proonga_globals));
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
	NULL, // PHP_RINIT(proonga),
	NULL, // PHP_RSHUTDOWN(proonga),
	PHP_MINFO(proonga),
	PHP_PROONGA_VERSION,
	PHP_MODULE_GLOBALS(proonga),
	PHP_GINIT(proonga),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_PROONGA
#	ifdef ZTS
	ZEND_TSRMLS_CACHE_DEFINE()
#	endif
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
