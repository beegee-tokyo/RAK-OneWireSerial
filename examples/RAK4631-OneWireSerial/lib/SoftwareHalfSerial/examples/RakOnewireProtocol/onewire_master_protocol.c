#include "onewire_master_api.h"
#include "onewire_master_protocol.h"

// #include "main.h"
// #include "if_debug.h"

#define REC_NUM             1
#define BUFF_SIZE           0x100

#ifndef NULL
#define NULL                0
#endif

#ifndef ENABLE
#define ENABLE              1
#endif

#ifndef DISABLE
#define DISABLE             0
#endif

#define SHORT_SWAP(X)       (((X&0xFF)<<8) | ((X>>8)&0xFF))
#define LSB_COMB(M,L)       ((L<<8) + M)
#define SUB(A,B)            ((A == 0) || (B > A) )? 0: (A - B)
#define CKTODO(X)           if (X!=NULL) X

#define f_memset(P,V,L)     do { for(U32 _i=0; _i<L; _i++) P[_i]=V; } while (0)
#define f_memcmp(A,B,L)     this_memcmp(A,B,L)
#define f_memcpy(A,B,L)     this_memcpy(A,B,L)

typedef RET_S32 (*program_process) ( U8 *data, U16 len );
typedef RET_S32 (*command_process) ( U8 pid, U8 sid ,SNHUB_GS_E gset, U8 ptye );

typedef struct { U8 size; } rakipso_tbl_t;
typedef struct { program_process req; program_process rsp; } protocol_process_t;

static SNHub_Record_t record[REC_NUM];
static SNHub_Menu_t menu;
static SNHub_Evt_t on_evt;
static U8 dataBuff[BUFF_SIZE];

static const command_process command_list[];
static const protocol_process_t protocol_list[];
static const rakipso_tbl_t rakipso_tbl[] =
{
    [RAK_IPSO_DIGITAL_INPUT]    = { .size = 1  },
    [RAK_IPSO_DIGITAL_OUTPUT]   = { .size = 1, },
    [RAK_IPSO_ANALOG_INPUT]     = { .size = 2, },
    [RAK_IPSO_ANALOG_OUT]       = { .size = 2, },
    [RAK_IPSO_NITROGEN]         = { .size = 2, },
    [RAK_IPSO_PHOSPHORUS]       = { .size = 2, },
    [RAK_IPSO_POTASSIUM]        = { .size = 2, },
    [RAK_IPSO_SALINITY]         = { .size = 2, },
    [RAK_IPSO_DISS_OXYGEN]      = { .size = 2, },
    [RAK_IPSO_ORP]              = { .size = 2, },
    [RAK_IPSO_COD]              = { .size = 2, },
    [RAK_IPSO_TURBIDITY]        = { .size = 2, },
    [RAK_IPSO_NO3]              = { .size = 2, },
    [RAK_IPSO_NH4PLUS]          = { .size = 2, },
    [RAK_IPSO_BOD]              = { .size = 2, },
    [RAK_IPSO_ILLUM_SENSOR]     = { .size = 4, },
    [RAK_IPSO_PRESENCE_SENSOR]  = { .size = 1, },
    [RAK_IPSO_TEMP_SENSOR]      = { .size = 2, },
    [RAK_IPSO_HUMIDITY_SENSOR]  = { .size = 1, },
    [RAK_IPSO_GAS]              = { .size = 2, },
    [RAK_IPSO_HP_HUMIDITY]      = { .size = 2, },
    [RAK_IPSO_ACCELEROMETER]    = { .size = 6, },
    [RAK_IPSO_BAROMETER]        = { .size = 2, },
    [RAK_IPSO_BATTERVALUE]      = { .size = 2, },
    [RAK_IPSO_PRECIPITATION]    = { .size = 2, },
    [RAK_IPSO_GASPERCENTAGE]    = { .size = 1, },
    [RAK_IPSO_CO2]              = { .size = 2, },
    [RAK_IPSO_SSN]              = { .size = 3, },
    [RAK_IPSO_HP_EC]            = { .size = 4, },
    [RAK_IPSO_DISTANCE]         = { .size = 4, },
    [RAK_IPSO_GYROMETER]        = { .size = 6, },
    [RAK_IPSO_GPS_LOCAL]        = { .size = 9, },
    [RAK_IPSO_GNSS_ENHANCED]    = { .size = 11,},
    [RAK_IPSO_VOC]              = { .size = 2, },
    [RAK_IPSO_GUSTWINDSPEED]    = { .size = 2, },
    [RAK_IPSO_STRIKES]          = { .size = 2, },
    [RAK_IPSO_CAPACITY]         = { .size = 1, },
    [RAK_IPSO_DC_CURRENT]       = { .size = 2, },
    [RAK_IPSO_DC_VOLTAGE]       = { .size = 2, },
    [RAK_IPSO_MOISTURE]         = { .size = 2, },
    [RAK_IPSO_WIND]             = { .size = 2, },
    [RAK_IPSO_WIND_DIR]         = { .size = 2, },
    [RAK_IPSO_EC]               = { .size = 2, },
    [RAK_IPSO_HP_PH]            = { .size = 2, },
    [RAK_IPSO_PH]               = { .size = 2, },
    [RAK_IPSO_PYRANOMETER]      = { .size = 2, },
    [RAK_IPSO_PM10]             = { .size = 2, },
    [RAK_IPSO_PM25]             = { .size = 2, },
    [RAK_IPSO_XYORIENTATION]    = { .size = 2, },
    [RAK_IPSO_NOISE]            = { .size = 2, },
    [RAK_IPSO_MODBUS]           = { .size = 64,},
    [RAK_IPSO_SDI12]            = { .size = 64,},
    [RAK_IPSO_BINARY2BYTE]      = { .size = 2, },
    [RAK_IPSO_BINARY4BYTE]      = { .size = 4, },
    [RAK_IPSO_FLOAT_IEEE754]    = { .size = 4, },
    [RAK_IPSO_INTEGER32]        = { .size = 4, },
    [RAK_IPSO_UINTEGER32]       = { .size = 4, },
    [RAK_IPSO_BINARYTLV]        = { .size = 64,},
};

