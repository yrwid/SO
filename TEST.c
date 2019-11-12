/**
 ********************************************************************************************************
 * @file    TEST.c
 * @author  Dawid Mudry & Karol Marciniak
 * @date    11.11.19
 * @brief   System overloading program for Operating systems laboratory uC/OS-II. 
 ********************************************************************************************************
 */


/*
*********************************************************************************************************
*                                               INCLUDES
*********************************************************************************************************
*/
#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                  512           /* Size of each task's stacks (# of WORDs)       */
#define  N_TASKS                        18            /* Number of identical tasks                     */
#define  BUF_SIZE					    30		      /* Buffor size for command line                  */
#define  queSize				        100		      /* Size od queue                                 */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];            /* Start task stack                              */
OS_STK        WDTTaskStk[TASK_STK_SIZE];              /* WDT task stack                                */


OS_EVENT      *displayQue;                            /* Pointer to display Queue                      */
OS_EVENT      *Sem;                                   /* Pointer to Semaphore using by SemTasks        */
OS_EVENT      *Que;                                   /* Pointer to Queue using  by QueTasks           */
OS_EVENT      *Box;                                   /* Pointer to Mbox using by BoxTasks             */
OS_EVENT      *edMbox;                                /* Pointer to Mbox using by keyboard/edit tasks  */
OS_EVENT      *WDTsem;                                /* Pointer ro Semaphore using by WDT/Sem/Que/Box */


INT32U semVal = 10;                                   /* Variablec protected by Sem semaphore          */
char taskNumbers[5] = {0};                            /* Variables thats are pass to tasks on create   */
INT32U WDTcheck[2][15] = {0};                         /* Matrix thats store Tasks informations protected by WDT sem */
INT8U mboxCount = 0;                                  /* Variable for Mbox message propagation         */

void          *editMsg[100];                          /* Place for queue pointers (displayQue)         */ 
void          *CommMsg[100];                          /* Place for queue pointers (Que)                */

struct queBuff							              /* Struct using by all tasks to communicate with displayTask  */			  
{
    int who;                                          /* Who sended message                            */ 
    char tasknr;                                      /* Taks Number 1-15                              */
	INT32U load;                                      /* Actual load dor loop                          */
	INT32U counter;                                   /* Task actual number of enters                  */
    char buffor[BUF_SIZE];                            /* Matrix used by editTask to save cmd buffor    */      
    
};


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  SemTask(void *data);                    /* Function prototypes of Semaphore task         */
        void  QueTask(void *data);                    /* Function prototypes of Queue task             */
        void  BoxTask(void *data);                    /* Function prototypes of Mail box task          */
        void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
        void  WDTTask(void *data);                    /* Function prototypes of WDT task               */   
static  void  TaskStartCreateTasks(void);             /* Function prototypes of Start Create tasks     */
static  void  TaskStartDispInit(void);                /* Function prototypes of Init display           */   
static  void  TaskStartDisp(void);                    /* Function prototypes of Start display          */
		
		void editTask(void *data);                    /* Function prototypes of Edit task              */   
		void keyboardTask(void *data);                /* Function prototypes of Keyboard task          */
		void displayTask(void * data);                /* Function prototypes of Display task           */


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

    // create semaphores, queues and mailbox 
    edMbox = OSMboxCreate(NULL);
    displayQue = OSQCreate(&editMsg[0],queSize);  
    Que = OSQCreate(&CommMsg[0],queSize);  
    Box = OSMboxCreate(NULL);
    Sem = OSSemCreate(1);    
    WDTsem = OSSemCreate(1);    

    // start parents task 
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
    PC_DispStr( 0,  1, "                            Dawid Mudry                                         ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                            Karol Marciniak                                     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_BLUE);
    PC_DispStr( 0,  5, "                                 WDT Alarm                                      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "NR  Type Load               Delta/Sek    Counter                Val     State   ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);    
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
        taskNumbers[i]=i+1;         //Create numbers for tasks 
    }

    //Create tasks for read numbers  and WDT  task
	OSTaskCreate(keyboardTask, 0, &TaskStk[0][TASK_STK_SIZE - 1], 1);       // Read from keyboard
	OSTaskCreate(editTask, 0, &TaskStk[1][TASK_STK_SIZE - 1], 2);           // Edits buffor
	OSTaskCreate(displayTask, 0, &TaskStk[2][TASK_STK_SIZE - 1], 3);        // Display on screen
    OSTaskCreate(WDTTask,0,&WDTTaskStk[TASK_STK_SIZE-1],4);                 // WDT task 

    for(i=3; i<8; i++)
    {
        OSTaskCreate(QueTask, &taskNumbers[i-3], &TaskStk[i][TASK_STK_SIZE - 1], i+2);       // Queue tasks
        OSTaskCreate(BoxTask, &taskNumbers[i-3], &TaskStk[i+5][TASK_STK_SIZE - 1], i+7);     // MBox tasks
        OSTaskCreate(SemTask, &taskNumbers[i-3], &TaskStk[i+10][TASK_STK_SIZE - 1], i+12);   // Semaphore tasks
    }
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

