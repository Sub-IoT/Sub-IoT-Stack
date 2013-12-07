//
// Generated file, do not edit! Created by opp_msgc 4.3 from src/physicalProcess/PhysicalProcessMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "PhysicalProcessMessage_m.h"

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




Register_Class(PhysicalProcessMessage);

PhysicalProcessMessage::PhysicalProcessMessage(const char *name, int kind) : cMessage(name,kind)
{
    this->value_var = 0;
    this->xCoor_var = 0;
    this->yCoor_var = 0;
    this->srcID_var = 0;
    this->sensorIndex_var = 0;
}

PhysicalProcessMessage::PhysicalProcessMessage(const PhysicalProcessMessage& other) : cMessage(other)
{
    copy(other);
}

PhysicalProcessMessage::~PhysicalProcessMessage()
{
}

PhysicalProcessMessage& PhysicalProcessMessage::operator=(const PhysicalProcessMessage& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void PhysicalProcessMessage::copy(const PhysicalProcessMessage& other)
{
    this->value_var = other.value_var;
    this->xCoor_var = other.xCoor_var;
    this->yCoor_var = other.yCoor_var;
    this->srcID_var = other.srcID_var;
    this->sensorIndex_var = other.sensorIndex_var;
}

void PhysicalProcessMessage::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->value_var);
    doPacking(b,this->xCoor_var);
    doPacking(b,this->yCoor_var);
    doPacking(b,this->srcID_var);
    doPacking(b,this->sensorIndex_var);
}

void PhysicalProcessMessage::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->value_var);
    doUnpacking(b,this->xCoor_var);
    doUnpacking(b,this->yCoor_var);
    doUnpacking(b,this->srcID_var);
    doUnpacking(b,this->sensorIndex_var);
}

double PhysicalProcessMessage::getValue() const
{
    return value_var;
}

void PhysicalProcessMessage::setValue(double value)
{
    this->value_var = value;
}

double PhysicalProcessMessage::getXCoor() const
{
    return xCoor_var;
}

void PhysicalProcessMessage::setXCoor(double xCoor)
{
    this->xCoor_var = xCoor;
}

double PhysicalProcessMessage::getYCoor() const
{
    return yCoor_var;
}

void PhysicalProcessMessage::setYCoor(double yCoor)
{
    this->yCoor_var = yCoor;
}

int PhysicalProcessMessage::getSrcID() const
{
    return srcID_var;
}

void PhysicalProcessMessage::setSrcID(int srcID)
{
    this->srcID_var = srcID;
}

int PhysicalProcessMessage::getSensorIndex() const
{
    return sensorIndex_var;
}

void PhysicalProcessMessage::setSensorIndex(int sensorIndex)
{
    this->sensorIndex_var = sensorIndex;
}

class PhysicalProcessMessageDescriptor : public cClassDescriptor
{
  public:
    PhysicalProcessMessageDescriptor();
    virtual ~PhysicalProcessMessageDescriptor();

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

Register_ClassDescriptor(PhysicalProcessMessageDescriptor);

PhysicalProcessMessageDescriptor::PhysicalProcessMessageDescriptor() : cClassDescriptor("PhysicalProcessMessage", "cMessage")
{
}

PhysicalProcessMessageDescriptor::~PhysicalProcessMessageDescriptor()
{
}

bool PhysicalProcessMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<PhysicalProcessMessage *>(obj)!=NULL;
}

const char *PhysicalProcessMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int PhysicalProcessMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount(object) : 5;
}

unsigned int PhysicalProcessMessageDescriptor::getFieldTypeFlags(void *object, int field) const
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
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *PhysicalProcessMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "value",
        "xCoor",
        "yCoor",
        "srcID",
        "sensorIndex",
    };
    return (field>=0 && field<5) ? fieldNames[field] : NULL;
}

int PhysicalProcessMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='v' && strcmp(fieldName, "value")==0) return base+0;
    if (fieldName[0]=='x' && strcmp(fieldName, "xCoor")==0) return base+1;
    if (fieldName[0]=='y' && strcmp(fieldName, "yCoor")==0) return base+2;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcID")==0) return base+3;
    if (fieldName[0]=='s' && strcmp(fieldName, "sensorIndex")==0) return base+4;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *PhysicalProcessMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "double",
        "double",
        "double",
        "int",
        "int",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : NULL;
}

const char *PhysicalProcessMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int PhysicalProcessMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    PhysicalProcessMessage *pp = (PhysicalProcessMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string PhysicalProcessMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    PhysicalProcessMessage *pp = (PhysicalProcessMessage *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->getValue());
        case 1: return double2string(pp->getXCoor());
        case 2: return double2string(pp->getYCoor());
        case 3: return long2string(pp->getSrcID());
        case 4: return long2string(pp->getSensorIndex());
        default: return "";
    }
}

bool PhysicalProcessMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    PhysicalProcessMessage *pp = (PhysicalProcessMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setValue(string2double(value)); return true;
        case 1: pp->setXCoor(string2double(value)); return true;
        case 2: pp->setYCoor(string2double(value)); return true;
        case 3: pp->setSrcID(string2long(value)); return true;
        case 4: pp->setSensorIndex(string2long(value)); return true;
        default: return false;
    }
}

const char *PhysicalProcessMessageDescriptor::getFieldStructName(void *object, int field) const
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
        NULL,
        NULL,
    };
    return (field>=0 && field<5) ? fieldStructNames[field] : NULL;
}

void *PhysicalProcessMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    PhysicalProcessMessage *pp = (PhysicalProcessMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