static unsigned builtin_popcount (unsigned u)
{
    u = (u & 0x55555555) + ((u >> 1) & 0x55555555);
    u = (u & 0x33333333) + ((u >> 2) & 0x33333333);
    u = (u & 0x0F0F0F0F) + ((u >> 4) & 0x0F0F0F0F);
    u = (u & 0x00FF00FF) + ((u >> 8) & 0x00FF00FF);
    u = (u & 0x0000FFFF) + ((u >> 16) & 0x0000FFFF);
    return u;
}

static U8 cal_chksum(U8 *data, U16 len)
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;

    U8 chsum = 0;
    chsum += builtin_popcount(rui3_api->type);
    chsum += builtin_popcount(rui3_api->flag);

    U16 payload_len = LSB_COMB(rui3_api->length.hbyte, rui3_api->length.lbyte);

    if ( payload_len > len )
    {
        return 0;
    }
    
    for (U16 i = 0 ; i < payload_len ; i++) 
    {
        chsum += builtin_popcount(rui3_api->payload[i]);
    }
    return chsum;
}

static RET_S32 this_memcmp( U8 *A, U8 *B, U16 len)
{
    for (U16 i = 0; i < len; i++)
    {
        if (A[i] != B[i])
        {
            return i;
        }
    }
    return 0;
}

static RET_S32 this_memcpy( U8 *A, U8 *B, U16 len)
{
    for (U16 i = 0; i < len; i++)
    {
        A[i] = B[i];
    }
    return 0;
}
static RET_S32  snhub_snsrdat_req_program( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    SNHub_Api_sData_Snsr_t *sData = (SNHub_Api_sData_Snsr_t*)(hub_api->payload);

    U16 ofs = 0;
    do
    {
        U8 pid = hub_api->source;
        U8 sid = sData->sid;
        U8 *val = &(sData->ipso);
        U8 val_len = rakipso_tbl[sData->ipso].size;
        on_evt( pid, sid, SNHUBAPI_EVT_SDATA_REQ, val, val_len + 1/* add chksum byte */);
        sData = (SNHub_Api_sData_Snsr_t *)&sData->value[val_len];
        ofs += sizeof(SNHub_Api_sData_Snsr_t) + val_len;
    } while ( ofs < (hub_api->payload_length - 1 /* less N-1 bytes */ ) );
    
    return RET_OK;
}

