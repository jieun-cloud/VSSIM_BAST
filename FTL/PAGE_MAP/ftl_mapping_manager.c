// File: ftl_mapping_manager.c
// Date: 18-Sep-2017
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2017
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#include "common.h"

ppn_t** mapping_table;
pbn_t** data_mapping_table; //Jieun add
block_list** log_block_list;//Jieun
int64_t* n_total_log_blocks; //Jieun

log_block_entry** log_block_mapping_table;
//int N_LOG_BLOCK = 4;
pthread_mutex_t* get_free_page_lock;
pthread_mutex_t* get_free_block_lock;
//pthread_mutex_t* get_free_log_lock;
int debug_index = 0;

int INIT_MAPPING_TABLE(int init_info)
{
	int i;
	int ret;

	int64_t n_total_pages;

	/* Allocation Memory for Mapping Table */
	mapping_table = (ppn_t**)calloc(sizeof(ppn_t*), N_IO_CORES);
	for(i=0; i<N_IO_CORES; i++){

		n_total_pages = vs_core[i].n_total_pages;

		mapping_table[i] = (ppn_t*)calloc(sizeof(ppn_t),
				n_total_pages);
		if(mapping_table[i] == NULL){
			printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
			return -1;
		}
	}	

	/* Init get free page lock */
	get_free_page_lock = (pthread_mutex_t*)calloc(sizeof(pthread_mutex_t),
					N_IO_CORES);
        if(get_free_page_lock == NULL){
                printf("ERROR[%s] Create per core free page lock fail!\n", __FUNCTION__);
                return -1;
        }
	else{
		for(i=0; i<N_IO_CORES; i++){
			pthread_mutex_init(&get_free_page_lock[i], NULL);
		}
	}

	/* Initialization Mapping Table */
	
	/* If mapping_table.dat file exists */
	if(init_info == 1){
		FILE* fp = fopen("./META/mapping_table.dat","r");
		if(fp != NULL){
			for(i=0; i<N_IO_CORES; i++){

				n_total_pages = vs_core[i].n_total_pages;

				ret = fread(mapping_table[i], sizeof(ppn_t), 
						n_total_pages, fp);
				if(ret == -1){
					printf("ERROR[%s] Read mapping table fail!\n", __FUNCTION__);
					return -1;
				}
			}

			return 1;
		}
		else{
			printf("ERROR[%s] fail to read mapping table from file!\n", __FUNCTION__);
			return -1;
		}
	}
	else{	
		int j;
		for(i=0; i<N_IO_CORES; i++){	
			
			n_total_pages = vs_core[i].n_total_pages;

			for(j=0; j<n_total_pages; j++){
				mapping_table[i][j].addr = -1;
			}
		}

		return 0;
	}
}

//Jieun add

int INIT_DATA_MAPPING_TABLE(int init_info)
{
	int i;
	int ret;

	int64_t n_total_blocks;

	/* Allocation Memory for Mapping Table */
	data_mapping_table = (pbn_t**)calloc(sizeof(pbn_t*), N_IO_CORES);
	for(i=0; i<N_IO_CORES; i++){

		n_total_blocks = vs_core[i].n_total_blocks;

		data_mapping_table[i] = (pbn_t*)calloc(sizeof(pbn_t),
				n_total_blocks);
		if(data_mapping_table[i] == NULL){
			printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
			return -1;
		}
	}	

	get_free_page_lock = (pthread_mutex_t*)calloc(sizeof(pthread_mutex_t),
					N_IO_CORES);
        if(get_free_page_lock == NULL){
                printf("ERROR[%s] Create per core free page lock fail!\n", __FUNCTION__);
                return -1;
        }
	else{
		for(i=0; i<N_IO_CORES; i++){
			pthread_mutex_init(&get_free_page_lock[i], NULL);
		}
	}
	/* Init get free page lock */
//	get_free_block_lock = (pthread_mutex_t*)calloc(sizeof(pthread_mutex_t),
//			N_IO_CORES);
//	if(get_free_block_lock == NULL){
//		printf("ERROR[%s] Create per core free page lock fail!\n", __FUNCTION__);
//		return -1;
//	}

//	else{
//		for(i=0; i<N_IO_CORES; i++){
//			pthread_mutex_init(&get_free_block_lock[i], NULL);
//		}
//	}

	/* Initialization Mapping Table */

	int j;
	for(i=0; i<N_IO_CORES; i++){	

		n_total_blocks = vs_core[i].n_total_blocks;

		for(j=0; j<n_total_blocks; j++){
			data_mapping_table[i][j].addr = -1;
		}
	}
	return 0;

}

//Jieun
int INIT_LOG_BLOCK_MAPPING_TABLE(int init_info)
{
	int i;
	int ret;

	/* Allocation Memory for Mapping Table */
	log_block_mapping_table = (log_block_entry**)calloc(sizeof(log_block_entry*), N_IO_CORES);

	for(i=0; i<N_IO_CORES; i++){
		log_block_mapping_table[i] = (log_block_entry*)calloc(sizeof(log_block_entry), N_LOG_BLOCK);
	}

	// For debug
	printf("Per core log blocks:%d\n", N_LOG_BLOCK);

	int j,k;
	for(i=0; i<N_IO_CORES; i++){
		for(j=0; j<N_LOG_BLOCK; j++){
			
			log_block_mapping_table[i][j].pbn.addr = -1;
			log_block_mapping_table[i][j].log_mapping_table=NULL;
	
		}
	}

	INIT_LOG_BLOCK_ENTRY();
	return 0;

}

