#include <node_api.h>
#include <vector>
#include <iostream>

#include "./scamp/src/SCAMP.cpp"
#include "./scamp/src/common.cpp"
#include "./scamp/src/cpu_stats.cpp"
#include "./scamp/src/tile.cpp"
#include "./scamp/src/qt_helper.cpp"
#include "./scamp/src/scamp_utils.cpp"
#include "./scamp/src/cpu_kernels.cpp"
#include "./scamp/src/kernel_common.cpp"

#include "./cast_arguments.cpp"
#include "./utils.cpp"

using SCAMP::do_SCAMP;

// Based on https://github.com/zpzim/SCAMP/blob/017512a1df64765c955ed5f102a405acfa9562ec/src/main.cpp
napi_value Method(napi_env env, napi_callback_info info) {
  bool output_pearson;
  std::vector<int> devices; 
  napi_value result, resultObject, matrix_profile, matrix_profile_index;
  
  ArgumentsCaster ac(env, info);

  InitProfileMemory(&ac.arguments);
  do_SCAMP(&ac.arguments, devices, 2);

  // Writing result back. TODO: support other profiles than PROFILE_TYPE_1NN_INDEX
  ac.parseBool("output_pearson", output_pearson, false);

  auto arr = ac.arguments.profile_a.data[0].uint64_value;
  int length = arr.size();

  check(env, napi_create_array_with_length(env, length - ac.arguments.window + 1, &matrix_profile));
  check(env, napi_create_array_with_length(env, length - ac.arguments.window + 1, &matrix_profile_index));
  
  for(int i = 0; i < arr.size(); i++) {
    SCAMP::mp_entry e;
    e.ulong = arr[i];

    output_pearson 
      ? napi_create_double(env, e.floats[0], &result)
      : napi_create_double(env, ConvertToEuclidean(e.floats[0], ac.arguments.window), &result);

    napi_set_element(env, matrix_profile, i, result);

    // If there is no match, set index to -1
    int index = e.floats[0] < -1 ? - 1 : e.ints[1] + 1;

    napi_create_int64(env, index, &result);
    napi_set_element(env, matrix_profile_index, i, result);
  }

  napi_create_object(env, &resultObject);
  napi_set_named_property(env, resultObject, "matrix_profile", matrix_profile);
  napi_set_named_property(env, resultObject, "matrix_profile_index", matrix_profile_index);

  return resultObject;
}

napi_value init(napi_env env, napi_value exports) {
  napi_value fn;

  // Create a new function
  check(env, napi_create_function(env, nullptr, 0, Method, nullptr, &fn));

  // Add the new function with name 'calculate'
  check(env, napi_set_named_property(env, exports, "calculate", fn));

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
