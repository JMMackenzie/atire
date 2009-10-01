/*
	ASSESSMENT_INEX.H
	-----------------
*/
#ifndef ASSESSMENT_INEX_H_
#define ASSESSMENT_INEX_H_

#include "assessment.h"

/*
	class ANT_ASSESSMENT_INEX
	-------------------------
*/
class ANT_assessment_INEX : public ANT_assessment
{
friend class ANT_assessment_factory;

protected:
	ANT_assessment_INEX() {}

public:
	ANT_assessment_INEX(ANT_memory *mem, char **docid_list, long long documents) : ANT_assessment(mem, docid_list, documents) {}
	ANT_relevant_document *read(char *filename, long long *reldocs);
} ;



#endif  /* ASSESSMENT_INEX_H_ */
