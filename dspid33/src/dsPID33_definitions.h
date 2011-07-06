/* ////////////////////////////////////////////////////////////////////////////
** Included in "dsPID.c", it contains definitions and variables initialization
/////////////////////////////////////////////////////////////////////////////*/

/*---------------------------------------------------------------------------*/
/* standard include														     */
/*---------------------------------------------------------------------------*/
#include "dsPID33_common.h"

/*---------------------------------------------------------------------------*/
/* Status bits			    											     */
/*---------------------------------------------------------------------------*/

/* Boot Segment Program Memory:No Boot Segment Program Memory
** Write Protect:Disabled **/
//_FBS(BSS_NO_FLASH & BWRP_WRPROTECT_OFF);

// External Oscillator
_FOSCSEL(FNOSC_PRI);			// Primary (XT, HS, EC) Oscillator 

// Clock Switching is enabled and Fail Safe Clock Monitor is disabled
								// OSC2 Pin Function: OSC2 is Clock Output
								// Primary Oscillator Mode: XT Crystanl
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF  & POSCMD_XT);  					

/* Background Debug Enable Bit: Device will Reset in user mode
** Debugger/Emulator Enable Bit: Reset in operational mode
** JTAG Enable Bit: JTAG is disabled
** ICD communication channel select bits: 
** communicate on PGC1/EMUC1 and PGD1/EMUD1 **/
//_FICD(BKBUG_ON & COE_ON & JTAGEN_OFF & ICS_PGD1); // valid until C30 3.11
_FICD(JTAGEN_OFF & ICS_PGD1);
/*Enable MCLR reset pin and turn on the power-up timers with 64ms delay.
PIN managed by PWM at reset*/           			 
_FPOR(FPWRT_PWR64 & PWMPIN_ON & HPOL_ON & LPOL_ON);				

/* Code Protect: Code Protect off
** Code Protect: Disabled
** Write Protect: Disabled **/ 
_FGS(GSS_OFF & GCP_OFF & GWRP_OFF);         			 

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* External defininitions                                                    */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//UART
//{
extern volatile unsigned char UartRxBuff[][2];
extern unsigned char 
		UartTxBuff[] __attribute__((space(dma),aligned(128)));
extern volatile unsigned char UartRxPtrIn;
extern unsigned char UartRxPtrOut;	
extern volatile int UartRxStatus;

extern volatile unsigned char Uart2RxPtrIn;
extern unsigned char Uart2RxPtrOut;
extern volatile int Uart2RxStatus;	
extern unsigned char TmpPtr;
extern unsigned char UartRxPtrData;
extern unsigned char UartRxCmd[];
extern unsigned char UartTmpBuff[][2];
extern const unsigned char Test[];
extern unsigned char TmpPtr2;
extern unsigned char Uart2RxPtrData;
int Port;			// Port = 0 for UART1, Port = 1 for UART2
int SendMapPort;	// to temporay store port number for delayed TX
int ResetPort;		// to temporay store port number for reset
//}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Global defininitions                                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

struct Bits{
	unsigned bit0:1;
	unsigned bit1:1;
	unsigned bit2:1;
	unsigned bit3:1;
	unsigned bit4:1;
	unsigned bit5:1;
	unsigned bit6:1;
	unsigned bit7:1;
	unsigned bit8:1;
	unsigned bit9:1;
	unsigned bit10:1;
	unsigned bit11:1;
	unsigned bit12:1;
	unsigned bit13:1;
	unsigned bit14:1;
	unsigned bit15:1;
};

struct Bits VARbits1;
struct Bits VARbits2;
volatile struct Bits VOLbits1;

//----- Flags

#define RC6Tick(x) (Delay1KTCYx(4*x),Delay100TCYx(4*x),Delay10TCYx(3*x))
#define	DELAY100MS Delay10KTCYx(100)	// 100ms

#if defined(__dsPIC33FJ64MC802__) || defined(__dsPIC33FJ128MC802__)
// -- compiling for a 28 pin DSC ***************************************

	#define LED1 _LATA4
	#define LED2 _LATB4
//	#define MOTOR_ENABLE1 _LATB1
//	#define MOTOR_ENABLE2 _LATB0
	#define DIR_485 _LATB9

#elif defined(__dsPIC33FJ64MC804__) || defined(__dsPIC33FJ128MC804__)
// -- compiling for a 44 pin DSC ***************************************

#ifdef DSNAVCON33

	#define LED1 _LATA8
	#define MOTOR_ENABLE1 _LATA7
	#define MOTOR_ENABLE2 _LATA10

#endif

#ifdef ROBOCONTROLLER
	
	#define LED1 _LATA8
	#define LED2 _LATA9
	#define AUX1 _LATA7
	#define AUX2 _LATA10
	#define MOTOR_ENABLE1 _LATA0
	#define MOTOR_ENABLE2 _LATA1
	#define DIR_485 _LATC3

#endif

#else

#error -- dsPIC33FJ not recognized. Accepted only 64/128MC802 or 64/128MC804

#endif

const long Tcy = 1000000/(float)(FCY)* 100000000;

//----- Variabili
int i = 0; 		// generic index
int j = 0; 		// generic index
long Blink = 0; // heartbeat blink index

#define UART_CONT_TIMEOUT 100			// continuos parameters TX in ms(debug)
unsigned int TxContFlag=0;				// continuos parameters TX flag (debug)
volatile int UartContTxTimer=UART_CONT_TIMEOUT;	// timer for cont parameters TX 
#define UART_CONT_TIMEOUT 100			// continuos parameters TX in ms(debug) 

