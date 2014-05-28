//
// Generated file, do not edit! Created by opp_msgc 4.3 from src/node/communication/mac/MacPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "MacPacket_m.h"

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




EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("MacControlMessage_type");
    if (!e) enums.getInstance()->add(e = new cEnum("MacControlMessage_type"));
    e->insert(MAC_BUFFER_FULL, "MAC_BUFFER_FULL");
);

MacRadioInfoExchange_type::MacRadioInfoExchange_type()
{
    RSSI = 0;
    LQI = 0;
}

void doPacking(cCommBuffer *b, MacRadioInfoExchange_type& a)
{
    doPacking(b,a.RSSI);
    doPacking(b,a.LQI);
}

void doUnpacking(cCommBuffer *b, MacRadioInfoExchange_type& a)
{
    doUnpacking(b,a.RSSI);
    doUnpacking(b,a.LQI);
}

class MacRadioInfoExchange_typeDescriptor : public cClassDescriptor
{
  public:
    MacRadioInfoExchange_typeDescriptor();
    virtual ~MacRadioInfoExchange_typeDescriptor();

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

Register_ClassDescriptor(MacRadioInfoExchange_typeDescriptor);

MacRadioInfoExchange_typeDescriptor::MacRadioInfoExchange_typeDescriptor() : cClassDescriptor("MacRadioInfoExchange_type", "")
{
}

MacRadioInfoExchange_typeDescriptor::~MacRadioInfoExchange_typeDescriptor()
{
}

bool MacRadioInfoExchange_typeDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<MacRadioInfoExchange_type *>(obj)!=NULL;
}

const char *MacRadioInfoExchange_typeDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int MacRadioInfoExchange_typeDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int MacRadioInfoExchange_typeDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *MacRadioInfoExchange_typeDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "RSSI",
        "LQI",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int MacRadioInfoExchange_typeDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='R' && strcmp(fieldName, "RSSI")==0) return base+0;
    if (fieldName[0]=='L' && strcmp(fieldName, "LQI")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *MacRadioInfoExchange_typeDescriptor::getFieldTypeString(void *object, int field) const
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
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *MacRadioInfoExchange_typeDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int MacRadioInfoExchange_typeDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    MacRadioInfoExchange_type *pp = (MacRadioInfoExchange_type *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string MacRadioInfoExchange_typeDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    MacRadioInfoExchange_type *pp = (MacRadioInfoExchange_type *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->RSSI);
        case 1: return double2string(pp->LQI);
        default: return "";
    }
}

bool MacRadioInfoExchange_typeDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    MacRadioInfoExchange_type *pp = (MacRadioInfoExchange_type *)object; (void)pp;
    switch (field) {
        case 0: pp->RSSI = string2double(value); return true;
        case 1: pp->LQI = string2double(value); return true;
        default: return false;
    }
}

const char *MacRadioInfoExchange_typeDescriptor::getFieldStructName(void *object, int field) const
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

void *MacRadioInfoExchange_typeDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    MacRadioInfoExchange_type *pp = (MacRadioInfoExchange_type *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(MacPacket);

MacPacket::MacPacket(const char *name, int kind) : cPacket(name,kind)
{
    this->source_var = 0;
    this->destination_var = 0;
    this->sequenceNumber_var = 0;
}

MacPacket::MacPacket(const MacPacket& other) : cPacket(other)
{
    copy(other);
}

MacPacket::~MacPacket()
{
}

MacPacket& MacPacket::operator=(const MacPacket& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void MacPacket::copy(const MacPacket& other)
{
    this->macRadioInfoExchange_var = other.macRadioInfoExchange_var;
    this->source_var = other.source_var;
    this->destination_var = other.destination_var;
    this->sequenceNumber_var = other.sequenceNumber_var;
}

void MacPacket::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->macRadioInfoExchange_var);
    doPacking(b,this->source_var);
    doPacking(b,this->destination_var);
    doPacking(b,this->sequenceNumber_var);
}

void MacPacket::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->macRadioInfoExchange_var);
    doUnpacking(b,this->source_var);
    doUnpacking(b,this->destination_var);
    doUnpacking(b,this->sequenceNumber_var);
}

MacRadioInfoExchange_type& MacPacket::getMacRadioInfoExchange()
{
    return macRadioInfoExchange_var;
}

void MacPacket::setMacRadioInfoExchange(const MacRadioInfoExchange_type& macRadioInfoExchange)
{
    this->macRadioInfoExchange_var = macRadioInfoExchange;
}

int MacPacket::getSource() const
{
    return source_var;
}

void MacPacket::setSource(int source)
{
    this->source_var = source;
}

int MacPacket::getDestination() const
{
    return destination_var;
}

void MacPacket::setDestination(int destination)
{
    this->destination_var = destination;
}

unsigned int MacPacket::getSequenceNumber() const
{
    return sequenceNumber_var;
}

void MacPacket::setSequenceNumber(unsigned int sequenceNumber)
{
    this->sequenceNumber_var = sequenceNumber;
}

