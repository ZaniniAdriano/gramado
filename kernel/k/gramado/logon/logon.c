/*
 * File: logon.c
 *
 * Descri��o:
 *     Arquivo principal do m�dulo logon do executive.
 *     *IMPORTANTE: Esse � o ambiente de logon.
 *                  � onde o programa de logon vai rodar.
 *                  Cria um desktop para o Logon.
 *
 *     MB - M�dulos inclu�dos no Kernel Base.
 *     No momento aparece a janela do Shell, que � um processo em user mode.
 *     Esse shell que aparece no momento do logon servir� para
 *     configura��es. Semelhante ao que acontece no window server.
 *     No ambiente de logon o comando ser� passado para o procedimento do
 *     Shell.
 *
 *     Mesma coisa se o ambiente for a gui.(user environment).O comando
 *     deve ser passado para o Shell, para configura��es.
 *
 *     O Logon est� na WindowStation0 e no Desktop0. 
 *     O desktop do logon tem a main window e a navigation bar, mas pode 
 * ser predefido, assim como na cria��o da gui.
 *
 * Observa��o:
 *     � nesse momento, antes de ativar o sistema multitarefas que pode-se
 * instalar as atualiza��es ou configura��es pendentes.
 *     A pend�ncia de configura��es ou atualiza��es ser�o gravadas em arquivos
 * de configura��oou em metafiles, que ser�o lidos nesse momento.
 * 
 *
 * Obs: A tela de logon poderia, como op��o, apresentar as configura��es
 *      gravadas em um metafile do sistema. Pois as configura��es s�o
 *      realizadas nesse momento. 
 *      Poderia mostrar configura��es salvas em vari�veis globais.
 *
 *      *IMPORTANTE: O logon deve ser um programa em user mode que chama rotinas aqui contidas.
 *
 * Quando logon virar um proceso em user mode:
 * +Registra o processo de logon
 * +Cria window stations e desktops. 
 *
 *    Vers�o 1.0, 2015, 2016.
 */
 
 
#include <kernel.h>



//Vari�veis internas.
//int logonStatus;
//...


// Internas.
void logon_create_screen();
void logon_create_background();
void logon_create_logo();
void logon_create_taskbar();
void logon_create_mainwindow();
void logon_create_controlmenu();
void logon_create_infobox();
void logon_create_messagebox();
void logon_create_debug();
void logon_create_navigationbar();
void logon_create_grid();
void logon_create_developer_screen();

int ExitLogon();


/* 
 * create_logon:
 *     Cria a interface gr�fica do Logon.
 *
 *     Fundo -> Screen, Background.
 *     Logo  -> Sobreposto. 
 *     Navigation bar.
 *     Usu�rio.
 *
 *    @todo Cria window stations, desktops ...
 *    @todo: logonCreate();
 *
 */
