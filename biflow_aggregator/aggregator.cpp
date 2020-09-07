/**
 * @file agregator_fields.cpp
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief 
 * @version 0.1
 * @date 31.8.2020
 *   
 * @copyright Copyright (c) 2020 CESNET
 */

#include "aggregator.h"
#include "aggregator_functions.h"

#include <limits>
#include <cassert>

using namespace aggregator;

void Flow_data::update(const time_t time_first, const time_t time_last) noexcept
{
    cnt++;
    if (first > time_first)
        first = time_first;
    if (last < time_last)
        last = time_last;
}

Flow_data::Flow_data()
{
    cnt = 0;
    last = 0;
    first = std::numeric_limits<time_t>::max();
}

const void *Field::post_processing(void *ag_data, std::size_t& typename_size, std::size_t& elem_cnt)
{
    typename_size = this->typename_size;
    if (post_proc_fnc)
        return post_proc_fnc(ag_data, elem_cnt);

    elem_cnt = 1;
    return static_cast<const void *>(ag_data);
}

void Field::aggregate(const void *src, void *dst)
{
    ag_fnc(src, dst);
}

void Field::init(void *tmplt_mem, const void *cfg)
{
    init_fnc(tmplt_mem, cfg);
}

template<typename T, typename K>
int Field_template::assign() noexcept
{
    ag_fnc = sorted_append<T, K>;
    post_proc_fnc = Sorted_append_data<T, K>::postprocessing;
    typename_size = sizeof(T);
    init_fnc = Sorted_append_data<T, K>::init;
    ag_data_size = sizeof(Sorted_append_data<T, K>);
    return 0;
}

template<typename T>
int Field_template::assign() noexcept
{
    ag_fnc = bitwise_and<T>;
    post_proc_fnc = nullptr;
    typename_size = sizeof(T);
    init_fnc = Basic_data<T>::init;
    ag_data_size = sizeof(Basic_data<T>);
    return 0;
}

template<Field_type ag_type, typename T>
int Field_template::assign() noexcept
{
    typename_size = sizeof(T);

    if (ag_type == SUM) {
        ag_fnc = sum<T>;
        post_proc_fnc = nullptr;
        init_fnc = Basic_data<T>::init;
        ag_data_size = sizeof(Basic_data<T>);
    } else if (ag_type == MIN) {
        ag_fnc = min<T>;
        post_proc_fnc = nullptr;
        init_fnc = Basic_data<T>::init;
        ag_data_size = sizeof(Basic_data<T>);
    } else if (ag_type == MAX) {
        ag_fnc = max<T>;
        post_proc_fnc = nullptr;
        init_fnc = Basic_data<T>::init;
        ag_data_size = sizeof(Basic_data<T>);
    } else if (ag_type == AVG) {
        ag_fnc = avg<T>;
        post_proc_fnc = Average_data<T>::postprocessing;
        init_fnc = Average_data<T>::init;
        ag_data_size = sizeof(Average_data<T>);
    } else if (ag_type == APPEND) {
        ag_fnc = append<T>;
        post_proc_fnc = Append_data<T>::postprocessing;
        init_fnc = Append_data<T>::init;
        ag_data_size = sizeof(Append_data<T>);
    } else {
        assert("Invalid Field type.");
        return 1;
    }
    return 0; 
}

