/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.scalar.conf 11805 2005-01-07 09:37:18Z dts12 $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <pcre.h>
#include "ram.h"
#include "command.h"

struct ram_cache_s
{
  uint32_t wired;
  uint32_t active;
  uint32_t inactive;
  uint32_t used;
  uint32_t free;
};

#define MAX_CACHE_AGE 10
#define OVECCOUNT 30

static struct ram_cache_s ram_cache;
static struct timeval ram_cache_timestamp;

void scale_value(char unit_char, uint32_t *val)
{
  switch(unit_char)
  {
    case 'K':
      *val /= 1024;
      break;
    case 'M':
      break;
    case 'G':
      *val *= 1024;
      break;
    case 'T':
      *val *= 1024 * 1024;
      break;      
    default:
      x_printf ("update_ram unknown unit '%c'", unit_char); 
  }
}

void match_and_scale (char *data, size_t data_len, char *type_str, uint32_t *val)
{
  const char *error;
  int erroffset;
  int ovector[OVECCOUNT];
  char *match_str;
  
  asprintf(&match_str, "\\b(\\d+)([KMGT]) %s[,\\.]", type_str);
  pcre *re = pcre_compile(match_str, 0, &error, &erroffset, NULL);
  free (match_str);
  if (re == NULL) { x_printf ("update_ram failed to compile regex for wired"); free (data); return; }
  
  int rc = pcre_exec(re, NULL, data, data_len, 0, 0, ovector, OVECCOUNT);
  if (rc > 2)  
  {
    char *value_str;
    asprintf (&value_str, "%.*s", ovector[3] - ovector[2], data + ovector[2]);
    *val = strtoul(value_str, NULL, 10);
    scale_value (*(data + ovector[4]), &ram_cache.wired);
    free (value_str);
  }
  else x_printf ("update_ram got no match for %s", type_str);
  
  pcre_free (re);
}

void update_ram()
{
  char *data = x_command_run("top -l 1 -n 0", 0);
  if (!data) return;
  size_t data_len = strlen(data);
  match_and_scale (data, data_len, "wired", &ram_cache.wired);
  match_and_scale (data, data_len, "active", &ram_cache.active);
  match_and_scale (data, data_len, "inactive", &ram_cache.inactive);
  match_and_scale (data, data_len, "used", &ram_cache.used);
  match_and_scale (data, data_len, "free", &ram_cache.free);
  gettimeofday(&ram_cache_timestamp, NULL);  
}

void update_ram_if_necessary ()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  if ((now.tv_sec - ram_cache_timestamp.tv_sec) > MAX_CACHE_AGE)
  {
    update_ram();
  }
}

/** Initializes the ram module */
void
init_ram(void)
{
    static oid ramWired_oid[] = { 1,3,6,1,4,1,20038,2,1,3,2,1 };
    static oid ramActive_oid[] = { 1,3,6,1,4,1,20038,2,1,3,2,2 };
    static oid ramInactive_oid[] = { 1,3,6,1,4,1,20038,2,1,3,2,3 };
    static oid ramUsed_oid[] = { 1,3,6,1,4,1,20038,2,1,3,2,4 };
    static oid ramFree_oid[] = { 1,3,6,1,4,1,20038,2,1,3,2,5 };

  DEBUGMSGTL(("ram", "Initializing\n"));
  
  memset (&ram_cache, 0, sizeof(struct ram_cache_s));
  memset (&ram_cache_timestamp, 0, sizeof(struct timeval));
  

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("ramWired", handle_ram,
                               ramWired_oid, OID_LENGTH(ramWired_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("ramActive", handle_ram,
                               ramActive_oid, OID_LENGTH(ramActive_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("ramInactive", handle_ram,
                               ramInactive_oid, OID_LENGTH(ramInactive_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("ramUsed", handle_ram,
                               ramUsed_oid, OID_LENGTH(ramUsed_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("ramFree", handle_ram,
                               ramFree_oid, OID_LENGTH(ramFree_oid),
                               HANDLER_CAN_RONLY
        ));
}

int
handle_ram (netsnmp_mib_handler *handler, 
            netsnmp_handler_registration *reginfo,
            netsnmp_agent_request_info   *reqinfo,
            netsnmp_request_info         *requests)
{
  uint32_t val = 0;
  switch(reqinfo->mode) 
  {
    case MODE_GET:
      update_ram_if_necessary();
      if (!strcmp(reginfo->handlerName, "ramWired")) val = ram_cache.wired;
      if (!strcmp(reginfo->handlerName, "ramActive")) val = ram_cache.active;
      if (!strcmp(reginfo->handlerName, "ramInactive")) val = ram_cache.inactive;
      if (!strcmp(reginfo->handlerName, "ramUsed")) val = ram_cache.used;
      if (!strcmp(reginfo->handlerName, "ramFree")) val = ram_cache.free;
      snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *)&val, sizeof(uint32_t));
      break;
    default:
      /* we should never get here, so this is a really bad error */
      snmp_log(LOG_ERR, "unknown mode (%d) in handle_ram\n", reqinfo->mode );
      return SNMP_ERR_GENERR;
  }

  return SNMP_ERR_NOERROR;
}