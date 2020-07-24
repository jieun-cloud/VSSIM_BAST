// File: ftl_mapping_manager.c
// Date: 18-Sep-2017
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2017
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#include "common.h"

ppn_t** mapping_table;
data_block_entry** data_mapping_table;
extern int N_LOG_BLOCK = 2;
log_block_entry** log_block_mapping_table;

pthread_mutex_t* get_free_page_lock;
pthread_mutex_t* get_free_block_lock;
pthread_mutex_t* get_free_log_lock;
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
	data_mapping_table = (data_block_entry**)calloc(sizeof(data_block_entry*), N_IO_CORES);
	for(i=0; i<N_IO_CORES; i++){

		n_total_blocks = vs_core[i].n_total_blocks;

		data_mapping_table[i] = (data_block_entry*)calloc(sizeof(data_block_entry),
				n_total_blocks);
		if(data_mapping_table[i] == NULL){
			printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
			return -1;
		}
	}	

	/* Init get free page lock */
	get_free_block_lock = (pthread_mutex_t*)calloc(sizeof(pthread_mutex_t),
			N_IO_CORES);
	if(get_free_block_lock == NULL){
		printf("ERROR[%s] Create per core free page lock fail!\n", __FUNCTION__);
		return -1;
	}

	else{
		for(i=0; i<N_IO_CORES; i++){
			pthread_mutex_init(&get_free_block_lock[i], NULL);
		}
	}

	/* Initialization Mapping Table */

	int j;
	for(i=0; i<N_IO_CORES; i++){	

		n_total_blocks = vs_core[i].n_total_blocks;

		for(j=0; j<n_total_blocks; j++){
			data_mapping_table[i][j].pbn.addr = -1;
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
		if(log_block_mapping_table[i] == NULL){
			printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
			return -1;
		}
	}	
	// For debug
	printf("Success to create log block mapping table");

	get_free_log_lock = (pthread_mutex_t*)calloc(sizeof(pthread_mutex_t),
			N_IO_CORES);
	if(get_free_log_lock == NULL){
		printf("ERROR[%s] Create per core free page lock fail!\n", __FUNCTION__);
		return -1;
	}

	else{
		for(i=0; i<N_IO_CORES; i++){
			pthread_mutex_init(&get_free_log_lock[i], NULL);
		}
	}

	int j,k;
	for(i=0; i<N_IO_CORES; i++){
		for(j=0; j<N_LOG_BLOCK; j++){
			
			log_block_mapping_table[i][j].pbn.addr = -1;
			log_block_mapping_table[i][j].log_mapping_table=NULL;
			//			log_block_entry log_entry = log_block_mapping_table[i][j];
			//			log_entry.pbn.addr=-1;

		}
	}

	printf("before initialize log block entry !!");
	INIT_LOG_BLOCK_ENTRY();
	//For debug
	
	printf("log block entry initialize success !!");
	return 0;

}

