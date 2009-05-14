/*
	ANT.C
	-----
*/
#include <stdio.h>
#include <string.h>
#include "str.h"
#include "memory.h"
#include "ctypes.h"
#include "search_engine.h"
#include "search_engine_btree_leaf.h"
#include "mean_average_precision.h"
#include "disk.h"
#include "relevant_document.h"
#include "time_stats.h"
#include "stemmer.h"
#include "stemmer_factory.h"
#include "assessment_factory.h"
#include "search_engine_forum_INEX.h"
#include "search_engine_forum_TREC.h"
#include "ant_param_block.h"
#include "version.h"

#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE (!FALSE)
#endif

enum {QREL_INEX, QREL_ANT};

/*
	SPECIAL_COMMAND()
	-----------------
*/
long special_command(char *command)
{
if (strcmp(command, ".quit") == 0)
	return FALSE;

return TRUE;
}

/*
	PERFORM_QUERY()
	---------------
*/
double perform_query(ANT_ANT_param_block *params, ANT_search_engine *search_engine, char *query, long long *matching_documents, long topic_id = -1, ANT_mean_average_precision *map = NULL)
{
ANT_time_stats stats;
long long now;
long did_query;
char token[1024];
char *token_start, *token_end;
long long hits;
size_t token_length;
ANT_search_engine_accumulator *ranked_list;
double average_precision = 0.0;
ANT_stemmer *stemmer;

/*
	if we're stemming then create the stemmer object
*/
stemmer = params->stemmer == 0 ? NULL : ANT_stemmer_factory::get_stemmer(params->stemmer, search_engine);

if (topic_id == -1)
	search_engine->stats_initialise();		// if we are command-line then report query by query stats

did_query = FALSE;
now = stats.start_timer();
search_engine->init_accumulators();

token_end = query;

while (*token_end != '\0')
	{
	token_start = token_end;
	while (!ANT_isalnum(*token_start) && *token_start != '\0')
		token_start++;
	if (*token_start == '\0')
		break;
	token_end = token_start;
	while (ANT_isalnum(*token_end) || *token_end == '+')
		token_end++;
	strncpy(token, token_start, token_end - token_start);
	token[token_end - token_start] = '\0';
	token_length = token_end - token_start;
	strlwr(token);

	/*
		process the next search term - either stemmed or not.
	*/
	if (stemmer == NULL)
		search_engine->process_one_search_term(token);
	else
		search_engine->process_one_stemmed_search_term(stemmer, token);

	did_query = TRUE;
	}

/*
	Rank the results list
*/
ranked_list = search_engine->sort_results_list(params->sort_top_k, &hits); // rank

if (topic_id == -1)
	{
	printf("Query '%s' found %lld documents ", query, hits);
	stats.print_time("(", stats.stop_timer(now), ")\n");
	if (did_query)
		search_engine->stats_text_render();
	}
else
	{
	printf("Topic:%ld Query '%s' found %lld documents ", topic_id, query, hits);
	stats.print_time("(", stats.stop_timer(now), ")");
//	if (did_query)
//		search_engine->stats_text_render();
	/*
		Compute the precision
	*/
	if (params->metric == ANT_ANT_param_block::MAP)
		average_precision = map->average_precision(topic_id, search_engine);
	else
		average_precision = map->average_generalised_precision(topic_id, search_engine);
	}

/*
	Return the number of document that matched the user's query
*/
*matching_documents = hits;

/*
	Clean up
*/
delete stemmer;

/*
	Return the precision
*/
return average_precision;
}

/*
	READ_DOCID_LIST()
	-----------------
*/
char **read_docid_list(long long *documents_in_id_list)
{
static char **document_list = NULL;
static long long len = 0;
ANT_disk disk;
char *document_list_buffer;

if (document_list == NULL)		// only read once if this routine is called multiple times (nasty)
	{
	if ((document_list_buffer = disk.read_entire_file("doclist.aspt")) == NULL)
		exit(printf("Cannot open document ID list file 'doclist.aspt'\n"));
	document_list = disk.buffer_to_list(document_list_buffer, &len);
	}

*documents_in_id_list = len;
return document_list;
}