//Jieun add
int INIT_LOG_BLOCK_ENTRY(void){
	int i, j, k;
	for(i=0;i<N_IO_CORES;i++){
		for(j=0; j<N_LOG_BLOCK; j++){

			log_block_mapping_table[i][j].log_mapping_table = (int64_t*)calloc(sizeof(int64_t), N_PAGES_PER_BLOCK);
			log_block_mapping_table[i][j].inverse_log_mapping_table = (void*)calloc(N_PAGES_PER_BLOCK, sizeof(int64_t));
			if(log_block_mapping_table[i][j].inverse_log_mapping_table == NULL){
				printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
				return -1;
			}

			log_block_mapping_table[i][j].pbn.path.flash = -1;
			log_block_mapping_table[i][j].pbn.path.plane = -1;
			log_block_mapping_table[i][j].pbn.path.block = -1;
			
			log_block_mapping_table[i][j].lbn = -1;
			log_block_mapping_table[i][j].w_index = 0;
			log_block_mapping_table[i][j].is_switch = 0;
			log_block_mapping_table[i][j].last_lpn = -1;
		}	
	}

	for(i=0;i<N_IO_CORES;i++){
		for(j=0;j<N_LOG_BLOCK;j++){
			for(k=0;k<N_PAGES_PER_BLOCK;k++){
				log_block_mapping_table[i][j].log_mapping_table[k] = -1;
				log_block_mapping_table[i][j].inverse_log_mapping_table[k] = -1;
			}
		}
	}
	//For debugging
/*	
	for(i=0;i<N_IO_CORES;i++){
		for(j=0;j<N_LOG_BLOCK;j++){
			log_block_entry cur_entry = log_block_mapping_table[i][j];
			printf("Log block mapping table:: log_table[core %d][index %d].pbn flash:%d\n",i,j,cur_entry.pbn.path.flash);
			printf("Log block mapping table:: log_table[core %d][index %d].lbn :%d\n",i,j,cur_entry.lbn);
			printf("Log block mapping table:: log_table[core %d][index %d].w_index :%d\n",i,j,cur_entry.w_index);
			printf("Log block mapping table:: log_table[core %d][index %d].is_switch :%d\n",i,j,cur_entry.is_switch);
			printf("Log block mapping table:: log_table[core %d][index %d].last_lpn :%d\n",i,j,cur_entry.last_lpn);

			
			for(k=0;k<N_PAGES_PER_BLOCK;k++){
			
					
			printf("log mapping tale : %d",cur_entry.log_mapping_table[k]);
			printf("log inverse mapping tale : %d",cur_entry.inverse_log_mapping_table[k]);
			}
		}
	}
 */
	return 0;
}
//Jieun
int ERASE_LOG_BLOCK_ENTRY(int core_id, int index){

	log_block_mapping_table[core_id][index].pbn.addr = -1;
	log_block_mapping_table[core_id][index].pbn.path.flash = -1;
	log_block_mapping_table[core_id][index].pbn.path.plane = -1;
	log_block_mapping_table[core_id][index].pbn.path.block = -1;

	log_block_mapping_table[core_id][index].lbn = -1;
	log_block_mapping_table[core_id][index].w_index = 0;
	log_block_mapping_table[core_id][index].last_lpn = -1;
	log_block_mapping_table[core_id][index].is_switch = 0;
	
	int k;
	for(k=0;k<N_PAGES_PER_BLOCK;k++){
		log_block_mapping_table[core_id][index].log_mapping_table[k] = -1;
		log_block_mapping_table[core_id][index].inverse_log_mapping_table[k] = -1;
	}
//	printf("Erase log entry\n");
//	printf("log pbn.addr:%d flash:%d\n", log_block_mapping_table[core_id][index].pbn.addr, log_block_mapping_table[core_id][index].pbn.path.flash);
	return 0;
}


void TERM_MAPPING_TABLE(void)
{
	int i;
	int64_t n_total_pages;

	FILE* fp = fopen("./META/mapping_table.dat","w");
	if(fp == NULL){
		printf("ERROR[%s] File open fail\n", __FUNCTION__);
		return;
	}

	/* Write the mapping table to file */
	for(i=0; i<N_IO_CORES; i++){
		n_total_pages = vs_core[i].n_total_pages;
		fwrite(mapping_table[i], sizeof(ppn_t),
			n_total_pages, fp);
	}

	/* Free memory for mapping table */
	for(i=0; i<N_IO_CORES; i++){
		free(mapping_table[i]);
	}
	free(mapping_table);

	fclose(fp);
}

//Jieun add
void INIT_LOG_BLOCK_LIST(void)
{
	int i;

	log_block_list = (block_list**)calloc(sizeof(block_list*), N_IO_CORES);

   for(i=0; i<N_IO_CORES; i++){
		log_block_list[i] = (block_list*)calloc(sizeof(block_list), 1);
	}
	n_total_log_blocks = (int64_t*)calloc(N_IO_CORES, sizeof(int64_t));
	for(i=0; i<N_IO_CORES; i++){
		n_total_log_blocks[i] = 0;
	}
	for(i=0; i<N_IO_CORES; i++){
		log_block_list[i]->head = NULL;
		log_block_list[i]->tail = NULL;
		log_block_list[i]->n_blocks = 0;
	}	
	
}


//Jieun

pbn_t GET_DATA_MAPPING_INFO(int core_id, int64_t lbn)
{
	pbn_t pbn = data_mapping_table[core_id][lbn];

	return pbn;
}
/*
// mode 1 : merge or switch the input log block entry
// mode 2 : Select one victim from log block list 
int64_t DO_MERGE_OR_SWITCH(int core_id, int mode){

}
*/
//Jieun
// -1: Log block matching & Log block is full
// 2: No matching

