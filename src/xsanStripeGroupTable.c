/*
 * Note: this file originally auto-generated by mib2c using
 *  : mib2c.iterate.conf 15999 2007-03-25 22:32:02Z dts12 $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "xsanStripeGroupTable.h"
#include "xsanNodeTable.h"
#include "xsanVolumeTable.h"
#include "xsanAffinityTable.h"
#include <pcre.h>
#include "log.h"
#include "util.h"

#define OVECCOUNT 90

/** Initializes the xsanStripeGroupTable module */
void
init_xsanStripeGroupTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_xsanStripeGroupTable();
}

/** Initialize the xsanStripeGroupTable table by defining its contents and how it's structured */
void
initialize_table_xsanStripeGroupTable(void)
{
    static oid xsanStripeGroupTable_oid[] = {1,3,6,1,4,1,20038,2,1,1,2};
    size_t xsanStripeGroupTable_oid_len   = OID_LENGTH(xsanStripeGroupTable_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "xsanStripeGroupTable",     xsanStripeGroupTable_handler,
              xsanStripeGroupTable_oid, xsanStripeGroupTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: xsanVolumeIndex */
                           ASN_INTEGER,  /* index: xsanStripeGroupIndex */
                           0);
    table_info->min_column = COLUMN_XSANSTRIPEGROUPINDEX;
    table_info->max_column = COLUMN_XSANSTRIPEGROUPUSEDMBYTES;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = xsanStripeGroupTable_get_first_data_point;
    iinfo->get_next_data_point  = xsanStripeGroupTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

struct xsanStripeGroupTable_entry  *xsanStripeGroupTable_head;

/* create a new row in the (unsorted) table */
struct xsanStripeGroupTable_entry *
xsanStripeGroupTable_createEntry(long  xsanVolumeIndex, long  xsanStripeGroupIndex) 
{
    struct xsanStripeGroupTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct xsanStripeGroupTable_entry);
    if (!entry)
        return NULL;

    entry->xsanVolumeIndex = xsanVolumeIndex;
    entry->xsanStripeGroupIndex = xsanStripeGroupIndex;
    entry->next = xsanStripeGroupTable_head;
    xsanStripeGroupTable_head = entry;
    
    return entry;
}

/* remove a row from the table */
void
xsanStripeGroupTable_removeEntry( struct xsanStripeGroupTable_entry *entry ) {
    struct xsanStripeGroupTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = xsanStripeGroupTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        xsanStripeGroupTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}


/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
xsanStripeGroupTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    *my_loop_context = xsanStripeGroupTable_head;
    return xsanStripeGroupTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
xsanStripeGroupTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct xsanStripeGroupTable_entry *entry = (struct xsanStripeGroupTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_typed_integer( idx, ASN_INTEGER, entry->xsanVolumeIndex );
        idx = idx->next_variable;
        snmp_set_var_typed_integer( idx, ASN_INTEGER, entry->xsanStripeGroupIndex );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
        return put_index_data;
    } else {
        return NULL;
    }
}

