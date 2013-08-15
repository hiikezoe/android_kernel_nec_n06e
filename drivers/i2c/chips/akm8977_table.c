/**********************************************************************
* File Name: drivers/i2c/chips/akm8977_table.c
* 
* (C) NEC CASIO Mobile Communications, Ltd. 2013
**********************************************************************/







#include <linux/akm8977_table.h>
#include <linux/akm8977.h>

extern int et_pedometerOFF(void);           
extern int et_pedometerON(void);            
extern int et_lowPassFilterOFF(void);       
extern int et_lowPassFilterON(void);        
extern int et_startMeasureCONT(void);       
extern int et_powerDown(void);              
extern int et_ignore(void);                 

extern int AKM_get_state(int state);        


const REGISTER_STATE_ST event_table[STATE_MAX][EVENT_MAX - 2] =
{
                
    {
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_lowPassFilterON,  ms : et_startMeasureCONT, nextState : STATE1__CONTON__LFPON__PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE4__CONTOFF_LFPOFF_PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF, },
  { slct1 : et_pedometerON,  slct2 : et_lowPassFilterON,  ms : et_startMeasureCONT, nextState : STATE2__CONTOFF_LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF, },
    },
		
    {
  { slct1 : et_ignore,       slct2 : et_lowPassFilterOFF, ms : et_powerDown,        nextState : STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE1__CONTON__LFPON__PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE1__CONTON__LFPON__PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE5__CONTON__LFPON__PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE1__CONTON__LFPON__PEDOOFF_VIBOFF, },
  { slct1 : et_pedometerON,  slct2 : et_ignore,           ms : et_ignore,           nextState : STATE3__CONTON__LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE1__CONTON__LFPON__PEDOOFF_VIBOFF, },
    },
		
    {
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE2__CONTOFF_LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE3__CONTON__LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE2__CONTOFF_LFPON__PEDOON__VIBOFF, },
  { slct1 : et_pedometerOFF, slct2 : et_lowPassFilterOFF, ms : et_powerDown,        nextState : STATE6__CONTOFF_LFPOFF_PEDOON__VIBON_, },
  { slct1 : et_pedometerOFF, slct2 : et_lowPassFilterOFF, ms : et_powerDown,        nextState : STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE2__CONTOFF_LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE2__CONTOFF_LFPON__PEDOON__VIBOFF, },
    },
		
    {
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE2__CONTOFF_LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE3__CONTON__LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE3__CONTON__LFPON__PEDOON__VIBOFF, },
  { slct1 : et_pedometerOFF, slct2 : et_ignore,           ms : et_ignore,           nextState : STATE7__CONTON__LFPON__PEDOON__VIBON_, },
  { slct1 : et_pedometerOFF, slct2 : et_ignore,           ms : et_ignore,           nextState : STATE1__CONTON__LFPON__PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE3__CONTON__LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE3__CONTON__LFPON__PEDOON__VIBOFF, },
    },
		
    {
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE4__CONTOFF_LFPOFF_PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_lowPassFilterON,  ms : et_startMeasureCONT, nextState : STATE5__CONTON__LFPON__PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE4__CONTOFF_LFPOFF_PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE4__CONTOFF_LFPOFF_PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE6__CONTOFF_LFPOFF_PEDOON__VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE0__CONTOFF_LFPOFF_PEDOOFF_VIBOFF, },
    },
		
    {
  { slct1 : et_ignore,       slct2 : et_lowPassFilterOFF, ms : et_powerDown,        nextState : STATE4__CONTOFF_LFPOFF_PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE5__CONTON__LFPON__PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE1__CONTON__LFPON__PEDOOFF_VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE5__CONTON__LFPON__PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE5__CONTON__LFPON__PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE7__CONTON__LFPON__PEDOON__VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE1__CONTON__LFPON__PEDOOFF_VIBOFF, },
    },
		
    {
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE6__CONTOFF_LFPOFF_PEDOON__VIBON_, },
  { slct1 : et_ignore,       slct2 : et_lowPassFilterON,  ms : et_startMeasureCONT, nextState : STATE7__CONTON__LFPON__PEDOON__VIBON_, },
  { slct1 : et_pedometerON,  slct2 : et_lowPassFilterON,  ms : et_startMeasureCONT, nextState : STATE2__CONTOFF_LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE6__CONTOFF_LFPOFF_PEDOON__VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE4__CONTOFF_LFPOFF_PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE6__CONTOFF_LFPOFF_PEDOON__VIBON_, },
  { slct1 : et_pedometerON,  slct2 : et_lowPassFilterON,  ms : et_startMeasureCONT, nextState : STATE2__CONTOFF_LFPON__PEDOON__VIBOFF, },
    },
		
    {
  { slct1 : et_ignore,       slct2 : et_lowPassFilterOFF, ms : et_powerDown,        nextState : STATE6__CONTOFF_LFPOFF_PEDOON__VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE7__CONTON__LFPON__PEDOON__VIBON_, },
  { slct1 : et_pedometerON,  slct2 : et_ignore,           ms : et_ignore,           nextState : STATE3__CONTON__LFPON__PEDOON__VIBOFF, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE7__CONTON__LFPON__PEDOON__VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE5__CONTON__LFPON__PEDOOFF_VIBON_, },
  { slct1 : et_ignore,       slct2 : et_ignore,           ms : et_ignore,           nextState : STATE7__CONTON__LFPON__PEDOON__VIBON_, },
  { slct1 : et_pedometerON,  slct2 : et_ignore,           ms : et_ignore,           nextState : STATE3__CONTON__LFPON__PEDOON__VIBOFF, },
    },
};

const REGISTER_ST state_table[STATE_MAX] =
{
   { slct1_peen : SLCT1_PEEN_OFF, slct2_lfen : SLCT2_LFEN_OFF, ms_mode : MS_MODE_OFF },
   { slct1_peen : SLCT1_PEEN_OFF, slct2_lfen : SLCT2_LFEN_ON , ms_mode : MS_MODE_ON  },
   { slct1_peen : SLCT1_PEEN_ON , slct2_lfen : SLCT2_LFEN_ON , ms_mode : MS_MODE_ON  },
   { slct1_peen : SLCT1_PEEN_ON , slct2_lfen : SLCT2_LFEN_ON , ms_mode : MS_MODE_ON  },
   { slct1_peen : SLCT1_PEEN_OFF, slct2_lfen : SLCT2_LFEN_OFF, ms_mode : MS_MODE_OFF },
   { slct1_peen : SLCT1_PEEN_OFF, slct2_lfen : SLCT2_LFEN_ON , ms_mode : MS_MODE_ON  },
   { slct1_peen : SLCT1_PEEN_OFF, slct2_lfen : SLCT2_LFEN_OFF, ms_mode : MS_MODE_OFF },
   { slct1_peen : SLCT1_PEEN_OFF, slct2_lfen : SLCT2_LFEN_ON , ms_mode : MS_MODE_ON  },
};