int GET_LOG_BLOCK_MAPPING_INFO(int core_id, int64_t lbn)
{
	int i;

	for(i=0;i<N_LOG_BLOCK;i++){
		
		if(lbn == log_block_mapping_table[core_id][i].lbn){
			return i;
		}
	}
	return -1;
}

ppn_t GET_LOG_PAGE_MAPPING_INFO(int core_id, int index, int lpn)
{
	pbn_t log_pbn = log_block_mapping_table[core_id][index].pbn;
	int64_t block_ofs = lpn % N_PAGES_PER_BLOCK;
	int64_t page = log_block_mapping_table[core_id][index].log_mapping_table[block_ofs];
	ppn_t ppn = PBN_TO_PPN(log_pbn, page);
	return ppn;

}
//Jieun
int CHECK_LOG_AREA_FULL(int core_id){
	int empty_index;
	empty_index = GET_EMPTY_INDEX(core_id);
	if(empty_index != -1){
		//printf("empty index:%d\n", empty_index);
		return empty_index;
	}
	else{
		return -1;
	}
}

//Jieun
int SELECT_VICTIM(int core_id, int select){
	if(select == -1){
		int i;
		for(i=0;i<N_LOG_BLOCK;i++){
			if(log_block_mapping_table[core_id][i].is_switch == 1)
			{	
				//printf("Switch available log block:%d\n", i);
				return i; //switch
			}
		}
		return -1;
	}
	else{

		if(log_block_mapping_table[core_id][select].is_switch == 1)
		{
			return 1; //switch
		}
		return 0;
	}
}

//Jieun
int CHECK_LOG_BLOCK_FULL(int core_id, int table_index){

	log_block_entry log_block = log_block_mapping_table[core_id][table_index];
	if(log_block.w_index == N_PAGES_PER_BLOCK){
		return -1;
	}
	else{
		return 0;
	}
}
void PRINT_LOG_TABLE(int core_id, int table_index){
	int k;
	for(k=0;k<N_PAGES_PER_BLOCK;k++){
		printf("log_page_mapping_table[logical block offset:%d] : %d\n",k, log_block_mapping_table[core_id][table_index].log_mapping_table[k]);
	}
}
//Jieun
int GET_EMPTY_INDEX(int core_id){
	int i;
	for(i=0;i<N_LOG_BLOCK;i++){
		if(log_block_mapping_table[core_id][i].w_index == 0){
			return i;
		}
	}
	return -1;
}
//Jieun
int UPDATE_NEW_LOG_PAGE_MAPPING(int core_id, int64_t index, int64_t lpn)
{
	int64_t block_ofs = lpn % N_PAGES_PER_BLOCK;

	uint32_t new_ppn = log_block_mapping_table[core_id][index].w_index;
	int64_t last_lpn = log_block_mapping_table[core_id][index].last_lpn;

	UPDATE_BLOCK_STATE_ENTRY(core_id, log_block_mapping_table[core_id][index].pbn, new_ppn, VALID); //Update valid array
//	CHECK_LOG_BITMAP(core_id, log_block_mapping_table[core_id][index].pbn, new_ppn);
	//For debugging
//	int valid_pages;
//	valid_pages = CHECK_VALID_PAGES(log_block_mapping_table[core_id][index].pbn);
//	printf("Valid pages in Log block:%d\n", valid_pages);
	log_block_mapping_table[core_id][index].log_mapping_table[block_ofs] = new_ppn;
	log_block_mapping_table[core_id][index].lbn = lpn / N_PAGES_PER_BLOCK;

	if(last_lpn - block_ofs != -1 || last_lpn == -3){
		log_block_mapping_table[core_id][index].last_lpn = -3;
		log_block_mapping_table[core_id][index].is_switch = 0;

	}
	else if(last_lpn != -3){
		if(last_lpn - block_ofs == -1){
			log_block_mapping_table[core_id][index].is_switch = 1;
			log_block_mapping_table[core_id][index].last_lpn = block_ofs;
		}
	}
	
	log_block_mapping_table[core_id][index].inverse_log_mapping_table[new_ppn] = block_ofs;
	log_block_mapping_table[core_id][index].w_index++;

	return SUCCESS;
}

//Jieun
//For debugging
void CHECK_LOG_ENTRY(int core_id, int64_t index, int64_t lpn)
{
	int64_t block_ofs = lpn % N_PAGES_PER_BLOCK;
	uint32_t new_ppn = log_block_mapping_table[core_id][index].w_index;
	int64_t last_lpn = log_block_mapping_table[core_id][index].last_lpn;

	printf("windex:%d last lpn:%d switch:%d lbn:%d\n", log_block_mapping_table[core_id][index].w_index, log_block_mapping_table[core_id][index].last_lpn, log_block_mapping_table[core_id][index].is_switch, log_block_mapping_table[core_id][index].lbn);
	
	printf("log_mapping_table[block_ofs:%d]: %d!\t", block_ofs, log_block_mapping_table[core_id][index].log_mapping_table[block_ofs]);
//	printf("inverse log_mapping_table[ppn:%d]: %d!\n", new_ppn, log_block_mapping_table[core_id][index].inverse_log_mapping_table[new_ppn]);
}

ppn_t GET_MAPPING_INFO(int core_id, int64_t lpn)
{
	ppn_t ppn = mapping_table[core_id][lpn];

	return ppn;
}

