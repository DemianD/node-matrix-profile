#ifndef __NODE_SCAMP_ARGUMENTS_CASTER
#define __NODE_SCAMP_ARGUMENTS_CASTER

#include <iostream>
#include <vector>

#include "./utils.cpp"

using std::string;
using std::vector;

using SCAMP::SCAMPArgs;
using SCAMP::SCAMPProfileType;

// Based on https://github.com/zpzim/SCAMP/blob/017512a1df64765c955ed5f102a405acfa9562ec/src/main.cpp
class ArgumentsCaster {
    private:
        napi_env env;
        napi_value raw_arguments;
    
    public:
        ArgumentsCaster(napi_env _env, napi_callback_info info);

        void parse();
        void print();
        bool hasArgument(string propertyName);
        void parseUint64t(string propertyName, uint64_t &result, int defaultValue);
        void parseInt64t(string propertyName, int64_t &result, int defaultValue);
        void parseBool(string propertyName, bool &result, bool defaultValue);
        void parseDouble(string propertyName, double &result, double defaultValue);
        void parseDoubleArray(string propertyName, vector<double> &result);
        void parseString(string propertyName, string &result, string defaultValue);

        SCAMPArgs arguments;
};

ArgumentsCaster::ArgumentsCaster(napi_env _env, napi_callback_info info): env(_env) {
    napi_value fnArgs[1];
    size_t argsLength = 1;
    check(env, napi_get_cb_info(env, info, &argsLength, fnArgs, 0, 0));

    raw_arguments = std::move(fnArgs[0]);
    parse();
}

void ArgumentsCaster::parse() {
    bool self_join, reduce_all_neighbors, double_precision, mixed_precision, single_precision;
    int64_t reduced_height, reduced_width;
    string profile_type_string = "1NN_INDEX";

    parseUint64t("window_size", arguments.window, 100);
    parseUint64t("max_tile_size", arguments.max_tile_size, 1 << 17);

    parseDoubleArray("timeseries_a", arguments.timeseries_a);
    parseDoubleArray("timeseries_b", arguments.timeseries_b);

    parseBool("keep_rows_separate", arguments.keep_rows_separate, false);
    
    parseBool("double_precision", double_precision, false);
    parseBool("mixed_precision", mixed_precision, false);
    parseBool("single_precision", single_precision, false);
    parseBool("is_aligned", arguments.is_aligned, false);
    parseBool("silent_mode", arguments.silent_mode, true);
    parseBool("reduce_all_neighbors", reduce_all_neighbors, false);

    if(!double_precision && !mixed_precision && !single_precision) {
        double_precision = true;
    }

    if(arguments.timeseries_b.empty()) {
        self_join = true;
        arguments.computing_rows = true;
        arguments.computing_columns = true;
    }
    else {
        self_join = false;
        arguments.computing_columns = true;
        arguments.computing_rows = arguments.keep_rows_separate;
    }

    arguments.has_b = !self_join;
    
    parseInt64t("distributed_start_row", arguments.distributed_start_row, -1);
    parseInt64t("distributed_start_col", arguments.distributed_start_col, -1);
    parseInt64t("max_matches_per_column", arguments.max_matches_per_column, 100);
    parseInt64t("reduced_height", reduced_height, -1);
    parseInt64t("reduced_width", reduced_width, -1);
    parseDouble("distance_threshold", arguments.distance_threshold, std::nan("NaN"));

    arguments.precision_type = GetPrecisionType(double_precision, mixed_precision, single_precision);
    SCAMPProfileType profile_type = ParseProfileType(profile_type_string);

    arguments.profile_type = profile_type;

    int n_x = arguments.timeseries_a.size() - arguments.window + 1;
    int n_y = self_join ? n_x : arguments.timeseries_b.size() - arguments.window + 1;

    arguments.profile_a.type = profile_type;
    arguments.profile_a.matrix_height = reduce_all_neighbors ? reduced_height : -1.0;
    arguments.profile_a.matrix_width = reduce_all_neighbors ? reduced_width : -1.0;
    arguments.profile_a.matrix_reduced_cols = std::ceil(n_x / static_cast<double>(reduced_width));
    arguments.profile_a.matrix_reduced_rows = std::ceil(n_y / static_cast<double>(reduced_height));
    arguments.profile_a.output_matrix = reduce_all_neighbors;

    arguments.profile_b.type = profile_type;
    arguments.profile_b.matrix_height = reduce_all_neighbors ? reduced_height : -1.0;
    arguments.profile_b.matrix_width = reduce_all_neighbors ? reduced_width : -1.0;
    arguments.profile_b.matrix_reduced_cols = std::ceil(n_x / static_cast<double>(reduced_width));
    arguments.profile_b.matrix_reduced_rows = std::ceil(n_y / static_cast<double>(reduced_height));
    arguments.profile_b.output_matrix = reduce_all_neighbors;
}

