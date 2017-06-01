/**
 *    Copyright(c) 2017 rryqszq4
 *
 *
 */

#ifndef NGX_HTTP_RUBY_MODULE_H
#define NGX_HTTP_RUBY_MODULE_H

#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_config.h>
#include <nginx.h>

#include "ngx_http_ruby_code.h"

#define NGX_HTTP_RUBY_MODULE_NAME "ngx_ruby"
#define NGX_HTTP_RUBY_MODULE_VERSION "0.0.1"

extern ngx_module_t ngx_http_ruby_module;
ngx_http_request_t *ngx_ruby_request;

typedef struct ngx_http_ruby_main_conf_s {
    unsigned enabled_content_handler:1;
} ngx_http_ruby_main_conf_t;

typedef struct ngx_http_ruby_loc_conf_s {
    ngx_http_ruby_code_t *content_file_code;
    ngx_http_ruby_code_t *content_inline_code;

    ngx_int_t (*content_handler)(ngx_http_request_t *r);
} ngx_http_ruby_loc_conf_t;

#endif