void update_stripegroups (char *data, size_t data_len, long xsanVolumeIndex)
{
     struct timeval now;
     gettimeofday (&now, NULL);

     const char *error;
     int erroffset;
     int ovector[OVECCOUNT];
     pcre *re = pcre_compile("\\n(Stripe Group \\d+ (?:[^\\n]+\\n)+)", 0, &error, &erroffset, NULL);

     if (re == NULL) { x_printf ("ERROR: update_stripegroups failed to compile regex"); return; }

     ovector[0] = 0;
     ovector[1] = 0;
     while(1)
     {
         int options = 0;                 /* Normally no options */
         int start_offset = ovector[1];   /* Start at end of previous match */

         if (ovector[0] == ovector[1])
         {
             if (ovector[0] == (int)data_len) break;
         }

         int rc = pcre_exec(
                             re,                   /* the compiled pattern */
                             NULL,                 /* no extra data - we didn't study the pattern */
                             data,              /* the subject string */
                             data_len,       /* the length of the subject */
                             start_offset,         /* starting offset in the subject */
                             options,              /* options */
                             ovector,              /* output vector for substring information */
                             OVECCOUNT);           /* number of elements in the output vector */
                         
         if (rc == PCRE_ERROR_NOMATCH)
         {
             if (options == 0) break;
             ovector[1] = start_offset + 1;
             continue;    /* Go round the loop again */
         }

         /* Other matching errors are not recoverable. */
         if (rc > 0)
         {
             /* Matched an Xsan Stripe Group. Vectors: 
              *    0 = Full text match for stripe group
              *    1(2) = Text of interest
              */
             char *stripegroup_data;
             asprintf (&stripegroup_data, "%.*s", ovector[3] - ovector[2], data + ovector[2]);
             size_t stripegroup_data_len = strlen(stripegroup_data);
             
             long stripeGroupIndex = extract_uint_from_regex (stripegroup_data, strlen(stripegroup_data), "^Stripe Group (\\d+)");
             
             struct xsanStripeGroupTable_entry *entry = xsanStripeGroupTable_head;
             while (entry)
             {
                 if (entry->xsanVolumeIndex == xsanVolumeIndex && entry->xsanStripeGroupIndex == stripeGroupIndex) break;
                 entry = entry->next;
             }
             if (!entry)
             {
                 entry = xsanStripeGroupTable_createEntry (xsanVolumeIndex, stripeGroupIndex);
             }
             entry->last_seen = now.tv_sec;
             
             extract_string_from_regex (stripegroup_data, stripegroup_data_len, "^Stripe Group \\d+ \\[([^\\]]+)\\]", &entry->xsanStripeGroupName, &entry->xsanStripeGroupName_len);
             
             char *status_str = NULL;
             size_t status_str_len = 0;
             extract_string_from_regex (stripegroup_data, stripegroup_data_len, "^Stripe Group \\d+ \\[[^\\]]+\\][ ]+Status:(\\w+)", &status_str, &status_str_len);
             if (!strcmp(status_str, "Up")) entry->xsanStripeGroupStatus = 1;
             else entry->xsanStripeGroupStatus = 0;
             free (status_str);
             
             extract_U64_from_regex (stripegroup_data, stripegroup_data_len, "Total Blocks:(\\d+)", &entry->xsanStripeGroupTotalBlocks);
             entry->xsanStripeGroupTotalKBlocks = scale_U64_to_K (&entry->xsanStripeGroupTotalBlocks);

             extract_U64_from_regex (stripegroup_data, stripegroup_data_len, "Reserved:(\\d+)", &entry->xsanStripeGroupReservedBlocks);
             entry->xsanStripeGroupReservedKBlocks = scale_U64_to_K (&entry->xsanStripeGroupReservedBlocks);

             extract_U64_from_regex (stripegroup_data, stripegroup_data_len, "Free:(\\d+)", &entry->xsanStripeGroupFreeBlocks);
             entry->xsanStripeGroupFreeKBlocks = scale_U64_to_K (&entry->xsanStripeGroupFreeBlocks);             
             
             entry->xsanStripeGroupUtilization = 100 - extract_uint_from_regex(stripegroup_data, stripegroup_data_len, "^[ ]+Total Blocks:.*\\((\\d+%)\\)$");
             
             extract_string_from_regex (stripegroup_data, stripegroup_data_len, "^[ ]+MultiPath Method:(.*)$", &entry->xsanStripeGroupMultiPathMethod, &entry->xsanStripeGroupMultiPathMethod_len);

             entry->xsanStripeGroupStripeDepth = extract_uint_from_regex(stripegroup_data, stripegroup_data_len, "^[ ]+Stripe Depth:(\\d+)");
             entry->xsanStripeGroupStripeBreadth = extract_uint_from_regex(stripegroup_data, stripegroup_data_len, "Stripe Breadth:(\\d+)");

             extract_string_from_regex (stripegroup_data, stripegroup_data_len, "^[ ]+Affinity Key:(.*)$", &entry->xsanStripeGroupAffinityKey, &entry->xsanStripeGroupAffinityKey_len); 
             if (entry->xsanStripeGroupAffinityKey) entry->xsanAffinityIndex = find_or_create_affinity_index(entry->xsanVolumeIndex, entry->xsanStripeGroupAffinityKey);
             
             entry->xsanStripeGroupRTLimitIOPerSecond = extract_uint_from_regex(stripegroup_data, stripegroup_data_len, "Realtime limit IO/sec:(\\d+)");
             entry->xsanStripeGroupNonRTReserveIOPerSecond = extract_uint_from_regex(stripegroup_data, stripegroup_data_len, "Non-Realtime reserve IO/sec:(\\d+)");
             entry->xsanStripeGroupRTCommittedIOPerSecond = extract_uint_from_regex(stripegroup_data, stripegroup_data_len, "Committed RTIO/sec:(\\d+)");
             entry->xsanStripeGroupNonRTClients = extract_uint_from_regex(stripegroup_data, stripegroup_data_len, "Non-RTIO clients:(\\d+)");
             entry->xsanStripeGroupNonRTHintIOPerSecond = extract_uint_from_regex(stripegroup_data, stripegroup_data_len, "Non-RTIO hint IO/sec:(\\d+)");

             u_long blockSize = blockSizeForVolumeIndex(entry->xsanVolumeIndex);
             entry->xsanStripeGroupTotalMBytes = blockSize * entry->xsanStripeGroupTotalKBlocks;    // Both are in K values and multiple to M 
             entry->xsanStripeGroupFreeMBytes = blockSize * entry->xsanStripeGroupFreeKBlocks;
             entry->xsanStripeGroupUsedMBytes = blockSize * (entry->xsanStripeGroupTotalKBlocks - entry->xsanStripeGroupFreeKBlocks);
             
             update_nodes_for_stripe_group(stripegroup_data, stripegroup_data_len, entry->xsanVolumeIndex, entry->xsanStripeGroupIndex);
             
             free (stripegroup_data);
             stripegroup_data = NULL;          
         }
         else
         {
             pcre_free(re);    /* Release memory used for the compiled pattern */
             return;
         }  
     }
     
     pcre_free(re);    /* Release memory used for the compiled pattern */

    /* Obsolescence Checking */
    struct xsanStripeGroupTable_entry *entry = xsanStripeGroupTable_head;
    while (entry)
    {
        struct xsanStripeGroupTable_entry *next = entry->next;
        if (entry->xsanVolumeIndex == xsanVolumeIndex && entry->last_seen != now.tv_sec)
        {
         /* Entry is obsolete */
         xsanStripeGroupTable_removeEntry(entry);
        }
        entry = next;
    }
 }