static RET_S32 snhub_provision_req_program( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    SNHub_Api_Provision_t *hub_api_prov = (SNHub_Api_Provision_t*)(hub_api->payload);

    U8 pktBuff[BUFF_SIZE];
    U16 pktLen = 0;

    switch (hub_api->payload_type)
    {
        case PLD_PROVI_TYPE_VER3:
            /* do nothing, just bypass */
            break;
        case PLD_PROVI_TYPE_VER1:
        case PLD_PROVI_TYPE_VER2:
        case PLD_PROVI_TYPE_BOOT:
        default:
            /* not support */
            return RET_ERROR;
    }

    U16 aid = 0;
    do
    {
        if (aid == REC_NUM)
        {
            return RET_ERROR;
        }

        if (record[aid].alive == DISABLE)
        {
            break;
        }
        else
        {
            if (f_memcmp(record[aid].info.sn.u,hub_api_prov->sn.u,sizeof(SERIALNUM)) == 0)
            {
                break;
            }
        }
    } while (aid++);
    
    if (record[aid].alive == DISABLE)
    {
        record[aid].alive = ENABLE;
        record[aid].snsrnum = hub_api_prov->snsr_num;
        f_memcpy((U8 *)&(record[aid].info),(U8 *)hub_api_prov,sizeof(SNHub_Api_Provision_t));
        for (U16 i = 0; i < record[aid].snsrnum; i++)
        {
            record[aid].snsrlist[i] = hub_api_prov->snsr_type[i].sid;
        }
    }

    f_memcpy(pktBuff,data,len);
    pktLen = len;

    rui3_api = (RUI3_Api_t *)pktBuff;
    hub_api = (SNHub_Api_t*)(rui3_api->payload);
    hub_api_prov = (SNHub_Api_Provision_t*)(hub_api->payload);

    U8 dest = hub_api->dest;
    U8 source = hub_api->source;
    
    rui3_api->flag = RUI3API_FLG_RSP;

    hub_api->dest = source;
    hub_api->source = dest;
    
    U8 pid = aid + 1; /* arry index to pid index */
    U8 sid = 0;
    hub_api_prov->provId = pid;

    U8 chsum = cal_chksum((U8*)rui3_api,pktLen);
    U16 recv_len = LSB_COMB(rui3_api->length.hbyte, rui3_api->length.lbyte);
    rui3_api->payload[recv_len] = chsum;

    on_evt(source, 0, SNHUBAPI_EVT_QSEND, pktBuff, pktLen);
    on_evt(source, 0, SNHUBAPI_EVT_ADD_PID, &pid, sizeof(pid));
    
    for (U16 i = 0; i < record[aid].snsrnum; i++)
    {
        sid = hub_api_prov->snsr_type[i].sid;
        on_evt(source, 0, SNHUBAPI_EVT_ADD_SID, &sid, sizeof(sid));
    }
    return RET_OK;
}

static RET_S32 snhub_snsrdat_rsp_program( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    SNHub_Api_sData_Snsr_t *sData = (SNHub_Api_sData_Snsr_t*)(hub_api->payload);

    U16 ofs = 0;
    do
    {
        U8 pid = hub_api->source;
        U8 sid = sData->sid;
        U8 *val = &(sData->ipso);
        U8 val_len = rakipso_tbl[sData->ipso].size;
        on_evt( pid, sid, SNHUBAPI_EVT_REPORT, val, val_len + 1 /* add chksum byte */);
        sData = (SNHub_Api_sData_Snsr_t *)&sData->value[val_len];
        ofs += sizeof(SNHub_Api_sData_Snsr_t) + val_len;
    } while ( ofs < (hub_api->payload_length - 1 /* less N-1 bytes */) );

    return RET_OK;
}

