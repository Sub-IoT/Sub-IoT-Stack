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

#include "CastaliaModule.h"

#define CASTALIA_PREFIX "Castalia|\t"

void CastaliaModule::finish()
{
	finishSpecific();
	if (simpleoutputs.size() == 0 && histograms.size() == 0)
		return;
	bool header = true;

	simpleOutputMapType::iterator i1;
	for (i1 = simpleoutputs.begin(); i1 != simpleoutputs.end(); i1++) {
		string descr = i1->first;
		simpleOutputByIndex *outByIndex = &i1->second;
		map<int,simpleOutputTypeDef>::iterator i2;
		for (i2 = outByIndex->byIndex.begin(); i2 != outByIndex->byIndex.end(); i2++) {
			int index = i2->first;
			simpleOutputTypeDef *out = &i2->second;
			if (out->data.size() == 0)
				continue;
			if (header) {
				EV << CASTALIA_PREFIX << "module:" << getFullPath() << endl;
				header = false;
			}
			EV << CASTALIA_PREFIX << "\t";
			if (index >= 0)
				EV << " index:" << index << " ";
			EV << "simple output name:" << descr << endl;
			map<string,double>::iterator i3;
			for (i3 = out->data.begin(); i3 != out->data.end(); i3++)
				EV << CASTALIA_PREFIX "\t\t" << i3->second << " " << i3->first << endl;
		}
	}
	simpleoutputs.clear();

	histogramOutputMapType::iterator i4;
	for (i4 = histograms.begin(); i4 != histograms.end(); i4++) {
		string descr = i4->first;
		histogramOutputByIndex *hist = &i4->second;
		if (!hist->active)
			continue;
		map <int,histogramOutputTypeDef>::iterator i5;
		for (i5 = hist->byIndex.begin(); i5 != hist->byIndex.end(); i5++) {
			int index = i5->first;
			histogramOutputTypeDef *histBuckets = &i5->second;
			if (header) {
				EV << CASTALIA_PREFIX << "module:" << getFullPath() << endl;
				header = false;
			}
			EV << CASTALIA_PREFIX << "\t";
			if (index >= 0)
				EV << " index:" << index << " ";
			EV << "histogram name:" << descr << endl;
			EV << CASTALIA_PREFIX << "\thistogram_min:" << hist->min <<
				" histogram_max:" << hist->max << endl;
			EV << CASTALIA_PREFIX << "\thistogram_values";
			for (int i = 0; i <= hist->numBuckets; i++)
				EV << " " << histBuckets->buckets[i];
			EV << endl;
		}
	}
	histograms.clear();
}

std::ostream & CastaliaModule::trace()
{
	if (hasPar("collectTraceInfo") && par("collectTraceInfo")) {
		return (ostream &) DebugInfoWriter::getStream() <<
			"\n" << setw(16) << simTime() << setw(40) << getFullPath() << " ";
	} else {
		return empty;
	}
}

std::ostream & CastaliaModule::debug()
{
	return cerr;
}

void CastaliaModule::declareOutput(const char *descr)
{
	simpleoutputs[descr].byIndex.clear();
}

void CastaliaModule::collectOutput(const char *descr, int index)
{
	if (index < 0)
		opp_error("Negative output index not permitted");
	collectOutputNocheck(descr, index, "", 1);
}

void CastaliaModule::collectOutput(const char *descr, int index,
						const char *label, double amt)
{
	if (index < 0)
		opp_error("Negative output index not permitted");
	collectOutputNocheck(descr, index, label, amt);
}

void CastaliaModule::collectOutputNocheck(const char *descr, int index,
						const char *label, double amt)
{
	simpleOutputMapType::iterator i = simpleoutputs.find(descr);
	if (i == simpleoutputs.end())
		return;
	i->second.byIndex[index].data[label] += amt;
}

void CastaliaModule::collectOutput(const char *descr, int index, const char *label)
{
	if (index < 0)
		opp_error("Negative output index not permitted");
	collectOutputNocheck(descr, index, label, 1);
}

void CastaliaModule::declareHistogram(const char *descr, double min,
					double max, int buckets)
{
	if (min >= max || buckets < 1)
		opp_error("ERROR: declareHistogram failed, bad parameters");

	histogramOutputByIndex hist;
	hist.min = min;
	hist.max = max;
	hist.cell = (max - min) / buckets;
	hist.numBuckets = buckets;
	hist.active = false;
	histograms[descr] = hist;
}

void CastaliaModule::collectHistogram(const char *descr, int index, double value)
{
	if (index < 0)
		opp_error("Negative output index not permitted");
	collectHistogramNocheck(descr, index, value);
}

void CastaliaModule::collectHistogramNocheck(const char *descr, int index, double value)
{
	histogramOutputMapType::iterator i1 = histograms.find(descr);
	if (i1 == histograms.end())
		return;
	int num;
	if (value < i1->second.min)
		return;
	else if (value >= i1->second.max)
		num = i1->second.numBuckets;
	else
		num = (int)floor((value - i1->second.min) / i1->second.cell);
	map <int,histogramOutputTypeDef>::iterator i2 = i1->second.byIndex.find(index);
	if (i2 == i1->second.byIndex.end()) {
		histogramOutputTypeDef histBuckets;
		histBuckets.buckets.resize(i1->second.numBuckets + 1);
		for (int i = 0; i <= i1->second.numBuckets; i++)
			histBuckets.buckets[i] = 0;
		i1->second.byIndex[index] = histBuckets;
	}
	i1->second.byIndex[index].buckets[num]++;
	i1->second.active = true;
}

void CastaliaModule::powerDrawn(double power)
{
    if (!classPointers.resourceManager) {
		string name(getName());
		if (name.compare("Radio") == 0) 
			classPointers.resourceManager = getParentModule()->getParentModule()->getSubmodule("ResourceManager");
		else if (name.compare("SensorManager") == 0)
			classPointers.resourceManager = getParentModule()->getSubmodule("ResourceManager");
		else
			opp_error("%s module has no rights to call drawPower() function", getFullPath().c_str());
        if (!classPointers.resourceManager) 
        	opp_error("Unable to find pointer to resource manager module");
    }

	ResourceManagerMessage *drawPowerMsg =
	    new ResourceManagerMessage("Power consumption message", RESOURCE_MANAGER_DRAW_POWER);
	drawPowerMsg->setPowerConsumed(power);
	sendDirect(drawPowerMsg, classPointers.resourceManager, "powerConsumption");
}
