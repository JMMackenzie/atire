/*
	NEXI.H
	------
*/
#ifndef NEXI_H_
#define NEXI_H_

#include <stdio.h>
#include "string_pair.h"
#include "NEXI_term.h"

class ANT_string_term;

/*
	class ANT_NEXI
	--------------
*/
class ANT_NEXI 
{
protected:
	static const size_t MAX_NEXI_TERMS = 1024;

private:
	long successful_parse;
	ANT_string_pair token;
	unsigned char *string, *at;
	ANT_NEXI_term *pool;

protected:
	size_t pool_used;

private:
	ANT_NEXI_term *get_NEXI_term(ANT_NEXI_term *parent, ANT_string_pair *tag, ANT_string_pair *term, long weight);
	ANT_NEXI_term *duplicate_path_chain(ANT_NEXI_term *start, ANT_NEXI_term **end_of_chain);
	long ispart(unsigned char *from, long length, unsigned char *next);
	ANT_string_pair *get_next_token(void);
	void read_path(ANT_string_pair *path);
	void parse_error(char *message);
	ANT_NEXI_term *about(void);
	long read_phrase(ANT_string_pair *string);
	long read_phraseless_term(ANT_string_pair *term);
	long read_term(ANT_string_pair *term);
	void read_operator(void);
	ANT_NEXI_term *numbers(void);
	ANT_NEXI_term *read_CO(ANT_string_pair *path, ANT_string_pair *terms);
	ANT_NEXI_term *read_CAS(void);

protected:
	virtual ANT_NEXI_term *next_free_node() { return pool + pool_used; } 

public:
	ANT_NEXI(long make_memory = 1) { pool = make_memory ? new ANT_NEXI_term[MAX_NEXI_TERMS] : NULL; pool_used = 0; }
	virtual ~ANT_NEXI() { delete [] pool; }
	ANT_NEXI_term *parse(char *expression);
	long get_success_state(void) { return successful_parse; }
} ;

#endif  /* NEXI_H_ */

