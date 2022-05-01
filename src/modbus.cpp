//
// Created by Ico on 14/02/2022.
//

#if USE_MODBUS
//----------------------------------------------------------------------------
void ModbusTaskSetup(void)
{
    mb.client();
    ModbusStart();
}
//----------------------------------------------------------------------------
void ModbusTaskLoop(void)
{
    digitalWrite(LED_BUILTIN, mb.isConnected(MbServer) ? LOW : HIGH);

    switch(ModbusControl.TaskState)
    {
        case MODBUS_START:
        {
            if(IsWifiConnected())
            {
                if(mb.isConnected(MbServer))
                {
                    ModbusControl.TaskState = MODBUS_CONNECTED;
                    Serial.printf("MODBUS: Connected.\n");
                }
                else
                {
                    Serial.printf("MODBUS: Connecting...\n");
                    mb.connect(MbServer,MODBUS_PORT);           // Try to connect if no connection (connect is synchronous: returns after connection is established)
                }
            }
        }
            break;
        case MODBUS_CONNECTED:
        {
            if(mb.isConnected(MbServer))
            {  // If still connected
                if(ModbusControl.prHreg && ModbusControl.nrHreg)
                {  // If any read to do...
//            Serial.printf("MODBUS: Starting read transaction\n");

                    ModbusControl.Trans = mb.readHreg(MbServer, ModbusControl.OfsrHreg, ModbusControl.prHreg, ModbusControl.nrHreg);  // Initiate Read HR from Modbus Slave
                    ModbusControl.Timer = SetTimer(MODBUS_TRANSACTION_TIME);
                    ModbusControl.TaskState = MODBUS_READ_TRANS;
                }
                else if(ModbusControl.pwHreg && ModbusControl.nwHreg)
                {  // else if any write to do...
//            Serial.printf("MODBUS: Starting write transaction -> [%d]\n",ModbusControl.pwHreg[0]);

                    ModbusControl.Trans = mb.writeHreg(MbServer, ModbusControl.OfswHreg, ModbusControl.pwHreg, ModbusControl.nwHreg);  // Initiate Write HR to Modbus Slave
                    ModbusControl.Timer = SetTimer(MODBUS_TRANSACTION_TIME);
                    ModbusControl.TaskState = MODBUS_WRITE_TRANS;
                }
// Send and receive
            }
            else
            {  // Reconnect
                Serial.printf("MODBUS: Connection lost. Restart connection\n");
                ModbusControl.TaskState = MODBUS_START;
            }
        }
            break;
        case MODBUS_READ_TRANS:
        {
            if(mb.isTransaction(ModbusControl.Trans))
            { // If read still pending
                if(IsTimerExpired(ModbusControl.Timer))
                {  // If timeout occurred, restart connection
                    Serial.printf("MODBUS: Read transaction timeout...\n");
                    ModbusControl.TaskState = MODBUS_RESTART;
                }
            }
            else if(ModbusControl.pwHreg && ModbusControl.nwHreg)
            {  // Start pending write if any
//         Serial.printf("MODBUS: Starting write transaction -> [%d]\n",ModbusControl.pwHreg[0]);

                ModbusControl.Trans = mb.writeHreg(MbServer, ModbusControl.OfswHreg, ModbusControl.pwHreg, ModbusControl.nwHreg);  // Initiate Write HR to Modbus Slave
                ModbusControl.Timer = SetTimer(MODBUS_TRANSACTION_TIME);
                ModbusControl.TaskState = MODBUS_WRITE_TRANS;
            }
            else
            {  // Restart communication cycle
                ModbusControl.TaskState = MODBUS_CONNECTED;
            }
        }
            break;
        case MODBUS_WRITE_TRANS:
        {
            if(mb.isTransaction(ModbusControl.Trans))
            {  // If write still pending
                if(IsTimerExpired(ModbusControl.Timer))
                { // If timeout occurred, restart connection
                    Serial.printf("MODBUS: Write transaction timeout...\n");
                    ModbusControl.TaskState = MODBUS_RESTART;
                }
            }
            else
            {  // Restart communication cycle
                ModbusControl.TaskState = MODBUS_CONNECTED;
            }
        }
            break;
        case MODBUS_RESTART:
        case MODBUS_END:
        {
            mb.disconnect(MbServer);
            mb.dropTransactions();              // Cancel all waiting transactions

            Serial.printf("MODBUS: Disconnect and %s\n",MODBUS_END == ModbusControl.TaskState ? "Go to IDLE" : "RESTART");

            ModbusControl.TaskState = MODBUS_END == ModbusControl.TaskState ? MODBUS_IDLE : MODBUS_START;
        }
            break;
        case MODBUS_IDLE:
        {
        }
            break;
        default:
        {
        }
            break;
    }
    mb.task();                      // Common local Modbus task
}
//----------------------------------------------------------------------------
void ModbusStart(void)
{
    memset((void*) &ModbusControl, 0, sizeof(ModbusControl));
    ModbusControl.TaskState = MODBUS_START;
    Serial.printf("ModBusStart.\n");
}
//----------------------------------------------------------------------------
void ModbusStop(bool fRestart)
{
    ModbusControl.TaskState = fRestart ? MODBUS_RESTART : MODBUS_END;
    Serial.printf("ModBusStop [%s].\n",fRestart ? "RESTART" : "END");
}
//----------------------------------------------------------------------------
void ModbusSetWHreg(uint16_t* p, uint16_t ofs, uint16_t n)
{
    ModbusControl.pwHreg = p;
    ModbusControl.nwHreg = n;
    ModbusControl.OfswHreg = ofs;
}
//----------------------------------------------------------------------------
void ModbusSetRHreg(uint16_t* p, uint16_t ofs, uint16_t n)
{
    ModbusControl.prHreg = p;
    ModbusControl.nrHreg = n;
    ModbusControl.OfsrHreg = ofs;
}
//----------------------------------------------------------------------------
#endif