int Field_template::set_templates(const Field_type ag_type, const ur_field_type_t ur_f_type)
{
    switch (ag_type) {
    case SUM:
        switch (ur_f_type) {
        case UR_TYPE_CHAR:   return assign<SUM, char>();
        case UR_TYPE_UINT8:  return assign<SUM, uint8_t>();
        case UR_TYPE_INT8:   return assign<SUM, int8_t>();
        case UR_TYPE_UINT16: return assign<SUM, uint16_t>();
        case UR_TYPE_INT16:  return assign<SUM, int16_t>();
        case UR_TYPE_UINT32: return assign<SUM, uint32_t>();
        case UR_TYPE_INT32:  return assign<SUM, int32_t>();
        case UR_TYPE_UINT64: return assign<SUM, uint64_t>();
        case UR_TYPE_INT64:  return assign<SUM, int64_t>();
        case UR_TYPE_FLOAT:  return assign<SUM, float>();
        case UR_TYPE_DOUBLE: return assign<SUM, double>();
        default:
            std::cerr << "Only char, int, uint, float and double can be used to SUM function." << std::endl;
            return 1;
        }
    case MIN:
        switch (ur_f_type) {
        case UR_TYPE_CHAR:   return assign<MIN, char>();
        case UR_TYPE_UINT8:  return assign<MIN, uint8_t>();
        case UR_TYPE_INT8:   return assign<MIN, int8_t>();
        case UR_TYPE_UINT16: return assign<MIN, uint16_t>();
        case UR_TYPE_INT16:  return assign<MIN, int16_t>();
        case UR_TYPE_UINT32: return assign<MIN, uint32_t>();
        case UR_TYPE_INT32:  return assign<MIN, int32_t>();
        case UR_TYPE_UINT64: return assign<MIN, uint64_t>();
        case UR_TYPE_INT64:  return assign<MIN, int64_t>();
        case UR_TYPE_FLOAT:  return assign<MIN, float>();
        case UR_TYPE_DOUBLE: return assign<MIN, double>();
        case UR_TYPE_TIME:   return assign<MIN, time_t>();
        case UR_TYPE_IP:     return assign<MIN, uint128_t>();
        default:
            std::cerr << "Only char, int, uint, float, double and ip can be used to MIN function." << std::endl;
            return 1;
        }
    case MAX:
        switch (ur_f_type) {
        case UR_TYPE_CHAR:   return assign<MAX, char>();
        case UR_TYPE_UINT8:  return assign<MAX, uint8_t>();
        case UR_TYPE_INT8:   return assign<MAX, int8_t>();
        case UR_TYPE_UINT16: return assign<MAX, uint16_t>();
        case UR_TYPE_INT16:  return assign<MAX, int16_t>();
        case UR_TYPE_UINT32: return assign<MAX, uint32_t>();
        case UR_TYPE_INT32:  return assign<MAX, int32_t>();
        case UR_TYPE_UINT64: return assign<MAX, uint64_t>();
        case UR_TYPE_INT64:  return assign<MAX, int64_t>();
        case UR_TYPE_FLOAT:  return assign<MAX, float>();
        case UR_TYPE_DOUBLE: return assign<MAX, double>();
        case UR_TYPE_TIME:   return assign<MAX, time_t>();
        case UR_TYPE_IP:     return assign<MAX, uint128_t>();
        default:
            std::cerr << "Only char, int, uint, float, double and ip can be used to MAX function." << std::endl;
            return 1;
        }
    case BIT_AND:
        switch (ur_f_type) {
        case UR_TYPE_CHAR:   return assign<char>();
        case UR_TYPE_UINT8:  return assign<uint8_t>();
        case UR_TYPE_INT8:   return assign<int8_t>();
        case UR_TYPE_UINT16: return assign<uint16_t>();
        case UR_TYPE_INT16:  return assign<int16_t>();
        case UR_TYPE_UINT32: return assign<uint32_t>();
        case UR_TYPE_INT32:  return assign<int32_t>();
        case UR_TYPE_UINT64: return assign<uint64_t>();
        case UR_TYPE_INT64:  return assign<int64_t>();
        default:
            std::cerr << "Only char, int and uint can be used to BIT AND function." << std::endl;
            return 1;
        }
    default:
        assert("Invalid case option.\n");
        return 1;
    }
}

