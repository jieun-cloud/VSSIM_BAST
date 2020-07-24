// File: ftl.c
// Date: 18-Sep-2017
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2017
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#include "common.h"

int g_init = 0;
int g_term = 0;
pthread_mutex_t term_lock = PTHREAD_MUTEX_INITIALIZER;
int PAGES_PER_BLOCK = 256;

FILE* fp_ch_util;
FILE* fp_w_event;
FILE* fp_wb_lat;
FILE* fp_rw_lat;

/* return value of each init function 
 * 	-1: return FAIL
 *	 0: First boot
 *	 1: Initialize from metadata files
 */
void FTL_INIT(void)
{
	int ret; 

	if(g_init == 0){
        	printf("[%s] start\n", __FUNCTION__);

		INIT_SSD_CONFIG();

		ret = INIT_PERF_CHECKER();
		if(ret == -1) goto fail;
	
		INIT_IO_BUFFER();

		ret = INIT_FLASH_INFO(ret);
		if(ret == -1) goto fail;

		INIT_VSSIM_CORE();	/* Init Flash -> Init Core */

		ret = INIT_MAPPING_TABLE(ret); /* Init Core -> Init Mapping */
		//Jieun add
		ret = INIT_DATA_MAPPING_TABLE(ret);
		ret = INIT_LOG_BLOCK_MAPPING_TABLE(ret);
		//For debug
		printf("data,log,log page mapping table init!!: %d", ret);
		if(ret == -1) goto fail;

#ifdef MONITOR_ON
		INIT_LOG_MANAGER();
#endif
		INIT_FLASH();
	
		printf("data mapping table init!!: %d", ret);
		g_init = 1;
		printf("[%s] complete\n", __FUNCTION__);

#ifdef GET_CH_UTIL_INFO
		fp_ch_util = fopen("./ch_util_info.txt", "a");
		if(fp_ch_util == NULL){
			printf("ERROR[%s] open ch_util_info file fail\n", __FUNCTION__);
		}
#endif
#ifdef GET_W_EVENT_INFO
		fp_w_event = fopen("./w_event_info.txt", "a");
		if(fp_w_event == NULL){
			printf("ERROR[%s] open w_event_info file fail\n", __FUNCTION__);
		}
#endif
#ifdef GET_WB_LAT_INFO
		fp_wb_lat = fopen("./wb_lat_info.txt", "a");
		if(fp_wb_lat == NULL){
			printf("ERROR[%s] open wb_lat_info file fail\n", __FUNCTION__);
		}
#endif
#ifdef GET_RW_LAT_INFO
		fp_rw_lat = fopen("./rw_lat_info.txt", "a");
		if(fp_rw_lat == NULL){
			printf("ERROR[%s] open rw_lat_info file fail\n", __FUNCTION__);
		}
#endif

		return;

fail:
		printf("[%s] init fail\n", __FUNCTION__);
		TERM_VSSIM_CORE();
		return;	
	}
}

void FTL_TERM(void)
{
	pthread_mutex_lock(&term_lock);

	if(g_term == 0){
		g_term = 1;

		printf("[%s] start\n", __FUNCTION__);
		WAIT_VSSIM_CORE_EXIT();

		TERM_IO_BUFFER();

		TERM_MAPPING_TABLE(); /* Term mapping -> Term core */

		TERM_VSSIM_CORE();
		TERM_FLASH_INFO();

		TERM_PERF_CHECKER();

#ifdef MONITOR_ON
		TERM_LOG_MANAGER();
#endif

		TERM_FLASH();

		printf("[%s] complete\n", __FUNCTION__);
	}
	pthread_mutex_unlock(&term_lock);

#ifdef GET_CH_UTIL_INFO
	fclose(fp_ch_util);
#endif
#ifdef GET_W_EVENT_INFO
	fclose(fp_w_event);
#endif
#ifdef GET_WB_LAT_INFO
	fclose(fp_wb_lat);
#endif
#ifdef GET_RW_LAT_INFO
	fclose(fp_rw_lat);
#endif

	return;
}

int FTL_READ(int core_id, uint64_t sector_nb, uint32_t length)
{
	int ret;

	ret = _FTL_READ(core_id, sector_nb, length);
	if(ret == FAIL)
		printf("ERROR[%s] _FTL_READ function returns FAIL\n", __FUNCTION__);		

	return ret;
}

