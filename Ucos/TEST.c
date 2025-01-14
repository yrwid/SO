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
#define  queSize				        10		      /* Size od queue                                 */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];            /* Start task stack                              */
OS_STK        propagationTaskStk[TASK_STK_SIZE];              /* WDT task stack                                */



OS_MEM        *CommMem;
OS_EVENT      *editQue;                            /* Pointer to display Queue                      */
OS_EVENT      *Sem;                                   /* Pointer to Semaphore using by SemTasks        */
OS_EVENT      *Que;                                   /* Pointer to Queue using  by QueTasks           */
OS_EVENT      *Box[5];                                   /* Pointer to Mbox using by BoxTasks             */
OS_EVENT      *displayMbox;                                /* Pointer to Mbox using by keyboard/edit tasks  */
OS_EVENT      *PropagationMbox;                                /* Pointer to Propagation Mbox */
typedef struct  
{
    INT32U load;
    INT8U taskNr;
} memStruct;

memStruct  CommBuf[25];                              // memBlock buff

INT32U semVal = 10;                                   /* Variablec protected by Sem semaphore          */
char taskNumbers[5] = {0};                            /* Variables thats are pass to tasks on create   */

void          *editMsg[10];                          /* Place for queue pointers (displayQue)         */ 
void          *CommMsg[10];                          /* Place for queue pointers (Que)                */

struct queBuff							              /* Struct using by all tasks to communicate with displayTask  */			  
{
    int who;                                          /* Who sended message                            */ 
    //tasks info
    char tasknr;                                      /* Taks Number 1-15                              */
	INT32U load;                                      /* Actual load dor loop                          */
	INT32U counter;                                   /* Task actual number of enters                  */
    INT32U delta;
    //buffor control
    char buffor[4][BUF_SIZE];                            /* Matrix used by editTask to save cmd buffor    */      
    int line;
    //Error control
    int Error;
    int mboxError[5];
    int queError; 
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
        void  propagationTask(void *data);                    /* Function prototypes of WDT task               */   
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
    INT8U i =0;
    INT8U memErr = 0 ;
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    //Initialize memBlock
    CommMem = OSMemCreate(CommBuf,25,sizeof(memStruct),&memErr);
    // create semaphores, queues and mailbox 
    displayMbox = OSMboxCreate(NULL);
    editQue = OSQCreate(&editMsg[0],queSize);  
    PropagationMbox = OSMboxCreate(NULL);
    //initialize Tasks comunication
    Que = OSQCreate(&CommMsg[0],queSize);  
    for(i =0; i<5; i++)
    {
        Box[i] = OSMboxCreate(NULL);
    }
    
