/*
	LINK_THIS.C
	-----------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../source/disk.h"
#include "link_parts.h"

#define REMOVE_ORPHAN_LINKS 1

#ifdef REMOVE_ORPHAN_LINKS
	#define ADD_ORPHAN_LINKS 1
	#define SUBTRACT_ORPHAN_LINKS (-1)
#endif

#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE (!FALSE)
#endif

class ANT_link_posting
{
public:
	long docid;						// target document
	long link_frequency;			// number of times the document occurs as the target of a link
	long doc_link_frequency;		// number of documents in which the phrase links to docid
public:
	static int compare(const void *a, const void *b);
} ;

class ANT_link_term
{
public:
	char *term;
	long postings_length;			// which is also the number of documents pointed to.
	ANT_link_posting *postings;
	long total_occurences;			// number of times the phrase occurs as a link in the collection
	long collection_frequency;		// number of times the phrase occurs in the colleciton
	long document_frequency;		// number of documents in which the phrase occurs
public:
	static int compare(const void *a, const void *b);
} ;

class ANT_link
{
public:
	char *place_in_file;
	char *term;
	double gamma;
	long target_document;

public:
	static int compare(const void *a, const void *b);
	static int final_compare(const void *a, const void *b);
	static int string_target_compare(const void *a, const void *b);
} ;

#define MAX_LINKS_IN_FILE (1024 * 1024)
ANT_link all_links_in_file[MAX_LINKS_IN_FILE];
long all_links_in_file_length = 0;

ANT_link links_in_orphan[MAX_LINKS_IN_FILE];
long links_in_orphan_length = 0;

long lowercase_only;					// are we in lowercase or mixed-case matching mode?

static char buffer[1024 * 1024];

/*
	COUNT_CHAR()
	------------
*/
long count_char(char *buffer, char what)
{
char *ch;
long times;

times = 0;
for (ch = buffer; *ch != '\0'; ch++)
	if (*ch == what)
		times++;

return times;
}

/*
	ANT_LINK::COMPARE()
	-------------------
*/
int ANT_link::compare(const void *a, const void *b)
{
ANT_link *one, *two;
double diff;
long cmp;

one = (ANT_link *)a;
two = (ANT_link *)b;

diff = two->gamma - one->gamma;
if (diff < 0)
	return -1;
else if (diff > 0)
	return 1;
else
	{
	if ((cmp = strcmp(one->term, two->term)) == 0)
		return one->place_in_file - two->place_in_file;
	else
		return cmp;
	}
}

/*
	ANT_LINK::FINAL_COMPARE()
	-------------------------
*/
int ANT_link::final_compare(const void *a, const void *b)
{
ANT_link *one, *two;
double diff;

one = (ANT_link *)a;
two = (ANT_link *)b;

diff = two->gamma - one->gamma;
if (diff < 0)
	return -1;
else if (diff > 0)
	return 1;
else
	return one->place_in_file - two->place_in_file;
}

/*
	ANT_LINK::STRING_TARGET_COMPARE()
	---------------------------------
*/
int ANT_link::string_target_compare(const void *a, const void *b)
{
ANT_link *one, *two;
int cmp;

one = (ANT_link *)a;
two = (ANT_link *)b;

if ((cmp = strcmp(one->term, two->term)) == 0)
	cmp = one->target_document - two->target_document;

return cmp;
}

/*
	ANT_LINK_TERM::COMPARE()
	------------------------
*/
int ANT_link_term::compare(const void *a, const void *b)
{
ANT_link_term *one, *two;

one = (ANT_link_term *)a;
two = (ANT_link_term *)b;

return strcmp(one->term, two->term);
}

/*
	ANT_LINK_POSTING::COMPARE()
	---------------------------
*/
int ANT_link_posting::compare(const void *a, const void *b)
{
ANT_link_posting *one, *two;

one = (ANT_link_posting *)a;
two = (ANT_link_posting *)b;

return two->link_frequency - one->link_frequency;
}

