//
// Generated file, do not edit! Created by opp_msgc 4.3 from src/wirelessChannel/WirelessChannelMessages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "WirelessChannelMessages_m.h"

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




Register_Class(WirelessChannelSignalBegin);

WirelessChannelSignalBegin::WirelessChannelSignalBegin(const char *name, int kind) : cMessage(name,kind)
{
    this->nodeID_var = 0;
    this->power_dBm_var = 0;
    this->carrierFreq_var = 0;
    this->bandwidth_var = 0;
    this->modulationType_var = 0;
    this->encodingType_var = 0;
}

WirelessChannelSignalBegin::WirelessChannelSignalBegin(const WirelessChannelSignalBegin& other) : cMessage(other)
{
    copy(other);
}

WirelessChannelSignalBegin::~WirelessChannelSignalBegin()
{
}

WirelessChannelSignalBegin& WirelessChannelSignalBegin::operator=(const WirelessChannelSignalBegin& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void WirelessChannelSignalBegin::copy(const WirelessChannelSignalBegin& other)
{
    this->nodeID_var = other.nodeID_var;
    this->power_dBm_var = other.power_dBm_var;
    this->carrierFreq_var = other.carrierFreq_var;
    this->bandwidth_var = other.bandwidth_var;
    this->modulationType_var = other.modulationType_var;
    this->encodingType_var = other.encodingType_var;
}

void WirelessChannelSignalBegin::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->nodeID_var);
    doPacking(b,this->power_dBm_var);
    doPacking(b,this->carrierFreq_var);
    doPacking(b,this->bandwidth_var);
    doPacking(b,this->modulationType_var);
    doPacking(b,this->encodingType_var);
}

void WirelessChannelSignalBegin::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->nodeID_var);
    doUnpacking(b,this->power_dBm_var);
    doUnpacking(b,this->carrierFreq_var);
    doUnpacking(b,this->bandwidth_var);
    doUnpacking(b,this->modulationType_var);
    doUnpacking(b,this->encodingType_var);
}

int WirelessChannelSignalBegin::getNodeID() const
{
    return nodeID_var;
}

void WirelessChannelSignalBegin::setNodeID(int nodeID)
{
    this->nodeID_var = nodeID;
}

double WirelessChannelSignalBegin::getPower_dBm() const
{
    return power_dBm_var;
}

void WirelessChannelSignalBegin::setPower_dBm(double power_dBm)
{
    this->power_dBm_var = power_dBm;
}

double WirelessChannelSignalBegin::getCarrierFreq() const
{
    return carrierFreq_var;
}

void WirelessChannelSignalBegin::setCarrierFreq(double carrierFreq)
{
    this->carrierFreq_var = carrierFreq;
}

double WirelessChannelSignalBegin::getBandwidth() const
{
    return bandwidth_var;
}

void WirelessChannelSignalBegin::setBandwidth(double bandwidth)
{
    this->bandwidth_var = bandwidth;
}

int WirelessChannelSignalBegin::getModulationType() const
{
    return modulationType_var;
}

void WirelessChannelSignalBegin::setModulationType(int modulationType)
{
    this->modulationType_var = modulationType;
}

int WirelessChannelSignalBegin::getEncodingType() const
{
    return encodingType_var;
}

void WirelessChannelSignalBegin::setEncodingType(int encodingType)
{
    this->encodingType_var = encodingType;
}

class WirelessChannelSignalBeginDescriptor : public cClassDescriptor
{
  public:
    WirelessChannelSignalBeginDescriptor();
    virtual ~WirelessChannelSignalBeginDescriptor();

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

Register_ClassDescriptor(WirelessChannelSignalBeginDescriptor);

WirelessChannelSignalBeginDescriptor::WirelessChannelSignalBeginDescriptor() : cClassDescriptor("WirelessChannelSignalBegin", "cMessage")
{
}

WirelessChannelSignalBeginDescriptor::~WirelessChannelSignalBeginDescriptor()
{
}

bool WirelessChannelSignalBeginDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<WirelessChannelSignalBegin *>(obj)!=NULL;
}

const char *WirelessChannelSignalBeginDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int WirelessChannelSignalBeginDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount(object) : 6;
}

unsigned int WirelessChannelSignalBeginDescriptor::getFieldTypeFlags(void *object, int field) const
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
        FD_ISEDITABLE,
    };
    return (field>=0 && field<6) ? fieldTypeFlags[field] : 0;
}

const char *WirelessChannelSignalBeginDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "nodeID",
        "power_dBm",
        "carrierFreq",
        "bandwidth",
        "modulationType",
        "encodingType",
    };
    return (field>=0 && field<6) ? fieldNames[field] : NULL;
}

int WirelessChannelSignalBeginDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='n' && strcmp(fieldName, "nodeID")==0) return base+0;
    if (fieldName[0]=='p' && strcmp(fieldName, "power_dBm")==0) return base+1;
    if (fieldName[0]=='c' && strcmp(fieldName, "carrierFreq")==0) return base+2;
    if (fieldName[0]=='b' && strcmp(fieldName, "bandwidth")==0) return base+3;
    if (fieldName[0]=='m' && strcmp(fieldName, "modulationType")==0) return base+4;
    if (fieldName[0]=='e' && strcmp(fieldName, "encodingType")==0) return base+5;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *WirelessChannelSignalBeginDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "double",
        "double",
        "double",
        "int",
        "int",
    };
    return (field>=0 && field<6) ? fieldTypeStrings[field] : NULL;
}

const char *WirelessChannelSignalBeginDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int WirelessChannelSignalBeginDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelSignalBegin *pp = (WirelessChannelSignalBegin *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string WirelessChannelSignalBeginDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelSignalBegin *pp = (WirelessChannelSignalBegin *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getNodeID());
        case 1: return double2string(pp->getPower_dBm());
        case 2: return double2string(pp->getCarrierFreq());
        case 3: return double2string(pp->getBandwidth());
        case 4: return long2string(pp->getModulationType());
        case 5: return long2string(pp->getEncodingType());
        default: return "";
    }
}

bool WirelessChannelSignalBeginDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelSignalBegin *pp = (WirelessChannelSignalBegin *)object; (void)pp;
    switch (field) {
        case 0: pp->setNodeID(string2long(value)); return true;
        case 1: pp->setPower_dBm(string2double(value)); return true;
        case 2: pp->setCarrierFreq(string2double(value)); return true;
        case 3: pp->setBandwidth(string2double(value)); return true;
        case 4: pp->setModulationType(string2long(value)); return true;
        case 5: pp->setEncodingType(string2long(value)); return true;
        default: return false;
    }
}

const char *WirelessChannelSignalBeginDescriptor::getFieldStructName(void *object, int field) const
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
        NULL,
    };
    return (field>=0 && field<6) ? fieldStructNames[field] : NULL;
}

void *WirelessChannelSignalBeginDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelSignalBegin *pp = (WirelessChannelSignalBegin *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(WirelessChannelSignalEnd);

WirelessChannelSignalEnd::WirelessChannelSignalEnd(const char *name, int kind) : cPacket(name,kind)
{
    this->nodeID_var = 0;
}

WirelessChannelSignalEnd::WirelessChannelSignalEnd(const WirelessChannelSignalEnd& other) : cPacket(other)
{
    copy(other);
}

WirelessChannelSignalEnd::~WirelessChannelSignalEnd()
{
}

WirelessChannelSignalEnd& WirelessChannelSignalEnd::operator=(const WirelessChannelSignalEnd& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void WirelessChannelSignalEnd::copy(const WirelessChannelSignalEnd& other)
{
    this->nodeID_var = other.nodeID_var;
}

void WirelessChannelSignalEnd::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->nodeID_var);
}

void WirelessChannelSignalEnd::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->nodeID_var);
}

int WirelessChannelSignalEnd::getNodeID() const
{
    return nodeID_var;
}

void WirelessChannelSignalEnd::setNodeID(int nodeID)
{
    this->nodeID_var = nodeID;
}

