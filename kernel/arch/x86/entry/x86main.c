/*
 * Gramado Operating System - The main file for the kernel.
 * (c) Copyright 2015~2019 - Fred Nora.
 *
 * File: kernel/entry/x86/x86main.c 
 * 
 * Description:
 *     This is the Kernel Base. 
 *     It's the mains file for a 32 bit x86 Kernel. 
 *     The type is 'hybrid'.
 *     The entry point is in 'head.s'.
 *
 * The first three system utilities are: 
 * IDLE.BIN, SHELL.BIN and TASKMAN.BIN.
 *
 * The Kernel area is the first 4MB of real memory.
 * The image was loaded in the address 0x00100000 and the entry point is in 
 * the address 0x00101000. The logic address for the Kernel image is 
 * 0xC0001000 and the entry point is 0xC0001000.
 * 
 * @todo: Create a log file.
 *
 *  In this file:
 *  ============
 *      + mainSetCr3
 *      + startStartIdle
 *      + x86main - The entry point for a C part of the Kernel.
 *
 * Revision History:
 *     2015      - Created by Fred Nora.
 *     2016~2018 - Revision.
 *     //... 
 */ 


#include <kernel.h>


// # external dependencies #

// Variables from Boot Loader.
extern unsigned long SavedBootBlock;    //Boot Loader Block.
extern unsigned long SavedLFB;          //LFB address.  
extern unsigned long SavedX;            //Screen width. 
extern unsigned long SavedY;            //Screen height.
extern unsigned long SavedBPP;          //Bits per pixel.
//...

// Args.
extern unsigned long kArg1;
extern unsigned long kArg2;
extern unsigned long kArg3;
extern unsigned long kArg4;
//...

// Boot mode.
extern unsigned long SavedBootMode;


// Task switching support.
extern void turn_task_switch_on (void);


extern void clear_nt_flag (void);


//char copyright[] =
//"Copyright (c) 2005-2019 \n\tFred Nora.  All rights reserved.\n\n";


static inline void mainSetCr3 (unsigned long value){

    __asm__ ( "mov %0, %%cr3" : : "r" (value) );
};


/*
 *********************************************************
 * x86mainStartFirstThread:
 *      #interna
 *      
 * Seleciona a primeira thread para rodar e salta para user mode.
 * Returns program control from an exception or interrupt handler 
 * to a program or procedure that was interrupted by an exception, 
 * an external interrupt, or a software-generated interrupt.  
 */

