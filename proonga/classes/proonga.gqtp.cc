#include "proonga.gqtp.h"

/* {{{ proto void proonga::__construct()
 */
PHP_METHOD(proonga_gqtp_class, __construct)
{
    php_proonga_gqtp_object *db_obj;
    zval *object = getThis();
    char *host;
    size_t host_len;
    zend_long port, flags = 0;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(host, host_len)
        Z_PARAM_LONG(port)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    if (!host_len) {
        zend_throw_exception(NULL, "Database name is empty.", 0);
        return ;
    }

    db_obj = Z_PROONGA_GQTP_P(object);

    grn_init();
    grn_ctx_init(&db_obj->ctx, 0);

    if (grn_ctx_connect(&db_obj->ctx, host, port, flags) != GRN_SUCCESS) {
        zend_throw_exception(NULL, "Unable to connect to groonga server.", 0);
        return ;
    }

    /* 出力タイプの設定 */
    if (GRN_SUCCESS != grn_ctx_set_output_type(&db_obj->ctx, GRN_CONTENT_JSON)) {
        zend_throw_exception(NULL, "Unable to set the output type.", 0);
        return ;
    }
}
/* }}} */

/* {{{ proto void proonga::__destruct()
 */
PHP_METHOD(proonga_gqtp_class, __destruct)
{
    php_proonga_gqtp_object *db_obj;
    zval *object = getThis();

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    db_obj = Z_PROONGA_GQTP_P(object);
    grn_ctx_fin(&db_obj->ctx);
    grn_fin();
}
/* }}} */

/* {{{ proto array proonga::exec(string command[, array params])
 */
PHP_METHOD(proonga_gqtp_class, exec)
{
    php_proonga_gqtp_object *db_obj;
    char *command;
    size_t command_len;
    smart_str query = {0};
    zval *object = getThis(), *params = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(command, command_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    db_obj = Z_PROONGA_GQTP_P(object);

    smart_str_appendl(&query, "/d/", strlen("/d/"));
    smart_str_appendl(&query, command, command_len);
    smart_str_appendl(&query, ".json", strlen(".json"));

    if (NULL != params) {
        smart_str param = {0};
        long enc_type = PHP_QUERY_RFC1738;

        php_url_encode_hash_ex(
            HASH_OF(params), &param, 
            NULL, 0,    // num_prefix
            NULL, 0,    // key_prefix
            NULL, 0,    // key_suffix
            (Z_TYPE_P(params) == IS_OBJECT ? params : NULL), 
            NULL, enc_type
        );
        smart_str_appendc(&query, '?');
        smart_str_append_smart_str(&query, &param);
        smart_str_free(&param);
    }

    grn_ctx_send(&db_obj->ctx, ZSTR_VAL(query.s), ZSTR_LEN(query.s), 0);
    smart_str_free(&query);

    if (GRN_SUCCESS == db_obj->ctx.rc) {
        char *recv = NULL;
        uint32_t recv_len = 0;
        int32_t recv_flg;

        grn_ctx_recv(&db_obj->ctx, &recv, &recv_len, &recv_flg);
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