/*
	READ_INDEX()
	------------
*/
ANT_link_term *read_index(char *filename, long *terms_in_collection)
{
FILE *fp;
ANT_link_term *all_terms, *term;
long unique_terms, postings, current;
char *term_end, *from;

if ((fp = fopen(filename, "rb")) == NULL)
	exit(printf("Cannot index file:%s\n", filename));

fgets(buffer, sizeof(buffer), fp);
sscanf(buffer, "%d", &unique_terms);
term = all_terms = new ANT_link_term [unique_terms];

while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
	term_end = strchr(buffer, ':');
	postings = count_char (term_end, '>');
	term->term = strnnew(buffer, term_end - buffer);
	term->postings_length = postings;
	term->postings = new ANT_link_posting[term->postings_length];
	term->total_occurences = 0;

	sscanf(term_end + 2, "%d,%d", &(term->document_frequency), &(term->collection_frequency));
//	term_end = strchr(term_end + 1, ';';
	from = term_end;
	for (current = 0; current < postings; current++)
		{
		from = strchr(from, '<') + 1;
		sscanf(from, "%d,%d,%d", &(term->postings[current].docid), &(term->postings[current].doc_link_frequency), &(term->postings[current].link_frequency));
		term->total_occurences += term->postings[current].link_frequency;
		}
	if (term->postings_length > 1)
		qsort(term->postings, term->postings_length, sizeof(term->postings[0]), ANT_link_posting::compare);

	term++;
	}

*terms_in_collection = unique_terms;
return all_terms;
}


/*
	PUSH_LINK()
	-----------
*/
void push_link(char *place_in_file, char *buffer, long docid, double gamma)
{
ANT_link *current;

current = all_links_in_file + all_links_in_file_length;
current->place_in_file = place_in_file;
current->term = _strdup(buffer);
current->gamma = gamma;
current->target_document = docid;

all_links_in_file_length++;
if (all_links_in_file_length >= MAX_LINKS_IN_FILE)
	exit(printf("Too many links in this file (aborting)\n"));
}

/*
	DEDUPLICATE_LINKS()
	-------------------
*/
void deduplicate_links(void)
{
ANT_link *from, *to;

from = to = all_links_in_file + 1;

while (from < all_links_in_file + all_links_in_file_length)
	{
	if (strcmp(from->term, (from - 1)->term) != 0)
		{
		*to = *from;
		from++;
		to++;
		}
	else
		from++;
	}
all_links_in_file_length = to - all_links_in_file;
}

/*
	PRINT_HEADER()
	--------------
*/
void print_header(void)
{
puts("<inex-submission participant-id=\"4\" run-id=\"ASPT\" task=\"LinkTheWiki\" format=\"FOL\"><details><machine><cpu>Pentium 4</cpu><speed>2992 MHz</speed><cores>2</cores><hyperthreads>1</hyperthreads><memory>1GB</memory></machine><time>TBC</time></details><description>This is the description</description><collections><collection>wikipedia</collection></collections>");
}

/*
	PRINT_FOOTER()
	--------------
*/
void print_footer(void)
{
puts("</inex-submission>");
}

/*
	PRINT_LINKS()
	-------------
*/
void print_links(long orphan_docid)
{
long links_to_print = 250;
long result;
char *s1 = "<link><anchor><file>";
char *s2 = ".xml</file><offset>0</offset><length>0</length></anchor><linkto><file>";
char *s3 = ".xml</file><bep>0</bep></linkto></link>";
char *orphan_name = "Unknown";

printf("<topic file=\"%d.xml\" name=\"%s\"><outgoing>\n", orphan_docid, orphan_name);

for (result = 0; result < (all_links_in_file_length < links_to_print ? all_links_in_file_length : links_to_print); result++)
	{
//	printf("%s %d (gamma:%2.2f)\n", all_links_in_file[result].term, all_links_in_file[result].target_document, all_links_in_file[result].gamma);
	printf("%s%d%s%d%s\n", s1, orphan_docid, s2, all_links_in_file[result].target_document, s3);
	}

puts("</outgoing><incoming><link><anchor><file>654321.xml</file><offset>445</offset><length>462</length></anchor><linkto><bep>1</bep></linkto></link></incoming>");
puts("</topic>");
}

