/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.scalar.conf 11805 2005-01-07 09:37:18Z dts12 $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <fcntl.h>
#include "command.h"
#include "raidInfo.h"
#include "util.h"
#include "main.h"
#include "log.h"

struct raid_cache_s
{
  int generalStatus;
  char *generalStatusMessage;
  size_t generalStatusMessage_len;
  int batteryStatus;
  char *batteryStatusMessage;
  size_t batteryStatusMessage_len;
  char *hardwareVersion;
  size_t hardwareVersion_len;
  char *firmwareVersion;
  size_t firmwareVersion_len;
  int writeCache;
};

static struct raid_cache_s raid_cache;
static struct timeval raid_cache_timestamp;

#define MAX_CACHE_AGE 10

/** Initializes the raidInfo module */
void
init_raidInfo(void)
{
    static oid raidGeneralStatus_oid[] = { 1,3,6,1,4,1,20038,2,1,6,1,1 };
    static oid raidGeneralStatusMessage_oid[] = { 1,3,6,1,4,1,20038,2,1,6,1,2 };
    static oid raidBatteryStatus_oid[] = { 1,3,6,1,4,1,20038,2,1,6,1,3 };
    static oid raidBatteryStatusMessage_oid[] = { 1,3,6,1,4,1,20038,2,1,6,1,4 };
    static oid raidHardwareVersion_oid[] = { 1,3,6,1,4,1,20038,2,1,6,1,5 };
    static oid raidFirmwareVersion_oid[] = { 1,3,6,1,4,1,20038,2,1,6,1,6 };
    static oid raidWriteCache_oid[] = { 1,3,6,1,4,1,20038,2,1,6,1,7 };

  DEBUGMSGTL(("raidInfo", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("raidGeneralStatus", handle_raidGeneralStatus,
                               raidGeneralStatus_oid, OID_LENGTH(raidGeneralStatus_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("raidGeneralStatusMessage", handle_raidGeneralStatusMessage,
                               raidGeneralStatusMessage_oid, OID_LENGTH(raidGeneralStatusMessage_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("raidBatteryStatus", handle_raidBatteryStatus,
                               raidBatteryStatus_oid, OID_LENGTH(raidBatteryStatus_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("raidBatteryStatusMessage", handle_raidBatteryStatusMessage,
                               raidBatteryStatusMessage_oid, OID_LENGTH(raidBatteryStatusMessage_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("raidHardwareVersion", handle_raidHardwareVersion,
                               raidHardwareVersion_oid, OID_LENGTH(raidHardwareVersion_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("raidFirmwareVersion", handle_raidFirmwareVersion,
                               raidFirmwareVersion_oid, OID_LENGTH(raidFirmwareVersion_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("raidWriteCache", handle_raidWriteCache,
                               raidWriteCache_oid, OID_LENGTH(raidWriteCache_oid),
                               HANDLER_CAN_RONLY
        ));
}

void update_raid ()
{
  char *data = NULL;
  size_t data_len = 0;
  if (xsan_debug())
  {
    /* Use example data */
    int fd = open ("../examples/raidutil_list_status.txt", O_RDONLY);
    data = malloc(65536);
    data_len = read(fd, data, 65535);
    data[data_len] = '\0';
    close(fd);
  }
  else
  {
    /* Use live data */
    data = x_command_run("raidutil list status", 0);
    data_len = strlen(data);
  }

  if (!data) return;

  printf("Data: '%s'\n", data);

  /* General Status */
  if (extract_string_from_regex(data, data_len, "^General Status:\\s+(.*)$", &raid_cache.generalStatusMessage, &raid_cache.generalStatusMessage_len))
  {
    /* Match status string */
    if (strstr(raid_cache.generalStatusMessage, "Good")) raid_cache.generalStatus = 1;
    else if (strstr(raid_cache.generalStatusMessage, "Issues Found")) raid_cache.generalStatus = 2;
  }
  else raid_cache.generalStatus = 0;  // Unknown

  /* Battery */
  if (extract_string_from_regex(data, data_len, "^Battery Status:\\s+(.*)$", &raid_cache.batteryStatusMessage, &raid_cache.batteryStatusMessage_len))
  {
    /* Match status string */
    if (strstr(raid_cache.batteryStatusMessage, "Charged")) raid_cache.batteryStatus = 1;
    else if (strstr(raid_cache.batteryStatusMessage, "Conditioning")) raid_cache.batteryStatus = 2;
    else if (strstr(raid_cache.batteryStatusMessage, "Charging (insufficient charge)")) raid_cache.batteryStatus = 3;
    else if (strstr(raid_cache.batteryStatusMessage, "Not Charging (insufficient charge)")) raid_cache.batteryStatus = 4;
  }
  else raid_cache.batteryStatus = 0;  // Unknown

  /* Verions */
  extract_string_from_regex(data, data_len, "Hardware Version ([^/]*)", &raid_cache.hardwareVersion, &raid_cache.hardwareVersion_len);
  extract_string_from_regex(data, data_len, "Firmware (.*)$", &raid_cache.firmwareVersion, &raid_cache.firmwareVersion_len);

  /* Write cache */
  char *writecache_str = NULL;
  size_t writecache_len = 0;
  extract_string_from_regex(data, data_len, "Write Cache (.*)$", &writecache_str, &writecache_len);
  printf ("WRITE CACHE: %s\n", writecache_str);
  if (strstr(writecache_str, "enabled")) raid_cache.writeCache = 1;
  else raid_cache.writeCache = 0;

  /* Set cache timestamp */
  gettimeofday(&raid_cache_timestamp,NULL);

  /* Cleanup */
  free(data);
  data = NULL;
  data_len = 0;
}

void update_raid_if_necessary ()
{
  struct timeval now; 
  gettimeofday(&now,NULL);
  if ((now.tv_sec - raid_cache_timestamp.tv_sec) > MAX_CACHE_AGE)
  {
    update_raid();
  }
}

int
handle_raidGeneralStatus(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

      case MODE_GET:
            update_raid_if_necessary();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &raid_cache.generalStatus,
                                     sizeof(raid_cache.generalStatus));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_raidGeneralStatus\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_raidGeneralStatusMessage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
            update_raid_if_necessary();
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) raid_cache.generalStatusMessage,
                                     raid_cache.generalStatusMessage_len);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_raidGeneralStatusMessage\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_raidBatteryStatus(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
            update_raid_if_necessary();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &raid_cache.batteryStatus,
                                     sizeof(raid_cache.batteryStatus));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_raidBatteryStatus\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_raidBatteryStatusMessage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
            update_raid_if_necessary();
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) raid_cache.batteryStatusMessage,
                                     raid_cache.batteryStatusMessage_len);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_raidBatteryStatusMessage\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_raidHardwareVersion(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
            update_raid_if_necessary();
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) raid_cache.hardwareVersion,
                                     raid_cache.hardwareVersion_len);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_raidHardwareVersion\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_raidFirmwareVersion(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
            update_raid_if_necessary();
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) raid_cache.firmwareVersion,
                                     raid_cache.firmwareVersion_len);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_raidFirmwareVersion\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_raidWriteCache(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
            update_raid_if_necessary();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &raid_cache.writeCache,
                                     sizeof(raid_cache.writeCache));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_raidWriteCache\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