/** handles requests for the xsanStripeGroupTable table */
int
xsanStripeGroupTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct xsanStripeGroupTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct xsanStripeGroupTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_XSANVOLUMEINDEX:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->xsanVolumeIndex);
                break;
            case COLUMN_XSANSTRIPEGROUPNAME:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                 (u_char*)table_entry->xsanStripeGroupName,
                                          table_entry->xsanStripeGroupName_len);
                break;
            case COLUMN_XSANSTRIPEGROUPSTATUS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->xsanStripeGroupStatus);
                break;
            case COLUMN_XSANSTRIPEGROUPTOTALBLOCKS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER64,
                                            (u_char *)&table_entry->xsanStripeGroupTotalBlocks, sizeof(U64));
                break;
            case COLUMN_XSANSTRIPEGROUPRESERVEDBLOCKS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER64,
                                            (u_char *)&table_entry->xsanStripeGroupReservedBlocks, sizeof(U64));
                break;
            case COLUMN_XSANSTRIPEGROUPFREEBLOCKS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER64,
                                            (u_char *)&table_entry->xsanStripeGroupFreeBlocks, sizeof(U64));
                break;
            case COLUMN_XSANSTRIPEGROUPTOTALKBLOCKS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupTotalKBlocks);
                break;
            case COLUMN_XSANSTRIPEGROUPRESERVEDKBLOCKS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupReservedKBlocks);
                break;
            case COLUMN_XSANSTRIPEGROUPFREEKBLOCKS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupFreeKBlocks);
                break;
            case COLUMN_XSANSTRIPEGROUPUTILIZATION:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupUtilization);
                break;
            case COLUMN_XSANSTRIPEGROUPMULTIPATHMETHOD:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                 (u_char*)table_entry->xsanStripeGroupMultiPathMethod,
                                          table_entry->xsanStripeGroupMultiPathMethod_len);
                break;
            case COLUMN_XSANSTRIPEGROUPSTRIPEDEPTH:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->xsanStripeGroupStripeDepth);
                break;
            case COLUMN_XSANSTRIPEGROUPSTRIPEBREADTH:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->xsanStripeGroupStripeBreadth);
                break;
            case COLUMN_XSANSTRIPEGROUPAFFINITYKEY:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                 (u_char*)table_entry->xsanStripeGroupAffinityKey,
                                          table_entry->xsanStripeGroupAffinityKey_len);
                break;
            case COLUMN_XSANSTRIPEGROUPRTLIMITIOPERSECOND:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupRTLimitIOPerSecond);
                break;
            case COLUMN_XSANSTRIPEGROUPNONRTRESERVEIOPERSECOND:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupNonRTReserveIOPerSecond);
                break;
            case COLUMN_XSANSTRIPEGROUPRTCOMMITTEDIOPERSECOND:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupRTCommittedIOPerSecond);
                break;
            case COLUMN_XSANSTRIPEGROUPNONRTCLIENTS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupNonRTClients);
                break;
            case COLUMN_XSANSTRIPEGROUPNONRTHINTIOPERSECOND:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupNonRTHintIOPerSecond);
                break;
            case COLUMN_XSANSTRIPEGROUPTOTALMBYTES:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupTotalMBytes);
                break;
            case COLUMN_XSANSTRIPEGROUPFREEMBYTES:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupFreeMBytes);
                break;
            case COLUMN_XSANSTRIPEGROUPUSEDMBYTES:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->xsanStripeGroupUsedMBytes);
                break;
            default:
                netsnmp_set_request_error(reqinfo, request,
                                          SNMP_NOSUCHOBJECT);
                break;
            }
        }
        break;

    }
    return SNMP_ERR_NOERROR;
}