/*
	FIND_TERM_IN_LIST()
	-------------------
*/
ANT_link_term *find_term_in_list(char *value, ANT_link_term *list, long list_length, long this_docid)
{
long low, high, mid;

low = 0;
high = list_length;
while (low < high)
	{
	mid = (low + high) / 2;
	if (strcmp(list[mid].term, value) < 0)
		low = mid + 1;
	else
		high = mid;
	}

#ifdef REMOVE_ORPHAN_LINKS
/*
	remove terms that are in the anchor list but have no postings because the only document used the anchor was this one.
*/
while (low < list_length)
	if (list[low].postings[0].doc_link_frequency == 0)		// we've been deleted so this is a miss
		low++;
	else
		break;
/*
	remove terms that point to this document
*/
while (low < list_length)
	if (list[low].postings_length == 1 && list[low].postings[0].docid == this_docid)		// we point to this document so we delete it
		low++;
	else
		break;
#endif

if ((low < list_length) && (strcmp(value, list[low].term) == 0))
	return &list[low];		// match
else
	{
	if (low < list_length)
		return &list[low];		// not found in list but not after the last term in the list
	else
		return NULL;
	}
}

/*
	GENERATE_COLLECTION_LINK_SET()
	------------------------------
*/
void generate_collection_link_set(char *original_file)
{
char *file, *pos, *end, *copy;
long id;

links_in_orphan_length = 0;
file = _strdup(original_file);
pos = strstr(file, "<collectionlink");
while (pos != NULL)
	{
	pos = strstr(pos, "xlink:href=");
	pos = strchr(pos, '"');
	id = atol(pos + 1);
	pos = strchr(pos, '>');
	end = strstr(pos, "</collectionlink");

	copy = strnnew(pos + 1, end - pos - 1);
	string_clean(copy, lowercase_only);

	links_in_orphan[links_in_orphan_length].term = copy;
	links_in_orphan[links_in_orphan_length].gamma = 0;	
	links_in_orphan[links_in_orphan_length].target_document = id;

	links_in_orphan_length++;
	if (links_in_orphan_length >= MAX_LINKS_IN_FILE)
		exit(printf("Too many links present in orphan a priori\n"));
	pos = strstr(pos, "<collectionlink");
	}
delete [] file;

qsort(links_in_orphan, links_in_orphan_length, sizeof(*links_in_orphan), ANT_link::string_target_compare);
}

/*
	ADD_OR_SUBTRACT_ORPHAN_LINKS()
	------------------------------
*/
void add_or_subtract_orphan_links(long add_or_subtract, ANT_link_term *link_index, long terms_in_index)
{
long current, posting;
ANT_link_term key, *found;

for (current = 0; current < links_in_orphan_length; current++)
	{
	if (current > 0)
		if (strcmp(links_in_orphan[current].term, links_in_orphan[current - 1].term) == 0)		// same term
			if (links_in_orphan[current].target_document == links_in_orphan[current - 1].target_document)	// same target
				continue;			// don't do dupicate links as we are only computing the new DF.

	key.term = links_in_orphan[current].term;
	found = (ANT_link_term *)bsearch(&key, link_index, terms_in_index, sizeof(*link_index), ANT_link_term::compare);
	if (found != NULL)
		{
		for (posting = 0; posting < found->postings_length; posting++)
			if (found->postings[posting].docid == links_in_orphan[current].target_document)
				found->postings[posting].doc_link_frequency = found->postings[posting].doc_link_frequency + add_or_subtract;

		if (found->postings_length > 1)
			qsort(found->postings, found->postings_length, sizeof(found->postings[0]), ANT_link_posting::compare);
		}
	}
}

