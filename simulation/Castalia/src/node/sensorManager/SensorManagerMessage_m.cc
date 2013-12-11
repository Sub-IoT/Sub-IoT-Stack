//
// Generated file, do not edit! Created by opp_msgc 4.3 from src/node/sensorManager/SensorManagerMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "SensorManagerMessage_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




Register_Class(SensorReadingMessage);

SensorReadingMessage::SensorReadingMessage(const char *name, int kind) : cMessage(name,kind)
{
    this->sensedValue_var = 0;
    this->sensorType_var = 0;
    this->sensorIndex_var = 0;
}

SensorReadingMessage::SensorReadingMessage(const SensorReadingMessage& other) : cMessage(other)
{
    copy(other);
}

SensorReadingMessage::~SensorReadingMessage()
{
}

SensorReadingMessage& SensorReadingMessage::operator=(const SensorReadingMessage& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void SensorReadingMessage::copy(const SensorReadingMessage& other)
{
    this->sensedValue_var = other.sensedValue_var;
    this->sensorType_var = other.sensorType_var;
    this->sensorIndex_var = other.sensorIndex_var;
}

void SensorReadingMessage::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->sensedValue_var);
    doPacking(b,this->sensorType_var);
    doPacking(b,this->sensorIndex_var);
}

void SensorReadingMessage::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->sensedValue_var);
    doUnpacking(b,this->sensorType_var);
    doUnpacking(b,this->sensorIndex_var);
}

double SensorReadingMessage::getSensedValue() const
{
    return sensedValue_var;
}

void SensorReadingMessage::setSensedValue(double sensedValue)
{
    this->sensedValue_var = sensedValue;
}

const char * SensorReadingMessage::getSensorType() const
{
    return sensorType_var.c_str();
}

void SensorReadingMessage::setSensorType(const char * sensorType)
{
    this->sensorType_var = sensorType;
}

int SensorReadingMessage::getSensorIndex() const
{
    return sensorIndex_var;
}

void SensorReadingMessage::setSensorIndex(int sensorIndex)
{
    this->sensorIndex_var = sensorIndex;
}

class SensorReadingMessageDescriptor : public cClassDescriptor
{
  public:
    SensorReadingMessageDescriptor();
    virtual ~SensorReadingMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(SensorReadingMessageDescriptor);

SensorReadingMessageDescriptor::SensorReadingMessageDescriptor() : cClassDescriptor("SensorReadingMessage", "cMessage")
{
}

SensorReadingMessageDescriptor::~SensorReadingMessageDescriptor()
{
}

bool SensorReadingMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<SensorReadingMessage *>(obj)!=NULL;
}

const char *SensorReadingMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int SensorReadingMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int SensorReadingMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<3) ? fieldTypeFlags[field] : 0;
}

const char *SensorReadingMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "sensedValue",
        "sensorType",
        "sensorIndex",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int SensorReadingMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sensedValue")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "sensorType")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "sensorIndex")==0) return base+2;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *SensorReadingMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "double",
        "string",
        "int",
    };
    return (field>=0 && field<3) ? fieldTypeStrings[field] : NULL;
}

const char *SensorReadingMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int SensorReadingMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    SensorReadingMessage *pp = (SensorReadingMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string SensorReadingMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    SensorReadingMessage *pp = (SensorReadingMessage *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->getSensedValue());
        case 1: return oppstring2string(pp->getSensorType());
        case 2: return long2string(pp->getSensorIndex());
        default: return "";
    }
}

bool SensorReadingMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    SensorReadingMessage *pp = (SensorReadingMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setSensedValue(string2double(value)); return true;
        case 1: pp->setSensorType((value)); return true;
        case 2: pp->setSensorIndex(string2long(value)); return true;
        default: return false;
    }
}

const char *SensorReadingMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<3) ? fieldStructNames[field] : NULL;
}

void *SensorReadingMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    SensorReadingMessage *pp = (SensorReadingMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