static RET_S32 snhub_paramget_rsp_program( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    SNHub_Api_Param_Snsr_t *paramget = (SNHub_Api_Param_Snsr_t*)(hub_api->payload);
    
    U8 source = hub_api->source;
    
    on_evt(source, paramget->sid, SNHUBAPI_EVT_GET_INTV, (U8 *)&(paramget->intv), sizeof(paramget->intv));

    U8 enable = (paramget->rule == RULE_PERIODIC)?ENABLE:DISABLE;

    on_evt(source, paramget->sid, SNHUBAPI_EVT_GET_ENABLE, (U8 *)&enable, sizeof(enable));
    return RET_OK;
}

static RET_S32 snhub_paramget_command( U8 pid, U8 sid , SNHUB_GS_E gset, U8 ptye )
{
    U8 pktBuff[BUFF_SIZE];
    U16 pktLen = 0;
    U16 pldLen = 0;
    
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)pktBuff;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    SNHub_Api_Param_Snsr_t *paramset = (SNHub_Api_Param_Snsr_t*)(hub_api->payload);
    
    f_memset(pktBuff, 0, BUFF_SIZE);

    rui3_api->wakeup = WAKEUPBYTE;
    rui3_api->start  = DELIMTER;
    rui3_api->type   = RUI3API_TYPE_SENSORHUB;
    rui3_api->flag   = RUI3API_FLG_REQ;
    hub_api->source  = PID_MASTER;
    hub_api->dest    = pid;
    hub_api->sequence= ++menu.seq;
    pktLen = sizeof(SNHub_Api_t);

    switch (gset)
    {
        case SNHUB_GS_GET:
            hub_api->type    = SNHUB_TYPE_PARAMGET;
            pldLen += 1; // add sid byte
            break;
        case SNHUB_GS_SET:
            hub_api->type    = SNHUB_TYPE_PARAMSET;
            pldLen += sizeof(SNHub_Api_Param_Snsr_t);
            break;
        default:
            return RET_ERROR;
    }

    pktLen+=pldLen;
    hub_api->payload_length = pldLen;
    hub_api->payload_type = ptye;
    
    switch (ptye)
    {
        case PLD_PARMGSET_TYPE_SNSR_UPDATE:
            hub_api->payload[0] = sid;
            break;
        case PLD_PARMGSET_TYPE_RULE:
        case PLD_PARMGSET_TYPE_SNSR_INTV:
        case PLD_PARMGSET_TYPE_PRB_INTV:
        case PLD_PARMGSET_TYPE_SNSR_HTHR:
        case PLD_PARMGSET_TYPE_SNSR_LTHR:
        case PLD_PARMGSET_TYPE_PRB_TAGID:
        case PLD_PARMGSET_TYPE_PRB_TAGEN:
        case PLD_PARMGSET_TYPE_PRB_UPDATE:
        case PLD_PARMGSET_TYPE_CONF_UPDATE:
            /* not support */
            return RET_ERROR;
    }
    
    if ( gset == SNHUB_GS_SET )
    {
        paramset->intv = menu.intv;
        paramset->rule = menu.rule;
        menu.intv = 0;
        menu.rule = 0;
    }
    
    rui3_api->length.value = pktLen;
    rui3_api->length.value = SHORT_SWAP(rui3_api->length.value);
    rui3_api->payload[pktLen] = cal_chksum((U8*)rui3_api,BUFF_SIZE);

    pktLen += sizeof(RUI3_Api_t);
    pktLen += 1 /* add checksum len*/;

    on_evt(PID_MASTER, sid, SNHUBAPI_EVT_QSEND, pktBuff, pktLen);

    return RET_OK;
}

