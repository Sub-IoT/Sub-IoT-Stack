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

#include "WirelessChannelTemporal.h"

//Constructor for channelTemporalModel, the key parameter here is the input file to read the 
//model data from. Whole parsing of the file is done here
channelTemporalModel::channelTemporalModel(const char *file, int rng)
{
	correlationTime = NULL;
	coherencePDF = NULL;
	rngNum = rng;

	std::string s;
	std::ifstream f(file);
	const char *ct;
	int param_count = 0;

	if (!f.is_open()) {
		std::cout << "[TemporalModel] Unable to open " << file << std::endl;
		exit(1);
	}

    /************************************************************************
     * The parameters file has the following syntax:
     *	- comment lines start with %
     *	- The file contains two parts: header and body
     *	- header is a definition of three key parameters of the model: 
     *	    Signal variability (dB): [MIN:RESOLUTION:MAX]
     *	    Correlation times (msec): [T1,T2,..,TN], where Tn < Tn-1 for any n, 
     *						     i.e. a list of decreasing numbers
     *      Coherence time (msec): Tc
     *	- body is a list of PDFs for each pair of correlation time value and
     *	  signal variability value defined in the header. Also a separate PDF
     *	  for coherence time is expected.
     *  - pdf description syntax:
     *    TIME,[value]: level_description; [letter= level_description;] ... [letter= level_description;]
     *	  level_description:= number ... number [letter] ... [letter]
     *
     ************************************************************************/

	while (getline(f, s)) {
		cStringTokenizer t(s.c_str(), ":");
		while ((ct = t.nextToken())) {
			while (ct[0] == ' ') {
				ct++;
			}
			if (!ct[0] || ct[0] == '%') {
				break;
			}

			if (param_count > 2) {	//param_count is the number of header parameters that was parsed already
				//parsing of the body may begin only when all three header parameters are found
				//Since we are parsing the body, each valid line here is a pdf description. 
				//And valid pdf description can only start with a time value (either from correlation 
				//times list or coherence time)
				float time = parseFloat(ct);
				if (time == coherenceTime) {	//we discovered coherence pdf definition
					if (coherencePDF) {
						std::cout << "[TemporalModel] ERROR two coherence PDFs defined" << std::endl;
						exit(1);
					}
					coherencePDF = new PDFType;
					parsePDF(t.nextToken(), coherencePDF);
				} else {	//this is not coherence pdf, so the signal variability value is expected
					while (ct[0] && ct[0] != ',') {
						ct++;
					}
					float value = parseFloat(++ct);

					//index in the pdfs array is calculated based on the signal value
					//the pdf is then placed in the main array according to the time and index values
					int index = calculateValueIndex(value);
					for (int i = 0; i < numOfCorrelationTimes; i++) {
						if (correlationTime[i].time == time) {
							if (correlationTime[i].pdfs[index].numOfLayers > 0) {
								std::cout << "[TemporalModel] ERROR two pdfs defined for the same ID: "
								    << time << "," << value << "(" << index << ")" << std::endl;
								exit(1);
							} else {
								parsePDF(t.nextToken(), &correlationTime[i].pdfs[index]);
							}
						}
					}
				}

			//Parse the header parameters (param_count <= 2)
			} else if (strncmp(ct, SIGNAL_VAR, strlen(SIGNAL_VAR)) == 0) {
				minSignalVariation = parseFloat(t.nextToken());
				signalVariationResolution = parseFloat(t.nextToken());
				maxSignalVariation = parseFloat(t.nextToken());
				if (minSignalVariation >= maxSignalVariation) {
					std::cout << "[TemporalModel] ERROR: " << SIGNAL_VAR <<
					    " min value cannot exceed max value (format min:res:max)" << std::endl;
					exit(1);
				}
				//calculate the value of numOfSignalVariationValues based on
				//the min, max and resolution that was provided
				numOfSignalVariationValues = lround((maxSignalVariation - minSignalVariation) 
						/ signalVariationResolution) + 1;
				param_count++;
			} else if (strncmp(ct, CORR_TIME, strlen(CORR_TIME)) == 0) {
				t.setDelimiter(",");
				std::vector < std::string > v = t.asVector();
				numOfCorrelationTimes = v.size();
				if (numOfCorrelationTimes == 0) {
					std::cout << "[TemporalModel] ERROR: " << CORR_TIME <<
					    " expected comma separated list of decreasing numbers" << std::endl;
					exit(1);
				}
				//create and initialise the main correlationTime array based on the values in the correlation time list
				correlationTime = new correlationTimeType[numOfCorrelationTimes];
				for (int i = 0; i < numOfCorrelationTimes; i++) {
					correlationTime[i].time = parseFloat(v[i].c_str());
					correlationTime[i].pdfs = new PDFType[numOfSignalVariationValues];
					if (i > 0 && correlationTime[i].time >= correlationTime[i - 1].time) {
						std::cout << "[TemporalModel] ERROR: " << CORR_TIME <<
						    " expected comma separated list of decreasing numbers" << std::endl;
						exit(1);
					}
					//each PDF is initialised with 0 layers. After the parsing all PDFs are 
					//checked to have atleast one layer otherwise the file is incorrect or 
					//incomplete and the model will not work
					for (int j = 0; j < numOfSignalVariationValues; j++) {
						correlationTime[i].pdfs[j].numOfLayers = 0;
					}
				}
				param_count++;
			} else if (strncmp(ct, COH_TIME, strlen(COH_TIME)) == 0) {
				coherenceTime = parseFloat(t.nextToken());
				param_count++;
			}
		}
	}
	f.close();

	//check that each PDF has atleast one layer
	for (int i = 0; i < numOfCorrelationTimes; i++) {
		for (int j = 0; j < numOfSignalVariationValues; j++) {
			if (correlationTime[i].pdfs[j].numOfLayers == 0) {
				std::cout << "[TemporalModel] ERROR: missing PDF description for " << 
					correlationTime[i].time << "," << (j * signalVariationResolution)
				    + minSignalVariation << std::endl;
				exit(1);
			}
		}
	}
}

