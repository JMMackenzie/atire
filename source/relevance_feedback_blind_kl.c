/*
	RELEVANCE_FEEDBACK_BLIND_KL.C
	-----------------------------
*/
#include "relevance_feedback_blind_kl.h"
#include "memory_index_one.h"
#include "term_divergence_kl.h"

/*
	ANT_RELEVANCE_FEEDBACK_BLIND_KL::FEEDBACK()
	-------------------------------------------
*/
ANT_memory_index_one_node **ANT_relevance_feedback_blind_kl::feedback(ANT_search_engine_result *result, long documents_to_examine, long terms_to_fetch)
{
ANT_term_divergence_kl divergence;

populate(result, documents_to_examine);
one->kl_divergence(&divergence, search_engine);
return one->top_n_terms(terms_to_fetch);
}

