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
#define  N_TASKS                        18       /* Number of identical tasks                          */
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
char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */

OS_EVENT      *displayQue;
OS_EVENT      *Sem;
OS_EVENT      *Que;                                 /* Kolejka                                       */
OS_EVENT      *Box;                              /* MBox wysylajaca strukture                     */
OS_EVENT      *edMbox;


INT32U semVal = 0;
char taskNumbers[15] = {0};
INT8U mboxCount = 0;

void          *editMsg[100];
void          *CommMsg[100];

struct queBuff										  
{
    int who;
    char tasknr;
	INT32U load;
	INT32U counter;
    char buffor[BUFFOR_ROZMIAR]; 
    
};
/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  SemTask(void *data);                       /* Function prototypes of tasks                  */
        void  QueTask(void *data); 
        void  BoxTask(void *data); 
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

    edMbox = OSMboxCreate(NULL);
    displayQue = OSQCreate(&editMsg[0],queSize);  
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

    for(i = 0; i<5; i++)
    {
        taskNumbers[i]=i+1;  
    }

    //tasks for read numbers  
	OSTaskCreate(TaskKlawiatura, 0, &TaskStk[0][TASK_STK_SIZE - 1], 1); //Read from keyboard
	OSTaskCreate(TaskEdycja, 0, &TaskStk[1][TASK_STK_SIZE - 1], 2); //Edits buffor
	OSTaskCreate(TaskWyswietlanie, 0, &TaskStk[2][TASK_STK_SIZE - 1], 3); //Display on screen 

    for(i=3; i<8; i++)
    {
        OSTaskCreate(QueTask, &taskNumbers[i-3], &TaskStk[i][TASK_STK_SIZE - 1], i+1); 
        OSTaskCreate(BoxTask, &taskNumbers[i-3], &TaskStk[i+5][TASK_STK_SIZE - 1], i+6);
        OSTaskCreate(SemTask, &taskNumbers[i-3], &TaskStk[i+10][TASK_STK_SIZE - 1], i+11);
    }
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/
void  QueTask (void *pdata)
{
    INT8U doSomething = 0;
    INT32U *queMessPtr = 0;
    INT32U i = 0;
    INT8U  err;
    struct queBuff dis[1];
    dis->tasknr = *(char*)pdata+5 ; 
    
    dis->counter = 0; 
    dis->load = 0;
    dis->who = 2;


    for (;;) {
        dis->counter++;
        PC_DispStr(72,dis->tasknr+1,"WORK",DISP_FGND_BLACK + DISP_BGND_RED);
        OS_ENTER_CRITICAL();
        queMessPtr =  (INT32U *)OSQAccept(Que);
        if(queMessPtr)
        {                                            
            dis->load = *queMessPtr;                                                                                   
        }
        OS_EXIT_CRITICAL();
        for(i = 0; i < dis->load; i++)
        { 
            doSomething = 1;
        }

        OSMboxPost(edMbox,dis);
        PC_DispStr(72,dis->tasknr+1,"DONE",DISP_FGND_BLACK + DISP_BGND_GREEN);
        OSTimeDly(1);                                                    
    }
} 



void  BoxTask (void *pdata)
{
    INT8U doSomething = 0;
    INT32U *boxMessPtr = 0;
    INT32U i = 0;
    INT8U  err;
    struct queBuff dis[1];
    dis->tasknr = *(char*)pdata+10 ; 
    
    dis->counter = 0; 
    dis->load = 0;
    dis->who = 2;


    for (;;) {
        dis->counter++;
        PC_DispStr(72,dis->tasknr+1,"WORK",DISP_FGND_BLACK + DISP_BGND_RED);
        OS_ENTER_CRITICAL();
        boxMessPtr =  (INT32U *)OSMboxAccept(Box);
       
        if(boxMessPtr)
        {                                            
            dis->load = *boxMessPtr; 
            mboxCount--;   
            OSMboxPost(Box,boxMessPtr);                                                                               
        }
        OS_EXIT_CRITICAL();
        for(i = 0; i < dis->load; i++)
        { 
            doSomething = 1;
        }

        OSMboxPost(edMbox,dis);
        PC_DispStr(72,dis->tasknr+1,"DONE",DISP_FGND_BLACK + DISP_BGND_GREEN);
        OSTimeDly(1);                                                    
    }
} 