// ADC
int ADCValue[2] = {0,0};		// 64 sample average ADC AN0, AN1 reading
#define ADC_CALC_FLAG VOLbits1.bit6	// enable ADC value average calculus
#define ADC_OVLD_LIMIT 800			// in mA
#define ADC_OVLD_TIME	100			// n x 10ms
char ADCOvldCount[2] = {0,0};		// how long overload status last 	

// Input Capture (speed measurement)
volatile int Ic1Indx = 0;			// samples buffer index for IC1
volatile int Ic2Indx = 0;			// samples buffer index for IC2
volatile unsigned int Ic1PrevPeriod;	// previous sample for IC1
volatile unsigned int Ic2PrevPeriod;	// previous sample for IC2
volatile long Ic1Period = 0;			// n samples average value for IC1
volatile long Ic2Period = 0;			// n samples average value for IC2
volatile unsigned int Ic1CurrPeriod;	// current value for IC1
volatile unsigned int Ic2CurrPeriod;	// current value for IC2
long Tmr2OvflwCount1;					// timer 2 overflow for IC1
long Tmr2OvflwCount2;					// timer 2 overflow for IC2
#define IC1_FIRST VOLbits1.bit7		  // first encoder pulse for IC1
#define IC2_FIRST VOLbits1.bit8		  // first encoder pulse for IC2

#define PID1_CALC_FLAG VOLbits1.bit0	  // PID and speed elaboration flag
#define PID2_CALC_FLAG VOLbits1.bit1	  // PID and speed elaboration flag

/*
-> [4] [7] [19]
THIS PARAMETERS ARE VALID FOR RINO ROBOTIC PLATFORM.
HERE JUST AS AN EXAMPLE ON HOW TO CALCULATE THEM
Encoder = 300 cpr 
Gear reduction ratio = 30:1
Wheel speed = 200 rpm
Encoder pulses for each wheel turn = 9.000
Wheel diameter = 58mm -> circumference = 182,2123739mm
Space for each encoder pulse 1x mode Delta S = 0,020245819mm
Space for each encoder pulse 2x mode Delta S = 0,01012291mm
Space for each encoder pulse 4x mode Delta S = 0,005061455mm
Speed calculation K in micron/second = 298536736 -> [19]
*/
#define CPR 300
#define GEAR_RATIO 30
#define CPR_WHEEL CPR*GEAR_RATIO				// 9000
#define DIAMETER  0.058
#define CIRC  DIAMETER*PI						// 0.182212374
#define SPACE_ENC_1X (CIRC / (CPR_WHEEL))		// 2.0245819E-05
#define SPACE_ENC_2X (CIRC / (CPR_WHEEL * 2))	// 1.012291E-05
#define SPACE_ENC_4X (CIRC / (CPR_WHEEL * 4))
#define K_VEL ((SPACE_ENC_2X) / (TCY))			// TCY = 25,2341731266 ns

#define R 0							// right index
#define L 1							// right index

// Speed calculation K in m/s
long Kvel[2]; // K_VEL << 15
int VelDes[2] = {0,0};	// desired speed mm/s
int VelMes[2] = {0,0};	// measured speed mm/s	
int VelDesM = 0;		// mean measured speed mm/s [23]
// threshold to change PID calculation frequency. = Speed in mm/s *2^15/1000
#define VEL_MIN_PID 1600 // Approx 50mm/s
unsigned char PidCycle[2]; // counter for longer cycle PID
// constants for traveled distance calculation: SPACE_ENC_4X in mm
// see [2] in descrEng.exe in dsPID folder
long double Ksp[2];
float Axle;// base width, distance between center of the wheels
float SemiAxle;// Axle/2 useful for odometry calculation
float SemiAxle; // Axle / 2
int SpTick[2] = {0,0};	// distance traveled by a wheel as encoder pulses

float Spmm[2] = {0,0};	// distance traveled by a wheel as mm(SpTick[x]*Ksp[x])
float Space = 0;		// total distance traveled by the robot
#define SPMIN 0.01		// Minimum value to perform odometry
float CosPrev =	1;		// previous value for Cos(ThetaMes)
float SinPrev =	0;		// previous value for Sin(ThetaMes)
float PosXmes = 0;		// current X position coordinate
float PosYmes = 0;		// current Y position coordinate
float ThetaMes = 0;		// Current orientation angle
// #define PI 	  3.1415926536 180°
#define TWOPI 6.2831853072	// 360°
#define HALFPI 1.570796327	// 90°
#define QUARTPI 0.7853981634// 45°

long Vel[2];	// speed in m/s  << 15 (long) for PID1 and PID2

// PID [19d]
tPID PIDstruct1;
fractional abcCoefficient1[3] __attribute__ ((section (".xbss, bss, xmemory")));
fractional controlHistory1[3] __attribute__ ((section (".ybss, bss, ymemory")));
fractional kCoeffs1[] = {0,0,0};
#define KP1 kCoeffs1[0]
#define KI1 kCoeffs1[1]
#define KD1 kCoeffs1[2]
#define PID_OUT1 PIDstruct1.controlOutput
#define PID_MES1 PIDstruct1.measuredOutput
#define PID_REF1 PIDstruct1.controlReference

