/*
	ATIRE_API.H
	-----------
*/
#ifndef ATIRE_API_H_
#define ATIRE_API_H_

#include <limits.h>

class ANT_NEXI_ant;
class ANT_search_engine;
class ANT_stemmer;
class ANT_query_boolean;
class ANT_query;
class ANT_ranking_function;
class ANT_memory;
class ANT_relevant_document;
class ANT_mean_average_precision;
class ANT_search_engine_forum;
class ANT_assessment_factory;

/*
	class ATIRE_API
	---------------
*/
class ATIRE_API
{
public:
	enum { INDEX_IN_FILE = 0, INDEX_IN_MEMORY = 1, READABILITY_SEARCH_ENGINE = 2 };

private:
	ANT_memory *memory;						// ATIRE memory allocation scheme

	ANT_NEXI_ant *NEXI_parser;				// INEX CO / CAS queries
	ANT_query_boolean *boolean_parser;		// full boolean
	long segmentation;						// Chinese segmentation algorithm
	ANT_query *parsed_query;				// the parsed query
public:						// FIX THIS
	ANT_search_engine *search_engine;		// the search engine itself
private:
	ANT_ranking_function *ranking_function;	// the ranking function to use (default is the perameterless Divergence From Randomness)
	ANT_stemmer *stemmer;					// stemming function to use
	long query_type_is_all_terms;			// use the DISJUNCTIVE ranker but only find documents containing all of the search terms (CONJUNCTIVE)
	long long hits;							// how many documents were found at the last query
	long long sort_top_k;					// ranking is only accurate to this position in the results list

	char **document_list;					// list (in order) of the external IDs of the documents in the collection
	char **filename_list;					// the same list, but assuming filenames (parsed for INEX)
	char **answer_list;						// 
	long long documents_in_id_list;			// the length of the above two lists (the number of docs in the collection)
	char *mem1, *mem2;						// arrays of memory holding the above;

	ANT_assessment_factory *assessment_factory;		// the machinery to read different formats of assessments (INEX and TREC)
	ANT_relevant_document *assessments;		// assessments for measuring percision (at TREC and INEX)
	long long number_of_assessments;		// length of the assessments array
	ANT_mean_average_precision *map;		// the object that computes MAP scores
	ANT_search_engine_forum *forum_writer;	// the object that writes the results list in the INEX or TREC format
	long forum_results_list_length;			// maximum length of a results list for the evaluation form (INEX or TREC)

protected:
	char **read_docid_list(long long *documents_in_id_list, char ***filename_list, char **mem1, char **mem2);
	static char *max(char *a, char *b, char *c);

public:
	ATIRE_API();
	virtual ~ATIRE_API();

	/*
		What version are we?
	*/
	char *version(long *version_number = 0);

	/*
		Load all the necessary stuff for the search engine to start up
		This assumes we are in same directory as the index
	*/
	long open(long type);		// see the enum above for possible types (ORed together)

	/*
		Load an assessment file (for INEX or TREC)
	*/
	long load_assessments(char *assessments_filename);

	/*
		Set the chinese segmentation algorithm
	*/
	void set_segmentation(long segmentation) { this->segmentation = segmentation; }

	/*
		Parse a NEXI query
	*/
	long parse_NEXI_query(char *query);												// returns an error code (0 = no error, see query.h)

	/*
		Set the ranking function
		for BM25: k1 = p1, b = k2
		for LMD:  u = p1
		for LMJM: l = p1
	*/
	long set_ranking_function(long function, double p1, double p2);

	/*
		Set the stemming function
	*/
	long set_stemmer(long which_stemmer, long stemmer_similarity, double threshold);

	/*
		Load a document from the repository (if there is one)
	*/
	char *load_document(char *buffer, unsigned long *length, long long id);

	/*
		Given the query, do the seach, rank, and return the number of hits
	*/
	long long search(char *query, long long top_k = LLONG_MAX);

	/*
		Turn the numeric internal IDs into a list of external string IDs (post search)
	*/
	char **generate_results_list(void);

	/*
		What is the average precision of the query we've just done?
	*/
	double get_whole_document_precision(long query_id, long metric, long metric_n);

	/*
		Configre TREC or INEX output format
	*/
	long set_forum(long type, char *output_filename, char *participant_id, char *run_name, long forum_results_list_length);

	/*
		Write the results out in INEX or TREC format (as specified by set_form)
	*/
	void write_to_forum_file(long topic_id);
} ;

#endif /* ATIRE_API_H_ */