static RET_S32 snhub_provision_command( U8 pid, U8 sid , SNHUB_GS_E gset, U8 ptye )
{
    U8 pktBuff[BUFF_SIZE];
    U16 pktLen = 0;
    U16 pldLen = 0;
    
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)pktBuff;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    
    f_memset(pktBuff, 0, BUFF_SIZE);

    rui3_api->wakeup = WAKEUPBYTE;
    rui3_api->start  = DELIMTER;
    rui3_api->type   = RUI3API_TYPE_SENSORHUB;
    rui3_api->flag   = RUI3API_FLG_REQ;
    hub_api->source  = PID_MASTER;
    hub_api->dest    = pid;
    hub_api->sequence= ++menu.seq;
    pktLen = sizeof(SNHub_Api_t);

    switch (gset)
    {
        case SNHUB_GS_SET:
            hub_api->type    = SNHUB_TYPE_PROVISION;
            break;
        default:
        case SNHUB_GS_GET:
            return RET_ERROR;
    }

    pktLen+=pldLen;
    hub_api->payload_length = pldLen;
    hub_api->payload_type = ptye;
    
    switch (ptye)
    {
        default:
        case PLD_PROVI_TYPE_VER1:
        case PLD_PROVI_TYPE_VER2:
        case PLD_PROVI_TYPE_VER3:
            /* not support */
            return RET_ERROR;
        case PLD_PROVI_TYPE_BOOT:
            break;
    }
    
    rui3_api->length.value = pktLen;
    #define SHORT_SWAP(X)   (((X&0xFF)<<8) | ((X>>8)&0xFF))
    rui3_api->length.value = SHORT_SWAP(rui3_api->length.value);
    rui3_api->payload[pktLen] = cal_chksum((U8*)rui3_api,BUFF_SIZE);

    pktLen += sizeof(RUI3_Api_t);
    pktLen += 1 /* add checksum len*/;

    on_evt(PID_MASTER, sid, SNHUBAPI_EVT_QSEND, pktBuff, pktLen);

    return RET_OK;
}

static RET_S32 snhub_snsrdat_command( U8 pid, U8 sid , SNHUB_GS_E gset, U8 ptye )
{
    U8 pktBuff[BUFF_SIZE];
    U16 pktLen = 0;
    U16 pldLen = 0;
    
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)pktBuff;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    
    f_memset(pktBuff, 0, BUFF_SIZE);

    rui3_api->wakeup = WAKEUPBYTE;
    rui3_api->start  = DELIMTER;
    rui3_api->type   = RUI3API_TYPE_SENSORHUB;
    rui3_api->flag   = RUI3API_FLG_REQ;
    hub_api->source  = PID_MASTER;
    hub_api->dest    = pid;
    hub_api->sequence= ++menu.seq;
    pktLen = sizeof(SNHub_Api_t);

    switch (gset)
    {
        case SNHUB_GS_GET:
            hub_api->type    = SNHUB_TYPE_SENDAT;
            break;
        default:
        case SNHUB_GS_SET:
            return RET_ERROR;
    }

    pktLen+=pldLen;
    hub_api->payload_length = pldLen;
    hub_api->payload_type = ptye;
    
    switch (ptye)
    {
        case PLD_SDATA_TPYE_SENDAT:
            hub_api->payload[0] = sid;
            break;

        case PLD_SDATA_TPYE_VER:
        case PLD_SDATA_TPYE_PARAMSET:
        case PLD_SDATA_TPYE_CONTROL:
            /* not support */
            return RET_ERROR;
    }
    
    rui3_api->length.value = pktLen;
    rui3_api->length.value = SHORT_SWAP(rui3_api->length.value);
    rui3_api->payload[pktLen] = cal_chksum((U8*)rui3_api,BUFF_SIZE);

    pktLen += sizeof(RUI3_Api_t);
    pktLen += 1 /* add checksum len*/;

    on_evt(PID_MASTER, sid, SNHUBAPI_EVT_QSEND, pktBuff, pktLen);

    return RET_OK;
}

static RET_S32 verify_sequence( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    
    do
    {
        if ( rui3_api->flag == RUI3API_FLG_REQ)
        {
            break;
        }

        if ( menu.result != SNHUB_RES_OK )
        {
            if (hub_api->sequence == menu.seq)
            {
                menu.result = SNHUB_RES_OK;
            }
            else
            {
                on_evt(PID_MASTER, 0, SNHUBAPI_EVT_SEQ_ERR, NULL, 0);
            }
        }
    } while (0);

    return RET_OK;
}