tPID PIDstruct2;
fractional abcCoefficient2[3] __attribute__ ((section (".xbss, bss, xmemory")));
fractional controlHistory2[3] __attribute__ ((section (".ybss, bss, ymemory")));
fractional kCoeffs2[] = {0,0,0};
#define KP2 kCoeffs2[0]
#define KI2 kCoeffs2[1]
#define KD2 kCoeffs2[2]
#define PID_OUT2 PIDstruct2.controlOutput
#define PID_MES2 PIDstruct2.measuredOutput
#define PID_REF2 PIDstruct2.controlReference

#define RAMP_FLAG1 VARbits1.bit6	// acc/dec-eleration flag
#define RAMP_T_FLAG1 VARbits1.bit7	// acc/dec-eleration direction
#define RAMP_FLAG2 VARbits1.bit1	// acc/dec-eleration flag
#define RAMP_T_FLAG2 VARbits1.bit15	// acc/dec-eleration direction
fractional VelFin[2] = {0,0};		// temp for speed during acceleration	
//#define ACC 0.00025				// acceleration
//#define DEC 0.0005				// deceleration
  #define ACC 0.00005				// acceleration
  #define DEC 0.001					// deceleration

fractional Ramp1;					// acc/dec-eleration ramp slope
fractional Ramp2;					// acc/dec-eleration ramp slope

int Curr[2] = {0,0};				// motor current

/* Field mapping, regular size*/
#define CELL_SIZE 100		// Dimension of grid cell side in mm
#define MAP_SIZE 15000		// Abs Dimension of field (X = Y) in mm
#define HALF_MAP_SIZE 7500	// + - Dimension of field (X = Y) in mm
#define VAR_PER_BYTE 2		// # of cell var per byte

// ATTENTION when updating matrix size values, update as well the 
//	equivalent values on dsNavConsole software code
#define X_SIZE 75 // (MAP_SIZE/CELL_SIZE/VAR_PER_BYTE) MAX X array index
#define Y_SIZE 150 //(MAP_SIZE/CELL_SIZE) MAX Y array index
#define X_POINT_MAX 150 // [22g]
#define X_POINT_MIN 0   // [22g]
#define Y_POINT_MAX 150 // [22g]
#define Y_POINT_MIN 0   // [22g]

/*Field mapping, regular size */

/* Field mapping =========== FOR DEBUGGING PURPOSES ==================
	because MPLAB watch window becomes extremely slow with a big array
	of struct, during debugging this array must be decresead in size
#define CELL_SIZE 100		// Dimension of grid cell side in mm
#define MAP_SIZE 5000		// Abs Dimension of field (X = Y) in mm
#define HALF_MAP_SIZE 2500	// + - Dimension of field (X = Y) in mm
#define VAR_PER_BYTE 2		// # of cell var per byte

// ATTENTION when updating matrix size values, update as well the 
//	equivalent values on dsNavConsole software code
#define X_SIZE 25 // (MAP_SIZE/CELL_SIZE/VAR_PER_BYTE) MAX X array index
#define Y_SIZE 50 //(MAP_SIZE/CELL_SIZE) MAX Y array index
#define X_POINT_MAX 50 // [22g]
#define X_POINT_MIN 0  // [22g]
#define Y_POINT_MAX 50 // [22g]
#define Y_POINT_MIN 0  // [22g]
Field mapping =========== FOR DEBUGGING PURPOSES ==================*/

int Xshift = 0;	// [22g]
int Yshift = 0;	// [22g]

typedef struct
{
	unsigned char nib:4;
}
nibble;

typedef union
{
	unsigned char UC;
	struct
	{
		unsigned char nib0:4;
		unsigned char nib1:4;
	}TN;
}PackedBit;

void SetMap(int Xpoint, int Ypoint, nibble *CellVal);

PackedBit MapXY[X_SIZE][Y_SIZE];	// to store field mapping [22b]

// unsigned char MapXY[X_SIZE][Y_SIZE];	// to store field mapping [22b]
int XindxPrev=10000;		// previous X index 
int YindxPrev=10000;		// previous Y index 
#define SPACE_FLAG VARbits1.bit2
float MaxMapX = HALF_MAP_SIZE;	// current field limits
float MinMapX = -HALF_MAP_SIZE;
float MaxMapY = HALF_MAP_SIZE;
float MinMapY = -HALF_MAP_SIZE;
#define MAP_SEND_FLAG VOLbits1.bit4	// to send map to console
#define MAP_BUFF_FLAG VOLbits1.bit5 // to send another map grid row
unsigned char MapSendIndx;

// Angle PID [23]
tPID AnglePIDstruct;
fractional AngleabcCoefficient[3] 
	__attribute__ ((section (".xbss, bss, xmemory")));
fractional AnglecontrolHistory[3] 
	__attribute__ ((section (".ybss, bss, ymemory")));
fractional AngleKCoeffs[] = {0,0,0};
#define ANGLE_KP AngleKCoeffs[0]						// K parameters
#define ANGLE_KI AngleKCoeffs[1]
#define ANGLE_KD AngleKCoeffs[2]
#define ANGLE_PID_DES AnglePIDstruct.controlReference	// desired angle
#define ANGLE_PID_MES AnglePIDstruct.measuredOutput		// measured angle
#define ANGLE_PID_OUT AnglePIDstruct.controlOutput		// PID output
#define MAX_ROT_SPEED 512	// MAX speed (+ or -) of wheels during rotation
#define RAD2DEG 57.295779513	// = 180/PI
#define DEG2RAD 0.0174532925	// = PI/180
float ThetaDes = 0;	// desired orientation angle (set point) [23] (Rad)
float ThetaDesRef;	// to temporarely store angle (Rad)
int AngleCmp;		// compass bearing from sensors board (Deg * 10)
#define ORIENTATION_FLAG VARbits1.bit3	// to control Orientation() execution
#define ANGLE_OK_FLAG VARbits1.bit12	// target angle reached
#define MIN_THETA_ERR DEG2RAD	// acceptable angle error in radians = 1°

