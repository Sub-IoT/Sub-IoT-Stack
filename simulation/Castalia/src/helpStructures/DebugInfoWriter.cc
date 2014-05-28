/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis                 *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#include "DebugInfoWriter.h"

ofstream DebugInfoWriter::theFile;
string DebugInfoWriter::fileName;

DebugInfoWriter::DebugInfoWriter(const string & fName)
{
	fileName = fName;
}

void DebugInfoWriter::setDebugFileName(const string & fName)
{
	fileName = fName;
}

ofstream & DebugInfoWriter::getStream(void)
{
	if (!theFile.is_open())
		theFile.open(fileName.c_str(), ios::app);
	theFile.setf(ios::left);

	return theFile;
}

void DebugInfoWriter::closeStream(void)
{
	if (theFile.is_open())
		theFile.close();
}