int FTL_WRITE(int core_id, uint64_t sector_nb, uint32_t length)
{
	int n_pages;

	n_pages = _FTL_WRITE(core_id, sector_nb, length);
	if(n_pages == -1)
		printf("ERROR[%s] _FTL_WRITE function returns FAIL\n", __FUNCTION__);		

	/* If needed, perform foreground GC */

	return n_pages;
}

void FTL_DISCARD(int core_id, uint64_t sector_nb, uint32_t length)
{
	if(sector_nb + length > N_SECTORS){
		printf("ERROR[%s] Exceed Sector number\n", __FUNCTION__);
                return;
        }

#ifdef FTL_DEBUG
	printf("[%s] %d-core: Discard is called!\n", __FUNCTION__, core_id);
#endif

	uint64_t lba = sector_nb;
	int64_t lpn;
	int64_t lpn_4k;
	ppn_t ppn;
	pbn_t pbn;
	block_state_entry* bs_entry = NULL;
	uint32_t bitmap_index;

	uint32_t remain = length;
	uint32_t left_skip = sector_nb % SECTORS_PER_PAGE;

	int ret = FAIL;

	if(left_skip != 0 || (length % SECTORS_PER_4K_PAGE != 0)){
		printf("ERROR[%s] sector_nb: %lu, length: %u\n",
			__FUNCTION__, sector_nb, length);
		return;
	}

	while(remain > 0){

		/* Get the logical page number */
		lpn = lba / (int64_t)SECTORS_PER_PAGE;
		lpn_4k = lba / (int64_t)SECTORS_PER_4K_PAGE;
		
		/* Get the physical page number from the mapping table */
		ppn = GET_MAPPING_INFO(core_id, lpn);

		/* Get the block state entry of the ppn */
		pbn = PPN_TO_PBN(ppn);
		bs_entry = GET_BLOCK_STATE_ENTRY(pbn);	

		/* Update bitmap */
		bitmap_index = (uint32_t)(lpn_4k % BITMAP_SIZE);

		ret = CLEAR_BITMAP(bs_entry->valid_array, bitmap_index);
		if(ret == FAIL){
			return;
		}

		if(!TEST_BITMAP_MASK(bs_entry->valid_array, ppn.path.page)){
			bs_entry->n_valid_pages--;
		}

		lba += SECTORS_PER_4K_PAGE;
		remain -= SECTORS_PER_4K_PAGE;
		left_skip = 0;
	}

	return;
}

int _FTL_READ(int core_id, uint64_t sector_nb, uint32_t length)
{
#ifdef FTL_DEBUG
	printf("[%s] %d core: Start read %lu sector, %u length\n", 
			__FUNCTION__, core_id, sector_nb, length);
#endif

	if(sector_nb + length > N_SECTORS){
		printf("Error[%s] Exceed Sector number\n", __FUNCTION__); 
		return FAIL;	
	}

	int64_t lpn;
	ppn_t ppn;
	uint64_t lba = sector_nb;
	uint32_t remain = length;
	uint32_t left_skip = sector_nb % SECTORS_PER_PAGE;
	uint32_t right_skip = 0;
	uint32_t read_sects;
	uint32_t n_trimmed_pages = 0;

	int n_pages = 0;
	int n_read_pages = 0;

	void* ret_buf = NULL;

#ifdef GET_CH_UTIL_INFO
	int n_ch_util = 0;	
	double ch_util = 0;	
#endif

	while(remain > 0){

		if(remain > SECTORS_PER_PAGE - left_skip){
			right_skip = 0;
		}
		else{
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		}
		read_sects = SECTORS_PER_PAGE - left_skip - right_skip;

		lpn = lba / (int64_t)SECTORS_PER_PAGE;

		ret_buf = CHECK_WRITE_BUFFER(core_id, lba, read_sects);

		if(ret_buf != NULL){
			/* Hit Write Buffer */	
		}
		else {
			/* Check Mapping Table */
			ppn = GET_MAPPING_INFO(core_id, lpn);

			if(ppn.addr != -1){			
				/* Read data from NAND page */
				FLASH_PAGE_READ(core_id, ppn);

				n_read_pages++;
			}
			else{
				/* Trimmed pages  */
				n_trimmed_pages++;
			}
		}

		n_pages++;

		ret_buf = NULL;
		lba += read_sects;
		remain -= read_sects;
		left_skip = 0;
	}

	/* Wait until all flash io are completed */
	WAIT_FLASH_IO(core_id, READ, n_read_pages);

#ifdef GET_CH_UTIL_INFO
	if(n_pages > vs_core[core_id].n_flash)
		n_ch_util = vs_core[core_id].n_flash;
	else
		n_ch_util = n_pages;

	ch_util = (double) n_ch_util / vs_core[core_id].n_flash;

	fprintf(fp_ch_util, "R\t%d\t%d\t%d\t%lf\n", core_id, n_ch_util, n_pages, ch_util);
#endif

#ifdef FTL_DEBUG
	printf("[%s] Complete\n", __FUNCTION__);
#endif

	/* If thie read request is for trimmed data, mark it to the core req entry */
	if(n_pages == n_trimmed_pages){
		return TRIMMED;
	}

	return SUCCESS;
}

