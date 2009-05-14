/*
	ASSESSMENT_FACTORY.H
	--------------------
*/
#ifndef __ASSESSMENT_FACTORY_H__
#define __ASSESSMENT_FACTORY_H__

#include "assessment.h"

/*
	class ANT_ASSESSMENT_FACTORY
	----------------------------
*/
class ANT_assessment_factory : public ANT_assessment
{
private:
	ANT_assessment *factory;

public:
	ANT_assessment_factory(ANT_memory *mem, char **docid_list, long long documents) : ANT_assessment(mem, docid_list, documents) { factory = NULL; }
	virtual ~ANT_assessment_factory();

	virtual ANT_relevant_document *read(char *filename, long long *reldocs);
} ;



#endif __ASSESSMENT_FACTORY_H__