// **debug**
// unsigned char ErrNo[2][16];	// store the occurrence of any kind of error 
// unsigned char ErrNo2[16];	// for both MCs and for RX2
// **debug**

// Distance [24]
// field boundaries in mm. Diagonal is 10m
#define BOUND_X_MIN -7070		// left bottom
#define BOUND_Y_MIN -7070
#define BOUND_X_MAX 7070	// top right
#define BOUND_Y_MAX 7070

#define BOUND_FREE 500		// if close to goal don't care of boundaries
#define OBST_THRESHOLD 1500.00 // if far enough don't care of obstacles
#define OBST_MIN_DIST 50      // if too close another avoiding procedure

#define OBST_FIELD 16		// obstacle visibility (16 cells around robot)
// Table to compute distance from the object
int DistTable[33][33] __attribute__((space(auto_psv)))=
{
{2263,2193,2126,2062,2000,1942,1887,1836,1789,1746,1709,1676,1649,1628,1612,1603,1600,1603,1612,1628,1649,1676,1709,1746,1789,1836,1887,1942,2000,2062,2126,2193,2263},
{2193,2121,2052,1985,1921,1860,1803,1749,1700,1655,1616,1581,1552,1530,1513,1503,1500,1503,1513,1530,1552,1581,1616,1655,1700,1749,1803,1860,1921,1985,2052,2121,2193},
{2126,2052,1980,1910,1844,1780,1720,1664,1612,1565,1523,1487,1456,1432,1414,1404,1400,1404,1414,1432,1456,1487,1523,1565,1612,1664,1720,1780,1844,1910,1980,2052,2126},
{2062,1985,1910,1838,1769,1703,1640,1581,1526,1476,1432,1393,1360,1334,1315,1304,1300,1304,1315,1334,1360,1393,1432,1476,1526,1581,1640,1703,1769,1838,1910,1985,2062},
{2000,1921,1844,1769,1697,1628,1562,1500,1442,1389,1342,1300,1265,1237,1217,1204,1200,1204,1217,1237,1265,1300,1342,1389,1442,1500,1562,1628,1697,1769,1844,1921,2000},
{1942,1860,1780,1703,1628,1556,1487,1421,1360,1304,1253,1208,1170,1140,1118,1105,1100,1105,1118,1140,1170,1208,1253,1304,1360,1421,1487,1556,1628,1703,1780,1860,1942},
{1887,1803,1720,1640,1562,1487,1414,1345,1281,1221,1166,1118,1077,1044,1020,1005,1000,1005,1020,1044,1077,1118,1166,1221,1281,1345,1414,1487,1562,1640,1720,1803,1887},
{1836,1749,1664,1581,1500,1421,1345,1273,1204,1140,1082,1030,985,949,922,906,900,906,922,949,985,1030,1082,1140,1204,1273,1345,1421,1500,1581,1664,1749,1836},
{1789,1700,1612,1526,1442,1360,1281,1204,1131,1063,1000,943,894,854,825,806,800,806,825,854,894,943,1000,1063,1131,1204,1281,1360,1442,1526,1612,1700,1789},
{1746,1655,1565,1476,1389,1304,1221,1140,1063,990,922,860,806,762,728,707,700,707,728,762,806,860,922,990,1063,1140,1221,1304,1389,1476,1565,1655,1746},
{1709,1616,1523,1432,1342,1253,1166,1082,1000,922,849,781,721,671,632,608,600,608,632,671,721,781,849,922,1000,1082,1166,1253,1342,1432,1523,1616,1709},
{1676,1581,1487,1393,1300,1208,1118,1030,943,860,781,707,640,583,539,510,500,510,539,583,640,707,781,860,943,1030,1118,1208,1300,1393,1487,1581,1676},
{1649,1552,1456,1360,1265,1170,1077,985,894,806,721,640,566,500,447,412,400,412,447,500,566,640,721,806,894,985,1077,1170,1265,1360,1456,1552,1649},
{1628,1530,1432,1334,1237,1140,1044,949,854,762,671,583,500,424,361,316,300,316,361,424,500,583,671,762,854,949,1044,1140,1237,1334,1432,1530,1628},
{1612,1513,1414,1315,1217,1118,1020,922,825,728,632,539,447,361,283,224,200,224,283,361,447,539,632,728,825,922,1020,1118,1217,1315,1414,1513,1612},
{1603,1503,1404,1304,1204,1105,1005,906,806,707,608,510,412,316,224,141,100,141,224,316,412,510,608,707,806,906,1005,1105,1204,1304,1404,1503,1603},
{1600,1500,1400,1300,1200,1100,1000,900,800,700,600,500,400,300,200,100,0,100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500,1600},
{1603,1503,1404,1304,1204,1105,1005,906,806,707,608,510,412,316,224,141,100,141,224,316,412,510,608,707,806,906,1005,1105,1204,1304,1404,1503,1603},
{1612,1513,1414,1315,1217,1118,1020,922,825,728,632,539,447,361,283,224,200,224,283,361,447,539,632,728,825,922,1020,1118,1217,1315,1414,1513,1612},
{1628,1530,1432,1334,1237,1140,1044,949,854,762,671,583,500,424,361,316,300,316,361,424,500,583,671,762,854,949,1044,1140,1237,1334,1432,1530,1628},
{1649,1552,1456,1360,1265,1170,1077,985,894,806,721,640,566,500,447,412,400,412,447,500,566,640,721,806,894,985,1077,1170,1265,1360,1456,1552,1649},
{1676,1581,1487,1393,1300,1208,1118,1030,943,860,781,707,640,583,539,510,500,510,539,583,640,707,781,860,943,1030,1118,1208,1300,1393,1487,1581,1676},
{1709,1616,1523,1432,1342,1253,1166,1082,1000,922,849,781,721,671,632,608,600,608,632,671,721,781,849,922,1000,1082,1166,1253,1342,1432,1523,1616,1709},
{1746,1655,1565,1476,1389,1304,1221,1140,1063,990,922,860,806,762,728,707,700,707,728,762,806,860,922,990,1063,1140,1221,1304,1389,1476,1565,1655,1746},
{1789,1700,1612,1526,1442,1360,1281,1204,1131,1063,1000,943,894,854,825,806,800,806,825,854,894,943,1000,1063,1131,1204,1281,1360,1442,1526,1612,1700,1789},
{1836,1749,1664,1581,1500,1421,1345,1273,1204,1140,1082,1030,985,949,922,906,900,906,922,949,985,1030,1082,1140,1204,1273,1345,1421,1500,1581,1664,1749,1836},
{1887,1803,1720,1640,1562,1487,1414,1345,1281,1221,1166,1118,1077,1044,1020,1005,1000,1005,1020,1044,1077,1118,1166,1221,1281,1345,1414,1487,1562,1640,1720,1803,1887},
{1942,1860,1780,1703,1628,1556,1487,1421,1360,1304,1253,1208,1170,1140,1118,1105,1100,1105,1118,1140,1170,1208,1253,1304,1360,1421,1487,1556,1628,1703,1780,1860,1942},
{2000,1921,1844,1769,1697,1628,1562,1500,1442,1389,1342,1300,1265,1237,1217,1204,1200,1204,1217,1237,1265,1300,1342,1389,1442,1500,1562,1628,1697,1769,1844,1921,2000},
{2062,1985,1910,1838,1769,1703,1640,1581,1526,1476,1432,1393,1360,1334,1315,1304,1300,1304,1315,1334,1360,1393,1432,1476,1526,1581,1640,1703,1769,1838,1910,1985,2062},
{2126,2052,1980,1910,1844,1780,1720,1664,1612,1565,1523,1487,1456,1432,1414,1404,1400,1404,1414,1432,1456,1487,1523,1565,1612,1664,1720,1780,1844,1910,1980,2052,2126},
{2193,2121,2052,1985,1921,1860,1803,1749,1700,1655,1616,1581,1552,1530,1513,1503,1500,1503,1513,1530,1552,1581,1616,1655,1700,1749,1803,1860,1921,1985,2052,2121,2193},
{2263,2193,2126,2062,2000,1942,1887,1836,1789,1746,1709,1676,1649,1628,1612,1603,1600,1603,1612,1628,1649,1676,1709,1746,1789,1836,1887,1942,2000,2062,2126,2193,2263}
} ;//in program memory space

