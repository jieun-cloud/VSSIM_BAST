// File: ftl.h
// Date: 18-Sep-2017
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2017
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#ifndef _FTL_H_
#define _FTL_H_

#include "common.h"

extern FILE* fp_w_event;
extern FILE* fp_ch_util;
extern FILE* fp_wb_lat;
extern FILE* fp_rw_lat;

//Jieun 
extern int PAGES_PER_BLOCK;

void FTL_INIT(void);
void FTL_TERM(void);

int FTL_READ(int core_id, uint64_t sector_nb, uint32_t length);
int FTL_WRITE(int core_id, uint64_t sector_nb, uint32_t length);
void FTL_DISCARD(int core_id, uint64_t sector_nb, uint32_t length);

int _FTL_READ(int core_id, uint64_t sector_nb, uint32_t length);
int _FTL_WRITE(int core_id, uint64_t sector_nb, uint32_t length);
#endif