static RET_S32 verify_snhublen( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    
    U16 rui3_api_len = LSB_COMB(rui3_api->length.hbyte, rui3_api->length.lbyte);
    U16 hub_api_len = hub_api->payload_length;
    if ( rui3_api_len == (hub_api_len + sizeof(SNHub_Api_t)))
    {
        return RET_OK;
    }
    
    /* old version will add crc byte, keep this issue */
    if ( rui3_api_len == ((hub_api_len + sizeof(SNHub_Api_t)) - 1))
    {
        return RET_OK;
    }

    return RET_ERROR;
}

static RET_S32 verify_delimter( U8 *data, U16 len )
{
    for (U16 ofs = 0; ofs < len; ofs++)
    {
        if (data[ofs] == DELIMTER)
        {
            return ofs; /* recover wakeup byte*/;
        }
    }
    return RET_ERROR;
}

static RET_S32 verify_rui3type( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;

    switch (rui3_api->type)
    {
        case RUI3API_TYPE_SENSORHUB:
            return RET_OK;
        case RUI3API_TYPE_ECHO:
        case RUI3API_TYPE_ATCMD:
        default:
            break;
    }
    return RET_ERROR;
}

static RET_S32 verify_checksum( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;

    U8 chsum = cal_chksum((U8*)rui3_api,len);
    U16 payload_len = LSB_COMB(rui3_api->length.hbyte, rui3_api->length.lbyte);

    if (chsum == rui3_api->payload[payload_len])
    {
        return RET_OK;
    }
    
    on_evt(PID_MASTER, 0, SNHUBAPI_EVT_CHKSUM_ERR, NULL, 0);
    return RET_ERROR;
}

static RET_S32 verify_action( U8 *data, U16 len )
{
    RUI3_Api_t *rui3_api = (RUI3_Api_t *)data;
    SNHub_Api_t *hub_api = (SNHub_Api_t*)(rui3_api->payload);
    RET_S32 ret = 0;
    program_process program = NULL;

    switch (rui3_api->flag)
    {
        case RUI3API_FLG_REQ:
            program = protocol_list[hub_api->type].req;
            on_evt(hub_api->source, 0, SNHUBAPI_EVT_RECV_REQ, data, len);
            break;
        
        case RUI3API_FLG_RSP:
            program = protocol_list[hub_api->type].rsp;
            on_evt(hub_api->source, 0, SNHUBAPI_EVT_RECV_RSP, data, len);
            break;

        default:
            return RET_ERROR;
    }

    if( program != NULL)
    {
        program(data,len);
    }

    return ret;
}

static void api_init( SNHub_Evt_t this_on_evt )
{
    on_evt = this_on_evt;
    
    command_list[SNHUB_TYPE_PROVISION](PID_UNKNOW,0,SNHUB_GS_SET,PLD_PROVI_TYPE_BOOT);
}

static void api_process( U8 *msg, U16 len )
{
    U16 dataLen = 0;
    U16 dataOffset = 0;
    U8 *pdata = NULL;

    do
    {
        /* check default packet len */
        if (len < (sizeof(RUI3_Api_t) + sizeof(SNHub_Api_t)))
        {
            break;
        }
        
        /* keep wakeup byte set */
        dataOffset = verify_delimter(msg,len);
        if (dataOffset == RET_ERROR)
        {
            break;
        }
        
        /* copy data to local buff */
        f_memset(dataBuff, 0, BUFF_SIZE);
        f_memcpy(&dataBuff[1], &msg[dataOffset], (U8)len);

        dataBuff[0] = WAKEUPBYTE;
        pdata       = &dataBuff[0];
        dataLen     = len - dataOffset + 1 /* recover wakeup byte*/;


        /* verify packet checksum */
        if (verify_checksum(pdata, dataLen) != RET_OK)
        {
            break;
        }
        
        /* verify rui3 api type, type have to eq RUI3API_TYPE_SENSORHUB */
        if (verify_rui3type(pdata, dataLen) != RET_OK)
        {
            break;
        }

        /* verify rui3 api len and snhub len is mapping */
        if (verify_snhublen(pdata, dataLen) != RET_OK)
        {
            break;
        }

        /* verify sequence */
        verify_sequence(pdata, dataLen);

        /* todo request/response program */
        if (verify_action(pdata, dataLen) != RET_OK)
        {
            break;
        }
    } while (0);
}