// Table to compute distance from the object squared
long DistTableSqr[33][33] __attribute__((space(auto_psv)))=
{
{5121169,4809249,4519876,4251844,4000000,3771364,3560769,3370896,3200521,3048516,2920681,2808976,2719201,2650384,2598544,2569609,2560000,2569609,2598544,2650384,2719201,2808976,2920681,3048516,3200521,3370896,3560769,3771364,4000000,4251844,4519876,4809249,2263},
{4809249,4498641,4210704,3940225,3690241,3459600,3250809,3059001,2890000,2739025,2611456,2499561,2408704,2340900,2289169,2259009,2250000,2259009,2289169,2340900,2408704,2499561,2611456,2739025,2890000,3059001,3250809,3459600,3690241,3940225,4210704,4498641,2193},
{4519876,4210704,3920400,3648100,3400336,3168400,2958400,2768896,2598544,2449225,2319529,2211169,2119936,2050624,1999396,1971216,1960000,1971216,1999396,2050624,2119936,2211169,2319529,2449225,2598544,2768896,2958400,3168400,3400336,3648100,3920400,4210704,2126},
{4251844,3940225,3648100,3378244,3129361,2900209,2689600,2499561,2328676,2178576,2050624,1940449,1849600,1779556,1729225,1700416,1690000,1700416,1729225,1779556,1849600,1940449,2050624,2178576,2328676,2499561,2689600,2900209,3129361,3378244,3648100,3940225,2062},
{4000000,3690241,3400336,3129361,2879809,2650384,2439844,2250000,2079364,1929321,1800964,1690000,1600225,1530169,1481089,1449616,1440000,1449616,1481089,1530169,1600225,1690000,1800964,1929321,2079364,2250000,2439844,2650384,2879809,3129361,3400336,3690241,2000},
{3771364,3459600,3168400,2900209,2650384,2421136,2211169,2019241,1849600,1700416,1570009,1459264,1368900,1299600,1249924,1221025,1210000,1221025,1249924,1299600,1368900,1459264,1570009,1700416,1849600,2019241,2211169,2421136,2650384,2900209,3168400,3459600,1942},
{3560769,3250809,2958400,2689600,2439844,2211169,1999396,1809025,1640961,1490841,1359556,1249924,1159929,1089936,1040400,1010025,1000000,1010025,1040400,1089936,1159929,1249924,1359556,1490841,1640961,1809025,1999396,2211169,2439844,2689600,2958400,3250809,1887},
{3370896,3059001,2768896,2499561,2250000,2019241,1809025,1620529,1449616,1299600,1170724,1060900,970225,900601,850084,820836,810000,820836,850084,900601,970225,1060900,1170724,1299600,1449616,1620529,1809025,2019241,2250000,2499561,2768896,3059001,1836},
{3200521,2890000,2598544,2328676,2079364,1849600,1640961,1449616,1279161,1129969,1000000,889249,799236,729316,680625,649636,640000,649636,680625,729316,799236,889249,1000000,1129969,1279161,1449616,1640961,1849600,2079364,2328676,2598544,2890000,1789},
{3048516,2739025,2449225,2178576,1929321,1700416,1490841,1299600,1129969,980100,850084,739600,649636,580644,529984,499849,490000,499849,529984,580644,649636,739600,850084,980100,1129969,1299600,1490841,1700416,1929321,2178576,2449225,2739025,1746},
{2920681,2611456,2319529,2050624,1800964,1570009,1359556,1170724,1000000,850084,720801,609961,519841,450241,399424,369664,360000,369664,399424,450241,519841,609961,720801,850084,1000000,1170724,1359556,1570009,1800964,2050624,2319529,2611456,1709},
{2808976,2499561,2211169,1940449,1690000,1459264,1249924,1060900,889249,739600,609961,499849,409600,339889,290521,260100,250000,260100,290521,339889,409600,499849,609961,739600,889249,1060900,1249924,1459264,1690000,1940449,2211169,2499561,1676},
{2719201,2408704,2119936,1849600,1600225,1368900,1159929,970225,799236,649636,519841,409600,320356,250000,199809,169744,160000,169744,199809,250000,320356,409600,519841,649636,799236,970225,1159929,1368900,1600225,1849600,2119936,2408704,1649},
{2650384,2340900,2050624,1779556,1530169,1299600,1089936,900601,729316,580644,450241,339889,250000,179776,130321,99856,90000,99856,130321,179776,250000,339889,450241,580644,729316,900601,1089936,1299600,1530169,1779556,2050624,2340900,1628},
{2598544,2289169,1999396,1729225,1481089,1249924,1040400,850084,680625,529984,399424,290521,199809,130321,80089,50176,40000,50176,80089,130321,199809,290521,399424,529984,680625,850084,1040400,1249924,1481089,1729225,1999396,2289169,1612},
{2569609,2259009,1971216,1700416,1449616,1221025,1010025,820836,649636,499849,369664,260100,169744,99856,50176,19881,10000,19881,50176,99856,169744,260100,369664,499849,649636,820836,1010025,1221025,1449616,1700416,1971216,2259009,1603},
{2560000,2250000,1960000,1690000,1440000,1210000,1000000,810000,640000,490000,360000,250000,160000,90000,40000,10000,0,10000,40000,90000,160000,250000,360000,490000,640000,810000,1000000,1210000,1440000,1690000,1960000,2250000,1600},
{2569609,2259009,1971216,1700416,1449616,1221025,1010025,820836,649636,499849,369664,260100,169744,99856,50176,19881,10000,19881,50176,99856,169744,260100,369664,499849,649636,820836,1010025,1221025,1449616,1700416,1971216,2259009,1603},
{2598544,2289169,1999396,1729225,1481089,1249924,1040400,850084,680625,529984,399424,290521,199809,130321,80089,50176,40000,50176,80089,130321,199809,290521,399424,529984,680625,850084,1040400,1249924,1481089,1729225,1999396,2289169,1612},
{2650384,2340900,2050624,1779556,1530169,1299600,1089936,900601,729316,580644,450241,339889,250000,179776,130321,99856,90000,99856,130321,179776,250000,339889,450241,580644,729316,900601,1089936,1299600,1530169,1779556,2050624,2340900,1628},
{2719201,2408704,2119936,1849600,1600225,1368900,1159929,970225,799236,649636,519841,409600,320356,250000,199809,169744,160000,169744,199809,250000,320356,409600,519841,649636,799236,970225,1159929,1368900,1600225,1849600,2119936,2408704,1649},
{2808976,2499561,2211169,1940449,1690000,1459264,1249924,1060900,889249,739600,609961,499849,409600,339889,290521,260100,250000,260100,290521,339889,409600,499849,609961,739600,889249,1060900,1249924,1459264,1690000,1940449,2211169,2499561,1676},
{2920681,2611456,2319529,2050624,1800964,1570009,1359556,1170724,1000000,850084,720801,609961,519841,450241,399424,369664,360000,369664,399424,450241,519841,609961,720801,850084,1000000,1170724,1359556,1570009,1800964,2050624,2319529,2611456,1709},
{3048516,2739025,2449225,2178576,1929321,1700416,1490841,1299600,1129969,980100,850084,739600,649636,580644,529984,499849,490000,499849,529984,580644,649636,739600,850084,980100,1129969,1299600,1490841,1700416,1929321,2178576,2449225,2739025,1746},
{3200521,2890000,2598544,2328676,2079364,1849600,1640961,1449616,1279161,1129969,1000000,889249,799236,729316,680625,649636,640000,649636,680625,729316,799236,889249,1000000,1129969,1279161,1449616,1640961,1849600,2079364,2328676,2598544,2890000,1789},
{3370896,3059001,2768896,2499561,2250000,2019241,1809025,1620529,1449616,1299600,1170724,1060900,970225,900601,850084,820836,810000,820836,850084,900601,970225,1060900,1170724,1299600,1449616,1620529,1809025,2019241,2250000,2499561,2768896,3059001,1836},
{3560769,3250809,2958400,2689600,2439844,2211169,1999396,1809025,1640961,1490841,1359556,1249924,1159929,1089936,1040400,1010025,1000000,1010025,1040400,1089936,1159929,1249924,1359556,1490841,1640961,1809025,1999396,2211169,2439844,2689600,2958400,3250809,1887},
{3771364,3459600,3168400,2900209,2650384,2421136,2211169,2019241,1849600,1700416,1570009,1459264,1368900,1299600,1249924,1221025,1210000,1221025,1249924,1299600,1368900,1459264,1570009,1700416,1849600,2019241,2211169,2421136,2650384,2900209,3168400,3459600,1942},
{4000000,3690241,3400336,3129361,2879809,2650384,2439844,2250000,2079364,1929321,1800964,1690000,1600225,1530169,1481089,1449616,1440000,1449616,1481089,1530169,1600225,1690000,1800964,1929321,2079364,2250000,2439844,2650384,2879809,3129361,3400336,3690241,2000},
{4251844,3940225,3648100,3378244,3129361,2900209,2689600,2499561,2328676,2178576,2050624,1940449,1849600,1779556,1729225,1700416,1690000,1700416,1729225,1779556,1849600,1940449,2050624,2178576,2328676,2499561,2689600,2900209,3129361,3378244,3648100,3940225,2062},
{4519876,4210704,3920400,3648100,3400336,3168400,2958400,2768896,2598544,2449225,2319529,2211169,2119936,2050624,1999396,1971216,1960000,1971216,1999396,2050624,2119936,2211169,2319529,2449225,2598544,2768896,2958400,3168400,3400336,3648100,3920400,4210704,2126},
{4809249,4498641,4210704,3940225,3690241,3459600,3250809,3059001,2890000,2739025,2611456,2499561,2408704,2340900,2289169,2259009,2250000,2259009,2289169,2340900,2408704,2499561,2611456,2739025,2890000,3059001,3250809,3459600,3690241,3940225,4210704,4498641,2193},
{5121169,4809249,4519876,4251844,4000000,3771364,3560769,3370896,3200521,3048516,2920681,2808976,2719201,2650384,2598544,2569609,2560000,2569609,2598544,2650384,2719201,2808976,2920681,3048516,3200521,3370896,3560769,3771364,4000000,4251844,4519876,4809249,2263}
} ;//in program memory space








