//
// Created by Ico on 14/02/2022.
//

#ifndef SCD30_MODBUS_H
#define SCD30_MODBUS_H

#include <ModbusIP_ESP8266.h>

#define     MODBUS_SERVER              192, 168, 2, 31
#define     MODBUS_PORT                503

#define     MODBUS_TRANSACTION_TIME    100

typedef enum
{
    MODBUS_START,
    MODBUS_CONNECTED,
    MODBUS_WRITE_TRANS,
    MODBUS_READ_TRANS,
    MODBUS_RESTART,
    MODBUS_END,
    MODBUS_IDLE,
} MODBUS_TASK_STATE;


typedef struct
{
    uint8_t           TaskState;
    unsigned long     Timer;
    uint16_t*         pwHreg;
    uint16_t          nwHreg;
    uint16_t          OfswHreg;
    uint16_t*         prHreg;
    uint16_t          nrHreg;
    uint16_t          OfsrHreg;
    uint16_t          Trans;
} MODBUS_CONTROL;

MODBUS_CONTROL   ModbusControl = {0};

IPAddress         MbServer(MODBUS_SERVER);

ModbusIP mb;  //ModbusIP object


void ModbusTaskSetup(void);
void ModbusTaskLoop(void);
void ModbusStart(void);
void ModbusStop(bool fRestart);
void ModbusSetWHreg(uint16_t* p, uint16_t ofs, uint16_t n);
void ModbusSetRHreg(uint16_t* p, uint16_t ofs, uint16_t n);

#endif //SCD30_MODBUS_H