static const command_process command_list[] = 
{
    [SNHUB_TYPE_WAKUP]      = NULL,
    [SNHUB_TYPE_PROVISION]  = snhub_provision_command,
    [SNHUB_TYPE_PARAMSET]   = NULL,
    [SNHUB_TYPE_SENDAT]     = snhub_snsrdat_command,
    [SNHUB_TYPE_CONTROL]    = NULL,
    [SNHUB_TYPE_PARAMGET]   = snhub_paramget_command,
    [SNHUB_TYPE_ALERT]      = NULL,
    [SNHUB_TYPE_ERR]        = NULL,
    [SNHUB_TYPE_ACK]        = NULL,
    [SNHUB_TYPE_NACK]       = NULL,
    [SNHUB_TYPE_YMDM]       = NULL,
    [SNHUB_TYPE_IOC]        = NULL,
    [SNHUB_TYPE_MODBUS]     = NULL,
    [SNHUB_TYPE_SDI12]      = NULL,
    [SNHUB_TYPE_ADC]        = NULL,
    [SNHUB_TYPE_DIO]        = NULL,
};

static const protocol_process_t protocol_list[] =
{
    
    [SNHUB_TYPE_WAKUP]      = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_PROVISION]  = { .req=snhub_provision_req_program  ,.rsp=NULL },
    [SNHUB_TYPE_PARAMSET]   = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_SENDAT]     = { .req=snhub_snsrdat_req_program    ,.rsp=snhub_snsrdat_rsp_program },
    [SNHUB_TYPE_CONTROL]    = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_PARAMGET]   = { .req=NULL                         ,.rsp=snhub_paramget_rsp_program },
    [SNHUB_TYPE_ALERT]      = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_ERR]        = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_ACK]        = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_NACK]       = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_YMDM]       = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_IOC]        = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_MODBUS]     = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_SDI12]      = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_ADC]        = { .req=NULL                         ,.rsp=NULL },
    [SNHUB_TYPE_DIO]        = { .req=NULL                         ,.rsp=NULL },
};

static void api_get_snsr_data( U8 pid )
{
    if (pid == PID_MASTER )
    {
        return;
    }

    menu.result = SNHUB_RES_BUSY;
    CKTODO(command_list[SNHUB_TYPE_SENDAT])(pid,0,SNHUB_GS_GET,PLD_SDATA_TPYE_SENDAT);
}

static void api_get_snsr_param( U8 pid , U8 sid)
{
    if (pid == PID_MASTER )
    {
        return;
    }
    
    menu.result = SNHUB_RES_BUSY;
    CKTODO(command_list[SNHUB_TYPE_PARAMGET])(pid,sid,SNHUB_GS_GET,PLD_PARMGSET_TYPE_SNSR_UPDATE);
}

static void api_set_snsr_param( U8 pid , U8 sid, U8 enb, U32 intv)
{
    if (pid == PID_MASTER )
    {
        return;
    }

    menu.intv = intv;
    menu.rule = (enb == 0)?RULE_DISABLE:RULE_PERIODIC;
    menu.result = SNHUB_RES_BUSY;
    CKTODO(command_list[SNHUB_TYPE_PARAMGET])(pid,sid,SNHUB_GS_SET,PLD_PARMGSET_TYPE_SNSR_UPDATE);
}

static void api_set_provision()
{
    menu.result = SNHUB_RES_BUSY;
    CKTODO(command_list[SNHUB_TYPE_PROVISION])(PID_UNKNOW,0,SNHUB_GS_SET,PLD_PROVI_TYPE_BOOT);
}

const RakSNHub_Protocl_API_t  RakSNHub_Protocl_API = 
{
    .init       = api_init,
    .process    = api_process,

    .get.data   = api_get_snsr_data,
    .get.param  = api_get_snsr_param,

    .set.param  = api_set_snsr_param,

    .reboot     = api_set_provision,
};

