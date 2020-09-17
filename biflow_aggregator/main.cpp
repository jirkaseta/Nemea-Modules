/**
 * @file main.cpp
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief 
 * @version 0.1
 * @date 13.8.2020
 * 
 * @copyright Copyright (c) 2020 CESNET
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "aggregator.h"
#include "aggregator_functions.h"
#include "flat_hash_map.h"
#include "key_template.h"
#include "fields.h"
#include "configuration.h"
#include "linked_list.h"

#include <csignal>
#include <iostream>
#include <cstring>
#include <ctime>
#include <chrono>

#include <getopt.h>
#include <libtrap/trap.h>
#include <unirec/unirec.h>

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x),0)

#define TRAP_RECV_TIMEOUT 500000   // 0.5 second
#define TRAP_SEND_TIMEOUT 1000000   // 1 second

UR_FIELDS ( 
    time TIME_FIRST,
    time TIME_LAST,
    uint32 COUNT,
    ipaddr SRC_IP,
    ipaddr DST_IP
)

trap_module_info_t *module_info = NULL;
static int volatile stop = 0;

#define MODULE_BASIC_INFO(BASIC) \
    BASIC("biflow_aggregator", "TODO description.", 1, 1)

#define MODULE_PARAMS(PARAM) \
    PARAM('c', "config", "Configuration file in xml format.", required_argument, "filename") \
    PARAM('n', "name", "Name of config section.", required_argument, "name") \
    PARAM('e', "eof", "End when receive EOF.", no_argument, "flag") \
    PARAM('s', "size", "Max number of elements in flow cache.", required_argument, "number") \
    PARAM('a', "active-timeout", "Active timeout.", required_argument, "number") \
    PARAM('p', "passive-timeout", "Passive timeout.", required_argument, "number")

static void
termination_handler(const int signum) 
{
    if (signum == SIGTERM || signum == SIGINT) {
        std::cerr << "Signal " << signum << " caught, exiting module." << std::endl;
        stop = 1;
    }
}

static int
install_signal_handler(struct sigaction &sigbreak)
{
    static const int signum[] = {SIGINT, SIGTERM};

    sigbreak.sa_handler = termination_handler;
    sigemptyset(&sigbreak.sa_mask);
    sigbreak.sa_flags = 0;

    for (int i = 0; signum[i] != SIGTERM; i++) {
        if (sigaction(signum[i], &sigbreak, NULL) != 0) {
            std::cerr << "sigaction() error." << std::endl;
            return 1;
        }
    }
    return 0;
}

static bool 
send_record_out(ur_template_t *out_tmplt, void *out_rec)
{
    constexpr int max_try = 3;
    int ret;

    for (int i = 0; i < max_try; i++) {
        ret = trap_send(0, out_rec, ur_rec_fixlen_size(out_tmplt) + ur_rec_varlen_size(out_tmplt, out_rec));
        TRAP_DEFAULT_SEND_ERROR_HANDLING(ret, continue, break);
        return true;
    }
    std::cerr << "Cannot send record due to error or time_out" << std::endl;
    return false;
}

static void 
flush_single_flow(aggregator::Aggregator<FlowKey>& agg, const FlowKey& key, const aggregator::Flow_data& flow_data, ur_template_t *out_tmplt, void *out_rec) 
{
    ur_field_id_t field_id;
    aggregator::Field *field;
    std::size_t offset = 0;
    std::size_t elem_cnt;
    std::size_t size;
    void *key_data;

    std::tie(key_data, std::ignore) = key.get_key();

    // set mandatory fileds
    ur_set(out_tmplt, out_rec, F_TIME_FIRST, flow_data.time_first);
    ur_set(out_tmplt, out_rec, F_TIME_LAST, flow_data.time_last);
    ur_set(out_tmplt, out_rec, F_COUNT, flow_data.count);

    // Add key fields
    for (auto tmplt_field : Key_template::get_fields()) {  
        field_id = flow_data.reverse ? std::get<Key_template::REVERSE_ID>(tmplt_field) : std::get<Key_template::ID>(tmplt_field);
        std::memcpy(ur_get_ptr_by_id(out_tmplt, out_rec, field_id), std::addressof(static_cast<char *>(key_data)[offset]), ur_get_size(std::get<Key_template::ID>(tmplt_field)));
        offset += std::get<Key_template::SIZE>(tmplt_field);
    }

    // Add aggregated fields
    for (auto agg_field : agg.get_fields()) {
        field = std::addressof(agg_field.first);
        const void *agg_data = field->post_processing(&flow_data.data[agg_field.second], size, elem_cnt);
        if (ur_is_array(field->ur_field_id)) {
            ur_array_allocate(out_tmplt, out_rec, field->ur_field_id, elem_cnt);
            std::memcpy(ur_get_ptr_by_id(out_tmplt, out_rec, field->ur_field_id), agg_data, size * elem_cnt);
        } else {
            field_id = flow_data.reverse ? field->ur_field_reverse_id : field->ur_field_id;
            std::memcpy(ur_get_ptr_by_id(out_tmplt, out_rec, field_id), agg_data, size);
        }
    }
    (void) send_record_out(out_tmplt, out_rec);
}

static void 
flush_flow_cache(aggregator::Aggregator<FlowKey>& agg, ur_template_t *out_tmplt, void *out_rec)
{
    for (auto cache_data : agg.flow_cache) {
        flush_single_flow(agg, cache_data.first, cache_data.second, out_tmplt, out_rec); 
    }
}

static int process_format_change(
        Configuration& config,
        aggregator::Aggregator<FlowKey>& agg,
        ur_template_t *in_tmplt,
        ur_template_t **out_tmplt
        )
{
    ur_field_id_t field_id;
    ur_field_id_t reverse_field_id;

    agg.reset_fields();
    Key_template::reset();

    for (auto field_cfg : config.get_cfg_fields()) {
        field_id = ur_get_id_by_name(field_cfg.name.c_str());
        reverse_field_id = ur_get_id_by_name(field_cfg.reverse_name.c_str());

        if (!ur_is_present(in_tmplt, field_id)) {
            std::cerr << "Requested field " << field_cfg.name << " is not in input records, cannot continue." << std::endl;
            return 1;
        }

        if (reverse_field_id != UR_E_INVALID_NAME && !ur_is_present(in_tmplt, reverse_field_id)) {
            std::cerr << "Requested field " << field_cfg.reverse_name << " is not in input records, cannot continue." << std::endl;
            return 1;
        } else if (reverse_field_id == UR_E_INVALID_NAME) {
            reverse_field_id = field_id;
        } else {
            if (ur_get_size(field_id) != ur_get_size(reverse_field_id)) {
                std::cerr << "Name and reverse name field size is not equal, cannot continue." << std::endl;
                return 1;
            }
        }

        if (field_cfg.type == aggregator::KEY) {
            Key_template::update(field_id, reverse_field_id, ur_get_size(field_id));
        } else {
            aggregator::Field field(field_cfg, field_id, reverse_field_id);
            agg.add_field(field); 
        }
        if (field_cfg.to_output)
            config.add_field_to_template(field_cfg.name);
    }

    *out_tmplt = ur_create_output_template(0, config.out_tmplt.c_str(), NULL);
    if (*out_tmplt == NULL) {
        std::cerr << "Error: Output template could not be created." << std::endl;
        return 1;
    }

    return 0;
}

static int 
do_mainloop(Configuration& config)
{
    aggregator::Aggregator<FlowKey> agg = {};
    ur_template_t *in_tmplt;
    ur_template_t *out_tmplt = NULL;

    aggregator::Field *field;

    void *out_rec = NULL;

    FlowKey key;

    uint16_t flow_size;
    const void *flow_data;
    int recv_code;
    uint32_t cnt = 0;
    std::chrono::nanoseconds current_time;
    std::size_t t_passive = config.t_passive;
    std::size_t t_active = config.t_active;

    Dll<aggregator::Timeout_data> dll;
    agg.flow_cache.reserve(1000000);

    /* **** Create UniRec templates **** */
    in_tmplt = ur_create_input_template(0, "TIME_FIRST,TIME_LAST", NULL);
    if (in_tmplt == NULL) {
        std::cerr << "Error: Input template could not be created." << std::endl;
        return 1;
    }

    while (unlikely(stop == false)) {
        current_time = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());

        // Check timeouted flows
        for (node<aggregator::Timeout_data> *t_data = dll.begin(); t_data != NULL; t_data = t_data->next) {
            if (current_time >= t_data->value.timeout) { // timeouted
                auto data = agg.flow_cache.find(t_data->value.key);
                flush_single_flow(agg, t_data->value.key, data->second, out_tmplt, out_rec);
                agg.flow_cache.erase(data);
                dll.deleteFirst();
            } else
                break;
        }

        recv_code = TRAP_RECEIVE(0, flow_data, flow_size, in_tmplt);
        TRAP_DEFAULT_RECV_ERROR_HANDLING(recv_code, continue, break);

        if (unlikely(flow_size <= 1 && config.break_when_eof)) {
            stop = 1;
            break;
        }

        if (unlikely(TRAP_E_FORMAT_CHANGED == recv_code)) {
            if (process_format_change(config, agg, in_tmplt, std::addressof(out_tmplt)) != 0) {
                stop = 1;
                break;
            }
            out_rec = ur_create_record(out_tmplt, UR_MAX_SIZE);
            if (out_rec == NULL) {
                std::cerr << "Error: Output record could not be created." << std::endl;
                stop = 1;
                break;
            }
        }

        // Generate flow key
        bool is_key_reversed = key.generate(flow_data, in_tmplt, config.is_biflow_key);

        aggregator::Flow_data in_flow_data;

        auto insered_data = agg.flow_cache.insert(std::move(key), in_flow_data);
        if (insered_data.second == true) { // New key in cache
            (*insered_data.first).second.data = static_cast<char *>(agg.allocate_memory());
            node<aggregator::Timeout_data> *timeout_data = std::addressof((*insered_data.first).second.t_data);
            timeout_data->value.key = ((*insered_data.first).first);
            timeout_data->value.active_timeout = current_time + seconds(t_active); 
            timeout_data->value.timeout = current_time + seconds(t_passive);
            dll.insert(timeout_data);
        } else { // already inserted
            node<aggregator::Timeout_data> *timeout_data = std::addressof((*insered_data.first).second.t_data);
            if (current_time + seconds(t_passive) < timeout_data->value.active_timeout)
                timeout_data->value.timeout = current_time + seconds(t_passive);
            else
                timeout_data->value.timeout = timeout_data->value.active_timeout;
            dll.swap(timeout_data);
        }

        aggregator::Flow_data *cache_data = static_cast<aggregator::Flow_data *>(std::addressof((*insered_data.first).second));
        if (insered_data.second == true) {
            if (ur_is_present(in_tmplt, F_COUNT))
                cache_data->update(ur_get(in_tmplt, flow_data, F_COUNT));
            else
                cache_data->update(0);
        }
        for (auto agg_field : agg.get_fields()) {
            field = std::addressof(agg_field.first);
            if (ur_is_array(field->ur_field_id)) {
                aggregator::ur_array_data src_data;
                src_data.cnt_elements = ur_array_get_elem_cnt(in_tmplt, flow_data, field->ur_field_id);
                src_data.ptr_first = ur_get_ptr_by_id(in_tmplt, flow_data, field->ur_field_id);
                if (field->type == aggregator::SORTED_MERGE) {
                    src_data.sort_key = ur_get_ptr_by_id(in_tmplt, flow_data, field->ur_sort_key_id);
                    if (ur_is_array(field->ur_sort_key_id))
                        src_data.sort_key_elements = ur_array_get_elem_cnt(in_tmplt, flow_data, field->ur_sort_key_id);
                    else
                        src_data.sort_key_elements = 1;

                }
                field->aggregate(std::addressof(src_data), std::addressof(cache_data->data[agg_field.second]));
            } else {
                ur_field_id_t field_id = is_key_reversed ? field->ur_field_reverse_id : field->ur_field_id;
                field->aggregate(ur_get_ptr_by_id(in_tmplt, flow_data, field_id), std::addressof(cache_data->data[agg_field.second]));
            }
        }

        cache_data->update(ur_get(in_tmplt, flow_data, F_TIME_FIRST), ur_get(in_tmplt, flow_data, F_TIME_LAST), is_key_reversed);
        cnt++;
    }
 
    std::cout << "COUNTER: " << cnt << " R: " << "  " << agg.flow_cache.size() << "\n";

    // flush whole cache
    flush_flow_cache(agg, out_tmplt, out_rec);

    // clear resources
    ur_free_record(out_rec);
    ur_free_template(in_tmplt);
    ur_free_template(out_tmplt);
    return 0;
}


