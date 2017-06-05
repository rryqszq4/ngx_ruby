/**
 *    Copyright(c) 2017 rryqszq4
 *
 *
 */

#include <ruby.h>

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_conf_file.h>
#include <nginx.h>

#include "ngx_http_ruby_module.h"
#include "ngx_http_ruby_directive.h"
#include "ngx_http_ruby_handler.h"

#include "ruby/ruby_ngx.h"

static ngx_int_t ngx_http_ruby_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_ruby_handler_init(ngx_http_core_main_conf_t *cmcf, ngx_http_ruby_main_conf_t *rmcf);

static void *ngx_http_ruby_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_ruby_init_main_conf(ngx_conf_t *cf, void *conf);

static void *ngx_http_ruby_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_ruby_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_int_t ngx_http_ruby_init_worker(ngx_cycle_t *cycle);
static void ngx_http_ruby_exit_worker(ngx_cycle_t *cycle);

static ngx_command_t ngx_http_ruby_commands[] = {

    {ngx_string("content_by_ruby_file"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_ruby_content_file_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_ruby_content_file_handler
    },

    {ngx_string("content_by_ruby"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_ruby_content_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_ruby_content_inline_handler
    },

    ngx_null_command
};

static ngx_http_module_t ngx_http_ruby_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_http_ruby_init,                 /* postconfiguration */

    ngx_http_ruby_create_main_conf,     /* create main configuration */
    ngx_http_ruby_init_main_conf,       /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_http_ruby_create_loc_conf,      /* create location configuration */
    ngx_http_ruby_merge_loc_conf        /* merge location configuration */
};

ngx_module_t ngx_http_ruby_module = {
    NGX_MODULE_V1,
    &ngx_http_ruby_module_ctx,    /* module context */
    ngx_http_ruby_commands,       /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    ngx_http_ruby_init_worker,      /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    ngx_http_ruby_exit_worker,      /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_http_ruby_init(ngx_conf_t *cf)
{
    ngx_http_core_main_conf_t *cmcf;
    ngx_http_ruby_main_conf_t *rmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    rmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_ruby_module);

    ngx_ruby_request = NULL;

    if (ngx_http_ruby_handler_init(cmcf, rmcf) != NGX_OK){
        return NGX_ERROR;
    }

    return NGX_OK;
}

static ngx_int_t
ngx_http_ruby_handler_init(ngx_http_core_main_conf_t *cmcf, ngx_http_ruby_main_conf_t *rmcf)
{
    ngx_int_t i;
    ngx_http_handler_pt *h;
    ngx_http_phases phase;
    ngx_http_phases phases[] = {
        //NGX_HTTP_POST_READ_PHASE,
        NGX_HTTP_REWRITE_PHASE,
        NGX_HTTP_ACCESS_PHASE,
        NGX_HTTP_CONTENT_PHASE,
        NGX_HTTP_LOG_PHASE,
    };
    ngx_int_t phases_c;

    phases_c = sizeof(phases) / sizeof(ngx_http_phases);
    for (i = 0; i < phases_c; i++){
        phase = phases[i];
        switch (phase){
            /*case NGX_HTTP_POST_READ_PHASE:
                if (rmcf->enabled_post_read_handler){
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL){
                        return NGX_ERROR;
                    }
                    *h = ngx_http_ruby_post_read_handler;
                }
                break;
            case NGX_HTTP_REWRITE_PHASE:
                if (rmcf->enabled_rewrite_handler){
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL){
                        return NGX_ERROR;
                    }
                    *h = ngx_http_ruby_rewrite_handler;
                }
                break;
            case NGX_HTTP_ACCESS_PHASE:
                if (rmcf->enabled_access_handler){
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL){
                        return NGX_ERROR;
                    }
                    *h = ngx_http_ruby_access_handler;
                }
                break;*/
            case NGX_HTTP_CONTENT_PHASE:
                if (rmcf->enabled_content_handler){
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL){
                        return NGX_ERROR;
                    }
                    *h = ngx_http_ruby_content_handler;
                }
                break;
            /*case NGX_HTTP_LOG_PHASE:
                if (rmcf->enabled_log_handler){
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL){
                        return NGX_ERROR;
                    }
                    *h = ngx_http_ruby_log_handler;
                }
                break;*/
            default:
                break;
        }
    }

    return NGX_OK;
}

static void *
ngx_http_ruby_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_ruby_main_conf_t *rmcf;

    rmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_ruby_main_conf_t));
    if (rmcf == NULL){
        return NULL;
    }

    /*rmcf->state = ngx_pcalloc(cf->pool, sizeof(ngx_http_ruby_state_t));
    if (rmcf->state == NULL){
        return NULL;
    }*/


    return rmcf;
}

static char *
ngx_http_ruby_init_main_conf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}

static void *
ngx_http_ruby_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_ruby_loc_conf_t *rlcf;

    rlcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_ruby_loc_conf_t));
    if (rlcf == NULL){
        return NGX_CONF_ERROR;
    }

    //rlcf->rewrite_file_code = NGX_CONF_UNSET_PTR;
    //rlcf->rewrite_inline_code = NGX_CONF_UNSET_PTR;

    //rlcf->access_file_code = NGX_CONF_UNSET_PTR;
    //rlcf->access_inline_code = NGX_CONF_UNSET_PTR;

    rlcf->content_file_code = NGX_CONF_UNSET_PTR;
    rlcf->content_inline_code = NGX_CONF_UNSET_PTR;

    //rlcf->log_file_code = NGX_CONF_UNSET_PTR;
    //rlcf->log_inline_code = NGX_CONF_UNSET_PTR;

    return rlcf;
}

static char *
ngx_http_ruby_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    //ngx_http_core_loc_conf_t  *clcf;
    //clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    ngx_http_ruby_loc_conf_t *prev = parent;
    ngx_http_ruby_loc_conf_t *conf = child;

    //prev->rewrite_file_code = conf->rewrite_file_code;
    //prev->rewrite_inline_code = conf->rewrite_inline_code;

    //prev->access_file_code = conf->access_file_code;
    //prev->access_inline_code = conf->access_inline_code;

    prev->content_file_code = conf->content_file_code;
    prev->content_inline_code = conf->content_inline_code;

    //prev->log_file_code = conf->log_file_code;
    //prev->log_inline_code = conf->log_inline_code;

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_ruby_init_worker(ngx_cycle_t *cycle)
{

    ruby_init();
    ruby_init_loadpath();
    ruby_script("ngx_ruby");

    Init_ngx();

    return NGX_OK;
}

static void 
ngx_http_ruby_exit_worker(ngx_cycle_t *cycle)
{
    ruby_cleanup(0);
}