/**
 * @brief   Task overwatchs whether system is overload, checking if task got proper load value and calculate delta/sek
 */
void WDTTask(void *pdata)
{
    INT32U WDTcheckInside[2][15] = {0};                // WDT compare matrix 
    INT8U i;                                           // iterator 
    INT32U repairVal = 10;                             // When task detect overload set this value as new load
    INT8U WDTReset = 0;                                // Variable that indicates if alarm occured 
    char strBufforDelta[33] = {0};                     // Matrix to store delta/sek on display
    INT32U deltaCount[15] = {0};                       // Matrix to store delta/sek for each task 
    char clear[64] = "      \0";                       // clear string


    OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait five second                          */
    OSSemPend(WDTsem,0,0);                             // Pend WDT sem to read values from global variables 
    for(i = 0; i<15; i++)                              // initialize WDT task 
    {                                   
        WDTcheckInside[0][i] = WDTcheck[0][i];         
        WDTcheckInside[1][i] = WDTcheck[1][i];       
    }
    OSSemPost(WDTsem);
    PC_DispStr(46,5,"--OK--",DISP_FGND_BLACK + DISP_BGND_GREEN);            // Display WDT info aboyt alarm 
    for(;;)
    { 
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
        OSSemPend(WDTsem,0,0);                             // Pend WDT sem to read values from global variables 
        for(i = 0; i<15; i++)
        {
            deltaCount[i] = WDTcheck[0][i] - WDTcheckInside[0][i];                           // Calculate delta count 
            WDTcheckInside[0][i]  = WDTcheck[0][i];                                          // Save previously value 
            ultoa(deltaCount[i],strBufforDelta, 10 );                                        // uLong to char array 
            PC_DispStr(30,i+7,clear,DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);                 // Cleat 
            PC_DispStr(30,i+7,strBufforDelta,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);          // Display Delta/sek for each task 
        }
        

        for(i = 0; i<15; i++)                              // Looking if load was proper delivered to tasks 
        {
            OSSemPend(Sem,0,0);                            // Obtain semaphore to read global load value
            if(semVal == WDTcheck[1][i])                   // check if global varaible is equal to task inside load 
            {
                PC_DispStr(65 ,i+7,"OK",DISP_FGND_BLACK + DISP_BGND_GREEN);
            }
            else
            {
                PC_DispStr(65 ,i+7,"NO",DISP_FGND_BLACK + DISP_BGND_RED);
            }
            OSSemPost(Sem);
        }
        OSSemPost(WDTsem);

        if(deltaCount[2] == 0)                             // Loking for oveload, when 3 rd task is not responding reset system 
        {
            WDTReset++;
            PC_DispStr(46,5,"ALERT!",DISP_FGND_BLACK + DISP_BGND_RED);
        }
        else
        {
            WDTReset = 0;
            PC_DispStr(46,5,"--OK--",DISP_FGND_BLACK + DISP_BGND_GREEN);
        }
        if(WDTReset == 5)                                 // if 3rd task didnt respond 5 times (5 sek) enable reset procedure 
        {   
            // Insert value into global Sempahore tasks
            OSSemPend(Sem,0,0);
            semVal = 10;
            OSSemPost(Sem);

            // Clear up the que 
            while(OSQAccept(Que));

            // Insert into Que
            for (i = 0; i<5; i++)	
			{
				OSQPost(Que,&repairVal);
			}

            //clear up the mailbox 
            OSMboxAccept(Box);
            //insert into mailBox
            mboxCount = 5;
            OSMboxPost(Box, &repairVal);
            //reset WDT counter      
            WDTReset = 0;
            PC_DispStr(46,5,"--OK--",DISP_FGND_BLACK + DISP_BGND_GREEN);
        }
    }
}

/**
 * @brief   Queue task used for overload system load is provided via queue Que.
 */
