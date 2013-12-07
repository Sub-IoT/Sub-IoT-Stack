//
// Generated file, do not edit! Created by opp_msgc 4.3 from src/node/communication/radio/RadioControlMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "RadioControlMessage_m.h"

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
    cEnum *e = cEnum::find("RadioControlMessage_type");
    if (!e) enums.getInstance()->add(e = new cEnum("RadioControlMessage_type"));
    e->insert(CARRIER_SENSE_INTERRUPT, "CARRIER_SENSE_INTERRUPT");
    e->insert(RADIO_BUFFER_FULL, "RADIO_BUFFER_FULL");
);

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("BasicState_type");
    if (!e) enums.getInstance()->add(e = new cEnum("BasicState_type"));
    e->insert(RX, "RX");
    e->insert(TX, "TX");
    e->insert(SLEEP, "SLEEP");
);

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("RadioControlCommand_type");
    if (!e) enums.getInstance()->add(e = new cEnum("RadioControlCommand_type"));
    e->insert(SET_STATE, "SET_STATE");
    e->insert(SET_MODE, "SET_MODE");
    e->insert(SET_TX_OUTPUT, "SET_TX_OUTPUT");
    e->insert(SET_SLEEP_LEVEL, "SET_SLEEP_LEVEL");
    e->insert(SET_CARRIER_FREQ, "SET_CARRIER_FREQ");
    e->insert(SET_CCA_THRESHOLD, "SET_CCA_THRESHOLD");
    e->insert(SET_CS_INTERRUPT_ON, "SET_CS_INTERRUPT_ON");
    e->insert(SET_CS_INTERRUPT_OFF, "SET_CS_INTERRUPT_OFF");
    e->insert(SET_ENCODING, "SET_ENCODING");
);

Register_Class(RadioControlMessage);

RadioControlMessage::RadioControlMessage(const char *name, int kind) : cMessage(name,kind)
{
    this->radioControlMessageKind_var = 0;
}

RadioControlMessage::RadioControlMessage(const RadioControlMessage& other) : cMessage(other)
{
    copy(other);
}

RadioControlMessage::~RadioControlMessage()
{
}

RadioControlMessage& RadioControlMessage::operator=(const RadioControlMessage& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void RadioControlMessage::copy(const RadioControlMessage& other)
{
    this->radioControlMessageKind_var = other.radioControlMessageKind_var;
}

void RadioControlMessage::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->radioControlMessageKind_var);
}

void RadioControlMessage::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->radioControlMessageKind_var);
}

int RadioControlMessage::getRadioControlMessageKind() const
{
    return radioControlMessageKind_var;
}

void RadioControlMessage::setRadioControlMessageKind(int radioControlMessageKind)
{
    this->radioControlMessageKind_var = radioControlMessageKind;
}

class RadioControlMessageDescriptor : public cClassDescriptor
{
  public:
    RadioControlMessageDescriptor();
    virtual ~RadioControlMessageDescriptor();

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

Register_ClassDescriptor(RadioControlMessageDescriptor);

RadioControlMessageDescriptor::RadioControlMessageDescriptor() : cClassDescriptor("RadioControlMessage", "cMessage")
{
}

RadioControlMessageDescriptor::~RadioControlMessageDescriptor()
{
}

bool RadioControlMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RadioControlMessage *>(obj)!=NULL;
}

const char *RadioControlMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RadioControlMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount(object) : 1;
}

unsigned int RadioControlMessageDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *RadioControlMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "radioControlMessageKind",
    };
    return (field>=0 && field<1) ? fieldNames[field] : NULL;
}

int RadioControlMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='r' && strcmp(fieldName, "radioControlMessageKind")==0) return base+0;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RadioControlMessageDescriptor::getFieldTypeString(void *object, int field) const
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

const char *RadioControlMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "RadioControlMessage_type";
            return NULL;
        default: return NULL;
    }
}

int RadioControlMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RadioControlMessage *pp = (RadioControlMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string RadioControlMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RadioControlMessage *pp = (RadioControlMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getRadioControlMessageKind());
        default: return "";
    }
}

bool RadioControlMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RadioControlMessage *pp = (RadioControlMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setRadioControlMessageKind(string2long(value)); return true;
        default: return false;
    }
}

const char *RadioControlMessageDescriptor::getFieldStructName(void *object, int field) const
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

