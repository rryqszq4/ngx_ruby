/**
 *    Copyright(c) 2017 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_RUBY_CODE_H
#define NGX_HTTP_RUBY_CODE_H

#include <ngx_http.h>

extern ngx_http_request_t *ngx_ruby_request;

typedef enum code_type_t {
    NGX_HTTP_RUBY_CODE_TYPE_FILE,
    NGX_HTTP_RUBY_CODE_TYPE_STRING
} code_type_t;

typedef struct ngx_http_ruby_core_s {
    union code {
        char *file;
        char *string;
    } code;
    code_type_t code_type;
} ngx_http_ruby_code_t;

typedef struct ngx_http_ruby_rputs_chain_list_s {
    ngx_chain_t **last;
    ngx_chain_t *out;
} ngx_http_ruby_rputs_chain_list_t;

typedef struct ngx_http_ruby_ctx_s {
    ngx_http_ruby_rputs_chain_list_t *rputs_chain;
    unsigned request_body_more : 1;

    ngx_int_t exit_code;

} ngx_http_ruby_ctx_t;

ngx_http_ruby_code_t *ngx_http_ruby_code_from_file(ngx_pool_t *pool, ngx_str_t *code_file_path);
ngx_http_ruby_code_t *ngx_http_ruby_code_from_string(ngx_pool_t *pool, ngx_str_t *code_inline_str);

#endif