//Jieun add
int INSERT_LOG_BLOCK(int core_id, block_entry* new_log_block)
{
	block_list* cur_log_block_list = log_block_list[core_id];

	new_log_block->prev = NULL;
	new_log_block->next = NULL;

	/* Update log block list */
	if(cur_log_block_list->n_blocks == 0){
		cur_log_block_list->head = new_log_block;
		cur_log_block_list->tail = new_log_block;
	}
	else{
		cur_log_block_list->tail->next = new_log_block;
		new_log_block->prev = cur_log_block_list->tail;
		cur_log_block_list->tail = new_log_block;
	}

	/* Update the total number of victim blocks */
	cur_log_block_list->n_blocks++;
	n_total_log_blocks[core_id]++;
	return SUCCESS;
}

//Jieun add
void CHECK_LOG_BLOCK_LIST(int core_id)
{
	block_list* cur_log_block_list = log_block_list[core_id];
	if(cur_log_block_list -> head == NULL){
		printf("Log block list is empty!\n");
		printf("Core id:%d Total log blocks:%d\n", core_id, n_total_log_blocks[core_id]);
		return;
	}
	int i=0;
	block_entry* cur_log_block = cur_log_block_list->head;
	for(i=0;i<n_total_log_blocks[core_id];i++){
		printf("index:%d cur_block_entry flash%d plane:%d block%d\n", i,cur_log_block->pbn.path.flash, cur_log_block->pbn.path.plane, cur_log_block->pbn.path.block); 
		cur_log_block = cur_log_block->next;
	}
	printf("Core id:%d Total log blocks:%d\n", core_id, n_total_log_blocks[core_id]);
}
//Jieun add
int SEARCH_LOG_BLOCK(int core_id, pbn_t pbn)
{
	int i;
	for(i=0;i<N_LOG_BLOCK;i++){
		if(log_block_mapping_table[core_id][i].pbn.path.flash == pbn.path.flash && log_block_mapping_table[core_id][i].pbn.path.plane == pbn.path.plane \
				&& log_block_mapping_table[core_id][i].pbn.path.block == pbn.path.block){
			//printf("log head flash:%d, plane:%d, block:%d\n", pbn.path.flash, pbn.path.plane, pbn.path.block);
			return i;
		}
	}
	return -1;
}

block_entry* GET_VICTIM_LOG_BLOCK(int core_id, int index)
{
	if(index == -1){
		block_entry* victim_entry = log_block_list[core_id]->head;
		//printf("Victim block is head of the log block list\n");
		return victim_entry;
	}
	else{
		pbn_t pbn = log_block_mapping_table[core_id][index].pbn;

		block_entry* temp_entry;
		temp_entry = log_block_list[core_id]->head;

		int i;
		for(i=0;i<n_total_log_blocks[core_id];i++){
			if(temp_entry->pbn.path.flash == pbn.path.flash && temp_entry->pbn.path.plane == pbn.path.plane && temp_entry->pbn.path.block == pbn.path.block){
				//printf("Victim block's pbn:flash%d plane:%d block:%d\n", temp_entry->pbn.path.flash, temp_entry->pbn.path.plane, temp_entry->pbn.path.block);
				return temp_entry;
			}
			temp_entry = temp_entry->next;
		}
		printf("Something wrong!\n");
		return -1;
	}
}


//Jieun add
int POP_LOG_BLOCK(int core_id, block_entry* log_block)
{
	block_list* cur_log_block_list = log_block_list[core_id];

	if(cur_log_block_list -> n_blocks == 1){
		cur_log_block_list->head = NULL;
		cur_log_block_list->tail = NULL;
	}
	else if(log_block == cur_log_block_list->head){
		cur_log_block_list -> head = log_block -> next;
		cur_log_block_list -> head -> prev = NULL;
	}
	else if(log_block == cur_log_block_list -> tail){
		cur_log_block_list -> tail = log_block->prev;
		cur_log_block_list -> tail-> next = NULL;
	}
	else{
		log_block -> prev -> next = log_block -> next;
		log_block -> next-> prev = log_block -> prev;
	}
	log_block->prev=NULL;
	log_block->next=NULL;

	cur_log_block_list->n_blocks--;
	n_total_log_blocks[core_id]--;
	return SUCCESS;
}


//Jieun add
int UPDATE_OLD_LOG_PAGE_MAPPING(int core_id, int index, int64_t lpn){
//	log_block_entry log_block = log_block_mapping_table[core_id][index];
	int64_t block_ofs = lpn % N_PAGES_PER_BLOCK;
	
	if(log_block_mapping_table[core_id][index].log_mapping_table[block_ofs] == -1){
		return SUCCESS;
	}
	else{
		int64_t old_ppn = log_block_mapping_table[core_id][index].log_mapping_table[block_ofs];
		UPDATE_BLOCK_STATE_ENTRY(core_id, log_block_mapping_table[core_id][index].pbn, old_ppn, INVALID);
		
		//printf("Check old valid array!\n");
		//CHECK_LOG_BITMAP(core_id, log_block_mapping_table[core_id][index].pbn, old_ppn);
		
		log_block_mapping_table[core_id][index].inverse_log_mapping_table[old_ppn] = -1;
		return SUCCESS;
	}
}

