/* ////////////////////////////////////////////////////////////////////////////
** File:      dsPid33.c
*/                                  
 unsigned char  Ver[] = "dsPid33 2.2.4 Guiott 08-10"; // 26+1 char
/* Author:    Guido Ottaviani-->g.ottaviani@mediaprogetti.it<--
** Description: This is a porting on a single dsPIC33FJ64MC802 of previous 
**				double PID Motor Control (dsPID program) formerly performed 
**				with two dsPIC30F4012 plus odometry and field mapping formerly 
**				performedwith a single dsPIC30F3013 (dsODO program).   
**
** Detailed descriptions are on file "descrEng.txt" 
** numbers between brackets, eg.: [1] , are the references to the specific 
** decription into the file
**
-------------------------------------------------------------------------------
Copyright 2010 Guido Ottaviani
guido@guiott.com

	dsPID33 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    dsPID33 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with dsPID33.  If not, see <http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------------      
/////////////////////////////////////////////////////////////////////////////*/

#include "dsPID33_definitions.h"
#include "DEE Emulation 16-bit.h"

int main (void)
{
Settings();

LED1 = 1;			// [1]
LED2 = 0;
// MOTOR_ENABLE1 = 0;	// [1]
// MOTOR_ENABLE2 = 0;	// [1]

Tmr2OvflwCount1=0;
Tmr2OvflwCount2=0;
IC1_FIRST = 0;
IC2_FIRST = 0;
RAMP_FLAG1 = 0;
RAMP_FLAG2 = 0;
PID_REF1 = 0;
PID_REF2 = 0;
VelDecr = 1;

// to start first and second slower cycles over 1ms cycle
Cycle1 = 0;
Cycle2 = 0;
CYCLE1_FLAG = 0;
CYCLE2_FLAG = 0;

ORIENTATION_FLAG = 0;
MAP_SEND_FLAG=0;

/*Delay needed to ensure the program memory is not modified before 
  verification in programming procedure
 	http://forum.microchip.com/tm.aspx?m=326442 */
DelayN1ms(30);
DataEEInit();	// Initialize flash memory writing [33]
dataEEFlags.val = 0;

BlinkPeriod = NORM_BLINK_PER;	// LED1 blinking period (ms)
BlinkOn     = NORM_BLINK_ON;	// LED1 on time (ms)

ConstantsRead();	// get stored constant values from FLASH

InitPid1();
InitPid2();

ANGLE_PID_DES=0;
InitAnglePid();

DIST_PID_DES=0;
InitDistPid();


CONSOLE_DEBUG = 0;

ADC_CALC_FLAG = 0;
IdleSample = 0;
IdleCount = 0;

DIST_OK_FLAG  = 1;	// to kick start the scheduler sequence
SchedPtr = 0;		// [32]
	
RT_TIMER_FLAG = 0;	// disable real time timer
UsartSetting();		// initialize Usart1
Usart2Setting();	// initialize Usart2
TxContFlag = 0;		

ResetCount = 0;

ISR_Settings();	// Configures and enables ISRs	
	
// Send string 'Ver'
for (i=0; i<26; i++)
{
	UartTxBuff[i]=Ver[i]; 
}
DMA6CNT = 25;			// # of DMA requests
DMA6CONbits.CHEN  = 1;	// Re-enable DMA Channel
DMA6REQbits.FORCE = 1;	// Manual mode: Kick-start the first transfer	

MAP_BUFF_FLAG=1;

			
/*===========================================================================*/
/* Main loop                                                                 */
/*===========================================================================*/

while (1)	// start of idle cycle [25a]
{	
/* ----------------------------------------- one character coming from UART1 */	
if (UartRxPtrIn != UartRxPtrOut) UartRx();	// [6d]

/* ----------------------------------------- one character coming from UART2 */	
if (Uart2RxPtrIn != Uart2RxPtrOut) Uart2Rx();	// [6zd]

/* ------------------------ a command is coming from serial interface 1 or 2 */	 
if ((UartRxStatus == 99) || (Uart2RxStatus == 99)) Parser();

/* ------------------------------------- PID and speed calculation every 1ms */	
if (PID1_CALC_FLAG)	Pid1();
if (PID2_CALC_FLAG)	Pid2();

/* ----------------------------------------- DeadReckonig field mapping [22] */	 
if (CYCLE1_FLAG) DeadReckoning();

/* ---------------------------------------------------- Orientation PID [23] */	 
if (ORIENTATION_FLAG) Orientation();

/* ----------------------------------------------------------- Distance [24] */	
if (CYCLE2_FLAG) Navigation();

/*-- continuos parameters transmission without request (just for debug) [31] */		
if (TxContFlag && UartContTxTimer<=0) TxCont();	

/*----------------------------------------- ADC value average calculus  [2a] */		
if (ADC_CALC_FLAG) AdcCalc();

/*---------------------------------------------------------- Real time timer */		
if (RT_TIMER_FLAG) 
{
	if (RtTimer <= 0) 
	{
		TIMER_OK_FLAG = 1;
		RT_TIMER_FLAG = 0;
	}
}

/*-------------------------------------------------- Actions scheduler  [32] */
if (SCHEDULER_FLAG)	// [32a]	
{	
	if (ANGLE_OK_FLAG || DIST_OK_FLAG || TIMER_OK_FLAG) Scheduler();
}

/*-------------------------------------------------- To send map to console */
if (MAP_SEND_FLAG)
{
	if (Vel[R]==0 && Vel[L]==0)
	{
		SendMap();
	}
}

/*--------------------------------------------------- Heartbeat led linking */
if (Blink == BlinkOn)
{
	LED1 = 0;
	LED2 = 1;
	Blink ++;	
}

if (Blink >= BlinkPeriod)
{
	LED1 = 1;
	LED2 = 0;
	Blink = 0;
}

/*------------------------------------------- Idle percentage calculus  [25] */		
IdleCount ++;	// [25]
if (IdleSample >= IDLE_CYCLE)
{
	IdlePerc=(long)(IdleCount * IDLE_TIME_PERIOD) / (long)(IDLE_SAMPLE_TIME);
	IdleCount = 0;
	IdleSample = 0;
}

Nop();	// end of idle cycle [25a]

}/*....Main loop*/

}/*.....................................................................Main */


/*===========================================================================*/
/* Functions                                                                 */
/*===========================================================================*/

void SendMap(void)
{// send one row of grid map to console
	int TmpMap;
	
	if(MAP_BUFF_FLAG)
	{// DMA have sent the whole buffer
		MAP_BUFF_FLAG=0; // will be set again by DMA ISR
	
		UartTmpBuff[0][SendMapPort]=MapSendIndx; 	
		// starting points for X and Y coordinates
		TmpMap = Xshift;
		UartTmpBuff[1][SendMapPort]=TmpMap>>8; 
		UartTmpBuff[2][SendMapPort]=TmpMap;
		TmpMap = Yshift;
		UartTmpBuff[3][SendMapPort]=TmpMap>>8; 
		UartTmpBuff[4][SendMapPort]=TmpMap;
		
		for (i=0; i<X_SIZE; i++)
		{// put in buffer both nib0 and nib1 for that X coordinate
			UartTmpBuff[i+5][SendMapPort]=MapXY[i][MapSendIndx].UC;
		}
			
		TxParameters('$', X_SIZE+6, SendMapPort); 
		MAP_SEND_FLAG=0;
	}
}


void ThetaDesF(float Angle)
{// [23ca]
	ThetaDes = Angle;
	ThetaDesRef = ThetaDes;
	PIDInit(&AnglePIDstruct); 
}

