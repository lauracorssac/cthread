#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/error.h"
#include "../include/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

int scheduler_block_thread(csem_t *sem) {


	if (executing == NULL) return NULL_POINTER;
	if (sem == NULL) return NULL_POINTER;
	if (sem->fila == NULL) return NULL_POINTER;
	
	if (FirstFila2(executing) != SUCCESS_CODE) return EMPTY_LINE;
	
	TCB_t *executing_thread = (TCB_t *) executing->first->node;
	executing_thread->state = PROCST_BLOQ;
	
	if (DeleteAtIteratorFila2(executing) != SUCCESS_CODE) return LINE_OPERATION_ERROR;		
	if (AppendFila2(sem->fila, (void *)executing_thread)) return LINE_OPERATION_ERROR;
	
	
	TCB_t *next = malloc(sizeof(TCB_t));
	if (scheduler_get_first_ready_thread( &next ) != SUCCESS_CODE) return LINE_OPERATION_ERROR;
	
	if (AppendFila2(executing, (void *)next)) return LINE_OPERATION_ERROR;
	
	printf("swap started\n");
	if (swapcontext(&(executing_thread->context), &(next->context) ) != SUCCESS_CODE) return CONTEXT_ERROR;
	printf("swap finished\n");

	return SUCCESS_CODE;

}

int scheduler_free_thread(csem_t *sem) {

	if (sem == NULL) return NULL_POINTER;
	if (sem->fila == NULL) return NULL_POINTER;
	
	if (FirstFila2(sem->fila) != SUCCESS_CODE) return EMPTY_LINE;
	TCB_t *thread_to_wake = sem->fila->first->node;
	thread_to_wake->state = PROCST_APTO;
	
	if (DeleteAtIteratorFila2(sem->fila) != SUCCESS_CODE) return LINE_OPERATION_ERROR;
	return scheduler_insert_in_ready(thread_to_wake);
	
}

/**
 * If there is a thread to be executed, it will set the context and deal with the queues
 * @return: return SUCCESS_CODE when it gets a new thread to be executed otherwise an error
 * next will have the thread to be executed 
 */

int scheduler_get_first_ready_thread( TCB_t** next ) {
	
	if ( ready_high == NULL ) return NULL_POINTER;
	if ( next == NULL ) return NULL_POINTER;
	
	if ( FirstFila2(ready_high) == SUCCESS_CODE ) {
	
		*next = ready_high->first->node;
		return SUCCESS_CODE;
	
	} else if ((ready_medium != NULL) && ( FirstFila2(ready_medium) == SUCCESS_CODE )) { 
	
		*next = ready_medium->first->node;
		return SUCCESS_CODE;
	
	} else if ((ready_low != NULL) && ( FirstFila2(ready_low) == SUCCESS_CODE )) {
	
		*next = ready_low->first->node;
		return SUCCESS_CODE;
	
	} else {
		
		return NOTHING_TO_SCHEDULE;
	
	}
	
	//aqui tem a parada de curto circuito, tipo se uma for nula ele retorna antes de executar o resto do if, neam (interrogacao)
	
}

/**
 * If there is a thread to be executed, it will set the context and deal with the queues
 * @return: return if everything was ok
 */
 
int scheduler_schedule_next_thread() {
    
    if (executing == NULL) return EMPTY_LINE;
    
   	TCB_t *next = malloc(sizeof(TCB_t));
   	if (next == NULL) return MALLOC_ERROR;
    
    int next_result = scheduler_get_first_ready_thread(&next);
    if (next_result != SUCCESS_CODE) return next_result;
	
	int prio = next->prio;
	
	switch (prio) {
        case HIGH_PRIO:
        	
        	if (FirstFila2(ready_high) != SUCCESS_CODE) return EMPTY_LINE;
        	if (DeleteAtIteratorFila2(ready_high) != SUCCESS_CODE) return LINE_OPERATION_ERROR;	
        	if (AppendFila2(executing, (void *) next) != SUCCESS_CODE) return LINE_OPERATION_ERROR;
        	
        case MEDIUM_PRIO:
        
        	if (FirstFila2(ready_medium) != SUCCESS_CODE) return EMPTY_LINE;
        	if (DeleteAtIteratorFila2(ready_medium) != SUCCESS_CODE) return LINE_OPERATION_ERROR;	
        	if (AppendFila2(executing, (void *) next) != SUCCESS_CODE) return LINE_OPERATION_ERROR;
        	
        case LOW_PRIO:
            if (FirstFila2(ready_low) != SUCCESS_CODE) return EMPTY_LINE;
        	if (DeleteAtIteratorFila2(ready_low) != SUCCESS_CODE) return LINE_OPERATION_ERROR;	
        	if (AppendFila2(executing, (void *) next) != SUCCESS_CODE) return LINE_OPERATION_ERROR;
            
        default:
            return ERROR_PRIO_NOT_DEFINED;
    }
    
    if (setcontext(&(next->context)) == FAILED) return FAILED;
    return SUCCESS_CODE;
    
    // TENHO UMA DUVIDA NESSA: ao inves de chamarmos setcontext, nao deveriamos chamar swapcontext (interrogacao) tipo,
    // nao deveriamos salvar o atual em algum lugar (int)
    // o set context nem retorna se deu sucesso e retorna -1 se deu erro, 
    //entao faz sentido colocar um return success_code apos sua chamada (interrogacao)
   
}


int scheduler_insert_in_ready(TCB_t *thread) {
	
	if (ready == NULL) return NULL_POINTER;
	
	int prio = thread->prio;
	
	switch (prio) {
        case HIGH_PRIO:
        	return AppendFila2(ready_high, (void *) thread);
        case MEDIUM_PRIO:
        	return AppendFila2(ready_medium, (void *) thread);
        case LOW_PRIO:
            return AppendFila2(ready_low, (void *) thread);
        default:
            return ERROR_PRIO_NOT_DEFINED;
    }

}