void create_logon()
{
	struct window_d *hWindow; 	 
	
	//g_guiMinimal = 1;
	
	
	//Disable interrupts, lock task switch and scheduler.
	asm("cli");
	set_task_status(LOCKED); 	
	scheduler_lock();

	//
	// GUI Structure. 
	// 	
	
	gui = (void*) malloc( sizeof(struct gui_d) );
    if( (void*) gui == NULL){
	    printf("create_logon:");
		refresh_screen();
		while(1){}
	}else{
	    
        //Set session ,window station, desktop, window and menu.
	    current_usersession = 0;       // User Session.
	    current_windowstation = 0; // Window Station.
	    current_desktop = 0;       // Desktop.
	    current_window = 0;        // Window.
	    current_menu = 0;          // Menu.
		
		// Initialize Session, WindowStation, Desktop, Windows and Menus.
		  
		//user section.
		printf("create_logon: User Session..\n");
		init_user_session();
		
		//initialize window station default.	
	    printf("create_logon: Station..\n");   
	    init_window_station();  	
	
	    //initialize desktop default.
	    printf("create_logon: Desktop..\n");   
	    init_desktop(); 	    
	
	    //Inicia estrutura.	
	    printf("create_logon: Windows..\n");   
	    init_windows(); 

		//menus.
		printf("create_logon: Menu..\n");
	    init_menus();        
	    
		//init_menus( ... 	
	    //...
	};
	
	// Configura elementos da tela de login.
    if( g_guiMinimal == 1){
        SetLogonParameters(0,1,1,0,0,1,0,0,0,0,0,0); 	
	}else{
		
        SetLogonParameters(0,  //refresh         
                           1,  //*screen          
                           1,  //*background       
                           0,  //logo            
                           0,  //taskbar (autoriza e prepara para a utiliza��o de uma taskbar.)        
                           1,  //*Main.(Desktop window, �rea de trabalho)            
                           0,  //menu           
                           0,  //infobox         
                           0,  //messagebox       
                           0,  //debug           
                           0,  //navigationbar    
                           0); //grid (autoriza e prepara para a utiliza��o de um grid)	
	};	

    //
	// Draw.
	//
					   
draw_logon_stuffs:	
	   
	printf("create_logon: Draw..\n");   

	//Screen, Background and Logo. 
	if(gui->screenStatus == 1){ 
	    logon_create_screen(); 
	};	
	if(gui->backgroundStatus == 1){	
	    logon_create_background(); 
	};	
	if(gui->logoStatus == 1){ 
	    logon_create_logo(); 
	};	

	//Taskbar, Control menu and Messagebox.
    if(gui->taskbarStatus == 1){	
	    logon_create_taskbar(); 
	};
    if(gui->menuStatus == 1){ 
	    logon_create_controlmenu(); 
	};
    if(gui->messageboxStatus == 1){ 
	    logon_create_messagebox(); 
	};

	//Main window, Navigation bar and grid.    
	if(gui->mainStatus == 1){ 
	    logon_create_mainwindow(); 
	};
    if(gui->navigationbarStatus == 1){ 
	    logon_create_navigationbar(); 
	};
    if(gui->gridStatus == 1){ 
	    logon_create_grid(); 
	};
	
	
	/*
	 *  Client Area:
	 *
	 * Criar a janela gui->ClientArea.
	 * e inicializar seus par�mtros.
	 */
	 
	/* 
	gui->ClientArea = CreateWindow( 1, 0, 0, "ClientArea", 
	                                0, (480/8), 640, (480-(480/8)), 
							        gui->screen, 0, 0, COLOR_WINDOW ); 
	
	if( (void*) gui->ClientArea == NULL){
	    gui->ClientArea = gui->screen;
	}else{		
		RegisterWindow(gui->ClientArea);
	};	 
	 
	*/
	
	
	// Strings
	if( (void*) CurrentUser != NULL ){
		
        if(g_guiMinimal != 1)
		{		
	        //
			// Obs: As mensagens aqui s�o paar auxiliar o desenvolvedor no processo de inicializa��o.
			//
			
			//@todo: Informar o desktop atual.
			
			//draw_text(gui->main, 0, 0, COLOR_WHITE, "Logon:");
			//draw_text(gui->main, 0, 8, COLOR_WHITE, "=============");
			//draw_text(gui->main, 8*12, 8*5, COLOR_WHITE, "Welcome to Logon Desktop, press F1=START or F2=REBOOT!");
			draw_text(gui->main, 400 +8, 8*1, COLOR_WHITE, "Welcome");			
			draw_text(gui->main, 400 +8, 8*2, COLOR_WHITE, "(c) Copyright 2005-2017, Fred Nora");
			draw_text(gui->main, 400 +8, 8*3, COLOR_WHITE, "(under construction)");
			//draw_text(gui->main, 8*10, 8*12, COLOR_WHITE, "Welcome to Logon Desktop, press F1=START or F2=REBOOT!");
			//draw_text(gui->main, 8*10, 8*13, COLOR_WHITE, "Welcome to Logon Desktop, press F1=START or F2=REBOOT!");
			//draw_text(gui->main, 8*10, 8*14, COLOR_WHITE, "Welcome to Logon Desktop, press F1=START or F2=REBOOT!");
			//...
			
			//Op��es:
			//StatusBar( gui->screen, "Logon", "Press F1 to start User Environment or F2 to Reboot");
			//Message Box.
			//...
			//
	    };
		//Nothing.
	};
	
	    //draw_text(gui->screen, 8, 1*8, COLOR_WHITE, (unsigned char *) CurrentUser->name_address);
		//draw_text(gui->screen, 8, 2*8, COLOR_WHITE, (unsigned char *) CurrentUser->name_address);
		//draw_text(gui->screen, 8, 3*8, COLOR_WHITE, (unsigned char *) CurrentUser->name_address);
		//draw_text(gui->screen, 8, 4*8, COLOR_WHITE, (unsigned char *) CurrentUser->name_address);
		//draw_text(gui->screen, 8, 5*8, COLOR_WHITE, (unsigned char *) CurrentUser->name_address);
	
	//draw_text(gui->screen, 640/2, 8, COLOR_WHITE, "Press F1 to Log on");
	
    //StatusBar( gui->screen, "StatusBar: ", "Press F1 to Log on as default user!");



	
	
	//
	// Depois de criado todos os elementos gr�ficos, vamos criar em definitivo
	// a janela do desenvolvedor.
	//
	
//CREATING_DEVELOPER_SCREEN:
	//logon_create_developer_screen();
	
	
	//Done.
done:
	// Refresh. 	
	if(gui->refresh == 1)
	{
		refresh_screen();
		gui->refresh = 0;
	};
    gui->initialised = 1;
    return;
};