//Jieun add
int INIT_LOG_BLOCK_ENTRY(void){
	int i, j, k;
	for(i=0;i<N_IO_CORES;i++){
		for(j=0; j<N_LOG_BLOCK; j++){

			log_block_mapping_table[i][j].log_mapping_table = (int64_t*)calloc(sizeof(int64_t), N_PAGES_PER_BLOCK);
			if(log_block_mapping_table[i][j].log_mapping_table == NULL){
				printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
				return -1;
			}
		
	

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
			log_block_mapping_table[i][j].last_lpn = -2;
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


//Jieun
int UPDATE_DATA_BLOCK(int core_id, int64_t lbn, pbn_t pbn)
{
	data_block_entry cur_entry = data_mapping_table[core_id][lbn];
	cur_entry -> pbn = pbn;
	return SUCCESS;
}

int UPDATE_DATA_INVERSE_TABLE(int core_id, int64_t lpn, pbn_t pbn)
{
	data_mapping_table[core_id][lbn].inverse_data_mapping_table[ppn.path.page]=lpn;
	
	return SUCCESS;
}

//Jieun

pbn_t GET_DATA_MAPPING_INFO(int core_id, int64_t lbn)
{
	pbn_t pbn = data_mapping_table[core_id][lbn].pbn;

	return pbn;
}

//Jieun


ppn_t GET_MAPPING_INFO(int core_id, int64_t lpn)
{
	ppn_t ppn = mapping_table[core_id][lpn];

	return ppn;
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


//Jieun
int UPDATE_OLD_BLOCK_MAPPING(int core_id, int owner_core_id, int64_t lpn)
{
	ppn_t old_ppn;
	pbn_t old_pbn;
	block_state_entry* bs_entry;
	int64_t block_ofs;
	int64_t lbn;

	lbn = lpn / PAGES_PER_BLOCK;
	block_ofs = lpn % PAGES_PER_BLOCK;

	old_pbn = GET_DATA_MAPPING_INFO(owner_core_id, lbn);
	if(old_pbn.addr == -1){
		return SUCCESS;
	}
	/*
	else{
		//old_pbn = PPN_TO_PBN(old_ppn);
		old_ppn = PBN_TO_PPN(old_pbn, block_ofs);
		bs_entry = GET_BLOCK_STATE_ENTRY(old_pbn);
		if(bs_entry->core_id == -1 || bs_entry->core_id == core_id){

		//	UPDATE_BLOCK_STATE_ENTRY(core_id, old_pbn, block_ofs, INVALID);
			UPDATE_INVERSE_MAPPING(old_ppn, -1);
		}
	}*/
#ifdef FTL_DEBUG
		printf("[%s] %d-core, old pbn f %d p %d b %d p %d (n_valid: %d, C: %d)\n",
			__FUNCTION__, core_id, old_pbn.path.flash, old_pbn.path.plane,
			old_pbn.path.block, old_ppn.path.page, bs_entry->n_valid_pages, 
			COUNT_BLOCK_STATE_ENTRY(old_pbn));
#endif
	

	return SUCCESS;
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
	//pbn_t pbn;
	ppn_t ppn;
	int64_t lbn;
	int64_t block_ofs;
	
	lbn = lpn / PAGES_PER_BLOCK;
	block_ofs = lpn % PAGES_PER_BLOCK;
	
	/* Update DATA Mapping Table */
	data_mapping_table[core_id][lbn] = pbn;

	/* Update Inverse Page Mapping Table */
//	pbn = PPN_TO_PBN(ppn);
	ppn = PBN_TO_PPN(pbn, block_ofs);

#ifdef FTL_DEBUG
	block_state_entry* bs_entry = GET_BLOCK_STATE_ENTRY(pbn);
#endif
	
	UPDATE_BLOCK_STATE_ENTRY(core_id, pbn, block_ofs, VALID);
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

int CHECK_TABLE(int core_id, int64_t lpn)
{
	//pbn_t pbn;
	ppn_t ppn;
	int64_t lbn;
	int ret;
	int64_t block_ofs;

	lbn = lpn / PAGES_PER_BLOCK;
	block_ofs = lpn % PAGES_PER_BLOCK;
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
	block_ofs = lpn % PAGES_PER_BLOCK;
	ppn_t ppn;
	ppn = PBN_TO_PPN(pbn, block_ofs);
	lbn = lpn / PAGES_PER_BLOCK;

	//For debugging 
	block_state_entry* bs_entry = GET_BLOCK_STATE_ENTRY(pbn);
	bitmap_t valid_array = bs_entry->valid_array;
		
	ret = TEST_BITMAP_MASK(valid_array, ppn.path.page);

	printf("Core id: %d CHECK BITMAP :: (lbn,lpn,page,valid) : %d,%d,%d,%d\n", core_id, lbn, lpn, ppn.path.page, ret);
	return SUCCESS;
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
