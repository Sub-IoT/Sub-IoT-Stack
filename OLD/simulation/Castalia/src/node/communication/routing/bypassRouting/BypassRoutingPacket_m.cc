//
// Generated file, do not edit! Created by opp_msgc 4.3 from src/node/communication/routing/bypassRouting/BypassRoutingPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "BypassRoutingPacket_m.h"

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




Register_Class(BypassRoutingPacket);

BypassRoutingPacket::BypassRoutingPacket(const char *name, int kind) : RoutingPacket(name,kind)
{
    this->source_var = 0;
    this->destination_var = 0;
}

BypassRoutingPacket::BypassRoutingPacket(const BypassRoutingPacket& other) : RoutingPacket(other)
{
    copy(other);
}

BypassRoutingPacket::~BypassRoutingPacket()
{
}

BypassRoutingPacket& BypassRoutingPacket::operator=(const BypassRoutingPacket& other)
{
    if (this==&other) return *this;
    RoutingPacket::operator=(other);
    copy(other);
    return *this;
}

void BypassRoutingPacket::copy(const BypassRoutingPacket& other)
{
    this->source_var = other.source_var;
    this->destination_var = other.destination_var;
}

void BypassRoutingPacket::parsimPack(cCommBuffer *b)
{
    RoutingPacket::parsimPack(b);
    doPacking(b,this->source_var);
    doPacking(b,this->destination_var);
}

void BypassRoutingPacket::parsimUnpack(cCommBuffer *b)
{
    RoutingPacket::parsimUnpack(b);
    doUnpacking(b,this->source_var);
    doUnpacking(b,this->destination_var);
}

const char * BypassRoutingPacket::getSource() const
{
    return source_var.c_str();
}

void BypassRoutingPacket::setSource(const char * source)
{
    this->source_var = source;
}

const char * BypassRoutingPacket::getDestination() const
{
    return destination_var.c_str();
}

void BypassRoutingPacket::setDestination(const char * destination)
{
    this->destination_var = destination;
}

class BypassRoutingPacketDescriptor : public cClassDescriptor
{
  public:
    BypassRoutingPacketDescriptor();
    virtual ~BypassRoutingPacketDescriptor();

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

Register_ClassDescriptor(BypassRoutingPacketDescriptor);

BypassRoutingPacketDescriptor::BypassRoutingPacketDescriptor() : cClassDescriptor("BypassRoutingPacket", "RoutingPacket")
{
}

BypassRoutingPacketDescriptor::~BypassRoutingPacketDescriptor()
{
}

bool BypassRoutingPacketDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<BypassRoutingPacket *>(obj)!=NULL;
}

const char *BypassRoutingPacketDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int BypassRoutingPacketDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int BypassRoutingPacketDescriptor::getFieldTypeFlags(void *object, int field) const
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
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *BypassRoutingPacketDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "source",
        "destination",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int BypassRoutingPacketDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "source")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "destination")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *BypassRoutingPacketDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "string",
        "string",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *BypassRoutingPacketDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int BypassRoutingPacketDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    BypassRoutingPacket *pp = (BypassRoutingPacket *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string BypassRoutingPacketDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    BypassRoutingPacket *pp = (BypassRoutingPacket *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->getSource());
        case 1: return oppstring2string(pp->getDestination());
        default: return "";
    }
}

bool BypassRoutingPacketDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    BypassRoutingPacket *pp = (BypassRoutingPacket *)object; (void)pp;
    switch (field) {
        case 0: pp->setSource((value)); return true;
        case 1: pp->setDestination((value)); return true;
        default: return false;
    }
}

const char *BypassRoutingPacketDescriptor::getFieldStructName(void *object, int field) const
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
    };
    return (field>=0 && field<2) ? fieldStructNames[field] : NULL;
}

void *BypassRoutingPacketDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    BypassRoutingPacket *pp = (BypassRoutingPacket *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


