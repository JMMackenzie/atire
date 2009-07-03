/*
	READABILITY_FACTORY.C
	---------------------
*/
#include "readability_factory.h"
#include "readability_none.h"
#include "readability_dale_chall.h"

/*
	READABILITY_FACTORY::GET_NEXT_TOKEN()
	-------------------------------------
*/
ANT_string_pair *ANT_readability_factory::get_next_token()
{
return measure->get_next_token();
}

/*
	READABILITY_FACTORY::SET_DOCUMENT()
	-----------------------------------
*/
void ANT_readability_factory::set_document(unsigned char *document)
{
measure->set_document(document);
}

/*
	READABILITY_FACTORY::SCORE()
	----------------------------
*/
long ANT_readability_factory::score()
{
return this->measure->score();
}

/*
	READABILITY_FACTORY::SET_MEASURE()
	----------------------------------
*/
void ANT_readability_factory::set_measure(unsigned long m)
{
switch (m)
	{
	case NONE: 
		this->measure = new ANT_readability_none(); 
		break;
	case DALE_CHALL: 
		this->measure = new ANT_readability_dale_chall(); 
		break;
	default: 
		this->measure = new ANT_readability_none(); 
		break; // shouldn't happen
	}
}

/*
	READABILITY_FACTORY::SET_PARSER()
	---------------------------------
*/
void ANT_readability_factory::set_parser(ANT_parser *parser)
{
this->measure->set_parser(parser);
this->parser = parser;
}
