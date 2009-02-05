/*
	ANT.C
	-----
*/
#include <stdlib.h>
#include <time.h>
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
#include "stemmer_none.h"
#include "stemmer_porter.h"
#include "INEX_assessment.h"
#include "search_engine_forum.h"
#include "ga.h"
#include "ga_stemmer.h"
#include "ga_function.h"
#include "strgen.h"
#include "counter.h"

#define NUM_OF_GENERATIONS 200
#define POPULATION_SIZE 200

#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE (!FALSE)
#endif

static char *ANT_version_string = "Version 0.1 alpha";

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
double perform_query(ANT_search_engine *search_engine, char *query, long *matching_documents, long topic_id = -1, ANT_mean_average_precision *map = NULL)
{
ANT_time_stats stats;
long long now;
long did_query;
char token[1024];
char *token_start, *token_end;
long hits, token_length;
ANT_search_engine_accumulator *ranked_list;
double average_precision = 0.0;
GA_stemmer stemmer(search_engine);

if (topic_id == -1)
	search_engine->stats_initialise();		// if we are command-line then report query by query stats

did_query = FALSE;
now = stats.start_timer();
search_engine->init_accumulators();

token_end = query;

while (*token_end != '\0')
	{
	token_start = token_end;
	while (!ANT_isalpha(*token_start) && *token_start != '\0')
		token_start++;
	if (*token_start == '\0')
		break;
	token_end = token_start;
	while (ANT_isalpha(*token_end) || *token_end == '+')
		token_end++;
	strncpy(token, token_start, token_end - token_start);
	token[token_end - token_start] = '\0';
	token_length = token_end - token_start;
	strlwr(token);

	if ((token_length > 1) && (token[token_length - 1] == '+'))
		{
		token[token_length - 1] = '\0';
		search_engine->process_one_stemmed_search_term(&stemmer, token);
		}
	else
		search_engine->process_one_search_term(token);
	did_query = TRUE;
	}

ranked_list = search_engine->sort_results_list(search_engine->document_count(), &hits); // accurately rank all documents
//ranked_list = search_engine->sort_results_list(1500, &hits);		// accurately identify the top 1500 documents

if (topic_id == -1)
	{
	printf("Query '%s' found %ld documents ", query, hits);
	stats.print_time("(", stats.stop_timer(now), ")\n");
	if (did_query)
		search_engine->stats_text_render();
	}
else
	{
	printf("Topic:%ld Query '%s' found %ld documents ", topic_id, query, hits);
	stats.print_time("(", stats.stop_timer(now), ")");
//	if (did_query)
//		search_engine->stats_text_render();
	average_precision = map->average_precision(topic_id, search_engine);
	}

*matching_documents = hits;
return average_precision;
}

/*
	PERFORM_QUERY_W_STEMMER()
	---------------
*/
double perform_query_w_stemmer(ANT_search_engine *search_engine, char *query,
                               long *matching_documents, ANT_stemmer *stemmer, 
                               long topic_id = -1, ANT_mean_average_precision *map = NULL)
{
ANT_time_stats stats;
long long now;
long did_query;
char token[1024];
char *token_start, *token_end;
long hits, token_length;
ANT_search_engine_accumulator *ranked_list;
double average_precision = 0.0;

did_query = FALSE;
now = stats.start_timer();
search_engine->init_accumulators();

token_end = query;

while (*token_end != '\0')
	{
	token_start = token_end;
	while (!ANT_isalpha(*token_start) && *token_start != '\0')
		token_start++;
	if (*token_start == '\0')
		break;
	token_end = token_start;
	while (ANT_isalpha(*token_end) || *token_end == '+')
		token_end++;
	strncpy(token, token_start, token_end - token_start);
	token[token_end - token_start] = '\0';
	token_length = token_end - token_start;
	strlwr(token);
    DEC_COUNTER;
    search_engine->process_one_stemmed_search_term(stemmer, token);
    did_query = TRUE;
	}
 PRINT_COUNTER;
ranked_list = search_engine->sort_results_list(search_engine->document_count(), &hits); // accurately rank all documents
//ranked_list = search_engine->sort_results_list(1500, &hits);		// accurately identify the top 1500 documents

average_precision = map->average_precision(topic_id, search_engine);

*matching_documents = hits;
return average_precision;
}

/*
	READ_DOCID_LIST()
	-----------------
*/
char **read_docid_list(long *documents_in_id_list)
{
ANT_disk disk;
char *document_list_buffer, **document_list;

if ((document_list_buffer = disk.read_entire_file("doclist.aspt")) == NULL)
	exit(printf("Cannot open document ID list file 'doclist.aspt'\n"));
document_list = disk.buffer_to_list(document_list_buffer, documents_in_id_list);

return document_list;
}

