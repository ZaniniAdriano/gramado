
// crt0.c - testando a implementação do crt no processo shell.bin 


//usado para inicializar a rt na libc99

#include "types.h"
#include "stdio.h"   
#include "stddef.h"   
#include "stdlib.h"  


extern int GramadoMain( int argc, 
                        char *argv[], 
				        unsigned long long1, 
				        unsigned long long2 );


static char *argv[] = { 
    "-interactive",        //shell interativo
	"-login",              //login
	"Gramado Core Shell",  //nome do shell
	"test.sh",             //nome do arquivo de script.
	NULL 
};

static char *envp[] = { 
    "VFSROOT=root:/volume0",         //root dir do vfs
    "BOOTVOLUMEROOT=root:/volume1",  //root dir do volume de boot
    "SYSTEMVOLUMEROOT=root:/volume2", //root dir do volume do sistema
	NULL 
};


//
//
//
void crt0()
{
	//char **empty_string_pool;
    int app_response;	

//initRT:	

    //inicializando o suporte a alocação dinâmica de memória.
	libcInitRT();

	//inicializando o suporte ao fluxo padrão.
    stdioInitialize();	
	
	app_response = (int) GramadoMain( 3,     // Quantidade de argumentos.
	                                  argv,  // Vetor de argumentos.
									  0,     // Long1 
									  0 );   // Long2
	
	//
	// Chama kill ou exit de acordo com o problema ocorrido em main.
	// o erro pode vir no retorno ou implementa-se uma forma de pegar a execessão 
	// ocorrida durante a execussão de main.
	//
	
	switch(app_response)
	{
	    case 0:
		    exit(0);
            break;
 
        default:
		    //exit(1);
			die("crt0: EXIT ERROR! \n");
            break;		
	};
	
hang:	
    printf("crt0: EXIT ERROR! \n");
    printf("crt0: *Hang!\n");
	while(1)
	{
		asm("pause");
		asm("pause");
		asm("pause");
		asm("pause");
	};
};
