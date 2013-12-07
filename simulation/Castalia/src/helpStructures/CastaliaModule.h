/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev                                            *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#ifndef BASECASTALIAMODULE
#define BASECASTALIAMODULE

#include <map>
#include <vector>
#include <omnetpp.h>
#include <iostream>
#include <iomanip>

#include "DebugInfoWriter.h"
#include "CastaliaMessages.h"
#include "ResourceManagerMessage_m.h"

using namespace std;

struct nullstream:ostream {
	struct nullbuf:streambuf {
		int overflow(int c) {
			return traits_type::not_eof(c);
		}	
	}
	m_sbuf;
	nullstream():ios(&m_sbuf), ostream(&m_sbuf) { }
};

struct simpleOutputTypeDef {
	map <string,double> data;
};

struct histogramOutputTypeDef {
	vector <int> buckets;
};

struct simpleOutputByIndex {
	map <int, simpleOutputTypeDef> byIndex;
};

struct histogramOutputByIndex {
	double min;
	double max;
	double cell;
	int numBuckets;
	bool active;
	map <int, histogramOutputTypeDef> byIndex;
};

struct classPointersType {
	cModule *resourceManager;

	/* initialize the struct (C++ syntax) */
	classPointersType():resourceManager(NULL) { }
};

typedef map <string, simpleOutputByIndex> simpleOutputMapType;
typedef map <string, histogramOutputByIndex> histogramOutputMapType;

class CastaliaModule: public virtual cSimpleModule {
 private:
	simpleOutputMapType simpleoutputs;
	histogramOutputMapType histograms;

	classPointersType classPointers;

	void collectOutputNocheck(const char *, int, const char *, double);
	void collectHistogramNocheck(const char *, int, double);

 protected:
	virtual void finish();
	virtual void finishSpecific() { };
	std::ostream & trace();
	std::ostream & debug();

	nullstream empty;

	void declareOutput(const char *);

	void collectOutput(const char *, int);
	void collectOutput(const char *, int, const char *);
	void collectOutput(const char *, int, const char *, double);

	void collectOutput(const char *descr) {
		collectOutputNocheck(descr, -1, "", 1);
	}
	void collectOutput(const char *descr, const char *label) {
		collectOutputNocheck(descr, -1, label, 1);
	}
	void collectOutput(const char *descr, const char *label, double amt) {
		collectOutputNocheck(descr, -1, label, amt);
	}

	void declareHistogram(const char *, double, double, int);
	void collectHistogram(const char *, int, double);
	void collectHistogram(const char *descr, double value) {
		collectHistogramNocheck(descr, -1, value);
	}

	void powerDrawn(double);

};

#endif