    Sem = OSSemCreate(1);    
   // WDTsem = OSSemCreate(1);    

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
    PC_DispStr( 0,  5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "NR  Type Load               Delta/Sek    Counter           ERR          State   ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);    
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
    OSTaskCreate(propagationTask,0,&propagationTaskStk[TASK_STK_SIZE-1],4);                 // Propagate task 

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
 * @brief   propagation task used to catch errors and send load to each type of task(Sem,Que,Box).
 */
void propagationTask(void *pdata)
{
    INT8U err;
    memStruct *loadMem;
    struct queBuff dis[1];
    INT8U mboxErr[5] = {OS_NO_ERR};
    INT8U queErr = OS_NO_ERR;
    INT8U i=0;
    INT32U loadVal = 0;
    INT32U previouslyLoadVal = 0;

    // initialize data
    dis->who = 3;
    dis->Error = 0;
    dis->queError = 0;
    for(i=0; i<5; i++)
    {
        dis->mboxError[i] = 0;
    }

    for(;;)
    {
        loadVal = *(INT32U*)OSMboxPend(PropagationMbox,0,&err);
        if(loadVal == previouslyLoadVal)
        {
            continue;
        }
        OSSemPend(Sem,0,&err);
        semVal = loadVal;
        OSSemPost(Sem);

        //  insert into Que
        for (i = 0; i<5; i++)	
		{
            //Get dynamic memory for new load pointer
            loadMem = (memStruct *)OSMemGet(CommMem,&err);
            loadMem->load = loadVal; 
            loadMem->taskNr = i;
		    queErr = OSQPost(Que,loadMem);
		}

        //insert into mailBox
        for(i=0; i<5; i++)
        {
            //Get dynamic memory for new load pointer
            loadMem = (memStruct *)OSMemGet(CommMem,&err);
            loadMem->load = loadVal; 
            loadMem->taskNr = i;
            mboxErr[i] = OSMboxPost(Box[i], loadMem);
        }
        
        for(i = 0; i<5; i++)
        {
            if(mboxErr[i] == OS_MBOX_FULL)
            {
                dis->mboxError[i] = 1;
                dis->Error = 1;
            }
        }

        if(queErr == OS_Q_FULL)//OS_NO_ERR)
        {
            dis->queError = 1;
            dis->Error = 1;
        }

        if(dis->Error == 1)
        {
            OSMboxPost(displayMbox,dis);
            if(dis->queError == 1)
            {
                //enter into reapair sequence
                dis->who = 4; 
                OSTimeDly(600);                          /* Wait one second                          */
                loadVal = 10;
                //clear Que
                while(OSQAccept(Que));
                OSMboxAccept(Box);
                //send message with who=4 repair sequence
                OSMboxPost(displayMbox,dis);
                OSSemPend(Sem,0,&err);
                semVal = loadVal;
                OSSemPost(Sem);

                //  insert into Que
                for (i = 0; i<5; i++)	
		        {
		            queErr = OSQPost(Que,&loadVal);
		        }

                //insert into mailBox
                for(i=0; i<5; i++)
                {
                    mboxErr[i] = OSMboxPost(Box[i], &loadVal);
                }
                //out from repair sequence
                dis->who = 3;
                dis->queError = 0;
            }
            dis->Error = 0;
        }
      
        previouslyLoadVal = loadVal;
    }

}

/**
 * @brief   Queue task used for overload system load is provided via queue Que.
 */
void  QueTask(void *pdata)
{
    INT8U doSomething = 0;                                   // inside load loop varaible
    memStruct *queMessPtr = 0;                                  // Pointer to collect message from Que 
    INT32U i = 0;                                            // interator
    INT8U  err;                                              // error var
    INT8U in=0;
    OS_Q_DATA QueueData;
    INT32U snapCounter = 0;
    INT32U internalTime = 0;
   // INT32U delta = 0;
    struct queBuff dis[1];                                   // Struct to store all needed data 

    // initialize struct 
    dis->tasknr = *(char*)pdata+5;                          
    dis->counter = 0; 
    dis->load = 10;
    dis->who = 2;
    dis->delta = 200;
   
    internalTime = OSTimeGet();
    for (;;) {
        OSQQuery(Que, &QueueData);
        if(OSTimeGet() - internalTime >= 200)
        {
            internalTime = OSTimeGet();
            dis->delta = dis->counter - snapCounter ;
            snapCounter = dis->counter;
        }
        // Counter store informations about loop enter  from begining of program.
        dis->counter++;
        // Display information about work 
        PC_DispStr(72,dis->tasknr+1,"WORK",DISP_FGND_BLACK + DISP_BGND_RED);
        // Read from Que
        for(i = 0; i < QueueData.OSNMsgs; i++) 
        {
            queMessPtr =  (memStruct *)OSQAccept(Que);
            if(queMessPtr != 0)
            {   
                // if load is alredy set to current load dont read from que, let low priority task read load var         
                if((int)dis->tasknr == queMessPtr->taskNr+6 && in != 1) //dis->load != queMessPtr->load && in != 1)  //
                {
                    dis->load = queMessPtr->load;
                    err = OSMemPut(CommMem,queMessPtr);
                    in++;
                   // break;1111111
                }
                else
                {
                    OSQPost(Que, queMessPtr);
                }                                 
                                                                                                
            }
        }
        in = 0 ;

        // Send info to display
        OSMboxPost(displayMbox,dis);

        // Load loop 
        for(i = 0; i < dis->load; i++)
        { 
            doSomething = 1;
        }

        
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
    memStruct *boxMessPtr = 0;                                  // Pointer to collect message from MBox
    INT32U i = 0;                                            // Iterator
    INT32U snapCounter = 0;
    INT32U internalTime = 0;
    INT8U  err;                                              // Error varaible 
    struct queBuff dis[1];                                   // Struct to store all needed data      

    // Initialize data struct 
    dis->tasknr = *(char*)pdata+10 ; 
    dis->counter = 0; 
    dis->load = 10;
    dis->who = 2;
    dis->delta = 200;

    internalTime = OSTimeGet();
    for (;;) { 
        if(OSTimeGet() - internalTime >= 200)
        {
            internalTime = OSTimeGet();
            dis->delta = dis->counter - snapCounter ;
            snapCounter = dis->counter;
        }

        dis->counter++;
        PC_DispStr(72,dis->tasknr+1,"WORK",DISP_FGND_BLACK + DISP_BGND_RED);
       // OS_ENTER_CRITICAL();
        boxMessPtr =  (memStruct *)OSMboxAccept(Box[dis->tasknr-11]);

        if(boxMessPtr != NULL)
        {
             dis->load = boxMessPtr->load;  
             err = OSMemPut(CommMem,boxMessPtr);
        }                                   
        OSMboxPost(displayMbox,dis);                                                                                       

       // OS_EXIT_CRITICAL();
        for(i = 0; i < dis->load; i++)
        { 
            doSomething = 1;
        }
       
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
    INT32U snapCounter = 0;
    INT32U internalTime = 0;
    INT8U  err;
    struct queBuff dis[1];

    // Everything as above
    dis->tasknr = *(char*)pdata + 15; 
    dis->counter = 0; 
    dis->load = 10;
    dis->who = 2;
    dis->delta = 200;

    internalTime = OSTimeGet();
    for (;;) {
        if(OSTimeGet() - internalTime >= 200)
        {
            internalTime = OSTimeGet();
            dis->delta = dis->counter - snapCounter ;
            snapCounter = dis->counter;
        }
        
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

        OSMboxPost(displayMbox,dis);
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
			OSQPost(editQue,&Key);                       // Pass varaible to displaTask via Queue displayQue
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
    displayBuffor->line = 0;

	// Clear up display buffor
    for(i = 0; i<4; i++)
    {
        for(BufforPozycja = 0; BufforPozycja<BUF_SIZE; BufforPozycja++)
	    {
            displayBuffor->buffor[i][BufforPozycja] = 0;
	    }
    }
	

	BufforPozycja = 0;
	
	for(;;)
	{ 
		Klaw = (INT16S *)OSQPend(editQue,0,&err);
		down = (char)*Klaw;
        //  PC_DispChar(10,0,displayBuffor->line+48,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
        //  PC_DispChar(15,0,BufforPozycja+48,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);

		switch(down)
		{
			case 0x0D:			//enter  
                ValBuff = strtoul( displayBuffor->buffor[displayBuffor->line],NULL,10);
                OSMboxPost(PropagationMbox,&ValBuff);
                displayBuffor->line++;
                if(displayBuffor->line == 4) 
                {
                    displayBuffor->line =0;
                }
                //clear up next line
                for(BufforPozycja = 0; BufforPozycja<BUF_SIZE; BufforPozycja++)
	            {
                    displayBuffor->buffor[displayBuffor->line][BufforPozycja] = 0;
	            }
                BufforPozycja = 0;
			break;
			
			case 0x1B:			//esc
				PC_DOSReturn();
			break;
				
			case 0x08:			//backspace
				if(BufforPozycja>0)
				{  
					displayBuffor->buffor[displayBuffor->line][BufforPozycja-1] = 0;
					BufforPozycja--;
				}
			break;
				
			case 0x53:	//del
				BufforPozycja = 0;
                // Clear up buffor until all buffor will be clear
				while(BufforPozycja<BUF_SIZE)            
				{
					displayBuffor->buffor[displayBuffor->line][BufforPozycja] = 0;
					BufforPozycja++;
				}
				BufforPozycja = 0;
            break;

			default: // assign key to buffor (Add prevent letters feature)
				if(BufforPozycja<BUF_SIZE)
			    {
					displayBuffor->buffor[displayBuffor->line][BufforPozycja] = down;
					BufforPozycja++;
				}
			break;
		}	
        // Send data to displayTask
		OSMboxPost(displayMbox,displayBuffor);
        
	}
}


/**
 * @brief   Task for handling display stuff.
 */
void displayTask(void *pdata)
{
	struct queBuff *procesStruct = 0;                   // Pointer to displayMbox variable
    INT8U err;                                          // Error variable
    INT8U i =0;
    char strBufforLo[10] = {0};                         // Load buffor string for ultoa()
    char strBufforCn[10] = {0};                         // Counter buffor string for ultoa()
    char strBufforDe[10] = {0};                         // delta buffor string for ultoa()
    char clear[64] = "              \0";                // Clear up string
	pdata = pdata;                                      // Prevent warnings
							
	for(;;)
	{
		procesStruct = (struct queBuff*)OSMboxPend(displayMbox, 0,&err);                         // Collect data from Mail Box
        if(procesStruct->who == 1)                                                          // editTask sended a message
        {
            PC_DispStr(0,procesStruct->line,clear, DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
	        PC_DispStr(0,procesStruct->line,procesStruct->buffor[procesStruct->line], DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);    // display actual load value 	
        }
        else if(procesStruct->who == 2)                                                     // Que,Sem or Box task sended a message
        {
            ultoa(procesStruct->counter,strBufforCn, 10 );                                              // Convert load from task into string  
            ultoa(procesStruct->load,strBufforLo, 10 );   
            ultoa(procesStruct->delta,strBufforDe, 10 );                                               // Convert count from task into string
            PC_DispStr(9,procesStruct->tasknr+1,clear,DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);          // Clear load
            PC_DispStr(9,procesStruct->tasknr+1,strBufforLo,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);      // Display load
            PC_DispStr(30,procesStruct->tasknr+1,clear,DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);                 // Cleat 
            PC_DispStr(30,procesStruct->tasknr+1,strBufforDe,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);          // Display Delta/sek for each task
            PC_DispStr(41,procesStruct->tasknr+1,strBufforCn,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);     // Display count
        }
        else if(procesStruct->who == 3)
        {
            for(i =0; i<5; i++ )
            {
                if(procesStruct->mboxError[i] == 1)
                {
                    PC_DispStr(55,i+12,(char*)"MBOX FULL ERR", DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
                }        
            }

            if(procesStruct->queError == 1)
            {
                for(i =0; i<5; i++ )
                {
                    PC_DispStr(55,i+7,(char*)"QUE FULL ERR", DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
                }
            }
            
        }
        else if(procesStruct->who == 4)
        {
            PC_DispStr(35,5,(char*)"LOST VALUE !!", DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
            for(i =0; i<10; i++ )
            {
                 PC_DispStr(55,i+7,(char*)"              ", DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);
            }
        }
        else
        {
            PC_DispStr(0,0,(char*)"EROOR", DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);                       // Display error message var: who unknown  	
        }
	}
}