float VObX[3];	// relative position of obstacles
float VObY[3];

tPID DistPIDstruct;
fractional DistabcCoefficient[3] 
	__attribute__ ((section (".xbss, bss, xmemory")));
fractional DistcontrolHistory[3] 
	__attribute__ ((section (".ybss, bss, ymemory")));
fractional DistKCoeffs[] = {0,0,0};
#define DIST_KP DistKCoeffs[0]						// K parameters
#define DIST_KI DistKCoeffs[1]
#define DIST_KD DistKCoeffs[2]
#define DIST_PID_DES DistPIDstruct.controlReference	// desired distance
#define DIST_PID_MES DistPIDstruct.measuredOutput	// measured distance
#define DIST_PID_OUT DistPIDstruct.controlOutput	// PID output

#define DIST_ENABLE_FLAG VARbits1.bit14			// compute distance if enabled
float PosXdes = 0;		// desired X position coordinate
float PosYdes = 0;		// desired Y position coordinate
#define MIN_GOAL_DIST 450	// if less then decrease speed proportionally
float VelDecr;	// multiplied by VelDesM correct the speed [24d]

#define DIST_OK_FLAG VARbits1.bit11	// target distance reached
#define MIN_DIST_ERR 3				// acceptable distance error in mm

// slower cycles setting [13]	
volatile unsigned char Cycle1;
volatile unsigned char Cycle2;
#define CYCLE1_FLAG VOLbits1.bit2
#define CYCLE2_FLAG VOLbits1.bit3
#define	CICLE1_TMO 10		// starts first cycle every Xms
#define	CICLE2_TMO 5		// starts second cycle every N first cycle times

