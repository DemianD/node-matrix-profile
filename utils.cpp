#ifndef __NODE_SCAMP_UTILS
#define __NODE_SCAMP_UTILS

void check(napi_env env, napi_status status) {
  if (status != napi_ok) {    
    const napi_extended_error_info* error_info = NULL; 
    napi_get_last_error_info(env, &error_info);
    napi_throw_error(env, NULL, error_info->error_message); 
  }
}

#endif
