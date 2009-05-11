/*
	COMPRESS_ELIAS_DELTA.H
	----------------------
*/
#ifndef __COMPRESS_ELIAS_DELTA_H__
#define __COMPRESS_ELIAS_DELTA_H__

#include "compress_elias_gamma.h"

/*
	class ANT_COMPRESS_ELIAS_DELTA
	------------------------------
*/
class ANT_compress_elias_delta : protected ANT_compress_elias_gamma
{
protected:
	inline void encode(unsigned long long val)
        {
        long exp = ANT_floor_log2(++val);

        ANT_compress_elias_gamma::encode(exp + 1);
        bitstream.push_bits(val, exp);
        }

	inline unsigned long long decode(void)
        {
        long exp = (long)ANT_compress_elias_gamma::decode() - 1;

        return ((((unsigned long long)1) << exp) | bitstream.get_bits(exp)) - 1;
        }

public:
	ANT_compress_elias_delta() {}
	virtual ~ANT_compress_elias_delta() {}

	virtual long long compress(unsigned char *destination, long long destination_length, ANT_compressable_integer *source, long long source_integers);
	virtual void decompress(ANT_compressable_integer *destination, unsigned char *source, long long destination_integers);
} ;

#endif __COMPRESS_ELIAS_DELTA_H__
