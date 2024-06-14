#ifndef __onewire_master_protocol_h__
#define __onewire_master_protocol_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef S32
#define S32 int
#endif

#ifndef U8
#define U8 unsigned char
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef U32
#define U32 unsigned int
#endif

#ifndef ATT_PACKED
#define ATT_PACKED __attribute__((packed))
#endif

#ifndef RET_OK
#define RET_OK 0
#endif

#ifndef RET_ERROR
#define RET_ERROR -1
#endif

#ifndef RET_S32
#define RET_S32 S32
#endif

#define WAKEUPBYTE 0xFF
#define DELIMTER 0x7E
#define PID_UNKNOW 0xFF
#define PID_MASTER 0x00

#define RULE_DISABLE 0x00
#define RULE_PERIODIC 0x08

typedef union {
    U16 value;
    U16 msb;
    struct {
        U8 lbyte;
        U8 hbyte;
    };
} RUI3API_LEN_T;

typedef enum {
    RUI3API_TYPE_ECHO = 0,
    RUI3API_TYPE_ATCMD,
    RUI3API_TYPE_SENSORHUB,
    RUI3API_TYPE_MAX,
} RUI3API_TYPE_E;

typedef enum {
    RUI3API_FLG_REQ = 0,
    RUI3API_FLG_RSP,
    RUI3API_FLG_MAX,
} RUI3API_FLG_E;

typedef enum {
    SNHUB_GS_GET,
    SNHUB_GS_SET,
    SNHUB_GS_MAX,
} SNHUB_GS_E;

typedef enum {
    SNHUB_RES_OK,
    SNHUB_RES_BUSY,
    SNHUB_RES_MAX,
} SNHUB_RES_E;

typedef enum {
    SNHUB_TYPE_WAKUP = 0,
    SNHUB_TYPE_PROVISION,
    SNHUB_TYPE_PARAMSET,
    SNHUB_TYPE_SENDAT,
    SNHUB_TYPE_CONTROL,
    SNHUB_TYPE_PARAMGET,
    SNHUB_TYPE_ALERT,
    SNHUB_TYPE_ERR,
    SNHUB_TYPE_ACK,
    SNHUB_TYPE_NACK,
    SNHUB_TYPE_YMDM,
    SNHUB_TYPE_IOC,
    SNHUB_TYPE_MODBUS,
    SNHUB_TYPE_SDI12,
    SNHUB_TYPE_ADC,
    SNHUB_TYPE_DIO,
    SNHUB_TYPE_MAX,
} SNHUBAPI_TYPE_E;

typedef enum {
    PLD_SDATA_TPYE_VER,
    PLD_SDATA_TPYE_PARAMSET,
    PLD_SDATA_TPYE_SENDAT,
    PLD_SDATA_TPYE_CONTROL,
    PLD_SDATA_TPYE_MAX,
} PLD_SDATA_TYPE_E;

typedef enum {
    PLD_PROVI_TYPE_VER1,
    PLD_PROVI_TYPE_VER2,
    PLD_PROVI_TYPE_BOOT,
    PLD_PROVI_TYPE_VER3,
} PLD_PROVI_TYPE_E;

typedef enum {
    PLD_PARMGSET_TYPE_PRB_INTV = 0x1,
    PLD_PARMGSET_TYPE_SNSR_INTV,
    PLD_PARMGSET_TYPE_RULE,
    PLD_PARMGSET_TYPE_SNSR_HTHR,
    PLD_PARMGSET_TYPE_SNSR_LTHR,
    PLD_PARMGSET_TYPE_PRB_TAGID,
    PLD_PARMGSET_TYPE_PRB_TAGEN,
    PLD_PARMGSET_TYPE_PRB_UPDATE,
    PLD_PARMGSET_TYPE_SNSR_UPDATE,
    PLD_PARMGSET_TYPE_CONF_UPDATE,
} PLD_PARMGSET_TYPE_E;

typedef struct {
    U8 dest;
    U8 source;
    U8 sequence;
    SNHUBAPI_TYPE_E type;
    U8 payload_length;
    U8 payload_type;
    U8 payload[];
} ATT_PACKED SNHub_Api_t;

typedef struct {
    /*
        wakeup, RO
        default: 0xff
        bypass first byte
    */
    U8 wakeup;

    /*
        start, RO
        default: 0x7E
        start byte
    */
    U8 start;

    RUI3API_LEN_T length;

    RUI3API_TYPE_E type;

    RUI3API_FLG_E flag;

    U8 payload[];
} ATT_PACKED RUI3_Api_t;

typedef struct {
    U8 u[18];
} SERIALNUM;
typedef struct {
    U8 u[20];
} MDLNAME;
typedef struct {
    U8 sid;
    U8 ipso;
    U16 rule;
} SNSRNODE;

typedef struct {
    U8 hw_version;
    U8 sw_version[3];
    SERIALNUM sn;
    U8 provId;
    U8 reserved1[7];
    MDLNAME model_name;
    U8 reserved2[4];
    U8 snsr_num;
    SNSRNODE snsr_type[];
} ATT_PACKED SNHub_Api_Provision_t;

typedef struct {
    U8 sid;
    U32 intv;
    U16 rule;
    U8 thr_above[10];
    U8 thr_below[10];
    U8 tag[16];
} ATT_PACKED SNHub_Api_Param_Snsr_t;

typedef struct {
    U8 sid;
    U8 ipso;
    U8 value[];
} ATT_PACKED SNHub_Api_sData_Snsr_t;

typedef struct {
    U8 alive;
    SNHub_Api_Provision_t info;
    U8 snsrnum;
    U8 snsrlist[8];
} ATT_PACKED SNHub_Record_t;

typedef struct {
    U8 seq;
    U8 rule;
    U32 intv;
    SNHUB_RES_E result;
} ATT_PACKED SNHub_Menu_t;

#ifdef __cplusplus
}
#endif

#endif