// TODO sorted append ur_array ur_array
int Field_template::set_templates(const ur_field_type_t ur_f_type, const ur_field_type_t ur_sort_key_f_type)
{
    switch (ur_f_type) {
    case UR_TYPE_A_UINT8:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<uint8_t, char>();
        case UR_TYPE_UINT8:  return assign<uint8_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<uint8_t, int8_t>();
        case UR_TYPE_UINT16: return assign<uint8_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<uint8_t, int16_t>();
        case UR_TYPE_UINT32: return assign<uint8_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<uint8_t, int32_t>();
        case UR_TYPE_UINT64: return assign<uint8_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<uint8_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<uint8_t, float>();
        case UR_TYPE_DOUBLE: return assign<uint8_t, double>();
        case UR_TYPE_TIME:   return assign<uint8_t, time_t>();
        case UR_TYPE_IP:     return assign<uint8_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_INT8:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<int8_t, char>();
        case UR_TYPE_UINT8:  return assign<int8_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<int8_t, int8_t>();
        case UR_TYPE_UINT16: return assign<int8_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<int8_t, int16_t>();
        case UR_TYPE_UINT32: return assign<int8_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<int8_t, int32_t>();
        case UR_TYPE_UINT64: return assign<int8_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<int8_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<int8_t, float>();
        case UR_TYPE_DOUBLE: return assign<int8_t, double>();
        case UR_TYPE_TIME:   return assign<int8_t, time_t>();
        case UR_TYPE_IP:     return assign<int8_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_UINT16:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<uint16_t, char>();
        case UR_TYPE_UINT8:  return assign<uint16_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<uint16_t, int8_t>();
        case UR_TYPE_UINT16: return assign<uint16_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<uint16_t, int16_t>();
        case UR_TYPE_UINT32: return assign<uint16_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<uint16_t, int32_t>();
        case UR_TYPE_UINT64: return assign<uint16_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<uint16_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<uint16_t, float>();
        case UR_TYPE_DOUBLE: return assign<uint16_t, double>();
        case UR_TYPE_TIME:   return assign<uint16_t, time_t>();
        case UR_TYPE_IP:     return assign<uint16_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_INT16:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<int16_t, char>();
        case UR_TYPE_UINT8:  return assign<int16_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<int16_t, int8_t>();
        case UR_TYPE_UINT16: return assign<int16_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<int16_t, int16_t>();
        case UR_TYPE_UINT32: return assign<int16_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<int16_t, int32_t>();
        case UR_TYPE_UINT64: return assign<int16_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<int16_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<int16_t, float>();
        case UR_TYPE_DOUBLE: return assign<int16_t, double>();
        case UR_TYPE_TIME:   return assign<int16_t, time_t>();
        case UR_TYPE_IP:     return assign<int16_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_UINT32:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<uint32_t, char>();
        case UR_TYPE_UINT8:  return assign<uint32_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<uint32_t, int8_t>();
        case UR_TYPE_UINT16: return assign<uint32_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<uint32_t, int16_t>();
        case UR_TYPE_UINT32: return assign<uint32_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<uint32_t, int32_t>();
        case UR_TYPE_UINT64: return assign<uint32_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<uint32_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<uint32_t, float>();
        case UR_TYPE_DOUBLE: return assign<uint32_t, double>();
        case UR_TYPE_TIME:   return assign<uint32_t, time_t>();
        case UR_TYPE_IP:     return assign<uint32_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_INT32:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<int32_t, char>();
        case UR_TYPE_UINT8:  return assign<int32_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<int32_t, int8_t>();
        case UR_TYPE_UINT16: return assign<int32_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<int32_t, int16_t>();
        case UR_TYPE_UINT32: return assign<int32_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<int32_t, int32_t>();
        case UR_TYPE_UINT64: return assign<int32_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<int32_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<int32_t, float>();
        case UR_TYPE_DOUBLE: return assign<int32_t, double>();
        case UR_TYPE_TIME:   return assign<int32_t, time_t>();
        case UR_TYPE_IP:     return assign<int32_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_UINT64:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<uint64_t, char>();
        case UR_TYPE_UINT8:  return assign<uint64_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<uint64_t, int8_t>();
        case UR_TYPE_UINT16: return assign<uint64_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<uint64_t, int16_t>();
        case UR_TYPE_UINT32: return assign<uint64_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<uint64_t, int32_t>();
        case UR_TYPE_UINT64: return assign<uint64_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<uint64_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<uint64_t, float>();
        case UR_TYPE_DOUBLE: return assign<uint64_t, double>();
        case UR_TYPE_TIME:   return assign<uint64_t, time_t>();
        case UR_TYPE_IP:     return assign<uint64_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_INT64:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<int64_t, char>();
        case UR_TYPE_UINT8:  return assign<int64_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<int64_t, int8_t>();
        case UR_TYPE_UINT16: return assign<int64_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<int64_t, int16_t>();
        case UR_TYPE_UINT32: return assign<int64_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<int64_t, int32_t>();
        case UR_TYPE_UINT64: return assign<int64_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<int64_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<int64_t, float>();
        case UR_TYPE_DOUBLE: return assign<int64_t, double>();
        case UR_TYPE_TIME:   return assign<int64_t, time_t>();
        case UR_TYPE_IP:     return assign<int64_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_FLOAT:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<float, char>();
        case UR_TYPE_UINT8:  return assign<float, uint8_t>();
        case UR_TYPE_INT8:   return assign<float, int8_t>();
        case UR_TYPE_UINT16: return assign<float, uint16_t>();
        case UR_TYPE_INT16:  return assign<float, int16_t>();
        case UR_TYPE_UINT32: return assign<float, uint32_t>();
        case UR_TYPE_INT32:  return assign<float, int32_t>();
        case UR_TYPE_UINT64: return assign<float, uint64_t>();
        case UR_TYPE_INT64:  return assign<float, int64_t>();
        case UR_TYPE_FLOAT:  return assign<float, float>();
        case UR_TYPE_DOUBLE: return assign<float, double>();
        case UR_TYPE_TIME:   return assign<float, time_t>();
        case UR_TYPE_IP:     return assign<float, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_DOUBLE:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<double, char>();
        case UR_TYPE_UINT8:  return assign<double, uint8_t>();
        case UR_TYPE_INT8:   return assign<double, int8_t>();
        case UR_TYPE_UINT16: return assign<double, uint16_t>();
        case UR_TYPE_INT16:  return assign<double, int16_t>();
        case UR_TYPE_UINT32: return assign<double, uint32_t>();
        case UR_TYPE_INT32:  return assign<double, int32_t>();
        case UR_TYPE_UINT64: return assign<double, uint64_t>();
        case UR_TYPE_INT64:  return assign<double, int64_t>();
        case UR_TYPE_FLOAT:  return assign<double, float>();
        case UR_TYPE_DOUBLE: return assign<double, double>();
        case UR_TYPE_TIME:   return assign<double, time_t>();
        case UR_TYPE_IP:     return assign<double, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_IP:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<uint128_t, char>();
        case UR_TYPE_UINT8:  return assign<uint128_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<uint128_t, int8_t>();
        case UR_TYPE_UINT16: return assign<uint128_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<uint128_t, int16_t>();
        case UR_TYPE_UINT32: return assign<uint128_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<uint128_t, int32_t>();
        case UR_TYPE_UINT64: return assign<uint128_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<uint128_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<uint128_t, float>();
        case UR_TYPE_DOUBLE: return assign<uint128_t, double>();
        case UR_TYPE_TIME:   return assign<uint128_t, time_t>();
        case UR_TYPE_IP:     return assign<uint128_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_MAC:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<mac_addr_t, char>();
        case UR_TYPE_UINT8:  return assign<mac_addr_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<mac_addr_t, int8_t>();
        case UR_TYPE_UINT16: return assign<mac_addr_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<mac_addr_t, int16_t>();
        case UR_TYPE_UINT32: return assign<mac_addr_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<mac_addr_t, int32_t>();
        case UR_TYPE_UINT64: return assign<mac_addr_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<mac_addr_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<mac_addr_t, float>();
        case UR_TYPE_DOUBLE: return assign<mac_addr_t, double>();
        case UR_TYPE_TIME:   return assign<mac_addr_t, time_t>();
        case UR_TYPE_IP:     return assign<mac_addr_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_A_TIME:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<time_t, char>();
        case UR_TYPE_UINT8:  return assign<time_t, uint8_t>();
        case UR_TYPE_INT8:   return assign<time_t, int8_t>();
        case UR_TYPE_UINT16: return assign<time_t, uint16_t>();
        case UR_TYPE_INT16:  return assign<time_t, int16_t>();
        case UR_TYPE_UINT32: return assign<time_t, uint32_t>();
        case UR_TYPE_INT32:  return assign<time_t, int32_t>();
        case UR_TYPE_UINT64: return assign<time_t, uint64_t>();
        case UR_TYPE_INT64:  return assign<time_t, int64_t>();
        case UR_TYPE_FLOAT:  return assign<time_t, float>();
        case UR_TYPE_DOUBLE: return assign<time_t, double>();
        case UR_TYPE_TIME:   return assign<time_t, time_t>();
        case UR_TYPE_IP:     return assign<time_t, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    case UR_TYPE_STRING:
        switch (ur_sort_key_f_type) {
        case UR_TYPE_CHAR:   return assign<char, char>();
        case UR_TYPE_UINT8:  return assign<char, uint8_t>();
        case UR_TYPE_INT8:   return assign<char, int8_t>();
        case UR_TYPE_UINT16: return assign<char, uint16_t>();
        case UR_TYPE_INT16:  return assign<char, int16_t>();
        case UR_TYPE_UINT32: return assign<char, uint32_t>();
        case UR_TYPE_INT32:  return assign<char, int32_t>();
        case UR_TYPE_UINT64: return assign<char, uint64_t>();
        case UR_TYPE_INT64:  return assign<char, int64_t>();
        case UR_TYPE_FLOAT:  return assign<char, float>();
        case UR_TYPE_DOUBLE: return assign<char, double>();
        case UR_TYPE_TIME:   return assign<char, time_t>();
        case UR_TYPE_IP:     return assign<char, uint128_t>();
        default: 
            std::cerr << "Only char, int, uint, float, double, ip, mac and time can be used as SORTED_APPEND key." << std::endl;
            return 1;
        }
        break;
    default:
        std::cerr << "TODO." << std::endl;
        return 1;
    }
}