int _FTL_WRITE(int core_id, uint64_t sector_nb, uint32_t length)
{
#ifdef FTL_DEBUG
	printf("[%s] %d core: Start write %lu sector, %u length\n", 
			__FUNCTION__, core_id, sector_nb, length);
#endif

	if(sector_nb + length > N_SECTORS){
		printf("ERROR[%s] Exceed Sector number\n", __FUNCTION__);
                return -1;
        }

	uint64_t lba = sector_nb;
	int64_t lpn;
	ppn_t new_ppn;
	ppn_t old_ppn;
	pbn_t temp_pbn;

	uint32_t remain = length;
	uint32_t left_skip = sector_nb % SECTORS_PER_PAGE;
	uint32_t right_skip = 0;
	uint32_t write_sects;

	int ret = FAIL;
	int n_write_pages = 0;
	temp_pbn.addr = -1;

	// Jieun
	int64_t lbn;
	pbn_t old_pbn;
	ppn_t test_ppn;
	int64_t block_ofs;
	pbn_t old_log_pbn;
	ppn_t old_log_ppn;

	while(remain > 0){

		if(remain > SECTORS_PER_PAGE - left_skip){
			right_skip = 0;
		}
		else{
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		}

		write_sects = SECTORS_PER_PAGE - left_skip - right_skip;

		/*
		ret = GET_NEW_PAGE(core_id, temp_pbn, MODE_OVERALL, &new_ppn, 0);
		if(ret == FAIL){
			printf("ERROR[%s] Get new page fail \n", __FUNCTION__);
			return -1;
		}
		*/
#ifdef FTL_DEBUG
		printf("[%s] %d-core: get new page, f %d p %d b %d p %d (plane state: %d)\n",
				__FUNCTION__, core_id, new_ppn.path.flash,
				new_ppn.path.plane, new_ppn.path.block, 
				new_ppn.path.page,
				flash_i[new_ppn.path.flash].plane_i[new_ppn.path.plane].p_state);
#endif

		lpn = lba / (int64_t)SECTORS_PER_PAGE;

		// Jieun 
		lbn = lpn / PAGES_PER_BLOCK;	
		block_ofs = lpn % PAGES_PER_BLOCK;

		// Block-level mapping table check
		old_pbn = GET_DATA_MAPPING_INFO(core_id, lbn);
		old_ppn_ = PBN_TO_PPN(old_pbn, block_ofs);
		//For debugging
		printf("Core id : %d Start ftl write!! (lbn,lpn,offset):(%d,%d,%d)\n", core_id, lbn,lpn,block_ofs);

		if(old_pbn.addr == -1){
		   //For debugging
			printf("Core id : %d First write to data block! (lbn,lpn,offset):(%d,%d,%d)\n",core_id, lbn,lpn,block_ofs);
			block_entry* empty_block;	
			empty_block = GET_EMPTY_BLOCK(core_id, temp_pbn, MODE_OVERALL);
			
			POP_EMPTY_BLOCK(core_id, empty_block);

			//INSERT_VICTIM_BLOCK(core_id, empty_block);
			
			test_ppn = PBN_TO_PPN(empty_block -> pbn, block_ofs);
			FLASH_PAGE_WRITE(core_id, test_ppn);
			UPDATE_OLD_BLOCK_MAPPING(core_id, core_id, lpn);
			
			
			printf("Core id : %d First write to data block! check valid is 0!! (lbn,lpn,offset):(%d,%d,%d)\n",core_id, lbn,lpn,block_ofs);
			CHECK_BITMAP(core_id, empty_block->pbn, lpn);
			UPDATE_NEW_BLOCK_MAPPING(core_id, lpn, empty_block->pbn);
		    	
		   //For debugging
			printf("Core id : %d First write to data block! After map (lbn,lpn,offset):(%d,%d,%d)\n",core_id,lbn,lpn,block_ofs);		
			CHECK_TABLE(core_id, lpn);

			CHECK_BITMAP(core_id, empty_block->pbn, lpn);
		}
	
		else {
			
		   //For debugging
			printf("Core id : %d Data block mapping exists! (lbn,lpn,offset):(%d,%d,%d)\n",core_id, lbn,lpn,block_ofs);
			
			ret = CHECK_BITMAP(core_id, old_pbn, lpn);
			if(ret == 0){
				//For debugging
				printf("Core id : %d Page is not valid! (lbn,lpn,offset):(%d,%d,%d)\n",core_id, lbn,lpn,block_ofs);
				FLASH_PAGE_WRITE(core_id, old_ppn_);
				UPDATE_BLOCK_STATE_ENTRY(core_id, old_pbn, block_ofs, VALID);
				CHECK_BITMAP(core_id, old_pbn, lpn);
				UPDATE_INVERSE_MAPPING(old_ppn_, lpn);
			}
	
			
			else{
			
				printf("Page is valid! (lbn,lpn,offset):(%d,%d,%d)\n",lbn,lpn,block_ofs);
				/*
				 = GET_LOG_BLOCK_MAPPING_INFO(core_id, lbn);
				
				if(old_log_pbn.addr == -1){

					block_entry* log_block;	
					log_block = GET_EMPTY_BLOCK(core_id, temp_pbn, MODE_OVERALL);
					old_log_ppn = PBN_TO_PPN(log_block -> pbn, log_block -> w_index);

					FLASH_PAGE_WRITE(core_id, test_ppn);
					
					POP_EMPTY_BLOCK(core_id, log_block);
					INSERT_VICTIM_BLOCK(core_id, log_block);
					log_block -> w_index++;
					
					if(ret == FAIL){
						printf("ERROR[%s] Get new page fail \n", __FUNCTION__);
						return -1;
					}

					UPDATE_OLD_LOG_BLOCK_MAPPING(core_id, core_id, lbn);
					printf("First write to data block! check valid is 0!! (lbn,lpn,offset):(%d,%d,%d)\n",lbn,lpn,block_ofs);
					CHECK_BITMAP(core_id, empty_block->pbn, lpn);
					UPDATE_NEW_BLOCK_MAPPING(core_id, lpn, empty_block->pbn);

					//For debugging
					printf("First write to data block! After map (lbn,lpn,offset):(%d,%d,%d)\n",lbn,lpn,block_ofs);
					//			
					CHECK_TABLE(core_id, lpn);

					CHECK_BITMAP(core_id, empty_block->pbn, lpn);
					//	WRITE_TO_LOG_BLOCK(core_id, lpn);
				}
				else{
					
					block_entry* log_block = GET_EMPTY_BLOCK(core_id, temp_pbn, MODE_OVERALL);
					POP_EMPTY_BLOCK(core_id, log_block);
					INSERT_VICTIM_BLOCK(core_id, log_block);
				}
				*/

				FLASH_PAGE_WRITE(core_id, old_ppn_);

			}
			
		}
		/*
		if((left_skip || right_skip) && (old_ppn.addr != -1)){
// TEMP
//			FLASH_PAGE_READ(core_id, old_ppn);
//			WAIT_FLASH_IO(core_id, 1);
			
			FLASH_PAGE_WRITE(core_id, new_ppn);

//			PARTIAL_UPDATE_PAGE_MAPPING(core_id, core_id, lpn, new_ppn, \
					old_ppn, left_skip, right_skip);
		}
		else{
			ret = FLASH_PAGE_WRITE(core_id, new_ppn);

//			UPDATE_OLD_PAGE_MAPPING(core_id, core_id, lpn);
//			UPDATE_NEW_PAGE_MAPPING(core_id, lpn, new_ppn);
		}
		*/
		n_write_pages++;
		lba += write_sects;
		remain -= write_sects;
		left_skip = 0;
	}

#ifdef FTL_DEBUG
	printf("[%s] %d core: wait for writing %d pages\n",
			__FUNCTION__, core_id, n_write_pages);
#endif

#ifdef FTL_DEBUG
	printf("[%s] %d core: End\n", __FUNCTION__, core_id);
#endif
	return n_write_pages;
}