void  QueTask(void *pdata)
{
    INT8U doSomething = 0;                                   // inside load loop varaible
    INT32U *queMessPtr = 0;                                  // Pointer to collect message from Que 
    INT32U i = 0;                                            // interator
    INT8U  err;                                              // error var
    struct queBuff dis[1];                                   // Struct to store all needed data 

    // initialize struct 
    dis->tasknr = *(char*)pdata+5;                          
    dis->counter = 0; 
    dis->load = 10;
    dis->who = 2;
   
    
    for (;;) {
        // Counter store informations about loop enter  from begining of program.
        dis->counter++;
        // Display information about work 
        PC_DispStr(72,dis->tasknr+1,"WORK",DISP_FGND_BLACK + DISP_BGND_RED);
        // Enter critical to avoid program crash
        OS_ENTER_CRITICAL();
        // Read from Que
        queMessPtr =  (INT32U *)OSQAccept(Que);
        if(queMessPtr)
        {   
            // if load is alredy set to current load dont read from que, let low priority task read load var         
            if(*queMessPtr == dis->load)
            {
                OSQPost(Que, queMessPtr);
            }
            else
            {
                dis->load = *queMessPtr;
            }                                 
                                                                                               
        }
        OS_EXIT_CRITICAL();

        // Load loop 
        for(i = 0; i < dis->load; i++)
        { 
            doSomething = 1;
        }

        // Report to WDT
        OSSemAccept(WDTsem);
        WDTcheck[0][(INT8U)(dis->tasknr)-6] = dis->counter;
        WDTcheck[1][(INT8U)(dis->tasknr)-6] = dis->load;
        OSSemPost(WDTsem);
        // Send info to display
        OSMboxPost(edMbox,dis);
        PC_DispStr(72,dis->tasknr+1,"DONE",DISP_FGND_BLACK + DISP_BGND_GREEN);
        OSTimeDly(1);                                                    
    }
} 


/**
 * @brief   Queue task used for overload system load is provided via Mail box Box .
 */
void  BoxTask (void *pdata)
{
    INT8U doSomething = 0;                                   // inside load loop varaible
    INT32U *boxMessPtr = 0;                                  // Pointer to collect message from MBox
    INT32U i = 0;                                            // Iterator
    INT8U  err;                                              // Error varaible 
    struct queBuff dis[1];                                   // Struct to store all needed data      

    // Initialize data struct 
    dis->tasknr = *(char*)pdata+10 ; 
    dis->counter = 0; 
    dis->load = 10;
    dis->who = 2;

    // Everything as in Que task above.
    for (;;) { 
        dis->counter++;
        PC_DispStr(72,dis->tasknr+1,"WORK",DISP_FGND_BLACK + DISP_BGND_RED);
        OS_ENTER_CRITICAL();
        boxMessPtr =  (INT32U *)OSMboxAccept(Box);
       
        if(boxMessPtr && (mboxCount > 0))
        {         
            if(*boxMessPtr == dis->load)
            {
                OSMboxPost(Box,boxMessPtr); 
            }
            else
            {
                mboxCount--;
                OSMboxPost(Box,boxMessPtr); 
            }                                                        
            dis->load = *boxMessPtr;                                                                                      
        }
        OS_EXIT_CRITICAL();
        for(i = 0; i < dis->load; i++)
        { 
            doSomething = 1;
        }
        OSSemAccept(WDTsem);
        WDTcheck[0][(INT8U)(dis->tasknr)-6] = dis->counter;
        WDTcheck[1][(INT8U)(dis->tasknr)-6] = dis->load;
        OSSemPost(WDTsem);
        OSMboxPost(edMbox,dis);
        PC_DispStr(72,dis->tasknr+1,"DONE",DISP_FGND_BLACK + DISP_BGND_GREEN);
        OSTimeDly(1);                                                    
    }
} 


/**
 * @brief    Queue task used for overload system load is provided via semaphore Sem.
 */
void  SemTask (void *pdata)
{
    INT8U doSomething = 0;                          // As above 
    INT32U i = 0;
    INT8U  err;
    struct queBuff dis[1];

    // Everything as above
    dis->tasknr = *(char*)pdata + 15; 
    dis->counter = 0; 
    dis->load = 10;
    dis->who = 2;


    for (;;) {
        
        dis->counter++;
        PC_DispStr(72,dis->tasknr+1,"WORK",DISP_FGND_BLACK + DISP_BGND_RED);
        // Try to obtain semaphore Sem to read load value  
        if(OSSemAccept(Sem))
        {                                            
            dis->load = semVal;                                
            OSSemPost(Sem);                                                   
        }
            
        for(i = 0; i < dis->load; i++)
        { 
            doSomething = 1;
        }
        OSSemAccept(WDTsem);
        WDTcheck[0][(INT8U)(dis->tasknr)-6] = dis->counter;
        WDTcheck[1][(INT8U)(dis->tasknr)-6] = dis->load;
        OSSemPost(WDTsem);

        OSMboxPost(edMbox,dis);
        PC_DispStr(72,dis->tasknr+1,"DONE",DISP_FGND_BLACK + DISP_BGND_GREEN);
        OSTimeDly(1);                                                    
    }
} 


