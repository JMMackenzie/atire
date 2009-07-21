/*
	RANKING_FUNCTION_READABILITY.C
	------------------------------
*/
#include <math.h>
#include "ranking_function_readability.h"
#include "search_engine_btree_leaf.h"
#include "compress.h"
#include "search_engine_accumulator.h"
#include "search_engine_readability.h"

/*
	ANT_RANKING_FUNCTION_READABILITY::ANT_RANKING_FUNCTION_READABILITY()
	--------------------------------------------------------------------
*/
ANT_ranking_function_readability::ANT_ranking_function_readability(ANT_search_engine_readability *engine, double k1, double b) : ANT_ranking_function(engine)
{
this->k1 = k1;
this->b = b;
this->document_readability = engine->document_readability;
this->hardest_document = engine->hardest_document;
}

/*
	ANT_RANKING_FUNCTION_READABILITY::RELEVANCE_RANK_TOP_K()
	--------------------------------------------------------
*/
void ANT_ranking_function_readability::relevance_rank_top_k(ANT_search_engine_accumulator *accumulator, ANT_search_engine_btree_leaf *term_details, ANT_compressable_integer *impact_ordering)
{
const double k1_plus_1 = k1 + 1.0;
const double one_minus_b = 1.0 - b;
long docid;
double top_row, tf, idf, bm25;
ANT_compressable_integer *current, *end;

idf = log((double)(documents) / (double)term_details->document_frequency);
current = impact_ordering;
end = impact_ordering + term_details->impacted_length;
while (current < end)
	{
	tf = *current++;
	top_row = tf * k1_plus_1;
	docid = -1;
	while (*current != 0)
		{
		docid += *current++;
		//bm25 = (idf * (top_row / (tf + k1 * (one_minus_b + b * (document_lengths[docid] / mean_document_length)))));

		//accumulator[docid].add_rsv((0.5 * bm25) + (0.5 * (20000.0 - document_readability[docid]) / 1000.0));
		accumulator[docid].add_rsv((hardest_document - document_readability[docid]) / 1000.0);
		}
	current++;		// skip over the zero
	}
}