/*
	COMMAND_DRIVEN_ANT()
	--------------------
*/
void command_driven_ant(void)
{
ANT_memory memory;
char query[1024];
long last_to_list, hits, more, documents_in_id_list;
char **document_list, **answer_list;

printf("Ant %s\n", ANT_version_string);
puts("Written (w) 2008, 2009");
puts("Andrew Trotman, University of Otago");
puts("andrew@cs.otago.ac.nz");

document_list = read_docid_list(&documents_in_id_list);
answer_list = (char **)memory.malloc(sizeof(*answer_list) * documents_in_id_list);

ANT_search_engine search_engine(&memory);
printf("Index contains %ld documents\n", search_engine.document_count());
if (search_engine.document_count() != documents_in_id_list)
	exit(printf("There are %ld documents in the index, but %ld documents in the ID list (exiting)\n", search_engine.document_count(), documents_in_id_list));

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
			perform_query(&search_engine, query, &hits);
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
	GET_ANT_QRELS()
	---------------
*/
ANT_relevant_document *get_ant_qrels(ANT_memory *memory, char *qrel_file, long *qrel_list_length)
{
ANT_disk file_system;
ANT_relevant_document *all_assessments, *current_assessment;
FILE *qrel_fp;
char *entire_file, *ch;
long lines;
char text[80];

if ((entire_file = file_system.read_entire_file(qrel_file)) == NULL)
	exit(fprintf(stderr, "Cannot read qrel file:%s\n", qrel_file));

lines = 0;
for (ch = entire_file; *ch != '\0'; ch++)
	if (*ch == '\n')
		lines++;

current_assessment = all_assessments = (ANT_relevant_document *)memory->malloc(sizeof(*all_assessments) * lines);

if ((qrel_fp = fopen(qrel_file, "rb")) == NULL)
	exit(fprintf(stderr, "Cannot open topic file:%s\n", qrel_file));

while (fgets(text, sizeof(text), qrel_fp) != NULL)
	{
	if ((sscanf(text, "%ld %ld", &current_assessment->topic, &current_assessment->docid)) != 2)
		exit(printf("%s line %d:Cannot extract '<queryid> <docid>'", qrel_file, current_assessment - all_assessments));
	current_assessment++;
	}

fclose(qrel_fp);

*qrel_list_length = lines;
return all_assessments;
}

/*
	GET_QRELS()
	-----------
	This is highly inefficient, but because it only happens once that's OK.
*/
ANT_relevant_document *get_qrels(ANT_memory *memory, char *qrel_file, long *qrel_list_length, long qrel_format, char **uid_list, long uid_list_length)
{
if (qrel_format == QREL_INEX)
	{
	ANT_INEX_assessment qrel_reader(memory, uid_list, uid_list_length);
	return qrel_reader.read(qrel_file, qrel_list_length);
	}
else
	return get_ant_qrels(memory, qrel_file, qrel_list_length);
}

/*
	GA_ANT()
	--------
*/
void ga_ant(char *topic_file, char *qrel_file, long qrel_format)
{
ANT_relevant_document *assessments;
char query[1024];
long line, number_of_assessments, documents_in_id_list, *topic_ids = NULL;
ANT_memory memory;
FILE *fp;
char *query_text, **document_list;
char **all_queries = NULL;
GA *ga;

srand(time(NULL));

fprintf(stderr, "Ant %s Written (w) 2008, 2009 Andrew Trotman, University of Otago\n", ANT_version_string);
ANT_search_engine *search_engine = new ANT_search_engine(&memory);
fprintf(stderr, "Index contains %ld documents\n", search_engine->document_count());
init_strgen(search_engine);

document_list = read_docid_list(&documents_in_id_list);
assessments = get_qrels(&memory, qrel_file, &number_of_assessments, qrel_format, document_list, documents_in_id_list);
ANT_mean_average_precision *map = new ANT_mean_average_precision(&memory, assessments, number_of_assessments);

if ((fp = fopen(topic_file, "rb")) == NULL)
	exit(fprintf(stderr, "Cannot open topic file:%s\n", topic_file));
line = 1;
while (fgets(query, sizeof(query), fp) != NULL)
	{
	strip_end_punc(query);
    topic_ids = (long *) realloc(topic_ids, line * sizeof(topic_ids[0]));
	topic_ids[line - 1] = atol(query);
	if ((query_text = strchr(query, ' ')) == NULL)
		exit(printf("Line %ld: Can't process query as badly formed:'%s'\n", line, query));
    all_queries = (char **) realloc(all_queries, line * sizeof(all_queries[0]));
    all_queries[line - 1] = strdup(query_text);
	line++;
	}
fclose(fp);

ga = new GA(POPULATION_SIZE, 
            new GA_function(perform_query_w_stemmer, search_engine, line - 1, all_queries, topic_ids, map));
ga->run(NUM_OF_GENERATIONS);
// TODO: output some stats whilst running (to a file, specified on the command line perhaps)

search_engine->stats_text_render();
}


/*
	BATCH_ANT()
	-----------
*/
double batch_ant(char *topic_file, char *qrel_file, char *stemmer_file, long qrel_format)
{
ANT_relevant_document *assessments;
char query[1024];
long topic_id, line, number_of_assessments, hits, documents_in_id_list;
ANT_memory memory;
FILE *fp;
char *query_text, **document_list, **answer_list;
double average_precision, sum_of_average_precisions, mean_average_precision;
ANT_search_engine_forum output("ant.out");

fprintf(stderr, "Ant %s Written (w) 2008, 2009 Andrew Trotman, University of Otago\n", ANT_version_string);
ANT_search_engine search_engine(&memory);
fprintf(stderr, "Index contains %ld documents\n", search_engine.document_count());
GA_stemmer *stemmer = new GA_stemmer(&search_engine);
GA_individual *ind = new GA_individual();
if (stemmer_file) {
    ind->load(stemmer_file);
    stemmer->set_stemmer(ind);
}
document_list = read_docid_list(&documents_in_id_list);
answer_list = (char **)memory.malloc(sizeof(*answer_list) * documents_in_id_list);

assessments = get_qrels(&memory, qrel_file, &number_of_assessments, qrel_format, document_list, documents_in_id_list);
ANT_mean_average_precision map(&memory, assessments, number_of_assessments);

if ((fp = fopen(topic_file, "rb")) == NULL)
	exit(fprintf(stderr, "Cannot open topic file:%s\n", topic_file));


sum_of_average_precisions = 0.0;
line = 1;
output.INEX_init();
while (fgets(query, sizeof(query), fp) != NULL)
	{
	strip_end_punc(query);
	topic_id = atol(query);
	if ((query_text = strchr(query, ' ')) == NULL)
		exit(printf("Line %ld: Can't process query as badly formed:'%s'\n", line, query));

    if (stemmer_file) 
        average_precision = perform_query_w_stemmer(&search_engine, query_text, &hits, stemmer, topic_id, &map);
    else
        average_precision = perform_query(&search_engine, query_text, &hits, topic_id, &map);
	sum_of_average_precisions += average_precision;
	fprintf(stderr, "Topic:%ld Average Precision:%f\n", topic_id, average_precision);
	line++;
	search_engine.generate_results_list(document_list, answer_list, hits);
	output.INEX_export(topic_id, answer_list, hits);
	}
fclose(fp);
output.INEX_close();
mean_average_precision = sum_of_average_precisions / (double) (line - 1);
printf("Processed %ld topics (MAP:%f)\n", line - 1, mean_average_precision);

search_engine.stats_text_render();

return mean_average_precision;
}

/*
	USAGE()
	-------
*/
void usage(char *exename)
{
printf("Usage:\n%s\nor\n", exename);
printf("%s <topic_file> <qrel_file>\n", exename);
}

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
if (argc == 1)
	command_driven_ant();
else if (argc == 3)
	ga_ant(argv[1], argv[2], QREL_ANT);
#ifndef FIT_BM25
else if (argc == 4)
	batch_ant(argv[1], argv[2], argv[3], QREL_ANT); // Used to check baseline performance
#else
/*
	This code can be used for optimising the BM25 parameters.
	In order to make it work you'll need to change the code for 
	BM25 to declare and use the externs;
*/
else if (argc == 4)
	{
	FILE *outfile;
	extern double BM25_k1;
	extern double BM25_b;
    double map;

	outfile = fopen(argv[3], "wb");
	for (BM25_b = 0.1; BM25_b < 1.0; BM25_b += 0.1)
		fprintf(outfile, "%f ", BM25_b);
	fprintf(outfile, "\n");

	for (BM25_k1 = 0.1; BM25_k1 < 4.0; BM25_k1+= 0.1)
		{
		fprintf(outfile, "%f ", BM25_k1);
		for (BM25_b = 0.1; BM25_b < 1.0; BM25_b += 0.1)
			{
            map = batch_ant(argv[1], argv[2], NULL, QREL_ANT);
			fprintf(outfile, "%f ", map);
			}
		fprintf(outfile, "\n");
		}
	}
#endif
else
	usage(argv[0]);

return 0;
}