/*
 * ExitLogon:
 *     Fecha o ambeinte de logon.
 *     @todo: logonExit();
 */
int ExitLogon()
{					
    if( (void*) gui != NULL )
	{
        //CloseWindow();
		//...
		
		if( (void*) gui->screen != NULL ){
		    SetFocus(gui->screen);
	    };
	};
	
	//...
	
done:	
	logonStatus = 0;
	return (int) 0;
};

/*
 * SetGuiParameters:
 *     Configura a inicializa��o das janelas gerenciadas pelo kernel.
 *     @todo: Limite de janelas (+- 10)
 *
 */
void SetLogonParameters( int refresh,  // Flag. Colocar ou n�o o backbuffer na tela.
int screen,          // Tela do computador.
int background,      // Background da area de trabalho. 
int logo,            // Logo, Janela para imagem da area de trabalho.
int taskbar,         // Barra de tarefas.(icones para programas)
int main,            // Janela principal.( Desktop, ou Kernel Setup Utility ) 
int menu,            // Control menu da �rea de trabalho.
int infobox,         // Janelinha no canto, para alerta sobre sobre eventos ocorridos no sistema.
int messagebox,      // O message box do kernel.(cada app cria seu messagebox.) 
int debug,           // Janela do debug do kernel.(sonda por problemas)
int navigationbar,   // Janela de navega��o(semelhante a navigation bar dos dispositivos mobile) 
int grid             // Grid da janela principal.
//limites ...
)
{
    if( (void*) gui == NULL ){
	    return;	
	};	
	
	gui->refresh = refresh; 
	gui->screenStatus = screen;
	gui->backgroundStatus = background; 
	gui->logoStatus = logo;
	gui->mainStatus = main; 	
	gui->taskbarStatus = taskbar;
	gui->menuStatus = menu;
	gui->infoboxStatus = infobox;
	gui->messageboxStatus = messagebox;
	gui->debugStatus = debug;	
	gui->navigationbarStatus = navigationbar; 
	gui->gridStatus = grid;  

	//*Padr�es	
	
done:
    return;
};


/*
 * logon_create_screen:
 *     Cria a tela. 
 *     Atribuindo as dimens�es.
 *     ...
 */
void logon_create_screen()
{
    //
	// @todo: Os par�metros passados com as dimens�es foram
	//        passados pelo boot loader e est�o dispon�veis.
	//
	
	unsigned long Left = 0;
	unsigned long Top = 0;
	unsigned long Width = 800;    //@todo: usar vari�vel global com esse valor.
	unsigned long Height = 600;   //@todo: usar vari�vel global com esse valor.
	
    struct window_d *hWindow; 	
	
	// Screen - N�o tem parent window.
	hWindow = (void*) CreateWindow( 1, 0, VIEW_MINIMIZED, "Screen", 
	                                Left, Top, Width, Height, 
							        NULL, 0, 0, COLOR_BLACK );  
	if( (void*) hWindow == NULL ){
	    printf("logon_create_screen:");
		refresh_screen();
	    while(1){};
	}else{
	    
		RegisterWindow(hWindow);
	    windowLock(hWindow); 
		set_active_window(hWindow); 
		
		//a janela pertence ao desktop 0
	    //hWindow->desktop = (void*) desktop0;

	    if( (void*) gui == NULL){
		    return;
		}else{
	        gui->screen = (void*) hWindow;
		};
	};
	
done:
    //bugbug: N�o usar set focus nessa que � a primeira janela.	
    return; 
};


/*
 * logon_create_background:
 *     Cria o background. Pinta ele de uma cor.
 *
 */