class WirelessChannelSignalEndDescriptor : public cClassDescriptor
{
  public:
    WirelessChannelSignalEndDescriptor();
    virtual ~WirelessChannelSignalEndDescriptor();

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

Register_ClassDescriptor(WirelessChannelSignalEndDescriptor);

WirelessChannelSignalEndDescriptor::WirelessChannelSignalEndDescriptor() : cClassDescriptor("WirelessChannelSignalEnd", "cPacket")
{
}

WirelessChannelSignalEndDescriptor::~WirelessChannelSignalEndDescriptor()
{
}

bool WirelessChannelSignalEndDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<WirelessChannelSignalEnd *>(obj)!=NULL;
}

const char *WirelessChannelSignalEndDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int WirelessChannelSignalEndDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount(object) : 1;
}

unsigned int WirelessChannelSignalEndDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *WirelessChannelSignalEndDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "nodeID",
    };
    return (field>=0 && field<1) ? fieldNames[field] : NULL;
}

int WirelessChannelSignalEndDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='n' && strcmp(fieldName, "nodeID")==0) return base+0;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *WirelessChannelSignalEndDescriptor::getFieldTypeString(void *object, int field) const
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

const char *WirelessChannelSignalEndDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int WirelessChannelSignalEndDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelSignalEnd *pp = (WirelessChannelSignalEnd *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string WirelessChannelSignalEndDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelSignalEnd *pp = (WirelessChannelSignalEnd *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getNodeID());
        default: return "";
    }
}

bool WirelessChannelSignalEndDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelSignalEnd *pp = (WirelessChannelSignalEnd *)object; (void)pp;
    switch (field) {
        case 0: pp->setNodeID(string2long(value)); return true;
        default: return false;
    }
}

const char *WirelessChannelSignalEndDescriptor::getFieldStructName(void *object, int field) const
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

void *WirelessChannelSignalEndDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelSignalEnd *pp = (WirelessChannelSignalEnd *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(WirelessChannelNodeMoveMessage);

WirelessChannelNodeMoveMessage::WirelessChannelNodeMoveMessage(const char *name, int kind) : cMessage(name,kind)
{
    this->nodeID_var = 0;
    this->x_var = 0;
    this->y_var = 0;
    this->z_var = 0;
    this->phi_var = 0;
    this->theta_var = 0;
}

WirelessChannelNodeMoveMessage::WirelessChannelNodeMoveMessage(const WirelessChannelNodeMoveMessage& other) : cMessage(other)
{
    copy(other);
}

WirelessChannelNodeMoveMessage::~WirelessChannelNodeMoveMessage()
{
}

WirelessChannelNodeMoveMessage& WirelessChannelNodeMoveMessage::operator=(const WirelessChannelNodeMoveMessage& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void WirelessChannelNodeMoveMessage::copy(const WirelessChannelNodeMoveMessage& other)
{
    this->nodeID_var = other.nodeID_var;
    this->x_var = other.x_var;
    this->y_var = other.y_var;
    this->z_var = other.z_var;
    this->phi_var = other.phi_var;
    this->theta_var = other.theta_var;
}

void WirelessChannelNodeMoveMessage::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->nodeID_var);
    doPacking(b,this->x_var);
    doPacking(b,this->y_var);
    doPacking(b,this->z_var);
    doPacking(b,this->phi_var);
    doPacking(b,this->theta_var);
}

void WirelessChannelNodeMoveMessage::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->nodeID_var);
    doUnpacking(b,this->x_var);
    doUnpacking(b,this->y_var);
    doUnpacking(b,this->z_var);
    doUnpacking(b,this->phi_var);
    doUnpacking(b,this->theta_var);
}

int WirelessChannelNodeMoveMessage::getNodeID() const
{
    return nodeID_var;
}

void WirelessChannelNodeMoveMessage::setNodeID(int nodeID)
{
    this->nodeID_var = nodeID;
}

double WirelessChannelNodeMoveMessage::getX() const
{
    return x_var;
}

void WirelessChannelNodeMoveMessage::setX(double x)
{
    this->x_var = x;
}

double WirelessChannelNodeMoveMessage::getY() const
{
    return y_var;
}

void WirelessChannelNodeMoveMessage::setY(double y)
{
    this->y_var = y;
}

