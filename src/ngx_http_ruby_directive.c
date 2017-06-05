/**
 *    Copyright(c) 2016-2017 rryqszq4
 *
 *
 */

#include "ngx_http_ruby_module.h"
#include "ngx_http_ruby_directive.h"

char *
ngx_http_ruby_content_file_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_ruby_main_conf_t *rmcf;
    ngx_http_ruby_loc_conf_t *rlcf;
    ngx_str_t *value;
    ngx_http_ruby_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    rmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_ruby_module);
    rlcf = conf;

    if (rlcf->content_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_ruby_code_from_file(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    rlcf->content_file_code = code;
    rlcf->content_handler = cmd->post;
    rmcf->enabled_content_handler = 1;

    return NGX_CONF_OK;
}

char *
ngx_http_ruby_content_inline_phase(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_ruby_main_conf_t *rmcf;
    ngx_http_ruby_loc_conf_t *rlcf;
    ngx_str_t *value;
    ngx_http_ruby_code_t *code;

    if (cmd->post == NULL) {
        return NGX_CONF_ERROR;
    }

    rmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_ruby_module);
    rlcf = conf;

    if (rlcf->content_handler != NULL){
        return "is duplicated";
    }

    value = cf->args->elts;

    code = ngx_http_ruby_code_from_string(cf->pool, &value[1]);
    if (code == NGX_CONF_UNSET_PTR){
        return NGX_CONF_ERROR;
    }

    rlcf->content_inline_code = code;
    rlcf->content_handler = cmd->post;
    rmcf->enabled_content_handler = 1;

    return NGX_CONF_OK;
}