void logon_create_background()
{ 
    struct window_d *hWindow; 

	//Debug:
	//printf("logon_create_background createwindow\n");				   
	//refresh_screen();

	//O background pertence ao desktop0.
	hWindow = (void*) CreateWindow( 1, 0, 0, "Background", 
	                                0, 0, 800, 600, 
							        gui->screen, 0, 0, COLOR_BACKGROUND );
													
	if( (void*) hWindow == NULL){
	    printf("logon_create_background:");
		refresh_screen();
	    while(1){};
	}else{
	    
		RegisterWindow(hWindow);
	    windowLock(hWindow); 
		set_active_window(hWindow); 
	    //a janela pertence ao desktop 0
	    //hWindow->desktop = (void*) desktop0;

		if( (void*) gui == NULL){
		    return;
		}else{
	        gui->background = (void*) hWindow;
		};	

	};
	
done:
    SetFocus(hWindow);	
    return; 
};


/*
 * logon_create_logo:
 *     Cria a janela para o logo da area de trabalho.
 *     o logo da area de trabalho � a magem da area de trabalho.
 *
 *     Obs: Sem logo por enquanto.
 */
void logon_create_logo()
{ 
    return; //Cancelada! 
};


/*
 * logon_create_taskbar:
 *     Obs: N�o h� tarefas durante o ambiente de logon.
 *          N�o precisa de barra de tarefas.
 */
void logon_create_taskbar()
{ 
    return; //Cancelada!.
};


/*
 * logon_create_mainwindow:
 *      A �rea de trabalho.
 *      *Importante: � a �rea dispon�vel na tela para o aplicativo. 
 */
void logon_create_mainwindow()
{    	
	//Dimens�es:
	unsigned long Left = 0;
	unsigned long Top = 0;
	unsigned long Width = 800;    //@todo: usar vari�vel global com esse valor.
	unsigned long Height = 600;   //@todo: usar vari�vel global com esse valor.	

    struct window_d *hWindow; 
	
	//
	// A janela principal pertence ao desktop.
	//
	
	//
	// Estamos criando uma �rea de trabalho que
	// � o tamanho total da tela menos a �rea destinada
	// � barra de tarefas.
	// A barra de tarefas ainda n�o foi criada, mas
	// um tamanho inicial para ela ja foi reservado.
	//
	
	hWindow = (void*) CreateWindow( 1, 0, VIEW_MINIMIZED, "Area de Trabalho", 
	                                Left, Top, Width, (Height-32), 
							        gui->screen, 0, 0, COLOR_WINDOW ); 
	if( (void*) hWindow == NULL){
	    printf("logon_create_mainwindow:");
	    refresh_screen();
		while(1){};
	}else{   
	    		
	    RegisterWindow(hWindow);
	    windowLock(hWindow); 
		set_active_window(hWindow); 
		
	    //a janela pertence ao desktop 0
	    //hWindow->desktop = (void*) desktop0;
		
		if( (void*) gui == NULL){
		    return;
		}else{
	        gui->main = (void*) hWindow;
		};	
	};
	
	
	//
	//  Desktop Window:
	//      Criar a janela gui->desktop.
	//      Na verdade a janela do desktop
	//      � a janela da �rea de trabalho.
	//
	
    /*	
	gui->desktop = (void*) CreateWindow( 1, 0, VIEW_MINIMIZED, "Desktop Window", 
	                                0, 16, 800, 600-16-16, 
							        gui->screen, 0, 0, COLOR_WINDOW ); 
	*/
	
	gui->desktop = (void*) gui->main;
	
	if( (void*) gui->desktop == NULL){
		
		gui->desktop = (void*) gui->screen;
		
		if( (void*) gui->desktop == NULL){ 
		    goto done;
		};
	};
	
	//N�o registrar pois main menu ja est� registrada.
	//RegisterWindow(gui->desktop);
	
done:
    SetFocus(hWindow);
    return;	
};


/*
 * logon_create_controlmenu:
 *     Inicializar a estrutura do System menu, 
 *     para ser usado durante o ambiente de logon.
 *     
 *
 */
void logon_create_controlmenu()
{ 
    return; 
};


/*
 * logon_create_infobox:
 *     Janelinha de informa��es do sistema.
 *     Avisa o usu�rio sobre eventos do sistema na ora do logon,
 * como novos dispositivos encontrados.
 *     Inicalizar uma estrutura light de infobox.
 */
void logon_create_infobox()
{
   //@todo: trocar isso por message window, aquela janela amarela de mensagens.
    return; //Nothing for now.
};