channelTemporalModel::~channelTemporalModel()
{
	if (coherencePDF) {
		delete coherencePDF;
	}
	if (correlationTime) {
		for (int i = 0; i < numOfCorrelationTimes; i++) {
			for (int j = 0; j < numOfSignalVariationValues; j++) {
				for (int k = 0; k < correlationTime[i].pdfs[j].numOfLayers; k++) {
					PDFLayerType *l = &correlationTime[i].pdfs[j].layers[k];
					if (l->numOfValues > 0) {
						delete[]l->values;
					}
					if (l->numOfSublayers > 0) {
						delete[]l->sublayers;
					}
				}
				delete[]correlationTime[i].pdfs[j].layers;
			}
			delete[]correlationTime[i].pdfs;
		}
		delete[]correlationTime;
	}
}

//calculates index of a PDF in the array based on the given signal value f
int channelTemporalModel::calculateValueIndex(float f)
{
	if (f <= minSignalVariation) {
		return 0;
	}
	if (f >= maxSignalVariation) {
		return numOfSignalVariationValues - 1;
	}
	return lround((f - minSignalVariation) / signalVariationResolution);
}

//wrapper function for strtof(...)
float channelTemporalModel::parseFloat(const char *str)
{
	if (str != NULL && str[0]) {
		char *tmp = NULL;
		float result = strtof(str, &tmp);
		if (str != tmp) {
			return result;
		}
	}
	std::cout << "[TemporalModel] ERROR: Unable to parse number value from string '"
	    << str << "'" << std::endl;
	exit(1);
}

//Function to fill the PDFType structure pointed by pdf, based on a string pointed by str
void channelTemporalModel::parsePDF(const char *str, PDFType * pdf)
{
	cStringTokenizer t(str, ";");
	std::vector < std::string > v = t.asVector();
	pdf->numOfLayers = v.size();
	pdf->layers = new PDFLayerType[v.size()];
	for (int i = 0; i < (int)v.size(); i++) {
		parseLayer(v[i].c_str(), &pdf->layers[i]);
	}
	for (int i = 0; i < (int)v.size(); i++) {
		PDFLayerType *layer = &pdf->layers[i];
		for (int j = 0; j < (int)layer->numOfSublayers; j++) {
			for (int k = 0; k < (int)v.size(); k++) {
				if (layer->sublayers[j] == pdf->layers[k].id) {
					layer->sublayers[j] = k;
					break;
				}
				if (k == (int)v.size() - 1) {
					std::cout << "[TemporalModel] ERROR: Unable to parse PDF - malformed layers (string is '"
					    << str << "')" << std::endl;
					exit(1);
				}
			}
		}
	}
}