void x86mainStartFirstThread ( void ){

    int i;
    struct thread_d *Thread;

    //
    // Select the idle thread.
    //
    
	// #importante
	// Sempre checar a validade da estrutura.
    
    
    Thread = IdleThread; 
    

    if ( (void *) Thread == NULL )
    {
        panic ("x86mainStartFirstThread: Thread\n");

    } else {

        if ( Thread->saved != 0 )
        {
            printf ("x86mainStartFirstThread: saved\n");
            die ();
        };

        if ( Thread->used != 1 || Thread->magic != 1234)
        {
            printf ("x86mainStartFirstThread: tid={%d} magic \n", Thread->tid);
            die ();
        };

        set_current ( Thread->tid );       
        //...
    };

    // State  
    if ( Thread->state != STANDBY )
    {
        printf ("x86mainStartFirstThread: state tid={%d}\n", Thread->tid);
        die ();
    }

    // * MOVEMENT 2 ( Standby --> Running)
    if ( Thread->state == STANDBY )
    {
        Thread->state = RUNNING;
        queue_insert_data ( queue, (unsigned long) Thread, QUEUE_RUNNING);
    }

	//Current process.

    current_process = Thread->process->pid;    

	//
	// Done!
	//


    for ( i=0; i <= PRIORITY_MAX; i++ ){

        dispatcherReadyList[i] = (unsigned long) Thread;
    }

    IncrementDispatcherCount (SELECT_IDLE_COUNT);


	// turn_task_switch_on:
	//  + Creates a vector for timer irq, IRQ0.
	//  + Enable taskswitch. 

    turn_task_switch_on ();


	//timerInit8253 ( HZ );
	//timerInit8253 ( 800 );
    timerInit8253 ( 900 );

	//nesse momento usaremos o mapeamento do processo alvo ..
	//no mapeamento do processo alvo tambem tem o kernel
	//entao nao há problemas.

    //Set cr3 and flush TLB.
	//isso não é necessário se chamarmos spawn ela faz isso.
    mainSetCr3 ( (unsigned long) Thread->DirectoryPA );
    asm ("movl %cr3, %eax");
		//#todo: delay.
    asm ("movl %eax, %cr3");  


    clear_nt_flag ();   

	//vamos iniciar antes para que
	//possamos usar a current_tss quando criarmos as threads
	//init_gdt ();

    asm ("clts \n");


		//#debug.
		//vamos mostrar as informaçoes da primeira thread sempre..
		//quando falhar,veremos se ha algo de diferente.


		//if ( (void *) Thread != NULL )
		//{
		//    mostra_slot ( (int) Thread->tid );
		//    mostra_reg  ( (int) Thread->tid );
		//    refresh_screen ();
		//}    


	// #debug
    printf ("Go to user mode!\n");
    refresh_screen (); 

	// #importante
	// Mudamos para a última fase da inicialização.
	// Com isso alguns recursos somente para as fases anteriores
	// deverão ficar indisponíveis.

    KeInitPhase = 4;

	// # go!
	// Nos configuramos a idle thread em user mode e agora vamos saltar 
	// para ela via iret.

	// #todo:
	// #importante:
	// Podemos usr os endereços que estão salvos na estrutura.

	//#bugbug:
	//temos a questão da tss:
	//será que a tss está configurada apenas para a thread idle do INIT ??
	//temos que conferir isso.

	//base dos arquivos.

    unsigned char *buff1 = (unsigned char *) 0x00400000;


	//init


        if ( buff1[0] != 0x7F ||
             buff1[1] != 'E' ||
             buff1[2] != 'L' ||
             buff1[3] != 'F' )
        {
            printf ("x86mainStartFirstThread: init .ELF signature");
            die ();
        }

        printf (">>> IRET\n");
        refresh_screen ();

		//#test
		//vamos usar o ds do kernel para configurar a pilha.
		//nova pilha de ring0 em conformidade com a tss0; isso porque esp vai mudando com o tempo.

        asm volatile ( " movl $0x003FFFF0, %esp \n" 
                       " movl $0x23,       %ds:0x10(%esp)  \n"  // ss.
                       " movl $0x0044FFF0, %ds:0x0C(%esp)  \n"  // esp 
                       " movl $0x3000,     %ds:0x08(%esp)  \n"  // eflags.
                       " movl $0x1B,       %ds:0x04(%esp)  \n"  // cs.
                       " movl $0x00401000, %ds:0x00(%esp)  \n"  // eip. 
                       " movl $0x23, %eax  \n"
                       " mov %ax, %ds    \n"
                       " mov %ax, %es    \n"
                       " mov %ax, %fs    \n"
                       " mov %ax, %gs    \n"
                       " iret \n" );


	// Paranoia
    panic ("x86mainStartFirstThread: FAIL");
}


