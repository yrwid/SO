/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        10       /* Number of identical tasks                          */
#define  BUFFOR_ROZMIAR					30		 /*ROZMIAR BUFORA DLA ZNAK�W*/
#define  queSize				        100		 /*ROZMIAR KOLEJKI*/
//#define  LINIE_ILOSC					4		 /*	ILOSC LINII WYSWIETLANYCH*/
/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
OS_STK        TaskKlawiaturaStk[TASK_STK_SIZE]; 	//Klawiatura
OS_STK        TaskEdycjaStk[TASK_STK_SIZE];			//Edycja
OS_STK        TaskWyswietlanieStk[TASK_STK_SIZE];	//Wyswietlanie

char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */

OS_EVENT      *Sem;
OS_EVENT      *Que;                                 /* Kolejka                                       */
OS_EVENT      *Box;                              /* MBox wysylajaca strukture                     */

void          *CommMsg[10];

struct MboxBuff										  //struktura kolejki
{
  char			buffor[BUFFOR_ROZMIAR]; 
  int			X;                                  
  int			Y;
};


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task(void *data);                       /* Function prototypes of tasks                  */
        void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);
		
		void TaskEdycja(void *data);
		void TaskKlawiatura(void *data);
		void TaskWyswietlanie(void * data);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */
    
    Que = OSQCreate(&CommMsg[0],queSize);  
    Box = OSMboxCreate(NULL);
    Sem = OSSemCreate(1);                          
	
    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    char       s[100];
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Initialize the display                   */

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    TaskStartCreateTasks();                                /* Create all the application tasks         */

    for (;;) {
        TaskStartDisp();                                  /* Update the display                       */


        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}


/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDispInit (void)
{
    PC_DispStr( 0,  0, "                         uC/OS-II, The Real-Time Kernel                         ", DISP_FGND_WHITE + DISP_BGND_RED);
    PC_DispStr( 0,  1, "                             Dawid & Karol                                      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_BLUE);
    PC_DispStr( 0,  5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "NR  Type Load                           Counter                    Val  State   ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);    
    PC_DispStr( 0,  7, "01     Q                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  8, "02     Q                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  9, "03     Q                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 10, "04     Q                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 11, "05     Q                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 12, "06     M                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 13, "07     M                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 14, "08     M                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 15, "09     M                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 16, "10     M                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 17, "11     S                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 18, "12     S                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 19, "13     S                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 20, "14     S                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 21, "15     S                                                                        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_WHITE + DISP_BGND_BLUE);
    PC_DispStr( 0, 23, "#Task switch/sec:                                                               ", DISP_FGND_WHITE + DISP_BGND_BLUE);
    PC_DispStr( 0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_WHITE + DISP_BGND_RED);
}

/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDisp (void)
{
    char   s[80];


    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

#if OS_TASK_STAT_EN > 0
    sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
    PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
#endif

    sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "V%1d.%02d", OSVersion() / 100, OSVersion() % 100); /* Display uC/OS-II's version number    */
    PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    switch (_8087) {                                               /* Display whether FPU present          */
        case 0:
             PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 1:
             PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 2:
             PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 3:
             PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
    INT8U  i;

    //tasks for read numbers  
	OSTaskCreate(TaskKlawiatura, 0, &TaskKlawiaturaStk[TASK_STK_SIZE - 1], 1); //Read from keyboard
	OSTaskCreate(TaskEdycja, 0, &TaskEdycjaStk[TASK_STK_SIZE - 1], 2); //Edits buffor
	OSTaskCreate(TaskWyswietlanie, 0, &TaskWyswietlanieStk[TASK_STK_SIZE - 1], 3); //Display on screen 
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/
/*
void  Task (void *pdata)
{
    INT8U  x;
    INT8U  y;
    INT8U  err;


    for (;;) {
        OSSemPend(RandomSem, 0, &err);              Acquire semaphore to perform random numbers        
        x = random(80);                          /* Find X position where task number will appear      
        y = random(16);                          /* Find Y position where task number will appear      
        OSSemPost(RandomSem);                    /* Release semaphore                                  
                                                 /* Display the task number on the screen              
        PC_DispChar(x, y + 5, *(char *)pdata, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        OSTimeDly(1);                            /* Delay 1 clock tick                                 
    }
} */

void TaskKlawiatura(void *pdata)
{
	INT16S Klaw;					// buffor na klawisze 
	
	pdata = pdata;								
	
	for(;;)
	{
		if(PC_GetKey(&Klaw) == TRUE)
        {			
			OSMboxPost(Box, &Klaw);
		}
		OSTimeDly(1);										//opoznienie 1 cykl zegara 
	}
}

void TaskEdycja(void *pdata)
{
	struct MboxBuff displayBuffor[1];
	int BufforPozycja = 0;
	INT16S *Klaw = 0;
	char up,down;
	INT8U err;
	pdata = pdata;
	
	// czyścimy 1 linijke 
	for(BufforPozycja = 0; BufforPozycja<BUFFOR_ROZMIAR; BufforPozycja++)
	{
        displayBuffor->buffor[BufforPozycja] = 0x20;
		//buffor[linia].buffor[BufforPozycja] = 0x20;
	}
	displayBuffor->X = 0;
	displayBuffor->Y = 0;
	BufforPozycja = 0;
	
	for(;;)
	{   
		 Klaw = (INT16S *)OSMboxPend(Box,0,&err);
		 down = (char)*Klaw;

	//	 PC_DispChar(5, 24, up , DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		switch(down)
		{
			case 0x0D:			//enter
			    // Ack load for tasks
				break;
			
			case 0x1B:			//esc
				PC_DOSReturn();
				break;
				
			case 0x08:			//backspace
				if(BufforPozycja>0)
				{  
					displayBuffor->buffor[BufforPozycja-1] = 0x20;
					BufforPozycja--;
				}
				break;
				
			case 0x53:	//del
				BufforPozycja = 0;
				while(BufforPozycja<BUFFOR_ROZMIAR)
				{
					displayBuffor->buffor[BufforPozycja] = 0x20;
					BufforPozycja++;
				}
				BufforPozycja = 0;
                break;

			default:
				if(BufforPozycja<BUFFOR_ROZMIAR)
			    {
					displayBuffor->buffor[BufforPozycja] = down;
					BufforPozycja++;
				}
				break;
		}	
		OSQPost(Que, (void*)displayBuffor);
	}
}

void TaskWyswietlanie(void *pdata)
{
	struct MboxBuff *buffor2 = 0;
	//INT8U err;
	//INT16S key;
	pdata = pdata;
							
	for(;;)
	{
		buffor2 = (struct MboxBuff*)OSQPend(Que, 0,0);
		PC_DispClrRow(buffor2->Y, DISP_BGND_LIGHT_GRAY);
	    PC_DispStr(buffor2->X,buffor2->Y,buffor2->buffor, DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);	
	}
}