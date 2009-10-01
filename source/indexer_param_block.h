/*
	INDEXER_PARAM_BLOCK.H
	---------------------
*/
#ifndef INDEXER_PARAM_BLOCK_H_
#define INDEXER_PARAM_BLOCK_H_

/*
	class ANT_INDEXER_PARAM_BLOCK
	-----------------------------
*/
class ANT_indexer_param_block
{
public:
	enum { STAT_MEMORY = 1, STAT_TIME = 2, STAT_COMPRESSION = 4, STAT_SUMMARY = 8 } ;

private:
	int argc;
	char **argv;

public:
	long trec_docnos;					// extract the unique id of the document from the DOCNO XML element (-trec | -docno)
	long recursive;						// search for files to index in this directory and directories below (-r)
	unsigned long compression_scheme;	// bitstring of which compression schemes to use
	long compression_validation;		// decompress all compressed strings and measure the decompression performance
	long statistics;					// bit pattern of which stats to print at the end of indexing
	long logo;							// display (or suppress) the banner on startup
	long long reporting_frequency;		// the number of documents to index before reporting the memory usage stats
	long segmentation;					// need segmentation or not for east-asian languages, e.g. Chinese
	unsigned long readability_measure; 	// readability measure to calculate

protected:
	void compression(char *schemes);
	void readability(char *measures);
	void stats(char *stat_list);

public:
	ANT_indexer_param_block(int argc, char *argv[]);
	void usage(void);
	void help(void);
	long parse(void);
} ;

#endif  /* INDEXER_PARAM_BLOCK_H_ */