//Jieun add
int GET_NEW_LOG_PAGE(int core_id, pbn_t index, int mode, ppn_t* ppn, int table_index, int64_t lpn, int is_new)
{
	if(is_new == 1){
		block_entry* log_block;

		pthread_mutex_lock(&get_free_page_lock[core_id]);
		log_block = GET_EMPTY_BLOCK(core_id, index, mode);
		if(log_block == NULL){
			printf("ERROR[%s] Get log block fail\n", __FUNCTION__);
			exit(0);
			pthread_mutex_unlock(&get_free_page_lock[core_id]);
			return FAIL;
		}

		*ppn = PBN_TO_PPN(log_block->pbn, log_block->w_index);
		POP_EMPTY_BLOCK(core_id, log_block);
		
		INSERT_LOG_BLOCK(core_id, log_block);

		pthread_mutex_unlock(&get_free_page_lock[core_id]);
		//Update log block entry
		log_block_mapping_table[core_id][table_index].pbn = log_block->pbn;
		log_block_mapping_table[core_id][table_index].pbn.path.flash = log_block->pbn.path.flash;
		log_block_mapping_table[core_id][table_index].pbn.path.plane = log_block->pbn.path.plane;
		log_block_mapping_table[core_id][table_index].pbn.path.block = log_block->pbn.path.block;
//		log_block_mapping_table[core_id][table_index].lbn = lpn / N_PAGES_PER_BLOCK;
		
		return SUCCESS;
	}

	else{
		
		int64_t w_index = log_block_mapping_table[core_id][table_index].w_index;
		//printf("Next free page of log block : %d\n", w_index);
		pbn_t log_pbn;
		log_pbn.path.flash = log_block_mapping_table[core_id][table_index].pbn.path.flash;
		log_pbn.path.plane = log_block_mapping_table[core_id][table_index].pbn.path.plane;
		log_pbn.path.block = log_block_mapping_table[core_id][table_index].pbn.path.block;
		*ppn = PBN_TO_PPN(log_pbn, w_index);
		return SUCCESS;
	}
}

int GET_NEW_PAGE(int core_id, pbn_t index, int mode, ppn_t* ppn, int for_gc)
{
	block_entry* empty_block;

	/* Get free page lock */
	pthread_mutex_lock(&get_free_page_lock[core_id]);

	/* Get empty block from the flash list of the core */
	empty_block = GET_EMPTY_BLOCK(core_id, index, mode);
	if(empty_block == NULL){
		printf("ERROR[%s] Get empty block fail\n", __FUNCTION__);

		/* Release get free page lock */
		pthread_mutex_unlock(&get_free_page_lock[core_id]);
		return FAIL;
	}
	/* Calculate the ppn from the empty block */
	*ppn = PBN_TO_PPN(empty_block->pbn, empty_block->w_index);

	/* Increase current write index of the empty block */
	empty_block->w_index++;

	/* If the empty block is full, insert it to victim block list */
	if(empty_block->w_index == N_PAGES_PER_BLOCK){
		POP_EMPTY_BLOCK(core_id, empty_block);
		INSERT_VICTIM_BLOCK(core_id, empty_block);

		/* Check whether the plane need to GC */
		if(for_gc == 0)
			CHECK_EMPTY_BLOCKS(core_id, empty_block->pbn);
	}

	/* Release get free page lock */
	pthread_mutex_unlock(&get_free_page_lock[core_id]);

	return SUCCESS;
}
int GET_NEW_BLOCK(int core_id, pbn_t index, int mode, ppn_t* ppn, int64_t block_ofs)
{
	block_entry* empty_block;	
	pthread_mutex_lock(&get_free_page_lock[core_id]);
	empty_block = GET_EMPTY_BLOCK(core_id, index, mode);
	POP_EMPTY_BLOCK(core_id, empty_block);
	//printf("Allocated block flash:%d, plane:%d, block:%d\n", empty_block->pbn.path.flash, empty_block->pbn.path.plane, empty_block->pbn.path.block);
	INSERT_DATA_BLOCK(core_id, empty_block);
	//CHECK_DATA_BLOCK(core_id, empty_block);

	pthread_mutex_unlock(&get_free_page_lock[core_id]);
	*ppn = PBN_TO_PPN(empty_block->pbn, block_ofs);
}

//Jieun
int UPDATE_OLD_BLOCK_MAPPING(int core_id, int owner_core_id, int64_t lpn)
{
	ppn_t old_ppn;
	pbn_t old_pbn;
	block_state_entry* bs_entry;
	int64_t block_ofs;
	int64_t lbn;

	lbn = lpn / N_PAGES_PER_BLOCK;
	block_ofs = lpn % N_PAGES_PER_BLOCK;

	old_pbn = GET_DATA_MAPPING_INFO(owner_core_id, lbn);
	
	if(old_pbn.addr == -1){
		return SUCCESS;
	}
	else{
		printf("Something wrong!\n");
		exit(0);
	}
//	return SUCCESS;
}



int UPDATE_OLD_PAGE_MAPPING(int core_id, int owner_core_id, int64_t lpn)
{
	ppn_t old_ppn;
	pbn_t old_pbn;
	block_state_entry* bs_entry;

	old_ppn = GET_MAPPING_INFO(owner_core_id, lpn);

	if(old_ppn.addr == -1){
		return SUCCESS;
	}
	else{
		old_pbn = PPN_TO_PBN(old_ppn);

		bs_entry = GET_BLOCK_STATE_ENTRY(old_pbn);
		if(bs_entry->core_id == -1 || bs_entry->core_id == core_id){

			UPDATE_BLOCK_STATE_ENTRY(core_id, old_pbn, old_ppn.path.page, INVALID);
			UPDATE_INVERSE_MAPPING(old_ppn, -1);
		}

#ifdef FTL_DEBUG
		printf("[%s] %d-core, old pbn f %d p %d b %d p %d (n_valid: %d, C: %d)\n",
			__FUNCTION__, core_id, old_pbn.path.flash, old_pbn.path.plane,
			old_pbn.path.block, old_ppn.path.page, bs_entry->n_valid_pages, 
			COUNT_BLOCK_STATE_ENTRY(old_pbn));
#endif
	}

	return SUCCESS;
}


//Jieun

