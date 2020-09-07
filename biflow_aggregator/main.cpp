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

#include <csignal>
#include <iostream>

#include <getopt.h>
#include <libtrap/trap.h>
#include <unirec/unirec.h>

#include "flat_hash_map.h"
#include "key_template.h"
#include "aggregator.h"
#include "aggregator_functions.h"
#include "ipaddr.h"
#include "fields.h"
#include "configuration.h"

using namespace std;

#define TRAP_RECV_TIMEOUT 500000   // 0.5 second
#define TRAP_SEND_TIMEOUT 1000000   // 1 second

UR_FIELDS ( 
  time TIME_FIRST,
  time TIME_LAST,
  uint32 COUNT
)

trap_module_info_t *module_info = NULL;
static int volatile stop = 0;

#define MODULE_BASIC_INFO(BASIC) \
    BASIC("biflow_aggregator", "TODO description.", 1, 1)

#define MODULE_PARAMS(PARAM) \
    PARAM('c', "config", "Configuration file in xml format.", required_argument, "filename") \
    PARAM('t', "time_window", "Represents type of timeout and #seconds for given type before sending " \
        "record to output. Use as [G,A,P]:#seconds or M:#Active,#Passive (eg. -t \"m:10,25\"). " \
        "When not specified the default value (A:10) is used.", required_argument, "string") 


static void
termination_handler(const int signum) 
{
    if (signum == SIGTERM || signum == SIGINT) {
        cerr << "Signal " << signum << " caught, exiting module." << endl;
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
            cerr << "sigaction() error." << endl;
            return 1;
        }
    }
    return 0;
}

static int 
process_format_change(Configuration& config, ur_template_t *in_tmplt, aggregator::Aggregator<FlowKey>& agg)
{
    ur_field_id_t tmplt_id;
    ur_field_id_t field_id;
    bool found;

    agg.reset_fields();
    agg.reset_template();

    for (auto field_cfg : config.get_cfg_fields()) {
        tmplt_id = UR_ITER_BEGIN;
        found = false;

        field_id = ur_get_id_by_name(field_cfg.name.c_str());
        while ((tmplt_id = ur_iter_fields(in_tmplt, tmplt_id)) != UR_ITER_END) {
            if (field_id == tmplt_id) {
                found = true;
                break;
            }
        }

        if (field_cfg.type == aggregator::KEY) {
            if (found == false) {
                std::cerr << "Requested field " << field_cfg.name << " not in input records, cannot continue." << std::endl;
                return 1;
            } else {
                agg.update_template(field_id, ur_get_size(field_id));
                config.add_field_to_template(field_cfg.name);
            }
        } else {
            if (found == false) {
                std::cerr << "Requested field " << field_cfg.name << " not in input records, skip its aggregation." << std::endl;
                continue;
            } else {
                aggregator::Field field(field_cfg, field_id);
                agg.add_field(field); // std::forward
                config.add_field_to_template(field_cfg.name);
            }
        }
    }
    return 0;
}

bool send_record_out(ur_template_t *out_tmplt, void *out_rec)
{
    for (int i = 0; i < 3; i++) {
        int ret = trap_send(0, out_rec, ur_rec_fixlen_size(out_tmplt) + ur_rec_varlen_size(out_tmplt, out_rec));
        TRAP_DEFAULT_SEND_ERROR_HANDLING(ret, continue, break);
        return true;
    }
    std::cerr << "Cannot send record due to error or time_out\n";
    return false;
}

void create_record()
{

}

void flush_flow_cache(aggregator::Aggregator<FlowKey>& agg, ur_template_t *out_tmplt)
{
    static int a = 0;
    char *out_rec;
    if (!a) {
        out_rec = (char *)ur_create_record(out_tmplt, UR_MAX_SIZE);
        a++;
    }
    uint32_t cnt = 0;
    for (auto cache_data = agg.flow_cache.begin(); cache_data != agg.flow_cache.end(); cache_data++) {
        cnt++;
        FlowKey *flow_key = &cache_data->first;
        aggregator::Flow_data *flow_data = &cache_data->second;
        auto key = flow_key->get_key();
        ur_set(out_tmplt, out_rec, F_TIME_FIRST, flow_data->first);
        ur_set(out_tmplt, out_rec, F_TIME_LAST, flow_data->last);
        ur_set(out_tmplt, out_rec, F_COUNT, flow_data->cnt);
        std::size_t offset = 0;
        for (auto tmplt_field : agg.get_template_fields()) {
            memcpy((void *)&out_rec[out_tmplt->offset[tmplt_field.first]], &((char *)key.first)[offset], ur_get_size(tmplt_field.first));
            offset += tmplt_field.second;
        }
        for (auto field : agg.get_fields()) {
            std::size_t size;
            std::size_t elem_cnt;
            const void *ptr = field.first.post_processing(&flow_data->data[field.second], size, elem_cnt);
            if (ur_is_array(field.first.ur_field_id)) {
                ur_array_allocate(out_tmplt, out_rec, field.first.ur_field_id, elem_cnt);
                void *p = ur_get_ptr_by_id(out_tmplt, out_rec, field.first.ur_field_id);
                memcpy(p, ptr, size * elem_cnt);
            }
            else
                memcpy((void *)&out_rec[out_tmplt->offset[field.first.ur_field_id]], ptr, size);
        }
        send_record_out(out_tmplt, out_rec);



       // 
    }
    std::cout << "UNIQUE KEYS: " << cnt<<"\n";
}

