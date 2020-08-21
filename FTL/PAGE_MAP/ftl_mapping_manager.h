// File: ftl_mapping_manager.h
// Date: 18-Sep-2017
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2017
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#ifndef _MAPPING_MANAGER_H_
#define _MAPPING_MANAGER_H_

extern ppn_t** mapping_table;
extern pbn_t** data_mapping_table;
extern block_list** log_block_list;

typedef struct log_block_entry
{
	pbn_t pbn;
	int64_t lbn;
	uint32_t w_index;
	bool is_switch;
	int64_t last_lpn;
	int64_t* log_mapping_table;
	int64_t* inverse_log_mapping_table;
	
}log_block_entry;

extern log_block_entry** log_block_mapping_table;
int CHECK_LOG_DATA_MAPPING_INFO(int core_id, int64_t lbn);//Jieun add
int UPDATE_NEW_LOG_PAGE_MAPPING(int core_id, int64_t index, int64_t lpn); //Jieun add
int UPDATE_OLD_LOG_PAGE_MAPPING(int core_id, int index, int64_t lpn); //Jieun add
int GET_LOG_BLOCK_MAPPING_INFO(int core_id, int64_t lbn); //Jieun add
ppn_t GET_LOG_PAGE_MAPPING_INFO(int core_id, int index, int lpn); //Jieun add

void CHECK_LOG_ENTRY(int core_id, int64_t index, int64_t lpn); //For debug
void INIT_LOG_BLOCK_LIST(void); //Jieun add
int GET_NEW_LOG_PAGE(int core_id, pbn_t index, int mode, ppn_t* ppn, int table_index, int64_t lpn, int is_new); //Jieun add
int INSERT_LOG_BLOCK(int core_id, block_entry* new_log_block); //Jieun add
int CHECK_DATA_BLOCK(int core_id, block_entry* data_block); //Jieun add For debugging
int CHECK_LOG_BLOCK_FULL(int core_id, int table_index);//Jieun add
int CHECK_EMPTY_BLOCK_BITMAP(int core_id, bitmap_t valid_array);// Jieun add for debugging
void CHECK_LOG_BLOCK_LIST(int core_id); //For debugging
int UPDATE_BLOCK_INVERSE_MAPPING(pbn_t pbn,  int64_t lbn); //Jieun add
int INIT_LOG_BLOCK_ENTRY(void); //Jieun
int INIT_MAPPING_TABLE(int init_info);
int CHECK_VALID_PAGES(pbn_t pbn); //Jieun add
int GET_NEW_BLOCK(int core_id, pbn_t index, int mode, ppn_t* ppn, int64_t block_ofs); //Jieun add

int ERASE_LOG_BLOCK_ENTRY(int core_id, int index);//Jieun
int INIT_DATA_MAPPING_TABLE(int init_info); //Jieun

void TERM_MAPPING_TABLE(void);

ppn_t GET_MAPPING_INFO(int core_id, int64_t lpn);
pbn_t GET_DATA_MAPPING_INFO(int core_id, int64_t lbn);

int GET_NEW_PAGE(int core_id, pbn_t index, int mode, ppn_t* ppn, int for_gc);

int UPDATE_OLD_PAGE_MAPPING(int core_id, int owner_core_id, int64_t lpn);
int UPDATE_NEW_PAGE_MAPPING(int core_id, int64_t lpn, ppn_t ppn);

//Jieun added
int UPDATE_OLD_BLOCK_MAPPING(int core_id, int owner_core_id, int64_t lpn);
int UPDATE_NEW_BLOCK_MAPPING(int core_id, int64_t lpn, pbn_t pbn);
int CHECK_TABLE(int core_id, int64_t lpn); // For debugging
int CHECK_BITMAP(int core_id, pbn_t pbn, int64_t lpn); // For debugging
int CHECK_LOG_BITMAP(int core_id, pbn_t pbn, int64_t page); //For debugging
int SELECT_VICTIM(int core_id, int select);//Jieun
int DO_SWITCH(int core_id, block_entry* log_entry, int index);//Jieun
int DO_MERGE(int core_id, block_entry* victim_entry, int index);//Jieun
int SEARCH_LOG_BLOCK(int core_id, pbn_t pbn); //Jieun add
block_entry* GET_VICTIM_LOG_BLOCK(int core_id, int index); //Jieun add
int POP_LOG_BLOCK(int core_id, block_entry* log_block);
int GET_EMPTY_INDEX(int core_id); //Jieun add

int PARTIAL_UPDATE_PAGE_MAPPING(int core_id, int owner_core_id, int64_t lpn, ppn_t new_ppn,
		ppn_t old_ppn, uint32_t left_skip, uint32_t right_skip);
#endif
