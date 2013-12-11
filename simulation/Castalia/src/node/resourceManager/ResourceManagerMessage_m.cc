//
// Generated file, do not edit! Created by opp_msgc 4.3 from src/node/resourceManager/ResourceManagerMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "ResourceManagerMessage_m.h"

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




Register_Class(ResourceManagerMessage);

ResourceManagerMessage::ResourceManagerMessage(const char *name, int kind) : cMessage(name,kind)
{
    this->powerConsumed_var = 0;
}

ResourceManagerMessage::ResourceManagerMessage(const ResourceManagerMessage& other) : cMessage(other)
{
    copy(other);
}

ResourceManagerMessage::~ResourceManagerMessage()
{
}

ResourceManagerMessage& ResourceManagerMessage::operator=(const ResourceManagerMessage& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void ResourceManagerMessage::copy(const ResourceManagerMessage& other)
{
    this->powerConsumed_var = other.powerConsumed_var;
}

void ResourceManagerMessage::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->powerConsumed_var);
}

void ResourceManagerMessage::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->powerConsumed_var);
}

double ResourceManagerMessage::getPowerConsumed() const
{
    return powerConsumed_var;
}

void ResourceManagerMessage::setPowerConsumed(double powerConsumed)
{
    this->powerConsumed_var = powerConsumed;
}

class ResourceManagerMessageDescriptor : public cClassDescriptor
{
  public:
    ResourceManagerMessageDescriptor();
    virtual ~ResourceManagerMessageDescriptor();

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

Register_ClassDescriptor(ResourceManagerMessageDescriptor);

ResourceManagerMessageDescriptor::ResourceManagerMessageDescriptor() : cClassDescriptor("ResourceManagerMessage", "cMessage")
{
}

ResourceManagerMessageDescriptor::~ResourceManagerMessageDescriptor()
{
}

bool ResourceManagerMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<ResourceManagerMessage *>(obj)!=NULL;
}

const char *ResourceManagerMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int ResourceManagerMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount(object) : 1;
}

unsigned int ResourceManagerMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
    };
    return (field>=0 && field<1) ? fieldTypeFlags[field] : 0;
}

const char *ResourceManagerMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "powerConsumed",
    };
    return (field>=0 && field<1) ? fieldNames[field] : NULL;
}

int ResourceManagerMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='p' && strcmp(fieldName, "powerConsumed")==0) return base+0;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *ResourceManagerMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "double",
    };
    return (field>=0 && field<1) ? fieldTypeStrings[field] : NULL;
}

const char *ResourceManagerMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int ResourceManagerMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    ResourceManagerMessage *pp = (ResourceManagerMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string ResourceManagerMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    ResourceManagerMessage *pp = (ResourceManagerMessage *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->getPowerConsumed());
        default: return "";
    }
}

bool ResourceManagerMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    ResourceManagerMessage *pp = (ResourceManagerMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setPowerConsumed(string2double(value)); return true;
        default: return false;
    }
}

const char *ResourceManagerMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
    };
    return (field>=0 && field<1) ? fieldStructNames[field] : NULL;
}

void *ResourceManagerMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    ResourceManagerMessage *pp = (ResourceManagerMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