/**
 * @brief   Task for reading key from keyboard and pass them throught to editTask .
 */
void keyboardTask(void *pdata)
{
	INT16S Key;					// buffor na klawisze 
	pdata = pdata;		            // Prevent compilator warnings 						
	
	for(;;)
	{
		if(PC_GetKey(&Key) == TRUE)                         // Looking if key is pressed 
        {			
			OSQPost(displayQue,&Key);                       // Pass varaible to displaTask via Queue displayQue
		}
		OSTimeDly(6);										// Delay og 6 ticks because keyboard wont work faster 
	}
}


/**
 * @brief   Task for control and take decisions about display.
 */
void editTask(void *pdata)
{
	struct queBuff displayBuffor[1];                        // Placehold to store informations about dispay buffor of chars  
	int BufforPozycja = 0;                                  // Indicator where is buffor indicator
    INT32U  ValBuff = 0;                                    // Variable used to save return from ultoa() function
	INT16S *Klaw = 0;                                       // Pointer for key 
	char down;                                              // downside value of INT16U
    INT8U i;                                                // Iterator
    INT8U err;                                              // Error
	pdata = pdata;                                          // Prevent warnings
	
    displayBuffor->who = 1;                                 // Who send data? editTask in this case 

	// Clear up first line if any key was pressed 
	for(BufforPozycja = 0; BufforPozycja<BUF_SIZE; BufforPozycja++)
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
                //clear up the que 
                while(OSQAccept(Que));
                //  insert into Que
                for (i = 0; i<5; i++)	
			    {
				    OSQPost(Que,&ValBuff);
			    }
                //clear up the mailbox 
                OSMboxAccept(Box);
                //insert into mailBox
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
                // Clear up buffor until all buffor will be clear
				while(BufforPozycja<BUF_SIZE)            
				{
					displayBuffor->buffor[BufforPozycja] = 0;
					BufforPozycja++;
				}
				BufforPozycja = 0;
            break;

			default: // assign key to buffor (Add prevent letters feature)
				if(BufforPozycja<BUF_SIZE)
			    {
					displayBuffor->buffor[BufforPozycja] = down;
					BufforPozycja++;
				}
			break;
		}	
        // Send data to displayTask
		OSMboxPost(edMbox,displayBuffor);
        
	}
}


/**
 * @brief   Task for handling display stuff.
 */
void displayTask(void *pdata)
{
	struct queBuff *procesStruct = 0;                   // Pointer to displayMbox variable
    INT8U err;                                          // Error variable
    char strBufforLo[10] = {0};                         // Load buffor string for ultoa()
    char strBufforCn[10] = {0};                         // Counter buffor string for ultoa()
    char clear[64] = "              \0";                // Clear up string
	pdata = pdata;                                      // Prevent warnings
							
	for(;;)
	{
		procesStruct = (struct queBuff*)OSMboxPend(edMbox, 0,&err);                         // Collect data from Mail Box
        if(procesStruct->who == 1)                                                          // editTask sended a message
        {
            PC_DispClrRow(0, DISP_BGND_LIGHT_GRAY);
	        PC_DispStr(0,0,procesStruct->buffor, DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);     // display actual load value 	
        }
        else if(procesStruct->who == 2)                                                     // Que,Sem or Box task sended a message
        {
            ultoa(procesStruct->counter,strBufforCn, 10 );                                              // Convert load from task into string  
            ultoa(procesStruct->load,strBufforLo, 10 );                                                 // Convert count from task into string
            PC_DispStr(9,procesStruct->tasknr+1,clear,DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);          // Clear load
            PC_DispStr(9,procesStruct->tasknr+1,strBufforLo,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);      // Display load
            PC_DispStr(41,procesStruct->tasknr+1,strBufforCn,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);     // Display count
        }
        else
        {
            PC_DispStr(0,0,(char*)"EROOR", DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);                       // Display error message var: who unknown  	
        }
	}
}