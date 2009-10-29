/*
	MEMORY_INDEX.H
	--------------
*/
#ifndef MEMORY_INDEX_H_
#define MEMORY_INDEX_H_

#include "memory_index_stats.h"
#include "memory_index_hash_node.h"
#include "fundamental_types.h"
#include "compress_variable_byte.h"
#include "compression_factory.h"
#include "compression_text_factory.h"

class ANT_memory_index_hash_node;
class ANT_memory;
class ANT_string_pair;
class ANT_file;
class ANT_ranking_function;
class ANT_ranking_function_factory;

#define HASH_TABLE_SIZE (0x1000000)

class ANT_memory_index
{
public:
	enum { STAT_MEMORY = 1, STAT_TIME = 2, STAT_COMPRESSION = 4, STAT_SUMMARY = 8 } ;

private:
	ANT_string_pair *squiggle_length;
	long hashed_squiggle_length;
	ANT_string_pair *squiggle_document_offsets;
	ANT_memory_index_hash_node *hash_table[HASH_TABLE_SIZE];
	ANT_memory *memory;
	unsigned char *serialised_docids, *serialised_tfs;
	long long serialised_docids_size, serialised_tfs_size;
	long long largest_docno;
	ANT_memory_index_stats *stats;

	ANT_compress_variable_byte variable_byte;
	ANT_compression_factory *factory;
	ANT_compressable_integer *decompressed_postings_list, *impacted_postings;
	unsigned char *compressed_postings_list;
	long long compressed_postings_list_length;

	ANT_file *index_file;

	/*
		These are used for quantization.  We need to store the max and min rsv values 
		so that we can scale them.  We also need the document lengths array and some
		form of quantizer
	*/
	ANT_compressable_integer *document_lengths;
	ANT_ranking_function *quantizer;
	double maximum_collection_rsv, minimum_collection_rsv;

	/*
		Documents that are added to the repository are compressed using the compression
		factory first.  A list of offsets is stored in a whapping great array which is
		then written to disk as a footer before the index starts.
	*/
	long long documents_in_repository;
	ANT_compression_text_factory *factory_text;
	char *compressed_document_buffer;
	long compressed_document_buffer_size;
	

private:
	long hash(ANT_string_pair *string);
	ANT_memory_index_hash_node *find_node(ANT_memory_index_hash_node *root, ANT_string_pair *string);
	ANT_memory_index_hash_node *find_add_node(ANT_memory_index_hash_node *root, ANT_string_pair *string);
	long serialise_all_nodes(ANT_file *file, ANT_memory_index_hash_node *root);
	ANT_memory_index_hash_node *new_memory_index_hash_node(ANT_string_pair *string);
	long generate_term_list(ANT_memory_index_hash_node *root, ANT_memory_index_hash_node **into, long where, int32_t *length_of_longest_term, int64_t *highest_df);
	ANT_memory_index_hash_node **find_end_of_node(ANT_memory_index_hash_node **start);
	ANT_memory_index_hash_node **write_node(ANT_file *file, ANT_memory_index_hash_node **start);
	long long impact_order(ANT_compressable_integer *destination, ANT_compressable_integer *docid, unsigned char *term_frequency, long long document_frequency);
	double rsv_all_nodes(double *minimum, ANT_memory_index_hash_node *root);
	long long get_serialised_postings(ANT_memory_index_hash_node *root, long long *doc_size, long long *tf_size);
	void open_index_file(char *filename);
	void close_index_file(void);

	void text_render(ANT_memory_index_hash_node *root, unsigned char *serialised_docids, long doc_size, unsigned char *serialised_tfs, long tf_size);
	void text_render(ANT_compressable_integer *impact_ordering, size_t document_frequency);
	void text_render(ANT_compressable_integer *docid, unsigned char *term_frequency, long long document_frequency);

public:
	ANT_memory_index(char *filename);
	~ANT_memory_index();

	ANT_memory_index_hash_node *add_term(ANT_string_pair *string, long long docno);
	long serialise(ANT_ranking_function_factory *factory);
	void set_document_length(long long docno, long long length) { set_document_detail(squiggle_length, length); largest_docno = docno; } 
	void set_document_detail(ANT_string_pair *measure_name, long long length);
	void set_document_compression_scheme(unsigned long scheme) { factory_text->set_scheme(scheme); }
	void set_compression_scheme(unsigned long scheme) { factory->set_scheme(scheme); }
	void set_compression_validation(unsigned long validate) { factory->set_validation(validate); }
	void text_render(long what);
	long long get_memory_usage(void);
	void add_to_document_repository(char *document, long length);
} ;


/*
	ANT_MEMORY_INDEX::NEW_MEMORY_INDEX_HASH_NODE()
	----------------------------------------------
*/
inline ANT_memory_index_hash_node *ANT_memory_index::new_memory_index_hash_node(ANT_string_pair *string)
{
stats->unique_terms++;

return new (memory) ANT_memory_index_hash_node(memory, string, stats);
}

#endif  /* MEMORY_INDEX_H_ */
