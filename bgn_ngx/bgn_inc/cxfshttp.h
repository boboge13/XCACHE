/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@163.com
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#ifndef _CXFSHTTP_H
#define _CXFSHTTP_H

#include "type.h"
#include "debug.h"

#include "cstring.h"

#include "csocket.inc"
#include "task.inc"
#include "chttp.inc"

#include "cbuffer.h"
#include "cstrkv.h"
#include "chunk.h"

#include "http_parser.h"

#define CXFSHTTP_SOCKET_TIMEOUT_NSEC      CONN_TIMEOUT_NSEC

#define CXFSHTTP_REST_API_NAME            "/xfs"

EC_BOOL cxfshttp_log_start();

EC_BOOL cxfshttp_commit_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_http_post(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_http_get(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_http_head(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_commit_end(CHTTP_NODE *chttp_node, EC_BOOL result);
EC_BOOL cxfshttp_commit_response(CHTTP_NODE *chttp_node);
/**
    interface name rule:
    cxfshttp_is_http_[get|post|put|head|delete]_<op>
    cxfshttp_commit_<op>_[get|post|put|head|delete]_request
    cxfshttp_handle_<op>_[get|post|put|head|delete]_request
    cxfshttp_make_<op>_[get|post|put|head|delete]_response
    cxfshttp_commit_<op>_[get|post|put|head|delete]_response
**/

EC_BOOL cxfshttp_is_http_post_setsmf(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_setsmf_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_setsmf_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_setsmf_post_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_setsmf_post_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_head_getsmf(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_getsmf_head_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_getsmf_head_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_getsmf_head_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_getsmf_head_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_lock_req(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_lock_req_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_lock_req_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_lock_req_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_lock_req_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_unlock_req(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_unlock_req_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_unlock_req_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_unlock_req_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_unlock_req_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_unlock_notify_req(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_unlock_notify_req_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_unlock_notify_req_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_unlock_notify_req_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_unlock_notify_req_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_breathe(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_breathe_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_breathe_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_breathe_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_breathe_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_retire(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_retire_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_retire_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_retire_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_retire_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_recycle(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_recycle_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_recycle_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_recycle_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_recycle_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_flush(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_flush_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_flush_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_flush_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_flush_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_getsmf(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_getsmf_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_getsmf_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_getsmf_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_getsmf_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_dsmf(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_dsmf_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_dsmf_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_dsmf_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_dsmf_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_ddir(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_ddir_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_ddir_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_ddir_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_ddir_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_sexpire(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_sexpire_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_sexpire_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_sexpire_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_sexpire_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_post_update(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_update_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_update_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_update_post_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_update_post_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_post_renew(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_renew_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_renew_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_renew_post_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_renew_post_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_post_mexpire(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_mexpire_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_mexpire_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_mexpire_post_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_mexpire_post_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_post_mdsmf(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_mdsmf_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_mdsmf_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_mdsmf_post_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_mdsmf_post_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_post_mddir(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_mddir_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_mddir_post_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_mddir_post_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_mddir_post_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_logrotate(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_logrotate_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_logrotate_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_logrotate_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_logrotate_get_response(CHTTP_NODE *chttp_node);


EC_BOOL cxfshttp_is_http_get_actsyscfg(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_actsyscfg_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_actsyscfg_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_actsyscfg_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_actsyscfg_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_qtree(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_qtree_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_qtree_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_qtree_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_qtree_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_statusnp(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_statusnp_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_statusnp_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_statusnp_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_statusnp_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_statusdn(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_statusdn_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_statusdn_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_statusdn_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_statusdn_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_file_wait(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_file_wait_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_file_wait_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_file_wait_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_file_wait_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_file_notify(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_file_notify_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_file_notify_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_file_notify_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_file_notify_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_file_terminate(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_file_terminate_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_file_terminate_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_file_terminate_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_file_terminate_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_cond_wakeup(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_cond_wakeup_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_cond_wakeup_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_cond_wakeup_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_cond_wakeup_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_cond_terminate(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_cond_terminate_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_cond_terminate_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_cond_terminate_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_cond_terminate_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_renew_header(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_renew_header_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_renew_header_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_renew_header_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_renew_header_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_wait_header(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_wait_header_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_wait_header_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_wait_header_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_wait_header_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_locked_file_retire(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_locked_file_retire_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_locked_file_retire_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_locked_file_retire_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_locked_file_retire_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_xfs_up(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_up_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_xfs_up_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_xfs_up_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_up_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_xfs_down(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_down_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_xfs_down_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_xfs_down_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_down_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_xfs_add(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_add_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_xfs_add_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_xfs_add_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_add_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_xfs_del(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_del_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_xfs_del_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_xfs_del_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_del_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_xfs_list(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_list_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_xfs_list_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_xfs_list_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_xfs_list_get_response(CHTTP_NODE *chttp_node);

EC_BOOL cxfshttp_is_http_get_ngx_breathe(const CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_ngx_breathe_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_handle_ngx_breathe_get_request(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_make_ngx_breathe_get_response(CHTTP_NODE *chttp_node);
EC_BOOL cxfshttp_commit_ngx_breathe_get_response(CHTTP_NODE *chttp_node);

#endif /*_CXFSHTTP_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

