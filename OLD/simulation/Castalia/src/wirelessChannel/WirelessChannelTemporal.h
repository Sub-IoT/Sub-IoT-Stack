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

#ifndef __CHANNELMODEL_H
#define __CHANNELMODEL_H

#include <omnetpp.h>
#include "random.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>

#define SIGNAL_VAR "Signal variability (dB)"
#define CORR_TIME "Correlation times (msec)"
#define COH_TIME "Coherence time (msec)"

struct PDFLayerType {
	//This and any further reference to 'value' means a value of signal variation
	float *values;
	int *sublayers;
	int numOfTotalElements;
	int numOfValues;
	int numOfSublayers;
	char id;
};

struct PDFType {
	PDFLayerType *layers;
	int numOfLayers;
};

struct correlationTimeType {
	PDFType *pdfs;
	double time;
};

class channelTemporalModel {
 private:
	//the following parameters are read from the channel model file, parameter 'Signal Variability'
	float minSignalVariation;
	float maxSignalVariation;
	float signalVariationResolution;

	//the following are the dimensions of our PDF matrix. That is we expect one PDF for 
	//every combination of discrete signal value and correlation time interval
	int numOfSignalVariationValues;	//this value is calculated based on 'Signal Variability' parameter
	int numOfCorrelationTimes;		//this is taken directly from 'Correlation times' parameter

	int rngNum;						//random number generator to use

	double coherenceTime;			//value of time that needs to be exceeded in order to draw from coherencePDF

	correlationTimeType *correlationTime;	//this is the main matrix of PDFs
	PDFType *coherencePDF;					//a standalone coherence PDF is used when previous signal level is unknown 
											//or too old (i.e. time passed > coherenceTime)

	float drawFromPDF(PDFType *);
	float parseFloat(const char *);
	void parsePDF(const char *, PDFType *);
	void parseLayer(const char *, PDFLayerType *);
	int calculateValueIndex(float);
	int isValidLayer(const char *);

 public:
	 channelTemporalModel(const char *, int);
	~channelTemporalModel();
	double runTemporalModel(double, float *);
};

#endif