void ArgumentsCaster::print() {
    arguments.print();
}

bool ArgumentsCaster::hasArgument(string propertyName) {
    bool hasArgument;
    check(env, napi_has_named_property(env, raw_arguments, propertyName.c_str(), &hasArgument));

    return hasArgument;
}

void ArgumentsCaster::parseUint64t(string propertyName, uint64_t &result, int defaultValue) {
    napi_value raw;
    int64_t signedResult;

    if(!hasArgument(propertyName)) {
        result = defaultValue;
    }
    else {
        check(env, napi_get_named_property(env, raw_arguments, propertyName.c_str(), &raw));
        check(env, napi_get_value_int64(env, raw, &signedResult));

        result = signedResult;
    }
}

void ArgumentsCaster::parseInt64t(string propertyName, int64_t &result, int defaultValue) {
    napi_value raw;

    if(!hasArgument(propertyName)) {
        result = defaultValue;
    }
    else {
        check(env, napi_get_named_property(env, raw_arguments, propertyName.c_str(), &raw));
        check(env, napi_get_value_int64(env, raw, &result));
    }
}

void ArgumentsCaster::parseBool(string propertyName, bool &result, bool defaultValue) {
    napi_value raw;

    if(!hasArgument(propertyName)) {
        result = defaultValue;
    }
    else {
        check(env, napi_get_named_property(env, raw_arguments, propertyName.c_str(), &raw));
        check(env, napi_get_value_bool(env, raw, &result));
    }
}

void ArgumentsCaster::parseDouble(string propertyName, double &result, double defaultValue) {
    napi_value raw;

    if(!hasArgument(propertyName)) {
        result = defaultValue;
    }
    else {
        check(env, napi_get_named_property(env, raw_arguments, propertyName.c_str(), &raw));
        check(env, napi_get_value_double(env, raw, &result));
    }
}

void ArgumentsCaster::parseDoubleArray(string propertyName, vector<double> &result) {
    uint32_t length;
    napi_value raw_array, raw;

    if(!hasArgument(propertyName)) {
        return;
    }

    check(env, napi_get_named_property(env, raw_arguments, propertyName.c_str(), &raw_array));
    check(env, napi_get_array_length(env, raw_array, &length));

    double value;
    for (unsigned int i = 0; i < length; i++) {
        napi_get_element(env, raw_array, i, &raw);
        napi_get_value_double(env, raw, &value);

        result.push_back(value);
    }
}

void ArgumentsCaster::parseString(string propertyName, string &result, string defaultValue) {
    napi_value raw;
    char* buf; 
    buf = (char*)calloc(255 + 1, sizeof(char));

    if(!hasArgument(propertyName)) {
        result = defaultValue;
    }
    else {
        check(env, napi_get_named_property(env, raw_arguments, propertyName.c_str(), &raw));
        check(env, napi_get_value_string_utf8(env, raw, buf, 255, 0));
        
        result = buf;
    }
}

#endif
