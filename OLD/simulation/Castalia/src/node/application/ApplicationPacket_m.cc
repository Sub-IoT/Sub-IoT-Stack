//
// Generated file, do not edit! Created by opp_msgc 4.3 from src/node/application/ApplicationPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "ApplicationPacket_m.h"

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




AppNetInfoExchange_type::AppNetInfoExchange_type()
{
    RSSI = 0;
    LQI = 0;
    source = 0;
    destination = 0;
    timestamp = 0;
}

void doPacking(cCommBuffer *b, AppNetInfoExchange_type& a)
{
    doPacking(b,a.RSSI);
    doPacking(b,a.LQI);
    doPacking(b,a.source);
    doPacking(b,a.destination);
    doPacking(b,a.timestamp);
}

void doUnpacking(cCommBuffer *b, AppNetInfoExchange_type& a)
{
    doUnpacking(b,a.RSSI);
    doUnpacking(b,a.LQI);
    doUnpacking(b,a.source);
    doUnpacking(b,a.destination);
    doUnpacking(b,a.timestamp);
}

class AppNetInfoExchange_typeDescriptor : public cClassDescriptor
{
  public:
    AppNetInfoExchange_typeDescriptor();
    virtual ~AppNetInfoExchange_typeDescriptor();

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

Register_ClassDescriptor(AppNetInfoExchange_typeDescriptor);

AppNetInfoExchange_typeDescriptor::AppNetInfoExchange_typeDescriptor() : cClassDescriptor("AppNetInfoExchange_type", "")
{
}

AppNetInfoExchange_typeDescriptor::~AppNetInfoExchange_typeDescriptor()
{
}

bool AppNetInfoExchange_typeDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<AppNetInfoExchange_type *>(obj)!=NULL;
}

const char *AppNetInfoExchange_typeDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int AppNetInfoExchange_typeDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount(object) : 5;
}

unsigned int AppNetInfoExchange_typeDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *AppNetInfoExchange_typeDescriptor::getFieldName(void *object, int field) const
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
        "source",
        "destination",
        "timestamp",
    };
    return (field>=0 && field<5) ? fieldNames[field] : NULL;
}

int AppNetInfoExchange_typeDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='R' && strcmp(fieldName, "RSSI")==0) return base+0;
    if (fieldName[0]=='L' && strcmp(fieldName, "LQI")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "source")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "destination")==0) return base+3;
    if (fieldName[0]=='t' && strcmp(fieldName, "timestamp")==0) return base+4;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *AppNetInfoExchange_typeDescriptor::getFieldTypeString(void *object, int field) const
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
        "string",
        "string",
        "simtime_t",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : NULL;
}

const char *AppNetInfoExchange_typeDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int AppNetInfoExchange_typeDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    AppNetInfoExchange_type *pp = (AppNetInfoExchange_type *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string AppNetInfoExchange_typeDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    AppNetInfoExchange_type *pp = (AppNetInfoExchange_type *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->RSSI);
        case 1: return double2string(pp->LQI);
        case 2: return oppstring2string(pp->source);
        case 3: return oppstring2string(pp->destination);
        case 4: return double2string(pp->timestamp);
        default: return "";
    }
}

bool AppNetInfoExchange_typeDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    AppNetInfoExchange_type *pp = (AppNetInfoExchange_type *)object; (void)pp;
    switch (field) {
        case 0: pp->RSSI = string2double(value); return true;
        case 1: pp->LQI = string2double(value); return true;
        case 2: pp->source = (value); return true;
        case 3: pp->destination = (value); return true;
        case 4: pp->timestamp = string2double(value); return true;
        default: return false;
    }
}

const char *AppNetInfoExchange_typeDescriptor::getFieldStructName(void *object, int field) const
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

