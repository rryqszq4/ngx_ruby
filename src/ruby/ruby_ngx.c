/**
 *    Copyright(c) 2017 rryqszq4
 *
 *
 */

#include <ruby.h>
#include "ruby_ngx.h"
#include "../ngx_http_ruby_module.h"
#include "../ngx_http_ruby_code.h"

static VALUE cNginx;

static VALUE ruby_ngx_print(VALUE self, VALUE str);

static VALUE ruby_ngx_print(VALUE self, VALUE str)
{
    ngx_buf_t *b;
    ngx_http_ruby_rputs_chain_list_t *chain;
    ngx_http_ruby_ctx_t *ctx;
    ngx_http_request_t *r;
    u_char *u_str;
    ngx_str_t ns;

    r = ngx_ruby_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_ruby_module);
    
    ns.len = RSTRING_LEN(str);
    ns.data = (u_char *)RSTRING_PTR(str);

    if (ctx->rputs_chain == NULL){
        chain = ngx_pcalloc(r->pool, sizeof(ngx_http_ruby_rputs_chain_list_t));
        chain->out = ngx_alloc_chain_link(r->pool);
        chain->last = &chain->out;
    }else {
        chain = ctx->rputs_chain;
        (*chain->last)->next = ngx_alloc_chain_link(r->pool);
        chain->last = &(*chain->last)->next;
    }

    b = ngx_calloc_buf(r->pool);
    (*chain->last)->buf = b;
    (*chain->last)->next = NULL;

    u_str = ngx_pstrdup(r->pool, &ns);
    //u_str[ns.len] = '\0';
    (*chain->last)->buf->pos = u_str;
    (*chain->last)->buf->last = u_str + ns.len;
    (*chain->last)->buf->memory = 1;
    ctx->rputs_chain = chain;
    ngx_http_set_ctx(r, ctx, ngx_http_ruby_module);

    if (r->headers_out.content_length_n == -1){
        r->headers_out.content_length_n += ns.len + 1;
    }else {
        r->headers_out.content_length_n += ns.len;
    }

    return self;
}

void Init_ngx(void)
{
    cNginx = rb_define_class("Ngx", rb_cObject);
    rb_define_singleton_method(cNginx, "print", ruby_ngx_print, 1);
}
