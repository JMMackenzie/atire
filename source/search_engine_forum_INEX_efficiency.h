/*
	SEARCH_ENGINE_FORUM_INEX_EFFICIENCY.H
	--------------------------
*/
#ifndef __SEARCH_ENGINE_FORUM_INEX_EFFICIENCY_H__
#define __SEARCH_ENGINE_FORUM_INEX_EFFICIENCY_H__

#include "search_engine_forum.h"
#include "search_engine.h"

class ANT_search_engine_forum_INEX_efficiency : public ANT_search_engine_forum
{
public:
	ANT_search_engine_forum_INEX_efficiency(char *filename, char *participant_id, char *run_id, long result_list_length, char *task);
	virtual ~ANT_search_engine_forum_INEX_efficiency();

	void write(long topic_id, char **docids, long long hits, ANT_search_engine *search_engine);
private:
	static const char* const ID_PREFIX;
} ;


#endif __SEARCH_ENGINE_FORUM_INEX_EFFICIENCY_H__