// Idle time [25]
unsigned long IdleCount = 0;	// how many times it executes main loop
volatile unsigned long IdleSample=0;//sampling time to compute idle percentage
#define IDLE_CYCLE 20	
// LOOP_TIME = CICLE1_TMO * CICLE2_TMO	= 50ms	
// so -> IDLE_SAMPLE_TIME = LOOP_TIME * IDLE_CYCLE = 1000ms 
// in order to have percentage as int IDLE_SAMPLE_TIME * 10,000
#define IDLE_SAMPLE_TIME 10000000
// time in ns to execute main without any call
// measured with stopwatch in SIM [25a]
#define IDLE_TIME_PERIOD 3000 	// without optimizations
// #define IDLE_TIME_PERIOD 2225 	// with optimizations 3
// #define IDLE_TIME_PERIOD 2525 	// with optimizations s

int IdlePerc;				  // idle percentage

volatile unsigned int BlinkPeriod ;	// LED1 blinking period (ms)
unsigned int BlinkOn;		// LED1 on time (ms)
#define NORM_BLINK_PER 1000	// Period for OK condition
#define NORM_BLINK_ON   200	// ON TIME for OK condition
#define K_ERR_BLINK_PER 4000// Period for ERR condition
#define K_ERR_BLINK_ON 2000	// ON TIME for ERR condition


