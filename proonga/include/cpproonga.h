#pragma once

#include <iostream>
#include <vector>
#include <string>

extern "C" {
#include <groonga.h>
}   // extern "C"

namespace croco {

/**
 * Groongaクラス
 *
 * @package     faiss-server
 * @author      Yujiro Takahashi <yujiro@cro-co.co.jp>
 */
class CPProonga {
public:
    typedef struct _attribute {
        std::string name;
        std::string value;
    } attribute_t;

    typedef struct _associate {
        std::string key;
        std::string value;
    } associate_t;

private:
    grn_ctx _ctx;
    grn_obj *_db;

public:
    CPProonga() {
        _db = nullptr;
        grn_init();
        grn_ctx_init(&_ctx, 0);
    }
    ~CPProonga() {
        grn_ctx_fin(&_ctx);
        grn_fin();
    }
    void open(std::string path);
    void close();
    std::string exec(std::string command);
    std::string exec(std::string command, std::vector<CPProonga::attribute_t> attributes);
}; // class CPProonga

/**
 * 
 *
 * @access public
 * @param  std::string path
 * @return void
 */
inline void CPProonga::open(std::string path)
{
    GRN_DB_OPEN_OR_CREATE(&_ctx, path.c_str(), 0, _db);
    if (NULL == _db) {
        throw std::logic_error("Could not connect to database. " + path);
    }

    if (GRN_SUCCESS != grn_ctx_set_output_type(&_ctx, GRN_CONTENT_JSON)) {
        throw std::logic_error("Unable to set the output type.");
    }
}

/**
 * 
 *
 * @access public
 * @param  std::string path
 * @return void
 */
inline void CPProonga::close()
{
    if (nullptr == _db) {
        return ;
    }

    if (NULL == _db) {
        return ;
    }

    if (GRN_SUCCESS != grn_obj_close(&_ctx, _db)) {
        throw std::logic_error("Unable to close database.");
    }
}

/**
 * 
 *
 * @access public
 * @param  std::string command
 * @return std::string
 */
inline std::string CPProonga::exec(std::string command)
{
    grn_obj *cmd = grn_ctx_get(&_ctx, command.c_str(), command.length());
    if (NULL == cmd) {
        throw std::logic_error("Command is not found. " + command);
    }

    std::string json;
    grn_expr_exec(&_ctx, cmd, 0);
    if (GRN_SUCCESS == _ctx.rc) {
        char* recv;
        uint32_t recv_len;
        int32_t recv_flg;
        if (GRN_SUCCESS != grn_ctx_recv(&_ctx, &recv, &recv_len, &recv_flg)) {
            throw std::logic_error("Could not retrieve results. " + command);
        }
        json.assign(recv, recv_len);
    } // if (GRN_SUCCESS == _ctx.rc)

    grn_expr_clear_vars(&_ctx, cmd);
    grn_obj_unlink(&_ctx, cmd);

    return json;
}

/**
 * 
 *
 * @access public
 * @param  std::string command
 * @param  std::vector<CPProonga::attribute_t>
 * @return std::string
 */
inline std::string CPProonga::exec(std::string command, std::vector<CPProonga::attribute_t> attributes)
{
    grn_obj *cmd = grn_ctx_get(&_ctx, command.c_str(), command.length());
    if (NULL == cmd) {
        throw std::logic_error("Command is not found. " + command);
    }

    std::vector<grn_obj*> attrs;
    for (const auto &attribute : attributes) {
        grn_obj *attr = grn_expr_get_var(&_ctx, cmd, attribute.name.c_str(), attribute.name.length());
        grn_obj_reinit(&_ctx, attr, GRN_DB_TEXT, 0);
        GRN_TEXT_PUTS(&_ctx, attr, attribute.value.c_str());
        attrs.push_back(attr);
    } // for (const auto &attribute : attributes)

    std::string json;
    grn_expr_exec(&_ctx, cmd, 0);
    if (GRN_SUCCESS == _ctx.rc) {
        char* recv;
        uint32_t recv_len;
        int32_t recv_flg;
        if (GRN_SUCCESS != grn_ctx_recv(&_ctx, &recv, &recv_len, &recv_flg)) {
            throw std::logic_error("Could not retrieve results. " + command);
        }
        json.assign(recv, recv_len);
    } // if (GRN_SUCCESS == _ctx.rc)

    for (const auto& attr : attrs) {
        grn_obj_unlink(&_ctx, attr);
    }

    grn_expr_clear_vars(&_ctx, cmd);
    grn_obj_unlink(&_ctx, cmd);

    return json;
}

} // namespace croco