//inicializa s'o o init em /init
void x86StartInit (void){
    
    //
	// ## INIT ##
	//

	// Creating init process.
	
	// > Cria um diretório que é clone do diretório do kernel base 
	// Retornaremos o endereço virtual, para que a função create_process possa usar 
	// tanto o endereço virtual quanto o físico.
	
	// > UPROCESS_IMAGE_BASE;

    InitProcess = (void *) create_process ( NULL, NULL, NULL, 
                               (unsigned long) 0x00400000, 
                               PRIORITY_HIGH, 
                               (int) KernelProcess->pid, 
                               "INIT-PROCESS", 
                               RING3, 
                               (unsigned long ) CreatePageDirectory() );

    if ( (void *) InitProcess == NULL )
    {
        panic ("x86main: InitProcess\n");

    }else{

        fs_initialize_process_pwd ( InitProcess->pid, "no-directory" );

		//processor->IdleProcess = (void*) IdleProcess;	
    };


	//====================================================
	//Create Idle Thread. tid=0. ppid=0.
	
    IdleThread = (void *) KiCreateIdle ();

    if ( (void *) IdleThread == NULL )
    {
        panic ("x86main: IdleThread\n");

    }else{

        //IdleThread->ownerPID = (int) InitProcess->pid;

		//#importante
		//Thread.
		
        processor->CurrentThread = (void *) IdleThread;
        processor->NextThread    = (void *) IdleThread;
        processor->IdleThread    = (void *) IdleThread;


        IdleThread->tss = current_tss;


		//...

		// ## importante ## 
		// Temos aqui alguma configuração. 

        current_thread = IdleThread->tid;
        next_thread = IdleThread->tid;
        idle = IdleThread->tid; 

    };

    InitProcess->Heap = (unsigned long) g_gramadocore_init_heap_va;

    InitProcess->control = IdleThread;

	//registra um dos servidores do gramado core.
	//server_index, process, thread

    ipccore_register ( (int) 0, 
        (struct process_d *) InitProcess, 
        (struct thread_d *) IdleThread );

}



// inicializa 3 processos do gramado/init/core
// mais a thread em ring 0.