void *RadioControlMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RadioControlMessage *pp = (RadioControlMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(RadioControlCommand);

RadioControlCommand::RadioControlCommand(const char *name, int kind) : cMessage(name,kind)
{
    this->radioControlCommandKind_var = 0;
    this->state_var = RX;
    this->parameter_var = 0.0;
    this->name_var = "";
}

RadioControlCommand::RadioControlCommand(const RadioControlCommand& other) : cMessage(other)
{
    copy(other);
}

RadioControlCommand::~RadioControlCommand()
{
}

RadioControlCommand& RadioControlCommand::operator=(const RadioControlCommand& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void RadioControlCommand::copy(const RadioControlCommand& other)
{
    this->radioControlCommandKind_var = other.radioControlCommandKind_var;
    this->state_var = other.state_var;
    this->parameter_var = other.parameter_var;
    this->name_var = other.name_var;
}

void RadioControlCommand::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->radioControlCommandKind_var);
    doPacking(b,this->state_var);
    doPacking(b,this->parameter_var);
    doPacking(b,this->name_var);
}

void RadioControlCommand::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->radioControlCommandKind_var);
    doUnpacking(b,this->state_var);
    doUnpacking(b,this->parameter_var);
    doUnpacking(b,this->name_var);
}

int RadioControlCommand::getRadioControlCommandKind() const
{
    return radioControlCommandKind_var;
}

void RadioControlCommand::setRadioControlCommandKind(int radioControlCommandKind)
{
    this->radioControlCommandKind_var = radioControlCommandKind;
}

int RadioControlCommand::getState() const
{
    return state_var;
}

void RadioControlCommand::setState(int state)
{
    this->state_var = state;
}

double RadioControlCommand::getParameter() const
{
    return parameter_var;
}

void RadioControlCommand::setParameter(double parameter)
{
    this->parameter_var = parameter;
}

const char * RadioControlCommand::getName() const
{
    return name_var.c_str();
}

void RadioControlCommand::setName(const char * name)
{
    this->name_var = name;
}

class RadioControlCommandDescriptor : public cClassDescriptor
{
  public:
    RadioControlCommandDescriptor();
    virtual ~RadioControlCommandDescriptor();

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

Register_ClassDescriptor(RadioControlCommandDescriptor);

RadioControlCommandDescriptor::RadioControlCommandDescriptor() : cClassDescriptor("RadioControlCommand", "cMessage")
{
}

RadioControlCommandDescriptor::~RadioControlCommandDescriptor()
{
}

bool RadioControlCommandDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RadioControlCommand *>(obj)!=NULL;
}

const char *RadioControlCommandDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RadioControlCommandDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount(object) : 4;
}

unsigned int RadioControlCommandDescriptor::getFieldTypeFlags(void *object, int field) const
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
    };
    return (field>=0 && field<4) ? fieldTypeFlags[field] : 0;
}

const char *RadioControlCommandDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "radioControlCommandKind",
        "state",
        "parameter",
        "name",
    };
    return (field>=0 && field<4) ? fieldNames[field] : NULL;
}

int RadioControlCommandDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='r' && strcmp(fieldName, "radioControlCommandKind")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "state")==0) return base+1;
    if (fieldName[0]=='p' && strcmp(fieldName, "parameter")==0) return base+2;
    if (fieldName[0]=='n' && strcmp(fieldName, "name")==0) return base+3;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RadioControlCommandDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "double",
        "string",
    };
    return (field>=0 && field<4) ? fieldTypeStrings[field] : NULL;
}

const char *RadioControlCommandDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "RadioControlCommand_type";
            return NULL;
        case 1:
            if (!strcmp(propertyname,"enum")) return "BasicState_type";
            return NULL;
        default: return NULL;
    }
}

int RadioControlCommandDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RadioControlCommand *pp = (RadioControlCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string RadioControlCommandDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RadioControlCommand *pp = (RadioControlCommand *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getRadioControlCommandKind());
        case 1: return long2string(pp->getState());
        case 2: return double2string(pp->getParameter());
        case 3: return oppstring2string(pp->getName());
        default: return "";
    }
}

bool RadioControlCommandDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RadioControlCommand *pp = (RadioControlCommand *)object; (void)pp;
    switch (field) {
        case 0: pp->setRadioControlCommandKind(string2long(value)); return true;
        case 1: pp->setState(string2long(value)); return true;
        case 2: pp->setParameter(string2double(value)); return true;
        case 3: pp->setName((value)); return true;
        default: return false;
    }
}

const char *RadioControlCommandDescriptor::getFieldStructName(void *object, int field) const
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
    };
    return (field>=0 && field<4) ? fieldStructNames[field] : NULL;
}

void *RadioControlCommandDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RadioControlCommand *pp = (RadioControlCommand *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