//Function to fill PDFLayerType structure pointed by layer, based on a string pointed by str
void channelTemporalModel::parseLayer(const char *str, PDFLayerType * layer)
{
	layer->values = NULL;
	layer->sublayers = NULL;
	while (str[0] == ' ') {
		str++;
	}
	int tmp = 1;
	while (str[tmp] && (str[tmp] == ' ' || str[tmp] == '\t')) {
		tmp++;
	}
	if (str[tmp] == '=') {
		layer->id = str[0];
		str += tmp + 1;
	} else {
		layer->id = '-';
	}
	cStringTokenizer t(str, " ");
	std::vector < std::string > v = t.asVector();
	if (v.size() < 1) {
		std::cout << "[TemporalModel] ERROR: Bad layer syntax at '" << str << std::endl;
		exit(1);
	}

	layer->numOfTotalElements = v.size();
	int sublayers = 0;
	for (int i = 0; i < (int)v.size(); i++) {
		if (isValidLayer(v[i].c_str())) {
			sublayers++;
		}
	}

	layer->numOfValues = v.size() - sublayers;
	layer->numOfSublayers = sublayers;
	if (layer->numOfValues > 0) {
		layer->values = new float[layer->numOfValues];
	}
	if (layer->numOfSublayers > 0) {
		layer->sublayers = new int[layer->numOfSublayers];
	}
	sublayers = 0;
	int values = 0;

	for (int i = 0; i < (int)v.size(); i++) {
		if (isValidLayer(v[i].c_str())) {
			layer->sublayers[sublayers++] = v[i].c_str()[0];
		} else {
			layer->values[values++] = parseFloat(v[i].c_str());
		}
	}
}

int channelTemporalModel::isValidLayer(const char *c)
{
	if (!c[1]
	    && ((c[0] >= 'a' && c[0] <= 'z') || (c[0] >= 'A' && c[0] <= 'Z'))) {
		return 1;
	}
	return 0;
}

//This is the core function of the model, it will return a signal variation value from a given PDF
float channelTemporalModel::drawFromPDF(PDFType * pdf)
{
	int currentLayer = 0;	//we always start at the first layer
	int index = 0;
	PDFLayerType *layer;
	int guard = 0;

	while (guard < 100) {	//Since a PDF is allowed to have recursive layers 
							//(i.e. Layer linking to itself such as 'A = 1 2 3 4 5 A;')
							//we want to limit the number of allowed draws from a given PDF to 
							//avoid infinite recursion if a PDF has been defined carelessly
		guard++;
		layer = &pdf->layers[currentLayer];
		index = genk_intrand(rngNum, layer->numOfTotalElements);
		if (index < layer->numOfValues) {
			return layer->values[index];
		} else {
			currentLayer =
			    layer->sublayers[index - layer->numOfValues];
		}
	}
	std::cout << "[TemporalModel] ERROR: too deep recursion in channelTemporalModel::drawFromPDF"
	    << std::endl;
	exit(1);
}

//Another core function of the model. This function will update signal variation value pointed by value_ptr
//based on the time that has passed since last observation. It will return the time value which has been
//processed by the model (that is return value can never exceed input value of time)
//Also if time is greater than coherence Time, the value from coherencePDF is drawn, and input time value is 
//returned
double channelTemporalModel::runTemporalModel(double time, float *value_ptr)
{
	if (time == 0 || time >= coherenceTime) {
		*value_ptr = drawFromPDF(coherencePDF);
		return time;
	}
	double remaining_time = time;
	for (int i = 0; i < numOfCorrelationTimes; i++) {
		while (remaining_time >= correlationTime[i].time) {
			remaining_time -= correlationTime[i].time;
			*value_ptr = drawFromPDF(&correlationTime[i].pdfs[calculateValueIndex(*value_ptr)]);
		}
	}
	return time - remaining_time;
}