int UPDATE_NEW_BLOCK_MAPPING(int core_id, int64_t lpn, pbn_t pbn)
{
	ppn_t ppn;
	int64_t lbn;
	int64_t block_ofs;
	
	lbn = lpn / N_PAGES_PER_BLOCK;
	block_ofs = lpn % N_PAGES_PER_BLOCK;
	
	/* Update DATA Mapping Table */
	data_mapping_table[core_id][lbn] = pbn;

	UPDATE_BLOCK_STATE_ENTRY(core_id, pbn, block_ofs, VALID);
	UPDATE_BLOCK_STATE(core_id, pbn, DATA_BLOCK);

	return SUCCESS;
}

int CHECK_TABLE(int core_id, int64_t lpn)
{
	//pbn_t pbn;
	ppn_t ppn;
	int64_t lbn;
	int ret;
	int64_t block_ofs;

	lbn = lpn / N_PAGES_PER_BLOCK;
	block_ofs = lpn % N_PAGES_PER_BLOCK;
	pbn_t pbn = GET_DATA_MAPPING_INFO(core_id, lbn);
	ppn = PBN_TO_PPN(pbn, block_ofs);

	//For debugging 
	printf("Core id : %d CHECK TABLE :: (flash,plane,block,page):(%d,%d,%d,%d)\n",core_id, pbn.path.flash, pbn.path.plane, pbn.path.block, ppn.path.page);
	
	return SUCCESS;
}

int CHECK_BITMAP(int core_id, pbn_t pbn, int64_t lpn)
{
	int ret;
	int64_t block_ofs;
	int64_t lbn;
	block_ofs = lpn % N_PAGES_PER_BLOCK;
	
	ppn_t ppn;
	ppn = PBN_TO_PPN(pbn, block_ofs);
	lbn = lpn / N_PAGES_PER_BLOCK;

	block_state_entry* bs_entry = GET_BLOCK_STATE_ENTRY(pbn);
	bitmap_t valid_array = bs_entry->valid_array;
		
	ret = TEST_BITMAP_MASK(valid_array, ppn.path.page);
	

//	printf("Core id: %d CHECK BITMAP :: (lbn,lpn,page,valid) : %d,%d,%d,%d\n", core_id, lbn, lpn, ppn.path.page, ret);
	return ret;
}
//Jieun for debugging
int CHECK_EMPTY_BLOCK_BITMAP(int core_id, bitmap_t valid_array)
{
	int ret;
//	int64_t block_ofs;
//	int64_t lbn;
//	block_ofs = lpn % N_PAGES_PER_BLOCK;
//	ppn_t ppn;
//	ppn = PBN_TO_PPN(pbn, block_ofs);
//	lbn = lpn / N_PAGES_PER_BLOCK;
	ret = 1;
	int i;
	for(i=0;i<N_PAGES_PER_BLOCK;i++){
		if(TEST_BITMAP_MASK(valid_array, i) == 1){
			ret = 0;
		}
	}
	return ret;
}

int UPDATE_NEW_PAGE_MAPPING(int core_id, int64_t lpn, ppn_t ppn)
{
	pbn_t pbn;

	/* Update Page Mapping Table */
	mapping_table[core_id][lpn] = ppn;

	/* Update Inverse Page Mapping Table */
	pbn = PPN_TO_PBN(ppn);
	
	//For debugging 
	//block_state_entry* bs_entry = GET_BLOCK_STATE_ENTRY(pbn);

	//bitmap_t valid_array = bs_entry->valid_array;
	//int ret = TEST_BITMAP_MASK(valid_array, ppn.path.page);
	//printf("i = %d, Before update :: page : %d\t Valid State : %d\n",debug_index,  ppn.path.page, ret);
#ifdef FTL_DEBUG
	block_state_entry* bs_entry = GET_BLOCK_STATE_ENTRY(pbn);
#endif
	
	UPDATE_BLOCK_STATE_ENTRY(core_id, pbn, ppn.path.page, VALID);

	//ret = TEST_BITMAP_MASK(valid_array, ppn.path.page);
	//printf("i = %d, After update :: page : %d\t Valid State : %d\n",debug_index,  ppn.path.page, ret);

	//debug_index+=1;

	UPDATE_BLOCK_STATE(core_id, pbn, DATA_BLOCK);
	UPDATE_INVERSE_MAPPING(ppn, lpn);

#ifdef FTL_DEBUG
	printf("[%s] 2. %d-core, new pbn f %d p %d b %d p %d (n_valid: %d, C: %d)\n",
		__FUNCTION__, core_id, pbn.path.flash, pbn.path.plane,
		pbn.path.block, ppn.path.page, bs_entry->n_valid_pages, 
		COUNT_BLOCK_STATE_ENTRY(pbn));
#endif

	return SUCCESS;
}