void x86StartGramadoCore (void){

	//
	// ## INIT ##
	//

	// Creating init process.
	
	// > Cria um diretório que é clone do diretório do kernel base 
	// Retornaremos o endereço virtual, para que a função create_process possa usar 
	// tanto o endereço virtual quanto o físico.
	
	// > UPROCESS_IMAGE_BASE;

    InitProcess = (void *) create_process ( NULL, NULL, NULL, 
                               (unsigned long) 0x00400000, 
                               PRIORITY_HIGH, 
                               (int) KernelProcess->pid, 
                               "INIT-PROCESS", 
                               RING3, 
                               (unsigned long ) CreatePageDirectory() );

    if ( (void *) InitProcess == NULL )
    {
        panic ("x86main: InitProcess\n");

    }else{

        fs_initialize_process_pwd ( InitProcess->pid, "no-directory" );

		//processor->IdleProcess = (void*) IdleProcess;	
    };


	//====================================================
	//Create Idle Thread. tid=0. ppid=0.
	
    IdleThread = (void *) KiCreateIdle ();

    if ( (void *) IdleThread == NULL )
    {
        panic ("x86main: IdleThread\n");

    }else{

        //IdleThread->ownerPID = (int) InitProcess->pid;

		//#importante
		//Thread.
		
        processor->CurrentThread = (void *) IdleThread;
        processor->NextThread    = (void *) IdleThread;
        processor->IdleThread    = (void *) IdleThread;


        IdleThread->tss = current_tss;


		//...

		// ## importante ## 
		// Temos aqui alguma configuração. 

        current_thread = IdleThread->tid;
        next_thread = IdleThread->tid;
        idle = IdleThread->tid; 

    };

    InitProcess->Heap = (unsigned long) g_gramadocore_init_heap_va;

    InitProcess->control = IdleThread;

	//registra um dos servidores do gramado core.
	//server_index, process, thread

    ipccore_register ( (int) 0, 
        (struct process_d *) InitProcess, 
        (struct thread_d *) IdleThread );


	//==============================================   


	// #importante
	// Daqui pra baixo temos a opção de criarmos ou não os processos
	// e as threads. A configuração está em config.h

#ifdef ENTRY_CREATE_SHELL

    // Creating Shell process.
    ShellProcess = (void *) create_process ( NULL, NULL, NULL, 
                                (unsigned long) 0x00450000, 
                                PRIORITY_HIGH, 
                                (int) KernelProcess->pid, 
                                "SHELL-PROCESS", 
                                RING3, 
                                (unsigned long )  CreatePageDirectory() );

    if ( (void *) ShellProcess == NULL )
    {
        panic ("x86main: ShellProcess\n");

    }else{

       fs_initialize_process_pwd ( ShellProcess->pid, 
           current_workingdiretory_string );

        //...
    };


    //=============================================
    // Create shell Thread. tid=1. 
    ShellThread = (void *) KiCreateShell ();
	
    if( (void *) ShellThread == NULL )
	{
        panic ("x86main: ShellThread\n");

    }else{

        //ShellThread->ownerPID = (int) ShellProcess->pid;

        ShellThread->tss = current_tss;
		
		//...
    };


    ShellProcess->Heap = (unsigned long) g_gramadocore_shell_heap_va;
    ShellProcess->control = ShellThread; 
		
	//registra um dos servidores do gramado core.
	//server_index, process, thread

    ipccore_register ( (int) 1, 
        (struct process_d *) ShellProcess, 
        (struct thread_d *) ShellThread );

#endif



#ifdef ENTRY_CREATE_TASKMAN
	
    //Creating Taskman process. 
    TaskManProcess = (void *) create_process ( NULL, NULL, NULL, 
                                  (unsigned long) 0x004A0000, 
                                  PRIORITY_LOW, 
                                  KernelProcess->pid, 
                                  "TASKMAN-PROCESS", 
                                  RING3, 
                                  (unsigned long )  CreatePageDirectory() ); 

    if ( (void *) TaskManProcess == NULL ){

        panic ("x86main: TaskManProcess\n");

    }else{

        fs_initialize_process_pwd ( TaskManProcess->pid, "no-directory" );

        //...
    };

    //===================================
    //Create taskman Thread. tid=2.   

    TaskManThread = (void *) KiCreateTaskManager ();

    if ( (void *) TaskManThread == NULL )
    {
        panic ("x86main: TaskManThread\n");

    }else{

        //TaskManThread->ownerPID = (int) TaskManProcess->pid;

        TaskManThread->tss = current_tss;

        //...
    };

    TaskManProcess->Heap = (unsigned long) g_gramadocore_taskman_heap_va;
    TaskManProcess->control = TaskManThread;

	//registra um dos servidores do gramado core.
	//server_index, process, thread
    ipccore_register ( (int) 2, 
        (struct process_d *) TaskManProcess, 
        (struct thread_d *) TaskManThread );

#endif



#ifdef ENTRY_CREATE_KERNELTHREAD_RING0

    //===================================
    // Cria uma thread em ring 0.
    // Ok. isso funcionou bem.

	// >>>>> Como essa thread pertence ao processo kernel, então mudaremos ela 
	// um pouco pra cima, onde criamos o processo kernel.
	// obs: Mesmo não sendo ela o primeiro TID.

    RING0IDLEThread = (void *) KiCreateRing0Idle ();

    if( (void *) RING0IDLEThread == NULL )
    {
        panic ("x86main: RING0IDLEThread\n");

    }else{


        //RING0IDLEThread->ownerPID =  (int) KernelProcess->pid; 

		RING0IDLEThread->tss = current_tss;


		// #importante
		// Sinalizando que ainda não podemos usar as rotinas que dependam
		// de que o dead thread collector esteja funcionando.
		// Esse status só muda quando a thread rodar.

		dead_thread_collector_status = 0;
		//...
    };

#endif

}


//
//  ## MAIN ##
//


/*
 *************************************************
 * x86main: 
 *     The entry point for a C part of the Kernel.
 *
 * Function history:
 *     2015 - Created by Fred Nora.
 *     2016~2019 - Revision.
 */

