/* //////////////////////////////////////////////////////////////////////////*/
/* Included	in "dsPid_definitions.h", it contains functions prototypes       */
/* //////////////////////////////////////////////////////////////////////////*/

int PosIndx(float Pos);
void SendMap(void);
unsigned char GetMap(int Xpoint, int Ypoint);
void Pid1Calc (void);
void Pid2Calc (void);
void ThetaDesF(float Angle);
void ConstantsError(void);
void ConstantsDefaultW (void);
void ConstantsDefaultR (void);
float ObstacleAvoidance(float DPosX, float DPosY, int Dist);
void Scheduler(void);
void AdcCalc(void);
void TxCont(void);
unsigned char Slam(float PosX, float PosY, int Cell);
void ConstantsWrite(char Row);
void DelayN1ms(int n);
void DelayN10us(int n);
void InitDistPid(void);
void InitAnglePid(void);
void Navigation(void);
void Orientation(void);
void DeadReckoning(void);
void ConstantsRead(void);
unsigned char IncrCircPtr(int Port);
void InitPid1(void);
void InitPid2(void);
void Pid1(void);
void Pid2(void);
void TxParameters(char TxCmd,int TxCmdLen, int Port);
void UartTx(void);
void UartRxError(int Err, int Port);
unsigned char UartChkSum (unsigned char *,unsigned int);
void Parser (void);
void UartRx(void);
void Uart2Rx(void);
void Settings(void);
void ISR_Settings(void);
void UsartSetting(void);
void Usart2Setting(void);
void _ISR _INT1Interrupt(void);
void _ISR _U1RXInterrupt(void);
void _ISR _U1TXInterrupt(void);
void _ISR _IC1Interrupt(void);
void _ISR _T1Interrupt(void);
void _ISR _CNInterrupt(void);

// void _ISR _ADCInterrupt(void);
// void _ISR _QEIInterrupt(void);

