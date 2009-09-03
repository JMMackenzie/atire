/*
 * qfreq.cpp
 *
 *  Created on: Jan 15, 2009
 *      Author: monfee
 */

#include "qfreq.h"
#include "uniseg_settings.h"
#include "freq_loader.h"

#include <iostream>

using namespace std;


QFreq::QFreq() {
	k_ = -1;
	loaded_ = false;
}

QFreq::~QFreq() {

}

void QFreq::load_freq(int n, bool force) {

	if (k_ > -1 && !force)
		return;

	/************************************
	 *  LOAD FREQUENCY TABLE IN MEMORY
	 ***********************************/
	cerr << "loading frequency files " << endl;

	FreqLoader loader(freq_);

	if (UNISEG_settings::instance().freqs_path.length() <= 0) {
		cerr << "cann't find the frequency table path " << endl;
		exit(-1);
	}

	loader.load(UNISEG_settings::instance().freqs_path, n);
	freq_.cal_sum_n_avg();
	k_ = loader.count();

	loaded_ = true;
}

QFreq& QFreq::instance() {
	static QFreq inst;
	return inst;
}
