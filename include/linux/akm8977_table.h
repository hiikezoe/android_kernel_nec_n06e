/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/








#if !defined(AKM8977_TABLE_H_)
#define AKM8977_TABLE_H_

#define TABLE_REG   3

#define SLCT1_PEEN_OFF          0x00
#define SLCT1_PEEN_ON           0x04
#define SLCT2_LFEN_OFF          0x00
#define SLCT2_LFEN_ON           0x08
#define MS_MODE_OFF             0x00
#define MS_MODE_ON              0x02


typedef enum _EVENT_NUM
{
    EVENT_POWER_OFF = 0,
    EVENT_POWER_ON,
    EVENT_VIB_OFF,
    EVENT_VIB_ON,
    
    
    
    
    EVENT_PEDO_OFF,
    EVENT_PEDO_ON,
    EVENT_TIMEOUT,
    
    EVENT_SPK_OFF,
    EVENT_SPK_ON,
    
    EVENT_MAX,
    EVENT_NONE = (-1),
}EVENT_NUM;

typedef enum _STATE_NUM
{
    
    
    STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF = 0,
    
    STATE1__CONTON__LFPON__PEDOOFF_VIBOFF,
    
    STATE2__CONTOFF_LFPON__PEDOON__VIBOFF,
    
    STATE3__CONTON__LFPON__PEDOON__VIBOFF,
    
    STATE4__CONTOFF_LFPOFF_PEDOOFF_VIBON_,
    
    STATE5__CONTON__LFPON__PEDOOFF_VIBON_,
    
    STATE6__CONTOFF_LFPOFF_PEDOON__VIBON_,
    
    STATE7__CONTON__LFPON__PEDOON__VIBON_,
    
    
    
    
    
    
    
    
    
    STATE_MAX,
    STATE_NONE = (-1),
}STATE_NUM;

typedef struct register_state_tag
{
    int (*slct1)(void);
    int (*slct2)(void);
    int (*ms)(void);
    int nextState;
} REGISTER_STATE_ST;

typedef struct register_tag
{
    int slct1_peen;
    int slct2_lfen;
    int ms_mode;
} REGISTER_ST;

extern const REGISTER_STATE_ST event_table[STATE_MAX][EVENT_MAX - 2];
extern const REGISTER_ST state_table[STATE_MAX];

#endif 