int PARTIAL_UPDATE_PAGE_MAPPING(int core_id, int owner_core_id, int64_t lpn, ppn_t new_ppn,
		ppn_t old_ppn, uint32_t left_skip, uint32_t right_skip)
{
	uint32_t offset = left_skip / SECTORS_PER_4K_PAGE;
	uint32_t length = SECTORS_PER_PAGE - left_skip - right_skip;

	pbn_t dst_pbn = PPN_TO_PBN(new_ppn);
	pbn_t src_pbn = PPN_TO_PBN(old_ppn);

	block_state_entry* dst_bs_entry = GET_BLOCK_STATE_ENTRY(dst_pbn);	
	block_state_entry* src_bs_entry = GET_BLOCK_STATE_ENTRY(src_pbn);

	uint32_t dst_index = new_ppn.path.page;  
	uint32_t src_index = old_ppn.path.page;

	/* Get lock of the dst block state entry */
	pthread_mutex_lock(&dst_bs_entry->lock);

	dst_bs_entry->core_id = core_id;


	/* Copy bitmap info from src page to dst page */
	COPY_BITMAP_MASK(dst_bs_entry->valid_array, dst_index,
			src_bs_entry->valid_array, src_index);

	/* Validate sectors from left_skip to rigth skip */
	if(left_skip != 0 || right_skip != 0){
		while(length > 0){
			// For debug
			SET_BITMAP(dst_bs_entry->valid_array, dst_index * N_4K_PAGES + offset);

			length -=  (SECTORS_PER_4K_PAGE - left_skip % SECTORS_PER_4K_PAGE);
			left_skip = 0;
			offset++;
		}
	}

	/* Invalidate old ppn */
	UPDATE_OLD_PAGE_MAPPING(core_id, owner_core_id, lpn);

	/* Update Mapping Table */
	mapping_table[owner_core_id][lpn] = new_ppn;
	UPDATE_INVERSE_MAPPING(new_ppn, lpn);

	/* Update the number of valid pages */
	dst_bs_entry->n_valid_pages++;

#ifdef FTL_DEBUG
	printf("[%s] 2. %d-core: %d %d %d %d <- %d %d %d dst n_valid: %d (%d), src_n_valid: %d (%d)\n",
		__FUNCTION__, core_id, dst_pbn.path.flash, dst_pbn.path.plane, 
		dst_pbn.path.block, dst_index,
		src_pbn.path.flash, src_pbn.path.plane, src_pbn.path.block,
		dst_bs_entry->n_valid_pages, COUNT_BLOCK_STATE_ENTRY(dst_pbn), 
		src_bs_entry->n_valid_pages, COUNT_BLOCK_STATE_ENTRY(src_pbn));
#endif

	/* Update dst block state entry and release the lock */
	dst_bs_entry->core_id = -1;
	pthread_mutex_unlock(&dst_bs_entry->lock);

	return SUCCESS;
}

int CHECK_LOG_BITMAP(int core_id, pbn_t pbn, int64_t page)
{
	int ret;

	//For debugging 
	block_state_entry* bs_entry = GET_BLOCK_STATE_ENTRY(pbn);
	bitmap_t valid_array = bs_entry->valid_array;
		
	ret = TEST_BITMAP_MASK(valid_array, page);

//	printf("Core id: %d CHECK BITMAP :: (page,valid) : %d,%d\n", core_id, page, ret);
	return ret;
}
//Jieun add
int CHECK_VALID_PAGES(pbn_t pbn)
{
	int n_valid_pages = 0;
	block_state_entry* bs_entry = GET_BLOCK_STATE_ENTRY(pbn);
	bitmap_t valid_array = bs_entry->valid_array;
		
	int i;
	for(i=0;i<N_PAGES_PER_BLOCK;i++){
		if(TEST_BITMAP_MASK(valid_array, i) == 1){
			n_valid_pages++;
		}
	}

	return n_valid_pages;
}


