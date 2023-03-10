/* DOBRE EMILIA ILIANA - 323CB */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "exec_parser.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE 4096

static so_exec_t *exec; 
static struct sigaction defaultHndlr; 
static int fd; 


void map_page (so_seg_t* aux, int nr_page) {
	
	int out_of_file = 0 ; 

	if ( aux->file_size < aux->mem_size ) {
		if ( ( nr_page + 1 ) * PAGE_SIZE > aux->file_size ) {
			out_of_file = 1;
		}
	}

	/* mapare pagina: */

	char *mmap_rez; 
	int flags = MAP_PRIVATE | MAP_FIXED;
	void* start = (void*)aux->vaddr + nr_page * PAGE_SIZE;	
	int offset = aux->offset + nr_page * PAGE_SIZE;
	mmap_rez = mmap(start, PAGE_SIZE, aux->perm, flags, fd, offset);

	if ( mmap_rez == MAP_FAILED )
		return;

	/* zona de zeroizat */

	if ( out_of_file == 1 ) {
		uintptr_t zero = aux->vaddr + aux->file_size;
		int dif = ( nr_page + 1 ) * PAGE_SIZE - aux->file_size;
		memset((void*)zero, 0, dif);
	}

}


static void segv_handler(int signum, siginfo_t *info, void *context) {

	uintptr_t fault_adr = (uintptr_t)info->si_addr;

	/* aux = segment unde am segfault */

	struct so_seg* aux = NULL;
	for ( int i = 0; i < exec->segments_no; i++ ) {
		struct so_seg* segment = &exec->segments[i];
		if ( fault_adr >= segment->vaddr ) {
			if ( fault_adr < segment->mem_size + segment->vaddr ) {
				aux = segment;
				break;
			}
		}
	}

	/* CAZ1: nu exista aux*/

	if ( aux == NULL ) {
		defaultHndlr.sa_sigaction(signum, NULL, NULL);
		return;
	}

	/* CAZ2: aux are pag curenta mapata */

	int dim_pages = fault_adr - aux->vaddr;
	int nr_page  = dim_pages / PAGE_SIZE;

	if ( *( (int*)(aux->data) + nr_page ) == 1 ) {
		defaultHndlr.sa_sigaction(signum, NULL, NULL);
		return;
	}

	/* CAZ3: pagina trebuie mapata */

	map_page(aux, nr_page);

	/* marcheaza pagina mapata */

	*( (int*)(aux->data)+ nr_page ) = 1;

}


int so_init_loader(void) {

	int rc;
	struct sigaction newAct;
	
	memset(&newAct, 0, sizeof(newAct));
	newAct.sa_sigaction = segv_handler;
	newAct.sa_flags = SA_SIGINFO; 
	
	rc = sigaction(SIGSEGV, &newAct, &defaultHndlr);

	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;

}


int so_execute(char *path, char *argv[]) {

	/*deschid fisierul doar pentru citire*/

	fd = open(path, O_RDONLY);
	
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	/*aloc memorie in campul data*/

	for ( int i = 0; i < exec->segments_no; i++ ) {
		int nr_pagini = exec->segments[i].mem_size / PAGE_SIZE;
		if ( exec->segments[i].mem_size % PAGE_SIZE != 0 )
			nr_pagini++;
		exec->segments[i].data = calloc(nr_pagini, sizeof(int));
	}

	so_start_exec(exec, argv);

	return -1;

}