/*
 * logon_create_messagebox:
 *     @todo: Inicializa o message box a ser usado no ambiente de logon.
 *            Light message box. 
 */
void logon_create_messagebox()
{ 
    return; 
};


/*
 * logon_create_debug:
 *     Pequeno debug de estruturas e par�metros e logon.
 * 
 */
void logon_create_debug()
{ 
    return; //Nothing for now.
};


/*
 * logon_create_navigationbar:
 *     A barra de navega��o na tela de logon.
 *
 *     Nessa barra pode ter bot�es como desligar ou reboot.
 *     Barra de navega��o, semelhante as barras encontradas nos 
 * dispositivos m�veis.
 *     Usada para navega��o simples. 
 *     Fica em baixo.
 *     A barra de navega��o da tela de logon pode ser um pouco mais larga.
 *
 */
void logon_create_navigationbar()
{
    struct window_d *hWindow;  
	
	//A navigation bar pertence a janela principal.
	hWindow = (void*) CreateWindow( 1, 0, 0, "NavigationBar", 
	                                0, 0, 800, 24, 
									gui->screen, 0, 0, COLOR_BLACK); 
	if( (void*) hWindow == NULL)
	{
	    printf("logon_create_navigationbar:");
		refresh_screen();
	    while(1){};
	}else{
	    		
        //navigation bar como foreground window inicialmente.		
		//set_current_foreground(desktop0, gui->navigationbar);
		
		RegisterWindow(hWindow);
	    windowLock(hWindow); 
		set_active_window(hWindow); 
		
	    //a janela pertence ao desktop 0
	    //hWindow->desktop = (void*) desktop0;
		gui->navigationbar = (void*) hWindow;
	};

	// Buttons.
draw_buttons:	
    draw_button( gui->navigationbar, "MENU", 1, 2, 2, 48, 20, COLOR_BLACK);	
	
 /*   
	//Start
    draw_button( gui->navigationbar, "F1=START", 1,  
	             6*(480/8), 8, 100, 24, 
				 COLOR_WINDOW);	
    
				 
    //Reboot
    draw_button( gui->navigationbar, "F2=REBOOT", 1,  
	             8*(480/8), 8, 100, 24, 
				 COLOR_WINDOW);	
	
	*/
		
	
    //
    // Text.
    //
	
draw_texts:    
	draw_text( gui->navigationbar, 1*(600/8), 8, COLOR_WHITE, ".Logon "); 
	//draw_text( gui->navigationbar, 2*(480/8), 8, COLOR_WHITE, CurrentUser->name_address ); 
	
done:
    SetFocus(hWindow);		
	return; 
};


/*
 * logon_create_grid:
 *     O Kernel n�o cria grid em logon.
 */
void logon_create_grid()
{ 
	return; //Cancelada!
};


/*
 * logon_create_developer_screen:
 *     Cria a tela do desenvolvedor, a �nica tela em primeiro plano e com foco de
 * entrada durante a fase de desenvolvimento da interface gr�fica.
 *     � conveniente que essa tela seja de uma cor diferente, para os aplicativos 
 * testarem cores diferentes de fonte.
 *     Atribuindo as dimens�es.
 *     ...
 */