void  SemTask (void *pdata)
{
    INT8U doSomething = 0;
    INT32U i = 0;
    INT8U  err;
    struct queBuff dis[1];
    dis->tasknr = *(char*)pdata + 15; 
    
    dis->counter = 0; 
    dis->load = 0;
    dis->who = 2;


    for (;;) {
        
        dis->counter++;
        PC_DispStr(72,dis->tasknr+1,"WORK",DISP_FGND_BLACK + DISP_BGND_RED);
        if(OSSemAccept(Sem))
        {                                            
            dis->load = semVal;                                
            OSSemPost(Sem);                                                   
        }
            
        for(i = 0; i < dis->load; i++)
        { 
            doSomething = 1;
        }

        OSMboxPost(edMbox,dis);
        PC_DispStr(72,dis->tasknr+1,"DONE",DISP_FGND_BLACK + DISP_BGND_GREEN);
        OSTimeDly(1);                                                    
    }
} 

void TaskKlawiatura(void *pdata)
{
	INT16S Klaw;					// buffor na klawisze 
	pdata = pdata;								
	
	for(;;)
	{
		if(PC_GetKey(&Klaw) == TRUE)
        {			
			OSQPost(displayQue,&Klaw);
		}
		OSTimeDly(1);										//opoznienie 1 cykl zegara 
	}
}

void TaskEdycja(void *pdata)
{
	struct queBuff displayBuffor[1];
	int BufforPozycja = 0;
    INT32U  ValBuff = 0;
	INT16S *Klaw = 0;
	char up,down;
    INT8U i;
    INT8U err;
	pdata = pdata;
	
    displayBuffor->who = 1;

	// czyścimy 1 linijke 
	for(BufforPozycja = 0; BufforPozycja<BUFFOR_ROZMIAR; BufforPozycja++)
	{
        displayBuffor->buffor[BufforPozycja] = 0;
	}

	BufforPozycja = 0;
	
	for(;;)
	{ 
		Klaw = (INT16S *)OSQPend(displayQue,0,&err);
		down = (char)*Klaw;

		switch(down)
		{
			case 0x0D:			//enter  
                ValBuff = strtoul( displayBuffor->buffor,NULL,10);
                // insert into global variable
                OSSemPend(Sem,0,&err);
                semVal = ValBuff;
                OSSemPost(Sem);
                //  insert into Que
                for (i = 0; i<5; i++)	
			    {
				    OSQPost(Que,&ValBuff);
			    }
                mboxCount = 5;
                OSMboxPost(Box, &ValBuff);
			break;
			
			case 0x1B:			//esc
				PC_DOSReturn();
			break;
				
			case 0x08:			//backspace
				if(BufforPozycja>0)
				{  
					displayBuffor->buffor[BufforPozycja-1] = 0;
					BufforPozycja--;
				}
			break;
				
			case 0x53:	//del
				BufforPozycja = 0;
				while(BufforPozycja<BUFFOR_ROZMIAR)
				{
					displayBuffor->buffor[BufforPozycja] = 0;
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
		OSMboxPost(edMbox,displayBuffor);
        
	}
}

void TaskWyswietlanie(void *pdata)
{
	struct queBuff *procesStruct = 0;
    INT8U err;
    char strBufforLo[33] = {0};
    char strBufforCn[33] = {0};
    char clear[64] = "                                                               \0";
	pdata = pdata;
							
	for(;;)
	{
		procesStruct = (struct queBuff*)OSMboxPend(edMbox, 0,&err);
        if(procesStruct->who == 1)
        {
            PC_DispClrRow(0, DISP_BGND_LIGHT_GRAY);
	        PC_DispStr(0,0,procesStruct->buffor, DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);	
        }
        else if(procesStruct->who == 2)
        {
            ultoa(procesStruct->counter,strBufforCn, 10 );
            ultoa(procesStruct->load,strBufforLo, 10 );
            PC_DispStr(9,procesStruct->tasknr+1,clear,DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
            PC_DispStr(9,procesStruct->tasknr+1,strBufforLo,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
            PC_DispStr(41,procesStruct->tasknr+1,strBufforCn,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
        }
        else
        {
            PC_DispStr(0,0,(char*)"EROOR", DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);	
        }
	}
}