void Parser (void)	// [16]
{
	int ParserCount;		// parser index
	int C1;					// generic counter
	int C2;
	int C3;
	int TmpChk;

	unsigned int Ktmp;		// temp for PID coefficients
	int Ptmp;				// temp for position values
	int CurrTmp;			// temp for motors current value
	int Alpha;				// rotation angle in degrees
	long Tmp1Long;			// temp to combine long 
	long Tmp2Long;
	
	const float AngleS = 0.785398163;  // 45° angle for object on side
	
	if (UartRxStatus == 99) // the command come from UART1
	{
		TmpPtr = UartRxPtrData; // last data of header
		UartRxStatus = 0;
		Port=0;
	}
	else					// the command come from UART2
	{
		TmpPtr2 = Uart2RxPtrData; // last data of header
		Uart2RxStatus = 0;
		Port=1;
	}

	
	if (UartRxCmd[ResetPort] != '*')	// [28]
	{
		ResetCount = 0;
	}
	
	switch (UartRxCmd[Port])
	{
// --- Navigation values read			
		case 'A':		// all parameters request (mean)
			// VelMes = Int -> 2 byte (mm/s)
//			Ptmp = (VelMes[R] + VelMes[L]) >> 1;	// average speed
			VelInt[R]=(long)(Vel[R] * 1000)>>15;		// VelR
			VelInt[L]=(long)(Vel[L] * 1000)>>15;		// VelL
			Ptmp = (VelInt[R] + VelInt[L]) >> 1;	// average speed
			if (CONSOLE_DEBUG) //[30]
			{
				Ptmp = VelDesM;
				ADCValue[R]=abs(VelDesM);
				ADCValue[L]=abs(VelDesM);
				ThetaMes=ThetaDes;
			}
		 	UartTmpBuff[0][Port]=Ptmp>>8; 
			UartTmpBuff[1][Port]=Ptmp;
			// Curr = int -> 2byte (mA)
			CurrTmp = ADCValue[R]+ADCValue[L];		// total current
		 	UartTmpBuff[2][Port]=CurrTmp>>8;
			UartTmpBuff[3][Port]=CurrTmp;
			// PosXmes rounded in a Int -> 2 byte (mm)
			Ptmp = FLOAT2INT(PosXmes);				// PosX
		 	UartTmpBuff[4][Port]=Ptmp>>8; 
			UartTmpBuff[5][Port]=Ptmp;
			// PosYmes rounded in a Int -> 2 byte (mm)
			Ptmp = FLOAT2INT(PosYmes);				// PosY
		 	UartTmpBuff[6][Port]=Ptmp>>8; 
			UartTmpBuff[7][Port]=Ptmp;
			// ThetaMes rounded in a Int -> 2 byte (degrees)
			Ptmp = ThetaMes * RAD2DEG;				// Theta
			Alpha = FLOAT2INT(Ptmp);
		 	UartTmpBuff[8][Port]=Alpha>>8; 
			UartTmpBuff[9][Port]=Alpha;
			// angle reading from sensors board compass (deg * 10)
			UartTmpBuff[10][Port]=AngleCmp>>8; 
			UartTmpBuff[11][Port]=AngleCmp;
			// idle time in %
			UartTmpBuff[12][Port]=IdlePerc;

			TxParameters('A',13, Port);  
		break;

		case 'a':		// all parameters request (details)
			// VelInt = Int -> 2 byte
			VelInt[R]=(long)(Vel[R] * 1000)>>15;		// VelL
			VelInt[L]=(long)(Vel[L] * 1000)>>15;		// VelL
			if (CONSOLE_DEBUG) //[30]
			{
				VelInt[R]= VelDesM;
				VelInt[L]= VelDesM;
				ADCValue[R]=abs(VelDesM);
				ADCValue[L]=abs(VelDesM);
				ThetaMes=ThetaDes;
			}
		 	UartTmpBuff[0][Port]=VelInt[R]>>8; 
			UartTmpBuff[1][Port]=VelInt[R];
		 	UartTmpBuff[2][Port]=VelInt[L]>>8; 
			UartTmpBuff[3][Port]=VelInt[L];
			// ADCValue = int -> 2byte
		 	UartTmpBuff[4][Port]=ADCValue[R]>>8;	// CurrR
			UartTmpBuff[5][Port]=ADCValue[R];
			UartTmpBuff[6][Port]=ADCValue[L]>>8;	// CurrL
			UartTmpBuff[7][Port]=ADCValue[L];
			// Space = int -> 2byte
		 	UartTmpBuff[8][Port]=SpTick[R]>>8;		// SpTickR
			UartTmpBuff[9][Port]=SpTick[R];
			SpTick[R] = 0; // [19]
			UartTmpBuff[10][Port]=SpTick[L]>>8;		// SpTickL
			UartTmpBuff[11][Port]=SpTick[L];
			SpTick[L] = 0; // [19]
			TxParameters('a',12, Port);  
		break;
		
		
//--- Navigation parameters settings
		case 'D':		// setting reference coord. X, Y computing distance
						// [24] Mode C
			// High Byte * 256 + Low Byte
			Ptmp = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
				   (UartRxBuff[IncrCircPtr(Port)][Port]); // Dist
			PosXdes = PosXmes + (Ptmp * sin(ThetaDes));
			PosYdes = PosYmes + (Ptmp * cos(ThetaDes));
			DIST_ENABLE_FLAG = 1;	// enable distance computing [24a]
			SCHEDULER_FLAG = 0;	// [32a]
			if (CONSOLE_DEBUG) //[30]
			{
				PosXmes=PosXdes;
				PosYmes=PosYdes;
			}
		break;

		case 'O':		// setting ref. orientation angle in degrees (absolute)
						// [24] Mode A
			// High Byte * 256 + Low Byte
			Ptmp = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
				   (UartRxBuff[IncrCircPtr(Port)][Port]);
			ThetaDesF(Ptmp * DEG2RAD);
			DIST_ENABLE_FLAG = 0;	// disable distance computing [24a]
			VelDecr = 1;			// [24d]
			SCHEDULER_FLAG = 0;	// [32a]
			if (CONSOLE_DEBUG) //[30]
			{
				ThetaMes=ThetaDes;
			}
		break;

		case 'o':		// setting reference orientation angle in degrees
						// as a delta of the current orientation (relative) 
						// [24] Mode A
			// High Byte * 256 + Low Byte
			Ptmp = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
				   (UartRxBuff[IncrCircPtr(Port)][Port]);
			ThetaDesF( ThetaDes + (Ptmp * DEG2RAD));
			DIST_ENABLE_FLAG = 0;	// disable distance computing [24a]
			VelDecr = 1;			// [24d]
			SCHEDULER_FLAG = 0;	// [32a]
		break;

		case 'P':		// setting reference coord. X, Y in mm
						// [24] Mode B
			// High Byte * 256 + Low Byte
			PosXdes = (float)((UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
							(UartRxBuff[IncrCircPtr(Port)][Port]));
			PosYdes = (float)((UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
							(UartRxBuff[IncrCircPtr(Port)][Port]));
			DIST_ENABLE_FLAG = 1;	// enable distance computing [24a]
			SCHEDULER_FLAG = 0;	// [32a]
			if (CONSOLE_DEBUG) //[30]
			{
				PosXmes=PosXdes;
				PosYmes=PosYdes;
			}
		break;
		
		case 'S':		// setting reference speed (as mm/s) 
			// High Byte * 256 + Low Byte
			VelDesM = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
					  (UartRxBuff[IncrCircPtr(Port)][Port]);
			if (VelDesM >  999) VelDesM =  999; // range check
			if (VelDesM < -999) VelDesM = -999; // range check
			SCHEDULER_FLAG = 0;	// [32a]
		break;
		
		case 'H':	// immediate Halt without decelerating ramp.In this way it 
					// uses the brake effect of H bridge in LAP mode
			VelDesM = 0; 
			SCHEDULER_FLAG = 0;	// [32a]
			if (CONSOLE_DEBUG) //[30]
			{
				VelMes[R]=0;
				VelMes[L]=0;
				ADCValue[R]=0;
				ADCValue[L]=0;
			}
		break;
		
		
//--- Constant parameters setting		
		case 'J':		// setting PID coefficients for DistPid
			TmpChk=0;	// checksum to control permanent storage
			for (ParserCount=0; ParserCount < 5; ParserCount+=2)
			{
				// High Byte * 256 + Low Byte
				Ktmp = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
					   (UartRxBuff[IncrCircPtr(Port)][Port]);
				if (Ktmp > 9999) Ktmp = 9999; // range check
				TmpChk+=Ktmp;
				DataEEWrite(Ktmp,EE_DIST_KP+ParserCount/2);			
				DistKCoeffs[ParserCount/2] = Q15((float)(Ktmp)/10000);
			}
			InitDistPid();
			DataEEWrite(TmpChk,EE_CHK_DIST);
		break;

		case 'K':		// setting PID coefficients for Speed PIDs
			TmpChk=0;	// checksum to control permanent storage
			for (ParserCount=0; ParserCount < 5; ParserCount+=2)
			{
				// High Byte * 256 + Low Byte
				Ktmp = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
					   (UartRxBuff[IncrCircPtr(Port)][Port]);
				if (Ktmp > 9999) Ktmp = 9999; // range check
				TmpChk+=Ktmp;
				DataEEWrite(Ktmp,EE_KP1+ParserCount/2);
				kCoeffs1[ParserCount/2] = Q15((float)(Ktmp)/10000);
			}
			InitPid1();
			
			for (ParserCount=0; ParserCount < 5; ParserCount+=2)
			{
				// High Byte * 256 + Low Byte
				Ktmp = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
					   (UartRxBuff[IncrCircPtr(Port)][Port]);
				if (Ktmp > 9999) Ktmp = 9999; // range check
				TmpChk+=Ktmp;
				DataEEWrite(Ktmp,EE_KP2+ParserCount/2);
				kCoeffs2[ParserCount/2] = Q15((float)(Ktmp)/10000);
			}
			InitPid2();
			DataEEWrite(TmpChk,EE_CHK_SPEED);
		break;
		
		case 'k':		// settings PID coefficients for AnglePid
			TmpChk=0;	// checksum to control permanent storage
			for (ParserCount=0; ParserCount < 5; ParserCount+=2)
			{
				// High Byte * 256 + Low Byte
				Ktmp = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
					   (UartRxBuff[IncrCircPtr(Port)][Port]);
				if (Ktmp > 9999) Ktmp = 9999; // range check
				TmpChk+=Ktmp;
				DataEEWrite(Ktmp,EE_ANGLE_KP+ParserCount/2);
				AngleKCoeffs[ParserCount/2] = Q15((float)(Ktmp)/10000);
			}
			InitAnglePid();
			DataEEWrite(TmpChk,EE_CHK_ANGLE);
		break;
		
		case 'L':		// Kvel 	
			TmpChk=0;	// checksum to control permanent storage
			Tmp1Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp1Long;
			DataEEWrite(Tmp1Long,EE_KVEL1_H);
			Tmp2Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp2Long;
			DataEEWrite(Tmp2Long,EE_KVEL1_L);
			Kvel[R] = (Tmp1Long << 16) + Tmp2Long;

			Tmp1Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp1Long;
			DataEEWrite(Tmp1Long,EE_KVEL2_H);
			Tmp2Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp2Long;
			DataEEWrite(Tmp2Long,EE_KVEL2_L);
			Kvel[L] = (Tmp1Long << 16) + Tmp2Long;
			DataEEWrite(TmpChk,EE_CHK_KVEL);
		break;
		
		case 'M':		// Mechanical costants: Axle size, KspR, KspL [29]
			TmpChk=0;	// checksum to control permanent storage
			Tmp1Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp1Long;
			DataEEWrite(Tmp1Long,EE_AXLE_H);
			Tmp2Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp2Long;
			DataEEWrite(Tmp2Long,EE_AXLE_L);
			Axle = 	  (float)((Tmp1Long << 16) + Tmp2Long)/(float)(10000);
			
			
			Tmp1Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp1Long;
			DataEEWrite(Tmp1Long,EE_KSP1_H);
			Tmp2Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp2Long;
			DataEEWrite(Tmp2Long,EE_KSP1_L);
			Ksp[0] =  (float)((Tmp1Long << 16) + Tmp2Long)/(float)(1000000000);

			Tmp1Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp1Long;
			DataEEWrite(Tmp1Long,EE_KSP2_H);
			Tmp2Long = ((long)UartRxBuff[IncrCircPtr(Port)][Port] << 8) +
					   ((long)UartRxBuff[IncrCircPtr(Port)][Port]);
			TmpChk+=Tmp2Long;
			DataEEWrite(Tmp2Long,EE_KSP2_L);
			Ksp[1] =  (float)((Tmp1Long << 16) + Tmp2Long)/(float)(1000000000);
			DataEEWrite(TmpChk,EE_CHK_MECH);
		break;
		
		case 's':		// Scheduler parameters setting [32]	
			TmpChk=0;	// checksum to control permanent storage
		  	VelDesM = 0;
   	   		SCHEDULER_FLAG = 0;	// [32a]
			C3 = EE_SCHED;	// starting address for permanent storage
			for (C1=0; C1<16; C1++)
			{
				for (C2=0; C2<4; C2++)
				{
					SchedValues[C1][C2]=(UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
					   					(UartRxBuff[IncrCircPtr(Port)][Port]);
					TmpChk+=SchedValues[C1][C2];
					DataEEWrite(SchedValues[C1][C2],C3);
					C3 ++;
				}
			}
			DataEEWrite(TmpChk,EE_CHK_SCHED);
		break;
		
//--- Service
		case '*':		// Board reset [28]
		if (ResetCount < 3)
			{
				ResetCount ++;
				ResetPort = Port;
			}
			else
			{
				SET_CPU_IPL( 7 ); // disable all user interrupts
				DelayN1ms(200);
				asm("RESET");
			}
		break;
		
		case 'R':		// Firmware version request
			// Send string 'Ver'
			for (i=0; i<27; i++)
			{
		 		UartTmpBuff[i][Port]=Ver[i]; 
			}
			TxParameters('R',26, Port);
		break;
		
		case 'c':		// Continuos send mode
			// sends all data without request.
			//	0=OFF 
			//	1=send mean parameters
			//	2=send detailed parameters
			TxContFlag = UartRxBuff[RX_HEADER_LEN + 1][Port];
			UartContTxTimer=UART_CONT_TIMEOUT;	// timer reset
		break;

		case 'z':		// send back a text string, just for debug
			// Send string 'Test'
			for (i=0; i<27; i++)
			{
		 		UartTmpBuff[i][Port]=Test[i]; 
			}
			TxParameters('z',24, Port);
		break;
		
		case 'e':		// Read error code and reset error condition
			// Send error value
			BlinkPeriod = NORM_BLINK_PER;// LED1 blinking period (ms)
			BlinkOn     = NORM_BLINK_ON; // LED1 on time (ms)
			Blink = 0;
		 	UartTmpBuff[0][Port]=ErrCode>>8; 
			UartTmpBuff[1][Port]=ErrCode;
			TxParameters('e',2, Port);  
		break;
		
		case 'f':		// set "Console Debug" mode [30]
			CONSOLE_DEBUG = UartRxBuff[IncrCircPtr(Port)][Port];
		break;

		case '#':		// start scheduler sequence
			DIST_OK_FLAG  = 1;	// to kick start the scheduler sequence
			SchedPtr = 0;		// [32]
			SCHEDULER_FLAG = 1;	// [32a]
		break;
		
		
		case '$':		// all map grid sending request
			// disable any other procedure pending
			DIST_ENABLE_FLAG = 0;	// disable distance computing [24a]
			VelDecr = 1;			// [24d]
			SCHEDULER_FLAG = 0;		// [32a]
			VelDesM = 0;
			
			SendMapPort = Port;	// to temporay store port number for delayed TX
			MAP_SEND_FLAG=1;
			
			// index of row requested by console in 0 to Y_size range
			MapSendIndx = UartRxBuff[IncrCircPtr(Port)][Port];	
		break;
		
//--- Sensors
		case 'd':// Distance from objects, target and compass readings
			// Obj = unsigned int -> 2 byte (mm)
			Obj[0] = (UartRxBuff[IncrCircPtr(Port)][Port])*10; // Left object
			Obj[1] = (UartRxBuff[IncrCircPtr(Port)][Port])*10; // Center object
			Obj[2] = (UartRxBuff[IncrCircPtr(Port)][Port])*10; // Right object
			// Target = unsigned char -> 1 byte
			Target[0] = (UartRxBuff[IncrCircPtr(Port)][Port]); // Left target
			Target[1] = (UartRxBuff[IncrCircPtr(Port)][Port]); // Center target
			Target[2] = (UartRxBuff[IncrCircPtr(Port)][Port]); // Right target
			// Compass Bearing = int -> 2 byte (deg*10)
			AngleCmp = (UartRxBuff[IncrCircPtr(Port)][Port] << 8) + 
					   (UartRxBuff[IncrCircPtr(Port)][Port]);
			// [24g] relative position of obstacles.
			if (Obj[0] < OBST_THRESHOLD) // from object on the left
			{
				VObX[0] = Obj[0] * sin(ThetaMes-AngleS);
				VObY[0] = Obj[0] * cos(ThetaMes-AngleS);
				// mark obstacle in the cell
				Slam(PosXmes + VObX[0], PosYmes + VObY[0], 1); 
			}
	
			if (Obj[1] < OBST_THRESHOLD) // from object on the center
			{
				VObX[1] = Obj[1] * sin(ThetaMes);
				VObY[1] = Obj[1] * cos(ThetaMes);
				// mark obstacle in the cell
				Slam(PosXmes + VObX[1], PosYmes + VObY[1], 1); 
			}
		
			if (Obj[2] < OBST_THRESHOLD) // from object on the right
			{
				VObX[2] = Obj[2] * sin(ThetaMes+AngleS);
				VObY[2] = Obj[2] * cos(ThetaMes+AngleS);
				// mark obstacle in the cell
				Slam(PosXmes + VObX[2], PosYmes + VObY[2], 1); 
			}
		break;
		
		case 'Q':// Raw sensors data, dsNav -> console
			// PosXmes rounded in a Int -> 2 byte (mm)
			Ptmp = FLOAT2INT(PosXmes);				// PosX
		 	UartTmpBuff[0][Port]=Ptmp>>8; 
			UartTmpBuff[1][Port]=Ptmp;
			// PosYmes rounded in a Int -> 2 byte (mm)
			Ptmp = FLOAT2INT(PosYmes);				// PosY
		 	UartTmpBuff[2][Port]=Ptmp>>8; 
			UartTmpBuff[3][Port]=Ptmp;
			// ThetaMes rounded in a Int -> 2 byte (degrees)
			Ptmp = ThetaMes * RAD2DEG;				// Theta
			Alpha = FLOAT2INT(Ptmp);
		 	UartTmpBuff[4][Port]=Alpha>>8; 
			UartTmpBuff[5][Port]=Alpha;
			// VObX rounded in a Int -> 2 byte (cm)
			for(i=0; i<3; i++) // Buffer index from 6 to 17
			{
				Ptmp = FLOAT2INT(VObX[i]/10);	// Obj X coord in cm
			 	UartTmpBuff[6+(i*4)][Port]=Ptmp>>8; 
				UartTmpBuff[7+(i*4)][Port]=Ptmp;
				Ptmp = FLOAT2INT(VObY[i]/10);	// Obj Y coord in cm
			 	UartTmpBuff[8+(i*4)][Port]=Ptmp>>8; 
				UartTmpBuff[9+(i*4)][Port]=Ptmp;
			}
			
			TxParameters('Q',18, Port);  
		break;
		
		default:
			UartRxError(-7,Port); //	error: not a known command
		break;
	}
}

void Scheduler(void)	// [32]
{
	unsigned char SchedCode= 0;	// action to execute
	float DPosX;	// delta PosX
	float DPosY;	// delta PosY

	ANGLE_OK_FLAG = 0;
	DIST_OK_FLAG  = 0;
	TIMER_OK_FLAG = 0;
	SCHED_ANGLE_FLAG = 0;
	SCHED_DIST_FLAG = 0;
	
	/*	
	 Units 
	  code	
	  Angle(deg)	
	  Speed(mm/s)	
	  X(mm)	
	  Y(mm)	
	  D(mm)	
	  Delay(n x 50ms)

	  Meaning of parameters
		code	1		2		3
		0		
		1		V
		2		V(2)	D
		3		V(2)	Theta	
		4		V(2)	X		Y
		5		V(2)	X		Y
		6		Delay
	*/
	
	SchedCode = SchedValues[SchedPtr][0];
	switch (SchedCode)
		{     
   			case 0:	// stop	end of sequence
   	    		VelDesM = 0;
   	    		SCHEDULER_FLAG = 0;	// [32a]
			break;
			
			case 1:	// keep previous angle - set speed
   	    		VelDesM =  SchedValues[SchedPtr][1];
   	    		RT_TIMER_FLAG = 1;
				RtTimer = 2;	// wait 100ms for next step
			break;
			
			case 2:	// keep angle and speed(2)	-	set distance
				if (SchedValues[SchedPtr][1] != 0xFFFF)
				{
					VelDesM =  SchedValues[SchedPtr][1];
				}
				PosXdes=PosXmes+(SchedValues[SchedPtr][2]*sin(ThetaDes));
				PosYdes=PosYmes+(SchedValues[SchedPtr][2]*cos(ThetaDes));
				SCHED_DIST_FLAG = 1;	// wait for target distance
				DIST_ENABLE_FLAG = 1;	// enable distance computing [24a]
			break;
			
			case 3:	// keep speed(2) - set angle
   	    		SCHED_ANGLE_FLAG = 1;	// wait for target angle
				if (SchedValues[SchedPtr][1] != 0xFFFF)
				{
					VelDesM =  SchedValues[SchedPtr][1];
				}
				ThetaDesF(SchedValues[SchedPtr][2] * DEG2RAD);
				DIST_ENABLE_FLAG = 0;	// disable distance computing [24a]
				VelDecr = 1;			// [24d]
			break;
			
			case 4:	// keep speed(2) - set angle toward X, Y
   	    		SCHED_ANGLE_FLAG = 1;	// wait for target angle
				if (SchedValues[SchedPtr][1] != 0xFFFF)
				{
					VelDesM =  SchedValues[SchedPtr][1];
				}
   				DPosX=(float)(SchedValues[SchedPtr][2])-PosXmes;//delta on X
				DPosY=(float)(SchedValues[SchedPtr][3])-PosYmes;//delta on Y
//				if ((DPosX != 0) && (DPosY != 0))
				ThetaDesF(atan2f(DPosX,DPosY));//Tan(ThetaDes)=Sin/Cos=X/Y
			break;
			
			case 5:	// keep speed(2) - walk toward X, Y
				if (SchedValues[SchedPtr][1] != 0xFFFF)
				{
					VelDesM =  SchedValues[SchedPtr][1];
				}
				PosXdes = (float)(SchedValues[SchedPtr][2]);
				PosYdes = (float)(SchedValues[SchedPtr][3]);
				SCHED_DIST_FLAG = 1;	// wait for target distance
				DIST_ENABLE_FLAG = 1;	// enable distance computing [24a]
			break;
			
			case 6:	// wait n ms
				RT_TIMER_FLAG = 1;
				RtTimer = SchedValues[SchedPtr][1];	// n x 50ms
			break;
			
			default:// the same as 0
   	    		VelDesM = 0;
				SCHEDULER_FLAG = 0;	// [32a]
			break;
		}
		
	if (SchedPtr < 15)
	{
		SchedPtr ++;	// next step
	}
}	

void AdcCalc(void)
{
	extern int DmaAdc[2][64];
	int AdcCount = 0;
	long ADCValueTmp[2] = {0,0};// to store intermediate ADC calculations 

	ADC_CALC_FLAG = 0; // will be set by ISR when all data available
	// averages the 64 ADC values for each ANx
    for (AdcCount=0;AdcCount<64;AdcCount++)	
    {
		ADCValueTmp[R] += DmaAdc[R][AdcCount];
		ADCValueTmp[L] += DmaAdc[L][AdcCount];
    }
    ADCValue[R] = ADCValueTmp[R] >> 6; // [2a]
    ADCValue[L] = ADCValueTmp[L] >> 6; // [2a]	 	
    
    if (ADCValue[R] > ADC_OVLD_LIMIT)	// [2b]
    {
    	ADCOvldCount[R] ++;
    }
    else
    {
        ADCOvldCount[R] = 0;
    }
    
    if (ADCValue[L] > ADC_OVLD_LIMIT)	// [2b]
    {
    	ADCOvldCount[L] ++;
    }
    else
    {
        ADCOvldCount[L] = 0;
    }
    
    if ((ADCOvldCount[R]>ADC_OVLD_TIME)||(ADCOvldCount[L]>ADC_OVLD_TIME))
    {
    	ADCOvldCount[R]=0;
    	ADCOvldCount[R]=0;
    	VelDesM = 0; // immediate halt
		SCHEDULER_FLAG = 0;	// [32a]
		BlinkOn = 50;	// very fast blink for alarm
		BlinkPeriod = 100;
		ErrCode = -30;
    }
}


void InitDistPid(void)
{
//Initialize the PID data structure: PIDstruct
//Set up pointer to derived coefficients
DistPIDstruct.abcCoefficients = &DistabcCoefficient[0];
//Set up pointer to controller history samples
DistPIDstruct.controlHistory = &DistcontrolHistory[0]; 
// Clear the controler history and the controller output
PIDInit(&DistPIDstruct); 
//Derive the a,b, & c coefficients from the Kp, Ki & Kd
PIDCoeffCalc(&DistKCoeffs[0], &DistPIDstruct); 
}

void Navigation(void)	// called after odometry is performed
{
	float DPosX;	// delta PosX
	float DPosY;	// delta PosY
	float Dist;		// distance
	float VelDecrObj; // speed decreasing due to objects found
	
	CYCLE2_FLAG = 0; // will be set again by timer ISR
	
	if (DIST_ENABLE_FLAG)
	{
		// distance from goal, i.e.: distance from desired to current position
		DPosX = PosXdes - PosXmes;	   // X component of distance
		DPosY =	PosYdes - PosYmes;	   // Y component of distance
		Dist = sqrtf(powf(DPosX,2) + powf(DPosY,2));// Pythagora's Theorem
	}
	else
	{
		Dist = OBST_THRESHOLD;	// a reference value
		DPosX = Dist * sin(ThetaDesRef);
		DPosY = Dist * cos(ThetaDesRef);
	}
	
	if (Dist < MIN_DIST_ERR)	// goal reached
	{
		DIST_ENABLE_FLAG = 0;	// disable distance computing [24a]
		VelDecr = 1;			// [24d]
		VelDesM = 0;
		if (SCHED_DIST_FLAG)
		{
			DIST_OK_FLAG = 1;	// target ok
		}
		return; // job done
	}
	else
	{// set ThetaDes and VelDecr to reach the goal avoiding obstacles
		VelDecrObj=ObstacleAvoidance(DPosX, DPosY, Dist); 
	}	

	if (Dist < MIN_GOAL_DIST)	   // [24c]
	{
		DIST_PID_DES = 0;		   // the goal is to have dist=0 from target
		DIST_PID_MES = Q15((float)Dist/(float)MIN_GOAL_DIST);  // measured input
		PID(&DistPIDstruct);
		VelDecr = -_itofQ15(DIST_PID_OUT); // [24d]
		VelDecr *= VelDecrObj;
	}
	else
	{
		VelDecr = VelDecrObj;
	}
}

float ObstacleAvoidance(float DPosX, float DPosY, int Dist)
{
	float VX;				// X component of resultant vector
	float VY;				// Y component of resultant vector
	float VM;				// magnitude of resultant vector

	if(Obj[0]<OBST_MIN_DIST||Obj[1]<OBST_MIN_DIST||Obj[2]<OBST_MIN_DIST)
	{// set ThetaDes and VelDecr to avoid very close obstacles
		return -0.2; // walk backward at a reduced speed
	} 
	else
	{
/*		// adding all X & Y normalized components of single vectors we obtain
		// the X, Y components of the speed vector
		VallX = (VBXleft+VBXright+VObX[0]+VObX[1]+VObX[2]);
		VallY = (VBYbottom+VBYtop+VObY[0]+VObY[1]+VObY[2]);
		
		
		if (VallX != 0 && VallY != 0) ??????????????????????????????????????????????
		{
			if (DIST_ENABLE_FLAG)
			{
				
				Knorm = (float)(OBST_THRESHOLD) / Dist;
			}
			else
			{
				Knorm = 1;
			}
			VX = (DPosX * Knorm) - (VallX);
			VY = (DPosY * Knorm) - (VallY);
			//	relative magnitude of vector (0 to 1)
			VM = (sqrtf(powf(VX,2) + powf(VY,2)))/OBST_THRESHOLD;
		}
		else
		{
			VX = DPosX;
			VY = DPosY;
			VM = 1;
		}
*/		
		#ifdef NO_OBSTACLE
		#warning -- compiling with NO OBSTACLE *************************************
			VX = DPosX;
			VY = DPosY;
			VM = 1;
		#endif
		
		if ((VX != 0) && (VY != 0))
		{// phase of vector [22aa]
			ThetaDesF(atan2f(VX,VY)); // Tangent(ThetaDes) = Sin/Cos = X/Y
		}
		return VM;
	}
}

void InitAnglePid(void)
{
//Initialize the PID data structure: PIDstruct
//Set up pointer to derived coefficients
AnglePIDstruct.abcCoefficients = &AngleabcCoefficient[0];
//Set up pointer to controller history samples
AnglePIDstruct.controlHistory = &AnglecontrolHistory[0]; 
// Clear the controler history and the controller output
PIDInit(&AnglePIDstruct); 
//Derive the a,b, & c coefficients from the Kp, Ki & Kd
PIDCoeffCalc(&AngleKCoeffs[0], &AnglePIDstruct); 
}

void Orientation(void)	// [23]
{
	int DeltaVel;	// difference in speed between the wheels to rotate
	int RealVel;	// VelDesM after reduction controlled by Dist PID
	float Error;	
	
	ORIENTATION_FLAG = 0; // it will be restarted by DeadReckoning()
	
	if (ThetaDes < 0) ThetaDes += TWOPI;	// keep angle value positive
	if (ThetaMes < 0) ThetaMes += TWOPI;
	Error = ThetaMes - ThetaDes;
	if (Error > PI)	// search for the best direction to correct error [23a]
	{
		Error -= TWOPI;
	}
	else if (Error < -PI)
	{
		Error += TWOPI;
	}
	if (SCHED_ANGLE_FLAG)
	{
		if (fabsf(Error) < MIN_THETA_ERR) 
		{
			ANGLE_OK_FLAG = 1;	// target ok
		}
	}
	
	ANGLE_PID_DES = 0; 				// ref value translated to 0 [23c]
	ANGLE_PID_MES = Q15(Error/PI);	// current error [23b]
	PID(&AnglePIDstruct);
	
	DeltaVel = (ANGLE_PID_OUT >> 7);// MAX delta in int = 256 [23d]
	RealVel = VelDesM * VelDecr;	// [24d]
	VelDes[R] = RealVel - DeltaVel;	// [23e]
	VelDes[L] = RealVel + DeltaVel;
	
	if (VelDes[R] > MAX_ROT_SPEED)
	{
		VelDes[R] = MAX_ROT_SPEED;
		VelDes[L] = MAX_ROT_SPEED + (DeltaVel << 1);
	}
	else if (VelDes[R] < -MAX_ROT_SPEED)
	{
		VelDes[R] = -MAX_ROT_SPEED;
		VelDes[L] = -MAX_ROT_SPEED + (DeltaVel << 1);
	}
	else if (VelDes[L] > MAX_ROT_SPEED)
	{
		VelDes[L] = MAX_ROT_SPEED;
		VelDes[R] = MAX_ROT_SPEED - (DeltaVel << 1);
	}
	else if (VelDes[L] < -MAX_ROT_SPEED)
	{
		VelDes[L] = -MAX_ROT_SPEED;
		VelDes[R] = -MAX_ROT_SPEED - (DeltaVel << 1);
	}

	VelFin[R] = Q15((float)(VelDes[R])/1000);	// [23f]
	if (VelFin[R] != PID_REF1)
	{
		if (PID_REF1>=0 && VelFin[R] > PID_REF1)
		{
			Ramp1 = Q15( ACC);
			RAMP_T_FLAG1 = 1;
		}
		if (PID_REF1>=0 && VelFin[R] < PID_REF1) 
		{
			Ramp1 = Q15(-DEC);
			RAMP_T_FLAG1 = 0;
		}
		if (PID_REF1< 0 && VelFin[R] > PID_REF1) 
		{
			Ramp1 = Q15( DEC);
			RAMP_T_FLAG1 = 1;
		}
		if (PID_REF1< 0 && VelFin[R] < PID_REF1) 
		{
			Ramp1 = Q15(-ACC);
			RAMP_T_FLAG1 = 0;
		}
		RAMP_FLAG1 = 1; // acceleration ramp start
	}	
		
	VelFin[L] = Q15((float)(VelDes[L])/1000);
	if (VelFin[L] != PID_REF2)
	{
		if (PID_REF2>=0 && VelFin[L] > PID_REF2)
		{
			Ramp2 = Q15( ACC);
			RAMP_T_FLAG2 = 1;
		}
		if (PID_REF2>=0 && VelFin[L] < PID_REF2) 
		{
			Ramp2 = Q15(-DEC);
			RAMP_T_FLAG2 = 0;
		}
		if (PID_REF2< 0 && VelFin[L] > PID_REF2) 
		{
			Ramp2 = Q15( DEC);
			RAMP_T_FLAG2 = 1;
		}
		if (PID_REF2< 0 && VelFin[L] < PID_REF2) 
		{
			Ramp2 = Q15(-ACC);
			RAMP_T_FLAG2 = 0;
		}
		RAMP_FLAG2 = 1; // acceleration ramp start
	}
}

void DeadReckoning(void) // [22]
{
	float CosNow;		// current value for Cos
	float SinNow;		// current value for Sin
	float DSpace;		// delta traveled distance by the robot
	float DTheta;		// delta rotation angle
	float DPosX;		// delta space on X axis
	float DPosY;		// delta space on Y axis
	float SaMinusSb;
	float SrPlusSl;
	float Radius;
				
	CYCLE1_FLAG=0;
		
	Spmm[R]=SpTick[R]*Ksp[R];	// distance of right wheel in mm
	SpTick[R] = 0;				// rest counter for the next misure
	Spmm[L]=SpTick[L]*Ksp[L];	// distance of left wheel in mm
	SpTick[L] = 0;				// rest counter for the next misure

	#ifdef geographic			// [22aa]
		SaMinusSb=Spmm[L]-Spmm[R];
	#else
		SaMinusSb=Spmm[R]-Spmm[L];
	#endif
	
	SrPlusSl=Spmm[R]+Spmm[L];
	if (fabs(SaMinusSb) <= SPMIN)
	{// traveling in a nearly straight line [22a]
		DSpace=Spmm[R];
		
		#ifdef geographic			// [22aa]
			DPosX=DSpace*SinPrev;
			DPosY=DSpace*CosPrev;
		#else
			DPosX=DSpace*CosPrev;
			DPosY=DSpace*SinPrev;
		#endif

	}
	else if (fabs(SrPlusSl) <= SPMIN)
	{// pivoting around vertical axis without translation [22a]
		DTheta=SaMinusSb/Axle;
		ThetaMes=fmodf((ThetaMes+DTheta),TWOPI);//current orient. in 2PI range
		CosPrev=cosf(ThetaMes);	// for the next cycle
		SinPrev=sinf(ThetaMes);
		DPosX=0;
		DPosY=0;
		DSpace=0;
	}
	else
	{// rounding a curve	
		DTheta=SaMinusSb/Axle;
		ThetaMes=fmodf((ThetaMes+DTheta),TWOPI);//current orient. in 2PI range
		CosNow=cosf(ThetaMes);
		SinNow=sinf(ThetaMes);
		DSpace=SrPlusSl/2;
		Radius = (SemiAxle)*(SrPlusSl/SaMinusSb);
		
		#ifdef geographic			// [22aa]
			DPosX=Radius*(CosPrev-CosNow);	
			DPosY=Radius*(SinNow-SinPrev);
		#else
			DPosX=Radius*(SinNow-SinPrev);
			DPosY=Radius*(CosPrev-CosNow);
		#endif		

		CosPrev=CosNow;		// to avoid re-calculation on the next cycle
		SinPrev=SinNow;
	}

	Space += DSpace;	// total traveled distance
	PosXmes += DPosX;	// current position
	PosYmes += DPosY;
	
	Slam(PosXmes, PosYmes, 2); // mark a presence in this position cell

	ORIENTATION_FLAG = 1; // position coordinates computed, Angle PID can start
}

unsigned char Slam(float PosX, float PosY, int Cell)
{//Simultaneous Localization And Mapping
  	nibble CellValue;	// field map value of current cell 
 	int Xpoint;	// X coordinate value normalized in matrix range 
 	int Ypoint;	// Y index  
 	int OldPoint; // temp for previous field boundaries
  	int TempIndx;
  	
  	// field mapping [22b]
	// index in the range 0-Y_SIZE
	Xpoint=abs((__builtin_modsd((PosX+(HALF_MAP_SIZE)),(MAP_SIZE)))/CELL_SIZE);	
	Ypoint=abs((__builtin_modsd((PosY+(HALF_MAP_SIZE)),(MAP_SIZE)))/CELL_SIZE);
	
	if (PosX >= MaxMapX)	// [22e]
	{// compute the next cell beyond old boundary modulus MAP_SIZE
		OldPoint=abs((__builtin_modsd((MaxMapX+HALF_MAP_SIZE),MAP_SIZE))
			/CELL_SIZE);
		MaxMapX = PosX;
		MinMapX = MaxMapX - MAP_SIZE;
		Xshift = Xpoint + 1; // [22g]
		for (i=OldPoint; i<=Xpoint; i++)
		{
			for (TempIndx=0; TempIndx<Y_SIZE; TempIndx++)
			{// purge the translated portion of the field	
				SetMap(Xpoint, Ypoint, 0);
			}
		}
	}
	else if (PosX < MinMapX)
	{// compute the previous cell behind old boundary modulus MAP_SIZE
		OldPoint=abs((__builtin_modsd((MaxMapX+(HALF_MAP_SIZE)-CELL_SIZE),
			(MAP_SIZE)))/CELL_SIZE);
		MinMapX = PosX;
		MaxMapX = MinMapX + MAP_SIZE;
		Xshift = -Xpoint; // [22g]
		for (i=OldPoint; i>=Xpoint; i--)
		{
			for (TempIndx=0; TempIndx<Y_SIZE; TempIndx++)
			{// purge the translated portion of the field	
				SetMap(Xpoint, Ypoint, 0);
			}
		}
	}

	if (PosY >= MaxMapY)
	{// compute the next cell beyond old boundary modulus MAP_SIZE
		OldPoint=abs((__builtin_modsd((MaxMapY+HALF_MAP_SIZE),MAP_SIZE))
			/CELL_SIZE);
		MaxMapY = PosY;
		MinMapY = MaxMapY - MAP_SIZE;
		Yshift = Ypoint + 1; // [22g]
		for (i=OldPoint; i<=Ypoint; i++)
		{
			for (TempIndx=0; TempIndx<X_SIZE; TempIndx++)
			{// purge the translated portion of the field	
				SetMap(Xpoint, Ypoint, 0);
			}
		}
	}
	else if (PosY < MinMapY)
	{// compute the previous cell behind old boundary modulus MAP_SIZE
		OldPoint=abs((__builtin_modsd((MaxMapY+(HALF_MAP_SIZE)-CELL_SIZE),
			(MAP_SIZE)))/CELL_SIZE);
		MinMapY = PosY;
		MaxMapY = MinMapY + MAP_SIZE;
		Yshift = -Ypoint; // [22g]
		for (i=OldPoint; i>=Ypoint; i--)
		{
			for (TempIndx=0; TempIndx<X_SIZE; TempIndx++)
			{// purge the translated portion of the field	
				SetMap(Xpoint, Ypoint, 0);
			}
		}
	}

	CellValue.nib=GetMap(Xpoint, Ypoint);

	if ((Xpoint != XindxPrev) || (Ypoint != YindxPrev))//only when cell changes
	{	
		switch (Cell) // [22d]
		{
			case 1:
				if (CellValue.nib<7)
				{
					CellValue.nib++;
				}
			break;
			
			case 2: 
				if (CellValue.nib<8)// this overrides obstacle info
				{
					CellValue.nib = 8;
				}
				else if (CellValue.nib<10)
				{
					CellValue.nib++;
				}
			break;
			
			case 5:	// gas
				CellValue.nib = 12;
			break;
			
			case 6:	// light
				CellValue.nib = 13;
			break;

			case 7:	// sound
				CellValue.nib = 14;
			break;
		}
	
		SetMap(Xpoint, Ypoint, &CellValue);
		
		XindxPrev=Xpoint;
		YindxPrev=Ypoint;		
	}
	
	return CellValue.nib;
}

unsigned char GetMap(int Xpnt, int Ypnt) 
{
	int XindxH;	// High part of X index 
  	int XindxL;	// Low part  of X index 
  	div_t z;	// for div function
  	
 	//X index in range 0-X_SIZE (H) and remainder 0-(VAR_PER_BYTE-1) (L)
	z=div(Xpnt,VAR_PER_BYTE);
	XindxH=z.quot; // main index
	XindxL=z.rem;  // sub-index

	if(XindxL==0)
	{
		return (MapXY[XindxH][Ypnt].TN.nib0);
	}
	else
	{	
		return (MapXY[XindxH][Ypnt].TN.nib1);
	}
}

void SetMap(int Xpnt, int Ypnt, nibble *CellVal) 
{
	int XindxH;	// High part of X index 
  	int XindxL;	// Low part  of X index 
  	div_t z;	// for div function
  	
 	//X index in range 0-X_SIZE (H) and remainder 0-(VAR_PER_BYTE-1) (L)
	z=div(Xpnt,VAR_PER_BYTE);
	XindxH=z.quot; // main index
	XindxL=z.rem;  // sub-index

	if(XindxL==0)
	{
		MapXY[XindxH][Ypnt].TN.nib0 = CellVal->nib;
	}
	else
	{	
		MapXY[XindxH][Ypnt].TN.nib1 = CellVal->nib;
	}
}

void ConstantsDefaultW (void)
{
DataEEWrite(401,EE_KVEL1_H);
DataEEWrite(10405,EE_KVEL1_L);
DataEEWrite(401,EE_KVEL2_H);
DataEEWrite(10405,EE_KVEL2_L);
DataEEWrite(9999,EE_ANGLE_KP);	
DataEEWrite(8000,EE_ANGLE_KI);		
DataEEWrite(0001,EE_ANGLE_KD);
DataEEWrite(9999,EE_DIST_KP);	
DataEEWrite(8000,EE_DIST_KI);		
DataEEWrite(0001,EE_DIST_KD);
DataEEWrite(6000,EE_KP1);
DataEEWrite(7000,EE_KI1);
DataEEWrite(1000,EE_KD1);
DataEEWrite(6000,EE_KP2);
DataEEWrite(7000,EE_KI2);
DataEEWrite(1000,EE_KD2);
DataEEWrite(77,EE_KSP1_H);
DataEEWrite(15183,EE_KSP1_L);
DataEEWrite(77,EE_KSP2_H);
DataEEWrite(15183,EE_KSP2_L);
DataEEWrite(28,EE_AXLE_H);
DataEEWrite(17214,EE_AXLE_L);
}

void ConstantsDefaultR (void)	// get constant values as default
{
ANGLE_KP=Q15(0.9999);	
ANGLE_KI=Q15(0.7);		
ANGLE_KD=Q15(0.0001);	

// KP, KI, KD x Distance PID in fractional
// fractional, 3 x 2 = 6 bytes
DIST_KP=Q15(0.9999);		
DIST_KI=Q15(0.8);	
DIST_KD=Q15(0.0001);	

KP1=Q15(0.9);
KI1=Q15(0.7);
KD1=Q15(0.1);
KP2=Q15(0.9);
KI2=Q15(0.7);
KD2=Q15(0.1);		

Kvel[R] = 26555230; // x 1 value
Kvel[L] = 26487032; 

Ksp[R] = 0.005065008;
Ksp[L] = 0.0050520;
Axle = 184.8728;
	
SemiAxle = Axle/2;
}

void ConstantsError(void)		   // Constants paramters error occured
{
	BlinkPeriod = K_ERR_BLINK_PER;// LED1 blinking period (ms)
	BlinkOn     = K_ERR_BLINK_ON; // LED1 on time (ms)
	Blink = 0;
	ErrCode=-20;				  // store the last Error Code
	ConstantsDefaultR();		  // use default parameters
}
void ConstantsRead(void)	// get constant values from permament memory
{
long Tmp1Long;
long Tmp2Long;
int C1;					// generic counter
int C2;
int C3;
int TmpChk;

#ifdef NO_FLASH		// [34a]
#warning -- compiling with NO FLASH *************************************
	ConstantsDefaultR();
	return;
#endif

if (DataEERead(EE_KP1) == 0xFFFF) // default values if not programmed
{
	ConstantsError();
	return;
}

C3 = EE_SCHED;	// starting address for scheduler sequence storage
TmpChk=0;
for (C1=0; C1<16; C1++)
{
	for (C2=0; C2<4; C2++)
	{
		SchedValues[C1][C2]=(DataEERead(C3));
		TmpChk+=SchedValues[C1][C2];
		C3 ++;
	}
}
if (TmpChk!=DataEERead(EE_CHK_SCHED))
{
	ConstantsError();
	return;
}

/* Speed calculation K in micron/second = 26290341 [4] [7] [19]
   Speed calculation K in m/s as a power of 2 to semplify dsPID elaboration
   Kvel[] = (K << 15)
   long, 2 x 4 bytes = 8 byte
   Kvel[x] = 26290341;  x 1 value = 26290341  x 2 value = 13145171
*/
Tmp1Long=(long)DataEERead(EE_KVEL1_H);
TmpChk=Tmp1Long;
Tmp2Long=(long)DataEERead(EE_KVEL1_L);
TmpChk+=Tmp2Long;
Kvel[R] = ((Tmp1Long << 16) + Tmp2Long);
Tmp1Long=(long)DataEERead(EE_KVEL2_H);
TmpChk+=Tmp1Long;
Tmp2Long=(long)DataEERead(EE_KVEL2_L);
TmpChk+=Tmp2Long;
Kvel[L] = ((Tmp1Long << 16) + Tmp2Long);

if (TmpChk!=DataEERead(EE_CHK_KVEL))
{
	ConstantsError();
	return;
}
		
/* KP, KI, KD x Angle PID [23]
   fractional, 3 x 2 = 6 bytes
ANGLE_KP=0.9999
ANGLE_KI=0.8	
ANGLE_KD=0
*/	

Tmp1Long=DataEERead(EE_ANGLE_KP);
TmpChk=Tmp1Long;
ANGLE_KP=Q15((float)(Tmp1Long)/10000);
Tmp1Long=DataEERead(EE_ANGLE_KI);
TmpChk+=Tmp1Long;
ANGLE_KI=Q15((float)(Tmp1Long)/10000);
Tmp1Long=DataEERead(EE_ANGLE_KD);
TmpChk+=Tmp1Long;
ANGLE_KD=Q15((float)(Tmp1Long)/10000);	
	
if (TmpChk!=DataEERead(EE_CHK_ANGLE))
{
	ConstantsError();
	return;
}

/* KP, KI, KD x Dist PID [24]
   fractional, 3 x 2 = 6 bytes
DIST_KP=0.9999
DIST_KI=0.8	
DIST_KD=0
*/	

Tmp1Long=DataEERead(EE_DIST_KP);
TmpChk=Tmp1Long;
DIST_KP=Q15((float)(Tmp1Long)/10000);
Tmp1Long=DataEERead(EE_DIST_KI);
TmpChk+=Tmp1Long;
DIST_KI=Q15((float)(Tmp1Long)/10000);
Tmp1Long=DataEERead(EE_DIST_KD);
TmpChk+=Tmp1Long;
DIST_KD=Q15((float)(Tmp1Long)/10000);	
	
if (TmpChk!=DataEERead(EE_CHK_DIST))
{
	ConstantsError();
	return;
}

/* KP, KI, KD x Speed PID1 and PID2 in int x 10.000 
   fractional, 2MCs x 3params x 2bytes = 12 bytes
KP1=0.6
KI1=0.7
KD1=0.1
KP2=0.6
KI2=0.7
KD2=0.1	
*/	
Tmp1Long=DataEERead(EE_KP1);
TmpChk=Tmp1Long;
KP1=Q15((float)(Tmp1Long)/10000);
Tmp1Long=DataEERead(EE_KI1);
TmpChk+=Tmp1Long;
KI1=Q15((float)(Tmp1Long)/10000);
Tmp1Long=DataEERead(EE_KD1);
TmpChk+=Tmp1Long;
KD1=Q15((float)(Tmp1Long)/10000);	
Tmp1Long=DataEERead(EE_KP2);
TmpChk+=Tmp1Long;
KP2=Q15((float)(Tmp1Long)/10000);
Tmp1Long=DataEERead(EE_KI2);
TmpChk+=Tmp1Long;
KI2=Q15((float)(Tmp1Long)/10000);
Tmp1Long=DataEERead(EE_KD2);
TmpChk+=Tmp1Long;
KD2=Q15((float)(Tmp1Long)/10000);	

if (TmpChk!=DataEERead(EE_CHK_SPEED))
{
	ConstantsError();
	return;
}

/* constants for traveled distance calculation: SPACE_ENC_4X in mm [21]
	float, 2 x 4 = 8 bytes
Ksp[x] = 0.00506145483078356;	
*/
Tmp1Long=(long)DataEERead(EE_KSP1_H);
TmpChk=Tmp1Long;
Tmp2Long=(long)DataEERead(EE_KSP1_L);
TmpChk+=Tmp2Long;
Ksp[R]  = (float)(((Tmp1Long << 16) + Tmp2Long))/((float)(1000000000));	
Tmp1Long=(long)DataEERead(EE_KSP2_H);
TmpChk+=Tmp1Long;
Tmp2Long=(long)DataEERead(EE_KSP2_L);
TmpChk+=Tmp2Long;
Ksp[L]  = (float)(((Tmp1Long << 16) + Tmp2Long))/((float)(1000000000));	

/* base width, distance between center of the wheels [21]
	float, 1 x 4 = 4 bytes
Axle = 185.2222;
*/

Tmp1Long = (long)DataEERead(EE_AXLE_H);
TmpChk+=Tmp1Long;
Tmp2Long = (long)DataEERead(EE_AXLE_L);
TmpChk+=Tmp2Long;
Axle = ((float)((Tmp1Long << 16) + Tmp2Long))/((float)(10000));
if (TmpChk!=DataEERead(EE_CHK_MECH))
{
	ConstantsError();
	return;
}

SemiAxle = Axle/2;
}

void InitPid1(void)
{		
//Initialize the PID data structure: PIDstruct
//Set up pointer to derived coefficients
PIDstruct1.abcCoefficients = &abcCoefficient1[0];
//Set up pointer to controller history samples
PIDstruct1.controlHistory = &controlHistory1[0]; 
// Clear the controler history and the controller output
PIDInit(&PIDstruct1); 
//Derive the a,b, & c coefficients from the Kp, Ki & Kd
PIDCoeffCalc(&kCoeffs1[0], &PIDstruct1); 
}

void InitPid2(void)
{
//Initialize the PID data structure: PIDstruct
//Set up pointer to derived coefficients
PIDstruct2.abcCoefficients = &abcCoefficient2[0];
//Set up pointer to controller history samples
PIDstruct2.controlHistory = &controlHistory2[0]; 
// Clear the controler history and the controller output
PIDInit(&PIDstruct2); 
//Derive the a,b, & c coefficients from the Kp, Ki & Kd
PIDCoeffCalc(&kCoeffs2[0], &PIDstruct2); 
}

void Pid1(void)	
{
#ifdef SLOW_ENC	
	if (labs(Vel[R]) > VEL_MIN_PID) // more frequent PID cycle above a threshold
	{
		Pid1Calc();
	}
	else	// [19f]
	{
		PidCycle[R] ++; 
		if (PidCycle[R] >= 10)
		{
			Pid1Calc();
		}
	}
#else
	Pid1Calc();
#endif
}

void Pid1Calc(void) 
{// [19]
	long IcPeriodTmp;
	int IcIndxTmp;
	int PWM;
	
	IcPeriodTmp=Ic1Period;	// [19a]
	IcIndxTmp=Ic1Indx;
	Ic1Indx = 0;	
	Ic1Period=0;	
//	IC1_FIRST=0;
	PID1_CALC_FLAG = 0;
	Vel[R]=0;
	
	if (IcIndxTmp)	// motor is running [19c]
	{
		Vel[R] = Kvel[R]*IcIndxTmp/IcPeriodTmp;
	}

	SpTick[R] += (int)POS1CNT; // cast to signed to store direction [19]
	POS1CNT=0;

	// calcolo PID
	if (RAMP_FLAG1)	// the motor is acc/dec-elerating [19f]
	{
		PID_REF1 += Ramp1;
		if (RAMP_T_FLAG1)	
		{
			if (PID_REF1 >= VelFin[R]) 
			{
				PID_REF1 = VelFin[R];
				RAMP_FLAG1 = 0;	// acceleration is over
			}
		}
		else
		{
			if (PID_REF1 <= VelFin[R]) 
			{
				PID_REF1 = VelFin[R];
				RAMP_FLAG1 = 0;	// acceleration is over
			}
		}
	}
	
	PID_MES1 = (Vel[R]);			// speed in m/s
	PID(&PIDstruct1);
	PWM = (PID_OUT1 >> 4) + 2048 ;	// [19e]
	SetDCMCPWM1(1, PWM, 0);
//	MOTOR_ENABLE1 = 1;	// [1]		
}

void Pid2(void)	
{
#ifdef SLOW_ENC	
	if (labs(Vel[L]) > VEL_MIN_PID) // more frequent PID cycle above a threshold
	{
		Pid2Calc();
	}
	else	// [19f]
	{
		PidCycle[L] ++; 
		if (PidCycle[L] >= 10)
		{
			Pid2Calc();
		}
	}
#else
	Pid2Calc();
#endif
}

void Pid2Calc(void)
{// [19]
	long IcPeriodTmp;
	int IcIndxTmp;
	int PWM;

	IcPeriodTmp=Ic2Period;	// [19a]
	IcIndxTmp=Ic2Indx;
	Ic2Indx = 0;	
	Ic2Period=0;	
//	IC2_FIRST=0;
	PID2_CALC_FLAG = 0;
	Vel[L]=0;
	
	if (IcIndxTmp)	// motor is running [19c]
	{
		Vel[L] = Kvel[L]*IcIndxTmp/IcPeriodTmp;
	}

	SpTick[L] += (int)POS2CNT; // cast to signed to store direction [19]
	POS2CNT=0;

	// calcolo PID
	if (RAMP_FLAG2)	// the motor is acc/dec-elerating [19f]
	{
		PID_REF2 += Ramp2;
		if (RAMP_T_FLAG2)	
		{
			if (PID_REF2 >= VelFin[L]) 
			{
				PID_REF2 = VelFin[L];
				RAMP_FLAG2 = 0;	// acceleration is over
			}
		}
		else
		{
			if (PID_REF2 <= VelFin[L]) 
			{
				PID_REF2 = VelFin[L];
				RAMP_FLAG2 = 0;	// acceleration is over
			}
		}
	}
	
	PID_MES2 = (Vel[L]);			// speed in m/s
	PID(&PIDstruct2);
	PWM = (PID_OUT2 >> 4) + 2048 ;	// [19e]
	SetDCMCPWM1(2, PWM, 0);
//	MOTOR_ENABLE2 = 1;	// [1]			
}


void DelayN1ms(int n)
{
	int ms;
	for (ms = 0; ms < n; ms ++)
	{
		DelayN10us(100);
	}
}

void DelayN10us(int n) // [22]
{
	int DelayCount;
	for (DelayCount = 0; DelayCount < (57 * n); DelayCount ++);	
}

void TxCont(void)
{
	int Ptmp;				// temp for position values
	int CurrTmp;			// temp for motors current value
	int VelInt[2];			// speed in mm/s as an integer
	int Alpha;			// rotation angle in degrees

	UartContTxTimer=UART_CONT_TIMEOUT;	// timer reset
	if (TxContFlag == 1)
	{
		Ptmp = (VelMes[R] + VelMes[L]) >> 1;	// average speed
	 	UartTxBuff[0]=Ptmp>>8; 
		UartTxBuff[1]=Ptmp;
		UartTxBuff[2]=9;						// tab
		// Curr = int -> 2byte (mA)
		CurrTmp = ADCValue[R]+ADCValue[L];		// total current
	 	UartTxBuff[3]=CurrTmp>>8;
		UartTxBuff[4]=CurrTmp;
		UartTxBuff[5]=9;						// tab
		// PosXmes rounded in a Int -> 2 byte (mm)
		Ptmp = FLOAT2INT(PosXmes);				// PosX
	 	UartTxBuff[6]=Ptmp>>8; 
		UartTxBuff[7]=Ptmp;
		UartTxBuff[8]=9;						// tab
		// PosYmes rounded in a Int -> 2 byte (mm)
		Ptmp = FLOAT2INT(PosYmes);				// PosY
	 	UartTxBuff[9]=Ptmp>>8; 
		UartTxBuff[10]=Ptmp;
		UartTxBuff[11]=9;						// tab
		// ThetaMes rounded in a Int -> 2 byte (degrees)
		Ptmp = ThetaMes * RAD2DEG;				// Theta
		Alpha = FLOAT2INT(Ptmp);
	 	UartTxBuff[12]=Alpha>>8; 
		UartTxBuff[13]=Alpha;
		UartTxBuff[14]=10;						// LF
		UartTxBuff[15]=13;						// CR
		DMA6CNT = 15;	// # of DMA requests
	}
	else
	{
		// VelInt = Int -> 2 byte
		VelInt[R]=(long)(Vel[R] * 1000)>>15;		// VelR
	 	UartTxBuff[0]=VelInt[R]>>8; 
		UartTxBuff[1]=VelInt[R];
		UartTxBuff[2]=9;						// tab
		VelInt[L]=(long)(Vel[L] * 1000)>>15;		// VelL
	 	UartTxBuff[3]=VelInt[L]>>8; 
		UartTxBuff[4]=VelInt[L];
		UartTxBuff[5]=9;						// tab
		// ADCValue = int -> 2byte
	 	UartTxBuff[6]=ADCValue[R]>>8;			// CurrR
		UartTxBuff[7]=ADCValue[R];
		UartTxBuff[8]=9;						// tab
		UartTxBuff[9]=ADCValue[L]>>8;			// CurrL
		UartTxBuff[10]=ADCValue[L];
		UartTxBuff[11]=9;						// tab
		// Space = int -> 2byte
	 	UartTxBuff[12]=SpTick[R]>>8;			// SpTickR
		UartTxBuff[13]=SpTick[R];
		UartTxBuff[14]=9;						// tab
		SpTick[R] = 0; // [19]
		UartTxBuff[15]=SpTick[L]>>8;			// SpTickL
		UartTxBuff[16]=SpTick[L];
		SpTick[L] = 0; // [19]
		UartTxBuff[17]=10;						// LF
		UartTxBuff[18]=13;						// CR
		DMA6CNT = 18;	// # of DMA requests
	}

	DMA6CONbits.CHEN  = 1;		// Re-enable TX DMA Channel
	DMA6REQbits.FORCE = 1;		// Manual mode: Kick-start the first TX
}

/*---------------------------------------------------------------------------*/
/* Interrupt Service Routines                                                */
/*---------------------------------------------------------------------------*/
#ifndef TIMER_OFF
void _ISR_PSV _T1Interrupt(void)	// Timer 1 [13]
{
	_T1IF=0;   		// interrupt flag reset
	UartContTxTimer --;	// timer for continuos send mode
	Blink ++;			// heartbeat LED blink
	
	// cycle 0 actions
	PID1_CALC_FLAG = 1;	// PID1 and speed calculation enabled
	PID2_CALC_FLAG = 1;	// PID2 and speed calculation enabled
		
	Cycle1 ++;
	// cycle 1 actions
	if (Cycle1 >= CICLE1_TMO)
	{
		Cycle1 = 0;
		CYCLE1_FLAG = 1; // it's time to start first cycle actions [23]

		Cycle2 ++;
		// cycle 2 actions
		if (Cycle2 >= CICLE2_TMO)
		{
			Cycle2 = 0;
			CYCLE2_FLAG=1; // it's time to start second cycle actions [24]
			IdleSample ++; // [25]
			RtTimer --;	   // real time delay
			RndTimer ++;   // Timer for randomly avoid central obstacles
		}
	}
}

void _ISR_PSV _T2Interrupt(void)	// Timer 2 [12]
{
	_T2IF = 0;					// interrupt flag reset
	Tmr2OvflwCount1 ++;		// TMR2 overflow as occurred
	Tmr2OvflwCount2 ++;		// TMR2 overflow as occurred
}

#endif

void _ISR_PSV _DMA7Interrupt(void)	// DMA for ADC [2]
{ 	
	_DMA7IF = 0;	// interrupt flag reset
	ADC_CALC_FLAG = 1; // enable ADC average calculus
}


void _ISR_PSV _DMA6Interrupt(void)	// DMA for UART1 TX [6d]
{
	_DMA6IF = 0;	// interrupt flag reset
	MAP_BUFF_FLAG = 1; // ready to send another map grid row
}

void _ISR_PSV _DMA5Interrupt(void)	// DMA for UART2 TX [6zd]
{
	_DMA5IF = 0;	// interrupt flag reset
	MAP_BUFF_FLAG = 1; // ready to send another map grid row
}

void _ISR_PSV _IC1Interrupt(void)	// Input Capture 1 [7]
{
	_IC1IF = 0;					// interrupt flag reset
	if (IC1_FIRST)// first sample, stores TMR2 starting value [19b]
	{
		Ic1CurrPeriod = IC1BUF;
		if (Tmr2OvflwCount1 == 0)
		{
			Ic1Period += (Ic1CurrPeriod - Ic1PrevPeriod);
		}
		else
		{// [7a]
		#ifdef SLOW_ENC	
			Ic1Period += (Ic1CurrPeriod + (0xFFFF - Ic1PrevPeriod)
			 +(0xFFFF * (Tmr2OvflwCount1 - 1)));
		#else
			Ic1Period += (Ic1CurrPeriod + (0xFFFF - Ic1PrevPeriod));
		#endif
			Tmr2OvflwCount1 = 0;	
		}
		Ic1PrevPeriod = Ic1CurrPeriod;
		if (QEI1CONbits.UPDN)	// [7b]
		{
			Ic1Indx ++;
		}
		else
		{
			Ic1Indx --;
		}
	}
	else
	{
		Ic1PrevPeriod = IC1BUF;
		IC1_FIRST=1;
	}
}


void _ISR_PSV _IC2Interrupt(void)	// Input Capture 2 [7]
{
	_IC2IF = 0;					// interrupt flag reset
	if (IC2_FIRST)// first sample, stores TMR2 starting value [19b]
	{
		Ic2CurrPeriod = IC2BUF;
		if (Tmr2OvflwCount2 == 0)
		{
			Ic2Period += (Ic2CurrPeriod - Ic2PrevPeriod);
		}
		else
		{// [7a]
		#ifdef SLOW_ENC	
			Ic2Period += (Ic2CurrPeriod + (0xFFFF - Ic2PrevPeriod)
			 +(0xFFFF * (Tmr2OvflwCount2 - 1)));
		#else
			Ic2Period += (Ic2CurrPeriod + (0xFFFF - Ic2PrevPeriod));
		#endif
			Tmr2OvflwCount2 = 0;	
		}
		Ic2PrevPeriod = Ic2CurrPeriod;
		if (QEI2CONbits.UPDN)	// [7b]
		{
			Ic2Indx ++;
		}
		else
		{
			Ic2Indx --;
		}
	}
	else
	{
		Ic2PrevPeriod = IC2BUF;
		IC2_FIRST=1;
	}
}


void __attribute__ ((interrupt, no_auto_psv)) _U1ErrInterrupt(void)
{
	IFS4bits.U1EIF = 0; // Clear the UART1 Error Interrupt Flag
}

void __attribute__ ((interrupt, no_auto_psv)) _U2ErrInterrupt(void)
{
	IFS4bits.U2EIF = 0; // Clear the UART2 Error Interrupt Flag
}

//{
/* Disabled------------------
void _ISR_PSV _CNInterrupt(void)	// change Notification [3]
{
	_CNIF = 0;		// interrupt flag reset
}

void _ISR_PSV _INT1Interrupt(void)	// External Interrupt INT1 [8]
{
	_INT1IF = 0;    // interrupt flag reset
	ClrWdt();		// [1]
	LED1=0;			// [1]
}

void _ISR_PSV _U1TXInterrupt(void)	// UART TX [6a]
{
	_U1TXIF = 0;	// interrupt flag reset

	if (UartTxCntr < UartTxBuffSize)
	{
		WriteUART1(UartTxBuff[UartTxCntr]);
		UartTxCntr++;
	}
	else
	{// waits for UART sending complete to disable the peripheral
		TxFlag = 2;	
	}
}
------------------ Disabled */
//}
/*****************************************************************************/