void logon_create_developer_screen()
{
    //
	// @todo: Os par�metros passados com as dimens�es foram
	//        passados pelo boot loader e est�o dispon�veis.
	//
	
	//M�trica. 
	//Precisa ter boa altura.
	//N�o pode atrapalhar as outras que ser�o basicamente pequenas e centralizadas.
	//Ent�o ela deve ser estreita e do lado esquerdo... 
	//Deve ter uma cor distinta... na� pode preto, azul, nem verde, nem rosa...
	//Ser� laranjada.
	unsigned long Left   = 2;
	unsigned long Top    = 2;
	unsigned long Width  = 320;    //@todo: usar vari�vel global com esse valor.
	unsigned long Height = 480;    //@todo: usar vari�vel global com esse valor.
	
    struct window_d *hWindow; 	
	
	//
	//
	//
	//
	
	// Screen - N�o tem parent window.
	hWindow = (void*) CreateWindow( 3, 0, VIEW_NORMAL, "{} DEVELOPER SCREEN", 
	                                Left, Top, Width, Height, 
							        NULL, 0, COLOR_ORANGE, COLOR_BLUE );  
	if( (void*) hWindow == NULL ){
	    printf("logon_create_developer_screen:");
		refresh_screen();
	    while(1){};
	}else{
	    
		RegisterWindow(hWindow);
	    windowLock(hWindow);     //?? 
		set_active_window(hWindow); 
		
		//a janela pertence ao desktop 0
	    //hWindow->desktop = (void*) desktop0;

		//
		// Mais m�tricas.
		//
		hWindow->left    = Left;
	    hWindow->top     = Top;
		hWindow->width   = Width; 
		hWindow->height  = Height;
		hWindow->right   = (Left+Width);
		hWindow->bottom  = (Top+Height);
		//...
		
		//Cursor
		hWindow->cursor_x = Left;
		hWindow->cursor_y = Top;
		
		
		//salva na estrutura gui->
		if( (void*) gui == NULL){
		    return;
		}else{
	        gui->DEVELOPERSCREEN = (void*) hWindow;
		};
	};
	
done:
    //
	// *IMPORTANTE:
	//
	//  TRAVAREMOS A JANELA EM PRIMEIRO PLANO E COM O FOCO DE ENTRADA.
    //  NENHUMA OUTRA JANELA PODER� GANHAR O FOCO.
    //  
    // At� que a interface gr�fica fique mais robusta, apenas a janela 
    // dodesenvolvedor ficar� com o foco de entrada.  	
	//
	
	//ganho o foco.
	SetFocus(hWindow);
	
	//trava o foco nela.
	//n�o criaremos uma rotina para manipular essa vari�vel...
	//e ela ser� usada apenas uma vez. 
	// essa vari�vel ser� uma flag na rotina SetFocus(.).
	//e ficar� definida no come�o do arquivo window.h.
	_lockfocus = 1;
	

	//ERRO NA CRIA��O DA ESTRUTURA.
	if( (void*) gui->DEVELOPERSCREEN == NULL)
	{
		printf("logon_create_developer_screen: gui->DEVELOPERSCREEN");
		refresh_screen();
		while(1){};
	};
	
    return; 
};

/*
 * LogonProcedure:
 *     O procedimento de janela do Logon.
 *
 */																
unsigned long LogonProcedure( struct window_d *window, 
                              int msg, 
							  unsigned long long1, 
							  unsigned long long2 ) 
{
    
	//Obs: Deve ser simples para o m�dulo logon do kernel base.
	
	switch(msg)
	{			
		case MSG_SYSKEYDOWN:                 
            switch(long1)	       
            {   
				//Start	
				case VK_F1:
					ExitLogon();               
					startUserEnvironment(0,0);    
                    break;
					
				//Reboot	
                case VK_F2:
					ExitLogon();
				    KiReboot();				
                    break;				
														
				default:
                    //Nothing.
				    break;
		    };              
        break;
		
		default:
		    //Nothing.
		    break;
	};
	
// Done.
done:
	//Refresh screen. 
	if(VideoBlock.useGui == 1){
	    refresh_screen();
	};
	return (unsigned long) 0;
};

						
/*
 * init_logon:
 *     Inicializa o Logon.
 *     Obs: Aceita argumentos.
 * Argumentos:
 * -l ou /l; ...
 *
 */
int init_logon(int argc, char *argv[]) 
{    	
	int LogonFlag = 0;
	char *s;    //String
	
	//
	// Se n�o h� argumentos.
	//
	
	if(argc < 1){
		goto done;
    };
	
	//
	// Dependendo do argumento encontrado muda-se a flag.
	//
	
    while(--argc) 
    {
        s = *++argv;
        if (*s == '-' || *s == '/') 
		{
            while(*++s) 
			{
                switch(*s) 
				{
                    case 'l':
                        LogonFlag = 0;
                        break;

                    case 's':
                        LogonFlag = 0;
                        break;

                    case 'r':
					    LogonFlag = 0;
                        break;

                    case 'f':
					    LogonFlag = 0;
                        break;

                    default:    
					    //usage();
						break;
                };
            };
        }
        else 
		{
            //usage();
        }
    };
	
	//
	// Aqui deve-se habilitar as op��es de acordo
	// com a flag.
	//
	
	
	
	//...
	
done:
	//printf("init_logon: Initializing..\n");
    SetProcedure( (unsigned long) &LogonProcedure);	
    logonStatus = 1;	
	//g_logged = (int) 0;
	return (int) 0;
};

/*
int logonInit()
{}
*/

//
//fim.
//