/*
	USAGE()
	-------
*/
void usage(char *exename)
{
exit(printf("Usage:%s [-lowercase] <index> <file_to_link> ...\n", exename));
}

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
static char *seperators = " ";
ANT_disk disk;
char *file, *token, *where_to;
char **term_list, **first, **last, **current;
ANT_link_term *link_index, *index_term, *last_index_term;
long terms_in_index, orphan_docid, param, noom, index_argv_param;
double gamma, numerator, denominator;

if (argc < 3)
	usage(argv[0]);		// and exit

lowercase_only = FALSE;
index_argv_param = 1;

if (*argv[1] == '-')
	{
	if (strcmp(argv[1], "-lowercase") == 0)
		{
		lowercase_only = TRUE;
		index_argv_param = 2;
		}
	else
		usage(argv[0]);		// and exit
	}
link_index = read_index(argv[index_argv_param], &terms_in_index);

print_header();

for (param = index_argv_param + 1; param < argc; param++)
	for (file = disk.read_entire_file(disk.get_first_filename(argv[param])); file != NULL; file = disk.read_entire_file(disk.get_next_filename()))
		{
		all_links_in_file_length = 0;
		orphan_docid = get_doc_id(file);
#ifdef REMOVE_ORPHAN_LINKS
		generate_collection_link_set(file);
		add_or_subtract_orphan_links(SUBTRACT_ORPHAN_LINKS, link_index, terms_in_index);
#endif
		string_clean(file, lowercase_only);

		current = term_list = new char *[strlen(file)];		// this is the worst case by far
		for (token = strtok(file, seperators); token != NULL; token = strtok(NULL, seperators))
			*current++ = token;
		*current = NULL;

		for (first = term_list; *first != NULL; first++)
			{
			where_to = buffer;
			last_index_term = NULL;
			for (last = first; *last != NULL; last++)
				{
				if (where_to == buffer)
					{
					strcpy(buffer, *first);
					where_to = buffer + strlen(buffer);
					}
				else
					{
					*where_to++ = ' ';
					strcpy(where_to, *last);
					where_to += strlen(*last);
					}

				index_term = find_term_in_list(buffer, link_index, terms_in_index, orphan_docid);

				if (index_term == NULL)
					break;									// we're after the last term in the list

				if (strcmp(buffer, index_term->term) == 0)
					last_index_term = index_term;			// we're a term in the list, but might be a longer one so keep looking

				if (strncmp(buffer, index_term->term, strlen(buffer)) != 0)
					break;									// we can't be a substring so we're done
				}

			if (last_index_term != NULL)
				{
				noom = 0;
				denominator = (double)last_index_term->document_frequency;
#ifdef REMOVE_ORPHAN_LINKS
				denominator--;

				if (last_index_term->postings[noom].docid == orphan_docid)			// not alowed to use links that point to the orphan
					noom = 1;
#endif
				numerator = (double)last_index_term->postings[noom].doc_link_frequency;
				gamma = numerator / denominator;
				push_link(*first, last_index_term->term, last_index_term->postings[0].docid, gamma);
//				printf("%s -> %d (gamma = %2.2f / %2.2f)\n", last_index_term->term, last_index_term->postings[0].docid, numerator, denominator);
				}
			}

		qsort(all_links_in_file, all_links_in_file_length, sizeof(*all_links_in_file), ANT_link::compare);

		deduplicate_links();
		qsort(all_links_in_file, all_links_in_file_length, sizeof(*all_links_in_file), ANT_link::final_compare);

		print_links(orphan_docid);

		delete [] file;
		delete [] term_list;
#ifdef REMOVE_ORPHAN_LINKS
		add_or_subtract_orphan_links(ADD_ORPHAN_LINKS, link_index, terms_in_index);
#endif
		}
print_footer();

return 0;
}