double WirelessChannelNodeMoveMessage::getZ() const
{
    return z_var;
}

void WirelessChannelNodeMoveMessage::setZ(double z)
{
    this->z_var = z;
}

double WirelessChannelNodeMoveMessage::getPhi() const
{
    return phi_var;
}

void WirelessChannelNodeMoveMessage::setPhi(double phi)
{
    this->phi_var = phi;
}

double WirelessChannelNodeMoveMessage::getTheta() const
{
    return theta_var;
}

void WirelessChannelNodeMoveMessage::setTheta(double theta)
{
    this->theta_var = theta;
}

class WirelessChannelNodeMoveMessageDescriptor : public cClassDescriptor
{
  public:
    WirelessChannelNodeMoveMessageDescriptor();
    virtual ~WirelessChannelNodeMoveMessageDescriptor();

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

Register_ClassDescriptor(WirelessChannelNodeMoveMessageDescriptor);

WirelessChannelNodeMoveMessageDescriptor::WirelessChannelNodeMoveMessageDescriptor() : cClassDescriptor("WirelessChannelNodeMoveMessage", "cMessage")
{
}

WirelessChannelNodeMoveMessageDescriptor::~WirelessChannelNodeMoveMessageDescriptor()
{
}

bool WirelessChannelNodeMoveMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<WirelessChannelNodeMoveMessage *>(obj)!=NULL;
}

const char *WirelessChannelNodeMoveMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int WirelessChannelNodeMoveMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount(object) : 6;
}

unsigned int WirelessChannelNodeMoveMessageDescriptor::getFieldTypeFlags(void *object, int field) const
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
        FD_ISEDITABLE,
    };
    return (field>=0 && field<6) ? fieldTypeFlags[field] : 0;
}

const char *WirelessChannelNodeMoveMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "nodeID",
        "x",
        "y",
        "z",
        "phi",
        "theta",
    };
    return (field>=0 && field<6) ? fieldNames[field] : NULL;
}

int WirelessChannelNodeMoveMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='n' && strcmp(fieldName, "nodeID")==0) return base+0;
    if (fieldName[0]=='x' && strcmp(fieldName, "x")==0) return base+1;
    if (fieldName[0]=='y' && strcmp(fieldName, "y")==0) return base+2;
    if (fieldName[0]=='z' && strcmp(fieldName, "z")==0) return base+3;
    if (fieldName[0]=='p' && strcmp(fieldName, "phi")==0) return base+4;
    if (fieldName[0]=='t' && strcmp(fieldName, "theta")==0) return base+5;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *WirelessChannelNodeMoveMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "double",
        "double",
        "double",
        "double",
        "double",
    };
    return (field>=0 && field<6) ? fieldTypeStrings[field] : NULL;
}

const char *WirelessChannelNodeMoveMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int WirelessChannelNodeMoveMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelNodeMoveMessage *pp = (WirelessChannelNodeMoveMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string WirelessChannelNodeMoveMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelNodeMoveMessage *pp = (WirelessChannelNodeMoveMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getNodeID());
        case 1: return double2string(pp->getX());
        case 2: return double2string(pp->getY());
        case 3: return double2string(pp->getZ());
        case 4: return double2string(pp->getPhi());
        case 5: return double2string(pp->getTheta());
        default: return "";
    }
}

bool WirelessChannelNodeMoveMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelNodeMoveMessage *pp = (WirelessChannelNodeMoveMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setNodeID(string2long(value)); return true;
        case 1: pp->setX(string2double(value)); return true;
        case 2: pp->setY(string2double(value)); return true;
        case 3: pp->setZ(string2double(value)); return true;
        case 4: pp->setPhi(string2double(value)); return true;
        case 5: pp->setTheta(string2double(value)); return true;
        default: return false;
    }
}

const char *WirelessChannelNodeMoveMessageDescriptor::getFieldStructName(void *object, int field) const
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
        NULL,
    };
    return (field>=0 && field<6) ? fieldStructNames[field] : NULL;
}

void *WirelessChannelNodeMoveMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    WirelessChannelNodeMoveMessage *pp = (WirelessChannelNodeMoveMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


