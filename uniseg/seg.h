/*
 * SEG.H
 * -----
 *
 *  Created on: Nov 29, 2008
 *      Author: monfee
 */

#ifndef __SEG_H__
#define __SEG_H__

#include <string>
#include "uniseg_types.h"
#include "freq.h"
#include "clist.h"

class Seger {
protected:
	CList				*clist_;
	Freq				*freq_;    // the frequency records for the input text
	Freq				*allfreq_;
	word_ptr_type 		tw_ptr_;
	word_ptr_type 		tw_ptr_local_;
	string_type 		stream_;  // for the incoming document which will be segmented
	string_type			stream_out_;  // store the segmented text, words are seperated by spaces

	array_type			words_list_;   // top ranking candidate words aka segmented words
	//unsigned char 		**output_;  // same content with words_, but for the easy access of other programs

public:
	Seger(word_ptr_type tw_ptr);
	Seger(const string_type stream);
	Seger(const char* stream, size_t length);
	Seger();
	~Seger();

	//void load_frqs();
	virtual void start();

	//unsigned char **output();
	//int output(unsigned char **out);
	//void free_output();
	const unsigned char *output();

	void show_all();
	void free();

	void mark_the_seged();

	string_type& stream() { return stream_; }
	string_type& stream_out() { return stream_out_; }
	void input(unsigned char *, int length);
	void input(string_type& stream);

	int get_count() { return words_list_.size(); }
	const array_type& best_words() const { return words_list_; }

private:
	void init();
	void init_members();

protected:
	void make(CList& clist, string_type& str);
	void assign_freq();
	void assign_freq(word_ptr_type local_word, word_ptr_type global_word);
	void justify(unsigned int min = 2);
	virtual void do_some_calculations();

	virtual void seg();
	virtual void build();
	virtual void add_to_list(array_type& cwlist);
};

#endif /* __SEG_H__ */