void x86main (void){

    int Status = 0;

	// Se usaremos todos os processos do x server.
    int gramado_core = 0;


    debug_print ("==============\n");
    debug_print ("[x86] x86main:\n");


#ifdef ENTRY_VERBOSE
    debug_print ("[x86] x86main: Starting x86 kernel ..\n");
	//printf("x86main: Starting kernel..\n");
	//refresh_screen(); 
#endif


//initializeSystem:

	//
	// ## system ##
	//

	//#importante
	//#obs: É durante essa rotina que começamos a ter mensagens.
	
	//#importante
	//Daqui pra frente tem coisa que é dependente da arquitetura x86 e coisa
	//que não é ... talvez possamos mandar coisas
	//que não são dependentes para main.c
	
	//system.c

	// Construtor.
	// status.
    systemSystem (); 

    Status = (int) systemInit ();

    if ( Status != 0 )
    {
        debug_print ("[x86] x86main: systemInit fail\n");
        printf ("[x86] x86main: systemInit fail\n");

        KernelStatus = KERNEL_ABORTED;

        goto fail;
    }


	//
	//    #### GDT ####
	//
	
	// #obs
	// Vamos criar uma TSS e configurarmos a GDT,
	// assim poderemos usar a current_tss quando criarmos as threads
	// Essa função faz as duas coisas, cria a tss e configura a gdt.

    init_gdt ();


    debug_print ("[x86] x86main: processes and threads\n");


	//
	//  ## Processes ##
	//

    //=================================================
    // processes and threads initialization.
    // Creating processes and threads.
    // The processes are: Kernel, Idle, Shell, Taskman.
    // ps: The images are loaded in the memory.


//createProcesses:

	// #debug
    // printf ("creating kernel process ...\n");

    // Creating Kernel process. PID=0.


    KernelProcess = (void *) create_process ( NULL, // Window station.
                                 NULL, // Desktop.
                                 NULL, // Window.
                                 (unsigned long) 0xC0000000,  // base address. 
                                 PRIORITY_HIGH,               // Priority.
                                 (int) 0,                     // ppid.
                                 "KERNEL-PROCESS",            // Name.
                                 RING0,                       // iopl. 
                                 (unsigned long ) gKernelPageDirectoryAddress ); // Page directory.

    if ( (void *) KernelProcess == NULL )
    {
        panic ("[x86] x86main: : KernelProcess\n");

    }else{

        fs_initialize_process_pwd ( KernelProcess->pid, "no-directory" ); 

        //...
    };



#ifdef ENTRY_GRAMADO_CORE
    gramado_core = 1;
#endif


    if ( gramado_core == 1 )
    {
		printf ("[x86] x86main: Starting all processes\n");
	    
		//varios processos.
		x86StartGramadoCore ();
		
    }else{

		printf ("[x86] x86main: Starting only init\n");
		
		//apenas o init.
		x86StartInit ();
    };



	//
	//===============================================
	//


	//...


//Kernel base Debugger.
//doDebug:

#ifdef  ENTRY_DEBUG_CHECK_VALIDATIONS

    Status = (int) debug ();

    if ( Status != 0 )
    {
        printf ("[x86] x86main: debug\n");
        KernelStatus = KERNEL_ABORTED;

        goto fail;

    }else{
        KernelStatus = KERNEL_INITIALIZED;
    };

#endif



	//    ====================== ## TESTS ## =============================
	// begin - We can make some tests here.


    //Inicializando as variáveis do cursor piscante do terminal.
    //isso é um teste.
    timer_cursor_used = 0;   //desabilitado.
    timer_cursor_status = 0;




	//
	// ========= ## ps2 ## =============
	//

	// Initializing ps/2 controller.


	//#DEBUG
	//printf ("testing ps2\n");
	//refresh_screen(); 


    debug_print ("[x86] x86main: ps2\n");    
    ps2 ();



	//
	// Loading file tests.
	//



    // #obs: 
    // Me parece que isso vai funcionar sem problemas.

/*
	// 2MB --
    void *__buffer = (void *) allocPages ( 512 );

    fsLoadFile ( VOLUME1_FAT_ADDRESS, VOLUME1_ROOTDIR_ADDRESS, 
        "GRAMADO BMP", (unsigned long) __buffer );
        //"DENNIS  BMP", (unsigned long) __buffer );


    // #bugbug: A imagem sai cortada do lado direito.
	bmpDisplayBMP ( (char *) __buffer, 0, 0 );  
*/





	// #Aviso:
	// Isso funcionou, não mudar de lugar.
	// Mas isso faz parte da interface gráfica,
	// Só que somente nesse momento temos recursos 
	// suficientes para essa rotina funcionar.

    windowLoadGramadoIcons ();

	// #fonte:
	// Testando font NelsonCole2
	// #todo: Isso pode ficar no módulo gws ?

    gwsInstallFont ("NC2     FON");
    //gwsInstallFont ("NC3     FON");

    // #todo:
    // Temos que configurar informações sobre a fonte.
    gwsSetCurrentFontCharWidth (8);
    gwsSetCurrentFontCharHeight (8);

	//
	// ## testando suporte a salvamento de retângulo ##
	//
	
	//#debug
	//printf ("testando salvamento de retangulo\n");
	//refresh_screen();
	
	//isso funcionou ...
	initialize_saved_rect ();
	
	//testando salvar um retângulo ...
	//save_rect ( 0, 0, 100, 100 );
	
	//copiando aqui no backbuffer
	//show_saved_rect ( 20, 20, 100, 100 );
	
	//
	// ## servidor taskman ##
	//
	
	// ## Criando a janela do servidor taskman ## 
	// usada para comunicação.

    gui->taskmanWindow = (void *) CreateWindow ( 1, 0, VIEW_MINIMIZED, 
                                    "taskman-server-window", 
                                    1, 1, 1, 1, 
                                    gui->main, 0, 0, COLOR_WINDOW  ); 

    if ( (void *) gui->taskmanWindow == NULL )
    {
        panic ("[x86] x86main: taskmanWindow\n");

    } else {

		//inicializando a primeira mensagem
		////envia uma mensagem de teste para o servidor taskman

        gui->taskmanWindow->msg_window = NULL;
        gui->taskmanWindow->msg = 0; //temos uma mensagem.
        gui->taskmanWindow->long1 = 0;
        gui->taskmanWindow->long2 = 0;
        gui->taskmanWindow->newmessageFlag = 0;

		//gui->taskmanWindow->msg_window = NULL;
		//gui->taskmanWindow->msg = 123; //temos uma mensagem.
		//gui->taskmanWindow->long1 = 0;
		//gui->taskmanWindow->long2 = 0;
		//gui->taskmanWindow->newmessageFlag = 1;

    };


	// #debug
	// printf("\n");
	// refresh_screen();
	// die();


//    ====================== ## TESTS ## =============================
// #end.



#ifdef BREAKPOINT_TARGET_AFTER_ENTRY
	//#debug 
	//a primeira mensagem só aparece após a inicialização da runtime.
	//por isso não deu pra limpar a tela antes.

    printf ("[x86] x86main: after entry");
    refresh_screen(); 
    while (1){ asm ("hlt"); }

#endif	


	//
	// done !
	//


done:


    debug_print ("[x86] x86main: done\n");
    debug_print ("==============\n");


    // Return to assembly file, (head.s).
    if ( KernelStatus == KERNEL_INITIALIZED )
    {
        //
        // Starting idle thread.
        //
        
		printf("[x86] x86main: Initializing INIT ..\n");
        
#ifdef KERNEL_VERBOSE
    refresh_screen();
#endif        
   
		x86mainStartFirstThread ();        

        printf ("[x86] x86main: No idle thread selected\n");
        goto fail;
    };

fail:

	// #todo
	// Uma opção aqui é usarmos a tipagem void para essa função
	// e ao invés de retornarmos, apenas entrarmos na thread idle
	// em ring 0, isso depois de criadas as threads em user mode.

    debug_print ("[x86] x86main: fail\n");
    refresh_screen ();
}


//
// End.
//