void *AppNetInfoExchange_typeDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    AppNetInfoExchange_type *pp = (AppNetInfoExchange_type *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(ApplicationPacket);

ApplicationPacket::ApplicationPacket(const char *name, int kind) : cPacket(name,kind)
{
    this->applicationID_var = 0;
    this->sequenceNumber_var = 0;
    this->data_var = 0;
}

ApplicationPacket::ApplicationPacket(const ApplicationPacket& other) : cPacket(other)
{
    copy(other);
}

ApplicationPacket::~ApplicationPacket()
{
}

ApplicationPacket& ApplicationPacket::operator=(const ApplicationPacket& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void ApplicationPacket::copy(const ApplicationPacket& other)
{
    this->appNetInfoExchange_var = other.appNetInfoExchange_var;
    this->applicationID_var = other.applicationID_var;
    this->sequenceNumber_var = other.sequenceNumber_var;
    this->data_var = other.data_var;
}

void ApplicationPacket::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->appNetInfoExchange_var);
    doPacking(b,this->applicationID_var);
    doPacking(b,this->sequenceNumber_var);
    doPacking(b,this->data_var);
}

void ApplicationPacket::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->appNetInfoExchange_var);
    doUnpacking(b,this->applicationID_var);
    doUnpacking(b,this->sequenceNumber_var);
    doUnpacking(b,this->data_var);
}

AppNetInfoExchange_type& ApplicationPacket::getAppNetInfoExchange()
{
    return appNetInfoExchange_var;
}

void ApplicationPacket::setAppNetInfoExchange(const AppNetInfoExchange_type& appNetInfoExchange)
{
    this->appNetInfoExchange_var = appNetInfoExchange;
}

const char * ApplicationPacket::getApplicationID() const
{
    return applicationID_var.c_str();
}

void ApplicationPacket::setApplicationID(const char * applicationID)
{
    this->applicationID_var = applicationID;
}

unsigned int ApplicationPacket::getSequenceNumber() const
{
    return sequenceNumber_var;
}

void ApplicationPacket::setSequenceNumber(unsigned int sequenceNumber)
{
    this->sequenceNumber_var = sequenceNumber;
}

double ApplicationPacket::getData() const
{
    return data_var;
}

void ApplicationPacket::setData(double data)
{
    this->data_var = data;
}

class ApplicationPacketDescriptor : public cClassDescriptor
{
  public:
    ApplicationPacketDescriptor();
    virtual ~ApplicationPacketDescriptor();

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

Register_ClassDescriptor(ApplicationPacketDescriptor);

ApplicationPacketDescriptor::ApplicationPacketDescriptor() : cClassDescriptor("ApplicationPacket", "cPacket")
{
}

ApplicationPacketDescriptor::~ApplicationPacketDescriptor()
{
}

bool ApplicationPacketDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<ApplicationPacket *>(obj)!=NULL;
}

const char *ApplicationPacketDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int ApplicationPacketDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount(object) : 4;
}

unsigned int ApplicationPacketDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *ApplicationPacketDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "appNetInfoExchange",
        "applicationID",
        "sequenceNumber",
        "data",
    };
    return (field>=0 && field<4) ? fieldNames[field] : NULL;
}

int ApplicationPacketDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='a' && strcmp(fieldName, "appNetInfoExchange")==0) return base+0;
    if (fieldName[0]=='a' && strcmp(fieldName, "applicationID")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "sequenceNumber")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "data")==0) return base+3;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *ApplicationPacketDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "AppNetInfoExchange_type",
        "string",
        "unsigned int",
        "double",
    };
    return (field>=0 && field<4) ? fieldTypeStrings[field] : NULL;
}

const char *ApplicationPacketDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int ApplicationPacketDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    ApplicationPacket *pp = (ApplicationPacket *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string ApplicationPacketDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    ApplicationPacket *pp = (ApplicationPacket *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getAppNetInfoExchange(); return out.str();}
        case 1: return oppstring2string(pp->getApplicationID());
        case 2: return ulong2string(pp->getSequenceNumber());
        case 3: return double2string(pp->getData());
        default: return "";
    }
}

bool ApplicationPacketDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    ApplicationPacket *pp = (ApplicationPacket *)object; (void)pp;
    switch (field) {
        case 1: pp->setApplicationID((value)); return true;
        case 2: pp->setSequenceNumber(string2ulong(value)); return true;
        case 3: pp->setData(string2double(value)); return true;
        default: return false;
    }
}

const char *ApplicationPacketDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        "AppNetInfoExchange_type",
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<4) ? fieldStructNames[field] : NULL;
}

void *ApplicationPacketDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    ApplicationPacket *pp = (ApplicationPacket *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getAppNetInfoExchange()); break;
        default: return NULL;
    }
}