Field::Field(const Field_config cfg, const ur_field_id_t field_id)
{
    ur_field_type_t ur_field_type = ur_get_type(field_id);

    name = cfg.name;
    type = cfg.type;
    sort_name = cfg.sort_name;
    delimiter = cfg.delimiter;
    limit = cfg.limit;
    ur_field_id = field_id;

    if (type == SORTED_APPEND) {
        ur_sort_key_id = ur_get_id_by_name(sort_name.c_str());
        if (ur_sort_key_id == UR_E_INVALID_NAME) {
            throw std::runtime_error("Invalid sort key type.");
        }
        ur_sort_key_type = ur_get_type(ur_sort_key_id);
        if (set_templates(ur_field_type, ur_sort_key_type))
            throw std::runtime_error("Cannot set field template.");
    } else {
        if (set_templates(type, ur_field_type))
            throw std::runtime_error("Cannot set field template.");
    }
}


void Fields::add_field(Field field)
{
    fields_.emplace_back(std::make_pair(field, offset_));
    offset_ += field.ag_data_size;
}

void Fields::reset_fields() noexcept
{
    fields_.clear();
    offset_ = 0;
}

std::vector<std::pair<Field, std::size_t>> Fields::get_fields() noexcept 
{
    return fields_;
}

void* Fields::allocate_memory()
{
    void *m_ptr = new char[offset_](); // TODO PERF: Pre-allocated buffer
    void *memory = m_ptr;
    allocated_pointers_.emplace_back(m_ptr);

    for (auto data : fields_) {
        switch (data.first.type) {
        case SUM:
        case MAX:
        case BIT_AND:
        case AVG:
            data.first.init(memory, nullptr);
            break;
        case MIN:
            data.first.init(memory, memory);
            break;
        case APPEND: {
            struct Config_append cfg = {data.first.limit, data.first.delimiter};
            data.first.init(memory, &cfg);
            break;
        }
        case SORTED_APPEND: {
            struct Config_sorted_append cfg = {data.first.limit, data.first.delimiter, data.first.sort_type};
            data.first.init(memory, &cfg);
            break;
        }
        default:
            assert("Invalid case option.\n");
        }
        memory = ((char *)memory) + data.first.ag_data_size;
    }
    return m_ptr;
}