static int 
do_mainloop(Configuration& config, ur_template_t *in_tmplt)
{
    uint16_t in_rec_size;
    const void *in_rec;
    int ret;

    aggregator::Aggregator<FlowKey> agg = {}; 
    ur_template_t *out_tmplt;
    FlowKey key;
    uint32_t cnt = 0;

    while (!stop) {
        ret = TRAP_RECEIVE(0, in_rec, in_rec_size, in_tmplt);
        
        // Handle possible errors
        TRAP_DEFAULT_RECV_ERROR_HANDLING(ret, continue, break);

        // Check for end-of-stream message, close only when EOF caught
        if (in_rec_size <= 1) {
            stop = 1;
            break;
        }

        if (ret == TRAP_E_FORMAT_CHANGED) {
            if (process_format_change(config, in_tmplt, agg) != 0) {
                stop = 1;
                break;
            }
            key.init(agg.get_template_size());

            std::cout << config.out_tmplt.c_str() << "\n";
            out_tmplt = ur_create_output_template(0, config.out_tmplt.c_str(), NULL);
            if (out_tmplt == NULL) {
                std::cerr << "Error: Output template could not be created." << std::endl;
                stop = 1;
                break;
            }
        }

        // Generate flow key
        key.reset();
        for (auto field : agg.get_template_fields()) {
            key.update(ur_get_ptr_by_id(in_tmplt, in_rec, field.first), field.second);
        }

        aggregator::Flow_data in_flow_data;
        auto insered_data = agg.flow_cache.insert(key, in_flow_data);
        if (insered_data.second == true) { // New key in cache
            (*insered_data.first).second.data = static_cast<char *>(agg.allocate_memory());
        }

        aggregator::Flow_data *flow_data = static_cast<aggregator::Flow_data *>(&(*insered_data.first).second);
        if (insered_data.second == true) {
            flow_data->update((uint32_t)ur_get(in_tmplt, in_rec, F_COUNT));
        }
        for (auto field : agg.get_fields()) {
            if (ur_is_array(field.first.ur_field_id)) {
                aggregator::ur_array_data src_data;
                src_data.cnt_elements = ur_array_get_elem_cnt(in_tmplt, in_rec, field.first.ur_field_id);
                src_data.ptr_first = ur_get_ptr_by_id(in_tmplt, in_rec, field.first.ur_field_id);
                if (field.first.type == aggregator::SORTED_APPEND) {
                    src_data.sort_key = ur_get_ptr_by_id(in_tmplt, in_rec, field.first.ur_sort_key_id);
                    if (ur_is_array(field.first.ur_sort_key_id))
                        src_data.sort_key_elements = ur_array_get_elem_cnt(in_tmplt, in_rec, field.first.ur_sort_key_id);
                    else
                        src_data.sort_key_elements = 1;

                }
                field.first.aggregate(&src_data, &flow_data->data[field.second]);
            } else 
                field.first.aggregate(ur_get_ptr_by_id(in_tmplt, in_rec, field.first.ur_field_id), &flow_data->data[field.second]);
        }

        flow_data->update(ur_get(in_tmplt, in_rec, F_TIME_FIRST), ur_get(in_tmplt, in_rec, F_TIME_LAST));
        cnt++;
    }
 
    std::cout << "COUNTER: " << cnt << "\n";
    /////////////////////
    flush_flow_cache(agg, out_tmplt);

    return 0;
}

int
main(int argc, char **argv)
{
    Configuration config;
    struct sigaction sigbreak;
    ur_template_t *in_tmplt;
    int ret;
    char opt;

    // Macro allocates and initializes module_info structure according to MODULE_BASIC_INFO.
    INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS);

    // Let TRAP library parse program arguments, extract its parameters and initialize module interfaces
    TRAP_DEFAULT_INITIALIZATION(argc, argv, *module_info);

    // Install SIGINT and SIGTERN signal handler. 
    install_signal_handler(sigbreak);

    while ((opt = TRAP_GETOPT(argc, argv, module_getopt_string, long_options)) != -1) {
        ret = 0;
        switch (opt) {
        case 't':
            ret = config.set_timeout(optarg);
            break;
        case 'c':
            ret = config.parse_xml(optarg);
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

    // Set TRAP_RECEIVE() timeout to TRAP_RECV_TIMEOUT/1000000 seconds
    trap_ifcctl(TRAPIFC_INPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_RECV_TIMEOUT);

    // Set TRAP_RECEIVE() timeout to TRAP_RECV_TIMEOUT/1000000 seconds
    //trap_ifcctl(TRAPIFC_OUTPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_SEND_TIMEOUT);

    /* **** Create UniRec templates **** */
    in_tmplt = ur_create_input_template(0, "TIME_FIRST,TIME_LAST", NULL);
    if (in_tmplt == NULL) {
        std::cerr << "Error: Input template could not be created." << std::endl;
        goto failure;
    }

    /* **** Create new thread for checking timeouts **** */
    // TODO create timeout thread        
    
    ret = do_mainloop(config, in_tmplt);
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