class MacPacketDescriptor : public cClassDescriptor
{
  public:
    MacPacketDescriptor();
    virtual ~MacPacketDescriptor();

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

Register_ClassDescriptor(MacPacketDescriptor);

MacPacketDescriptor::MacPacketDescriptor() : cClassDescriptor("MacPacket", "cPacket")
{
}

MacPacketDescriptor::~MacPacketDescriptor()
{
}

bool MacPacketDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<MacPacket *>(obj)!=NULL;
}

const char *MacPacketDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int MacPacketDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount(object) : 4;
}

unsigned int MacPacketDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<4) ? fieldTypeFlags[field] : 0;
}

const char *MacPacketDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "macRadioInfoExchange",
        "source",
        "destination",
        "sequenceNumber",
    };
    return (field>=0 && field<4) ? fieldNames[field] : NULL;
}

int MacPacketDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='m' && strcmp(fieldName, "macRadioInfoExchange")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "source")==0) return base+1;
    if (fieldName[0]=='d' && strcmp(fieldName, "destination")==0) return base+2;
    if (fieldName[0]=='s' && strcmp(fieldName, "sequenceNumber")==0) return base+3;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *MacPacketDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "MacRadioInfoExchange_type",
        "int",
        "int",
        "unsigned int",
    };
    return (field>=0 && field<4) ? fieldTypeStrings[field] : NULL;
}

const char *MacPacketDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int MacPacketDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    MacPacket *pp = (MacPacket *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string MacPacketDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    MacPacket *pp = (MacPacket *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getMacRadioInfoExchange(); return out.str();}
        case 1: return long2string(pp->getSource());
        case 2: return long2string(pp->getDestination());
        case 3: return ulong2string(pp->getSequenceNumber());
        default: return "";
    }
}

bool MacPacketDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    MacPacket *pp = (MacPacket *)object; (void)pp;
    switch (field) {
        case 1: pp->setSource(string2long(value)); return true;
        case 2: pp->setDestination(string2long(value)); return true;
        case 3: pp->setSequenceNumber(string2ulong(value)); return true;
        default: return false;
    }
}

const char *MacPacketDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        "MacRadioInfoExchange_type",
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<4) ? fieldStructNames[field] : NULL;
}

void *MacPacketDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    MacPacket *pp = (MacPacket *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getMacRadioInfoExchange()); break;
        default: return NULL;
    }
}

Register_Class(MacControlMessage);

MacControlMessage::MacControlMessage(const char *name, int kind) : cMessage(name,kind)
{
    this->macControlMessageKind_var = 0;
}

MacControlMessage::MacControlMessage(const MacControlMessage& other) : cMessage(other)
{
    copy(other);
}

MacControlMessage::~MacControlMessage()
{
}

MacControlMessage& MacControlMessage::operator=(const MacControlMessage& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void MacControlMessage::copy(const MacControlMessage& other)
{
    this->macControlMessageKind_var = other.macControlMessageKind_var;
}

void MacControlMessage::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->macControlMessageKind_var);
}

void MacControlMessage::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->macControlMessageKind_var);
}

int MacControlMessage::getMacControlMessageKind() const
{
    return macControlMessageKind_var;
}

void MacControlMessage::setMacControlMessageKind(int macControlMessageKind)
{
    this->macControlMessageKind_var = macControlMessageKind;
}

class MacControlMessageDescriptor : public cClassDescriptor
{
  public:
    MacControlMessageDescriptor();
    virtual ~MacControlMessageDescriptor();

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

Register_ClassDescriptor(MacControlMessageDescriptor);

MacControlMessageDescriptor::MacControlMessageDescriptor() : cClassDescriptor("MacControlMessage", "cMessage")
{
}

MacControlMessageDescriptor::~MacControlMessageDescriptor()
{
}

bool MacControlMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<MacControlMessage *>(obj)!=NULL;
}

const char *MacControlMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int MacControlMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount(object) : 1;
}

unsigned int MacControlMessageDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *MacControlMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "macControlMessageKind",
    };
    return (field>=0 && field<1) ? fieldNames[field] : NULL;
}

int MacControlMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='m' && strcmp(fieldName, "macControlMessageKind")==0) return base+0;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *MacControlMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
    };
    return (field>=0 && field<1) ? fieldTypeStrings[field] : NULL;
}

const char *MacControlMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "MacControlMessage_type";
            return NULL;
        default: return NULL;
    }
}

int MacControlMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    MacControlMessage *pp = (MacControlMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string MacControlMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    MacControlMessage *pp = (MacControlMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getMacControlMessageKind());
        default: return "";
    }
}

bool MacControlMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    MacControlMessage *pp = (MacControlMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setMacControlMessageKind(string2long(value)); return true;
        default: return false;
    }
}

const char *MacControlMessageDescriptor::getFieldStructName(void *object, int field) const
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

void *MacControlMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    MacControlMessage *pp = (MacControlMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