int
main(int argc, char **argv)
{
    struct sigaction sigbreak;
    Configuration config;
    int ret;
    char opt;
    char *cfg_name = NULL;
    char *cfg_path = NULL;

    // Macro allocates and initializes module_info structure according to MODULE_BASIC_INFO.
    INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);

    // Let TRAP library parse program arguments, extract its parameters and initialize module interfaces
    TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

    // Install SIGINT and SIGTERN signal handler. 
    install_signal_handler(sigbreak);

    while ((opt = TRAP_GETOPT(argc, argv, module_getopt_string, long_options)) != -1) {
        ret = 0;
        switch (opt) {
        case 'a':
            ret = config.set_active_timeout(optarg);
            break;
        case 'p':
            ret = config.set_passive_timeout(optarg);
            break;
        case 'n':
            cfg_name = optarg;
            break;
        case 'c':
            cfg_path = optarg;
            break;
        case 'e':
            config.set_eof_break();
            break;
        case 's':
            ret = config.set_flow_cache_size(optarg);
            break;
        default:
            std::cerr << "Invalid argument " << opt << ", skipped..." << std::endl;
        }
        if (ret != 0)
            break;
    }

    if (ret != 0) {
        goto failure;
    }

    if (!cfg_path || !cfg_name) {
        std::cerr << "Config filename or config section missing." << std::endl;
        goto failure;
    }

    if (config.parse_xml(cfg_path, cfg_name) != 0)
        goto failure;

    if (config.t_passive > config.t_active) {
        std::cerr << "Passive timeout cannot be bigger than active timeout." << std::endl;
        goto failure;
    }

    // Set TRAP_RECEIVE() timeout to TRAP_RECV_TIMEOUT/1000000 seconds
    trap_ifcctl(TRAPIFC_INPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_RECV_TIMEOUT);

    // Set TRAP_RECEIVE() timeout to TRAP_RECV_TIMEOUT/1000000 seconds
    //trap_ifcctl(TRAPIFC_OUTPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_SEND_TIMEOUT);

    ret = do_mainloop(config);
    if (ret != 0)
        goto failure;

    FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
    TRAP_DEFAULT_FINALIZATION();
    return 0;

failure:
    FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);
    TRAP_DEFAULT_FINALIZATION();
    return 1;
}