/*
	COMMAND_DRIVEN_ANT()
	--------------------
*/
void command_driven_ant(ANT_ANT_param_block *params)
{
ANT_memory memory;
char query[1024];
long more;
long long last_to_list, hits, documents_in_id_list;
char **document_list, **answer_list;

document_list = read_docid_list(&documents_in_id_list);
answer_list = (char **)memory.malloc(sizeof(*answer_list) * documents_in_id_list);

ANT_search_engine search_engine(&memory);
printf("Index contains %lld documents\n", search_engine.document_count());
if (search_engine.document_count() != documents_in_id_list)
	exit(printf("There are %lld documents in the index, but %lld documents in the ID list (exiting)\n", search_engine.document_count(), documents_in_id_list));

puts("\nuse:\n\t.quit to quit\n\n");

more = TRUE;
while (more)
	{
	printf("]");
	if (fgets(query, sizeof(query), stdin) == NULL)
		more = FALSE;
	else
		{
		strip_end_punc(query);
		if (*query == '.')
			more = special_command(query);
		else
			{
			perform_query(params, &search_engine, query, &hits);
			last_to_list = hits > 10 ? 10 : hits;
			search_engine.generate_results_list(document_list, answer_list, last_to_list);
			for (long result = 0; result < last_to_list; result++)
				printf("%ld:%s\n", result + 1, answer_list[result]);
			}
		}
	}
puts("Bye");
}

/*

	BATCH_ANT()
	-----------
*/
double batch_ant(ANT_ANT_param_block *params, char *topic_file, char *qrel_file)
{
ANT_relevant_document *assessments;
char query[1024];
long topic_id, line;
long long hits, number_of_assessments, documents_in_id_list;
ANT_memory memory;
FILE *fp;
char *query_text, **document_list, **answer_list;
double average_precision, sum_of_average_precisions, mean_average_precision;
ANT_search_engine_forum_TREC output("ant.out", "4", "ANTWholeDoc", "RelevantInContext");
ANT_assessment_factory *factory;

ANT_search_engine search_engine(&memory);
fprintf(stderr, "Index contains %lld documents\n", search_engine.document_count());

document_list = read_docid_list(&documents_in_id_list);
answer_list = (char **)memory.malloc(sizeof(*answer_list) * documents_in_id_list);

factory = new ANT_assessment_factory(&memory, document_list, documents_in_id_list);
assessments = factory->read(qrel_file, &number_of_assessments);

ANT_mean_average_precision map(&memory, assessments, number_of_assessments);

if ((fp = fopen(topic_file, "rb")) == NULL)
	exit(fprintf(stderr, "Cannot open topic file:%s\n", topic_file));

sum_of_average_precisions = 0.0;
line = 1;
while (fgets(query, sizeof(query), fp) != NULL)
	{
	strip_end_punc(query);
	topic_id = atol(query);
	if ((query_text = strchr(query, ' ')) == NULL)
		exit(printf("Line %ld: Can't process query as badly formed:'%s'\n", line, query));

	average_precision = perform_query(params, &search_engine, query_text, &hits, topic_id, &map);
	sum_of_average_precisions += average_precision;
	fprintf(stderr, "Topic:%ld Average Precision:%f\n", topic_id, average_precision);
	line++;
	search_engine.generate_results_list(document_list, answer_list, hits);
	output.write(topic_id, answer_list, hits > 1500 ? 1500 : hits);			// top 1500 results only
	}
fclose(fp);
mean_average_precision = sum_of_average_precisions / (double) (line - 1);
printf("Processed %ld topics (MAP:%f)\n", line - 1, mean_average_precision);

search_engine.stats_text_render();

delete factory;

return mean_average_precision;
}

/*
	USAGE()
	-------
*/
void usage(char *exename)
{
printf("Usage:\n%s\nor\n", exename);
printf("%s [options...] <topic_file> <qrel_file>\n", exename);
}


/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
long last_param;
ANT_ANT_param_block params(argc, argv);

last_param = params.parse();

if (params.logo)
	puts(ANT_version_string);				// print the version string is we parsed the parameters OK

if (argc - last_param == 0)
	command_driven_ant(&params);
else if (argc - last_param == 2)
	batch_ant(&params, argv[last_param], argv[last_param + 1]);

#ifdef FIT_BM25
/*
	This code can be used for optimising the BM25 parameters.
*/
else if (argc == 4)
	{
	double map;
	FILE *outfile;
	extern double BM25_k1;
	extern double BM25_b;

	outfile = fopen(argv[3], "wb"); 
	fprintf(outfile, "%f ", 0.0);
	for (BM25_b = 0.1; BM25_b < 1.0; BM25_b += 0.1)
		fprintf(outfile, "%f ", BM25_b);
	fprintf(outfile, "\n");

	for (BM25_k1 = 0.1; BM25_k1 < 4.0; BM25_k1+= 0.1)
		{
		fprintf(outfile, "%f ", BM25_k1);
		for (BM25_b = 0.1; BM25_b < 1.0; BM25_b += 0.1)
			{
			map = batch_ant(argv[1], argv[2]);
			fprintf(outfile, "%f ", map);
			}
		fprintf(outfile, "\n");
		}
	}
	fclose(outfile);
#endif
else
	usage(argv[0]);

return 0;
}