//Jieun
int DO_SWITCH(int core_id, block_entry* log_entry, int index){
	
	int64_t lbn = log_block_mapping_table[core_id][index].lbn;
	//printf("Mapping lbn:%d\n", lbn);

	pbn_t data_block_pbn = GET_DATA_MAPPING_INFO(core_id, lbn);
	//printf("get data block pbn:flash:%d plane:%d block:%d\n", data_block_pbn.path.flash, data_block_pbn.path.plane, data_block_pbn.path.block);
	
	block_entry* data_entry = SEARCH_DATA_BLOCK(core_id, data_block_pbn);
	//printf("Search data block entry done!\n flash:%d plane:%d block:%d\n", data_entry->pbn.path.flash, data_entry->pbn.path.plane, data_entry->pbn.path.block);
	
	//Erase data block
	FLASH_BLOCK_ERASE(core_id, data_block_pbn);
	WAIT_FLASH_IO(core_id, BLOCK_ERASE, 1);

	//printf("Erase data block\n");

	data_mapping_table[core_id][lbn] = log_entry->pbn;		

	POP_LOG_BLOCK(core_id, log_entry);
	//printf("Pop from log block list!\n");

	INSERT_DATA_BLOCK(core_id, log_entry);
	//printf("Insert to data block list success!\n");
	
	POP_DATA_BLOCK(core_id, data_entry);
	//printf("Pop from data block list!\n");
	
	UPDATE_BLOCK_STATE(core_id, data_block_pbn, EMPTY_BLOCK);
	//printf("Check block bitmap is all invalid\n");
	
	pthread_mutex_lock(&get_free_page_lock[core_id]);
	INSERT_EMPTY_BLOCK(core_id, data_entry);
	pthread_mutex_unlock(&get_free_page_lock[core_id]);
	
	//printf("Update block to invalid & Insert to empty block list success!\n");
	ERASE_LOG_BLOCK_ENTRY(core_id, index);

	return 0;
}
//Jieun
int DO_MERGE(int core_id, block_entry* victim_entry, int index){
	
	bitmap_t victim_valid_array;
	bitmap_t data_valid_array;
	block_state_entry* victim_bs_entry;
	block_state_entry* data_bs_entry;

	pbn_t victim_pbn = victim_entry -> pbn;

	if(index == -1){
		index = SEARCH_LOG_BLOCK(core_id, victim_pbn);
		//printf("Victim's log block table index:%d\n", index);

		if(index==-1){
			//printf("No log block in log block list!\n");
			exit(0);
		}
	}

	int64_t lbn = log_block_mapping_table[core_id][index].lbn;
	//printf("Mapping lbn:%d\n", lbn);

	pbn_t data_block_pbn = GET_DATA_MAPPING_INFO(core_id, lbn);
	//printf("get data block pbn:flash:%d plane:%d block:%d\n", data_block_pbn.path.flash, data_block_pbn.path.plane, data_block_pbn.path.block);

	victim_bs_entry = GET_BLOCK_STATE_ENTRY(victim_pbn);
	data_bs_entry = GET_BLOCK_STATE_ENTRY(data_block_pbn);
	block_entry* data_entry = SEARCH_DATA_BLOCK(core_id, data_block_pbn);
	//printf("Search data block entry done!\n flash:%d plane:%d block:%d\n", data_entry->pbn.path.flash, data_entry->pbn.path.plane, data_entry->pbn.path.block);

	victim_valid_array = victim_bs_entry->valid_array;
	data_valid_array = data_bs_entry->valid_array;

	//Allocate new data block.

	pthread_mutex_lock(&get_free_page_lock[core_id]);
	block_entry* empty_block = GET_EMPTY_BLOCK(core_id, victim_pbn, MODE_INFLASH);
	//printf("Allocate new data block!\n");
	POP_EMPTY_BLOCK(core_id, empty_block);
	
	//printf("Pop from empty block list success!\n");
	INSERT_DATA_BLOCK(core_id, empty_block);
	
	pthread_mutex_unlock(&get_free_page_lock[core_id]);
	pbn_t empty_pbn = empty_block -> pbn;

	int64_t block_ofs;
	ppn_t old_ppn;
	ppn_t new_ppn;
	//int valid_pages = CHECK_VALID_PAGES(victim_pbn);
	int log_copy_pages=0;
	int data_copy_pages=0;
	int i;
	for(i=0;i<N_PAGES_PER_BLOCK;i++){
		if(TEST_BITMAP_MASK(victim_valid_array, i)){
			block_ofs =	log_block_mapping_table[core_id][index].inverse_log_mapping_table[i];
			if(block_ofs == -1){
				//printf("block offset is -1!!\n");
				return;
			}
//			printf("Valid page:: block offset:%d page: %d\n", block_ofs, i);	
			old_ppn = PBN_TO_PPN(victim_pbn, i);
			new_ppn = PBN_TO_PPN(empty_pbn, block_ofs);

		//	FLASH_PAGE_READ(core_id, old_ppn);
		//	printf("FLASH page read done!\n");
		//	WAIT_FLASH_IO(core_id, READ, 1);
			
		//	FLASH_PAGE_WRITE(core_id, new_ppn); 
		//	printf("FLASH page write done!\n");
		//	WAIT_FLASH_IO(core_id, WRITE, 1);
			
//			printf("Start to update new block state!\n");
		
			FLASH_PAGE_COPYBACK(core_id, new_ppn, old_ppn);
			WAIT_FLASH_IO(core_id, WRITE, 1);
			UPDATE_BLOCK_STATE_ENTRY(core_id, empty_block->pbn, block_ofs, VALID);
//			printf("Update new block state done!\n");
			
//			printf("Start to update old block state!\n");
			UPDATE_BLOCK_STATE_ENTRY(core_id, data_block_pbn, block_ofs, INVALID);
//			printf("Update old block state done!\n");
			log_copy_pages++;
		}
	}
	
//	printf("Valid pages of log blocks:%d\n", valid_pages);

	for(i=0;i<N_PAGES_PER_BLOCK;i++){
		if(TEST_BITMAP_MASK(data_valid_array, i)){
		//	printf("Data block copy!\n");
			old_ppn = PBN_TO_PPN(data_block_pbn, i);
			new_ppn = PBN_TO_PPN(empty_pbn, i);

			FLASH_PAGE_COPYBACK(core_id, new_ppn, old_ppn);
			WAIT_FLASH_IO(core_id, WRITE, 1);
			
		//	FLASH_PAGE_READ(core_id, old_ppn);
		//	printf("FLASH page read done!\n");
		//	WAIT_FLASH_IO(core_id, READ, 1);
			
		//	FLASH_PAGE_WRITE(core_id, new_ppn); 
		//	printf("FLASH page write done!\n");
		//	WAIT_FLASH_IO(core_id, WRITE, 1);
				
			UPDATE_BLOCK_STATE_ENTRY(core_id, empty_block->pbn, i, VALID);
			data_copy_pages++;
		}
	}

	//int new_valid_pages = CHECK_VALID_PAGES(empty_block->pbn);
	//printf("log copy pages:%d\t data copy pages:%d\t new valid:%d\n",log_copy_pages,data_copy_pages,new_valid_pages);
	data_mapping_table[core_id][lbn] = empty_block->pbn;

	//Erase Log block
	FLASH_BLOCK_ERASE(core_id, victim_pbn);
	WAIT_FLASH_IO(core_id, BLOCK_ERASE, 1);
	
	//Erase old data block
	FLASH_BLOCK_ERASE(core_id, data_block_pbn);
	WAIT_FLASH_IO(core_id, BLOCK_ERASE, 1);
	
	POP_LOG_BLOCK(core_id, victim_entry);
	UPDATE_BLOCK_STATE(core_id, victim_pbn, EMPTY_BLOCK);
	
	pthread_mutex_lock(&get_free_page_lock[core_id]);
	INSERT_EMPTY_BLOCK(core_id, victim_entry);
	pthread_mutex_unlock(&get_free_page_lock[core_id]);
	
	POP_DATA_BLOCK(core_id, data_entry);
	
	UPDATE_BLOCK_STATE(core_id, data_block_pbn, EMPTY_BLOCK);
	
	pthread_mutex_lock(&get_free_page_lock[core_id]);
	INSERT_EMPTY_BLOCK(core_id, data_entry);
	pthread_mutex_unlock(&get_free_page_lock[core_id]);
	
	ERASE_LOG_BLOCK_ENTRY(core_id, index);
	return index;
}
