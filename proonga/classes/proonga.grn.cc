#include "proonga.grn.h"

/* {{{ proto void proonga::__construct()
 */
PHP_METHOD(proonga_grn_class, __construct)
{
    php_proonga_grn_object *db_obj;
    zval *object = getThis();
    char *path;
    size_t path_len = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(path, path_len)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    db_obj = Z_PROONGA_GRN_P(object);

    croco::CPProonga *proonga = new croco::CPProonga();

    proonga->open(std::string(path, path_len));

    db_obj->handle = static_cast<ProongaHandle>(proonga);
}
/* }}} */

/* {{{ proto void proonga::__destruct()
 */
PHP_METHOD(proonga_grn_class, __destruct)
{
    php_proonga_grn_object *db_obj;
    zval *object = getThis();

    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    db_obj = Z_PROONGA_GRN_P(object);
    croco::CPProonga *proonga = static_cast<croco::CPProonga*>(db_obj->handle);
    proonga->close();
    delete proonga;
}
/* }}} */

/* {{{ proto array proonga::exec(string command[, array params])
 */
PHP_METHOD(proonga_grn_class, exec)
{
    php_proonga_grn_object *db_obj;
    char *command;
    size_t command_len;
    zval *object = getThis(), *params = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(command, command_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    db_obj = Z_PROONGA_GRN_P(object);
    croco::CPProonga *proonga = static_cast<croco::CPProonga*>(db_obj->handle);

    try {
        std::string json;
        if (NULL == params) {
            json = proonga->exec(command);
        } else {
            HashTable *ht = Z_ARRVAL_P(params);
            zend_hash_internal_pointer_reset(ht);
            zend_ulong size = zend_hash_num_elements(ht);

            std::vector<croco::CPProonga::attribute_t> attributes;
            for (zend_ulong idx=0; idx<size; idx++) {
                zend_string *name;
                zend_ulong index;
                if (zend_hash_get_current_key(ht, &name, &index) == HASH_KEY_IS_STRING) {
                    zval *zValue = zend_hash_get_current_data(ht);
                    zend_string *value = zval_get_string(zValue);
                    attributes.push_back({
                        std::string(name->val, name->len),
                        std::string(value->val, value->len)
                    });
                } // if (zend_hash_get_current_key(ht, &name, &index) == HASH_KEY_IS_STRING)
                zend_hash_move_forward(ht);
            } // for (int idx=0; idx<size; idx++)

            json = proonga->exec(command, attributes);
        } // if (NULL == params)
        array_init(return_value);

        php_json_decode_ex(
            return_value,
            json.c_str(),
            json.length(),
            PHP_JSON_OBJECT_AS_ARRAY,
            JSON_PARSER_GROONGA_DEPTH
        );
    } catch (...) {
        RETURN_FALSE;
    }
}
/* }}} */