int ErrCode;				// Error Code

#define CONSOLE_DEBUG VARbits1.bit13 // [30]

int VelInt[2];			// speed in mm/s as an integer

// Scheduler
unsigned char SchedPtr = 0;	// point to current step into the sequence
#define SCHED_ANGLE_FLAG VARbits1.bit8	// wait for target angle
#define SCHED_DIST_FLAG VARbits1.bit0	// wait for target distance

/*
// sequence array initialized for a 360° turn cw in 4 steps
int SchedValues[16][4]= {		{6,100,0,0},
								{3,0,90,0},
								{6,50,0,0},
								{3,0,180,0},
								{6,50,0,0},
								{3,0,270,0},
								{6,50,0,0},
								{3,0,0,0},
								{6,50,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0} };
*/
/*
// sequence array initialized for a 360° turn ccw in 4 steps
int SchedValues[16][4]= {		{6,100,0,0},
								{3,0,270,0},
								{6,50,0,0},
								{3,0,180,0},
								{6,50,0,0},
								{3,0,90,0},
								{6,50,0,0},
								{3,0,0,0},
								{6,50,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0} };
*/																

// sequence array initialized for a mini UMBmark of 1m x 1m CW
int SchedValues[16][4]= {		{6,100,0,0},
								{4,0,0,1000},
								{5,300,0,1000},
								{4,0,1000,1000},
								{5,300,1000,1000},
								{4,0,1000,0},
								{5,300,1000,0},
								{4,0,0,0},
								{5,300,0,0},
								{6,80,0,0},
								{3,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0} };	

/*
// sequence array initialized for a mini UMBmark of 2.5m x 3.5m CCW
// point the robot to 90°
int SchedValues[16][4]= {		{6,100,0,0},
								{4,0,0,2500},
								{5,300,0,2500},
								{4,0,-3500,2500},
								{5,300,-3500,2500},
								{4,0,-3500,0},
								{5,300,-3500,0},
								{4,0,0,0},
								{5,300,0,0},
								{6,80,0,0},
								{3,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0} };
*/						
/*								
// sequence array initialized for a maxi UMBmark of 4m forward
int SchedValues[16][4]= {		{6,100,0,0},
								{4,0,0,4000},
								{5,300,0,4000},
								{4,0,4000,4000},
								{5,300,4000,4000},
								{4,0,4000,0},
								{5,300,4000,0},
								{4,0,0,0},
								{5,300,0,0},
								{3,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0} };
*/								
/*
// sequence array initialized for a UMBmark of 4m reward
int SchedValues[16][4]= {		{6,100,0,0},
								{4,0,4000,0},
								{5,300,4000,0},
								{4,0,4000,4000},
								{5,300,4000,4000},
								{4,0,0,4000},
								{5,300,0,4000},
								{4,0,0,0},
								{5,300,0,0},
								{6,80,0,0},
								{3,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0} };
*/
/*
// sequence array initialized for a 10m straight forward
int SchedValues[16][4]= {		{6,50,0,0},
								{5,300,0,10000},
								{3,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0} };
*/
/*
// sequence array initialized for a RTC 10m diagonal
int SchedValues[16][4]= {		{6,100,0,0},
								{4,0,7200,7200},
								{5,300,7200,7200},
								{4,0,0,0},
								{5,300,0,0},
								{3,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0},
								{0,0,0,0} };
*/

volatile int RtTimer;				// real time delay using T1 timer
#define RT_TIMER_FLAG VARbits1.bit10	 // real time timer enable
#define TIMER_OK_FLAG VARbits1.bit9	 // time elapsed 
#define SCHEDULER_FLAG VARbits1.bit5 // enable scheduler [32a]

// dist in cm from objects on the three sides
unsigned int Obj[3] = {0x1000, 0x1000, 0x1000};
unsigned char Target[3] = {0, 0, 0};//highest sensor value on the three sides
volatile unsigned int RndTimer=0;// Timer for randomly avoid central obstacles
#define RND_TIMEOUT 100		// 100 * 50ms = 5 seconds
#define RND_FLAG VARbits1.bit4 // randomly decide in which direction to turn

// simulated EEPROM data addresses [33]
#define EE_KVEL1_H 0
#define EE_KVEL1_L 1
#define EE_KVEL2_H 2
#define EE_KVEL2_L 3
#define EE_CHK_KVEL 4

#define EE_ANGLE_KP 5	
#define EE_ANGLE_KI 6		
#define EE_ANGLE_KD 7
#define EE_CHK_ANGLE 8

#define EE_KP1 9
#define EE_KI1 10
#define EE_KD1 11
#define EE_KP2 12
#define EE_KI2 13
#define EE_KD2 14
#define EE_CHK_SPEED 15

#define EE_KSP1_H 16
#define EE_KSP1_L 17
#define EE_KSP2_H 18
#define EE_KSP2_L 19
#define EE_AXLE_H 20
#define EE_AXLE_L 21
#define EE_CHK_MECH 22

#define EE_DIST_KP 23	
#define EE_DIST_KI 24		
#define EE_DIST_KD 25
#define EE_CHK_DIST 26

#define EE_CHK_SCHED 27
#define EE_SCHED 28	// starting address for 128 locations

unsigned char ResetCount = 0;	// [28]

// VARbits2.bit0 and up available
// VOLbits1.bit9 and up available

/* Definitions end ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

