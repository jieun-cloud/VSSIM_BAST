// File: ftl_mapping_manager.h
// Date: 18-Sep-2017
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2017
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#ifndef _MAPPING_MANAGER_H_
#define _MAPPING_MANAGER_H_

extern ppn_t** mapping_table;
extern data_block_entry** data_mapping_table; //Jieun


typedef struct data_block_entry
{
	pbn_t pbn;
	int64_t lbn;
	int64_t* inverse_data_mapping_table;
	
}data_block_entry;

typedef struct log_block_entry
{
	pbn_t pbn;
	int64_t lbn;
	uint32_t w_index;
	bool is_switch;
	int last_lpn;
	int64_t* log_mapping_table;
	int64_t* inverse_log_mapping_table;
	
}log_block_entry;

extern log_block_entry** log_block_mapping_table;

int INIT_LOG_BLOCK_ENTRY(void); //Jieun
int INIT_MAPPING_TABLE(int init_info);

int INIT_DATA_MAPPING_TABLE(int init_info); //Jieun

void TERM_MAPPING_TABLE(void);

ppn_t GET_MAPPING_INFO(int core_id, int64_t lpn);
pbn_t GET_DATA_MAPPING_INFO(int core_id, int64_t lbn);
bool GET_LOG_BLOCK_MAPPING_INFO(int core_id, int64_t lbn);
ppn_t GET_LOG_MAPPING_INFO(int core_id, int64_t lpn);

int GET_NEW_PAGE(int core_id, pbn_t index, int mode, ppn_t* ppn, int for_gc);

int UPDATE_OLD_PAGE_MAPPING(int core_id, int owner_core_id, int64_t lpn);
int UPDATE_NEW_PAGE_MAPPING(int core_id, int64_t lpn, ppn_t ppn);

//Jieun added
int UPDATE_OLD_BLOCK_MAPPING(int core_id, int owner_core_id, int64_t lpn);
int UPDATE_NEW_BLOCK_MAPPING(int core_id, int64_t lpn, pbn_t pbn);
int CHECK_TABLE(int core_id, int64_t lpn); // For debugging
int CHECK_BITMAP(int core_id, pbn_t pbn, int64_t lpn); // For debugging

int PARTIAL_UPDATE_PAGE_MAPPING(int core_id, int owner_core_id, int64_t lpn, ppn_t new_ppn,
		ppn_t old_ppn, uint32_t left_skip, uint32_t right_skip);
#endif
