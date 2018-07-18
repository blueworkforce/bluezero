#include <b0/message/message.h>

#include <sstream>
#include <json/reader.h>
#include <json/writer.h>

namespace b0
{

namespace message
{

Message::~Message()
{
}

std::string Message::type() const
{
    return "Message";
}

void Message::parseFromJson(const Json::Value &v)
{
    serialization::MessageFields fields;
    fields.inValue = &v;
    serialize(fields);
}

void Message::parseFromString(const std::string &s)
{
    std::stringstream ss(s);
    Json::Value v;
    ss >> v;
    parseFromJson(v);
}

void Message::serializeToJson(Json::Value &v) const
{
    serialization::MessageFields fields;
    fields.outValue = &v;
    serialize(fields);
}

void Message::serializeToString(std::string &s) const
{
    Json::Value v;
    serializeToJson(v);
    Json::StreamWriterBuilder builder;
    s = Json::writeString(builder, v);
}

void Message::serialize(serialization::MessageFields &fields)
{
    return static_cast<const Message&>(*this).serialize(fields);
}

namespace serialization
{

void MessageFields::parse(bool &field, const Json::Value &v)
{
    field = v.asBool();
}

void MessageFields::parse(int &field, const Json::Value &v)
{
    field = v.asInt();
}

void MessageFields::parse(int64_t &field, const Json::Value &v)
{
    field = v.asInt64();
}

void MessageFields::parse(std::string &field, const Json::Value &v)
{
    field = v.asString();
}

void MessageFields::parse(Message &field, const Json::Value &v)
{
    field.parseFromJson(v);
}

void MessageFields::serialize(const bool &field, Json::Value &v)
{
    v = field;
}

void MessageFields::serialize(const int &field, Json::Value &v)
{
    v = field;
}

void MessageFields::serialize(const int64_t &field, Json::Value &v)
{
    v = field;
}

void MessageFields::serialize(const std::string &field, Json::Value &v)
{
    v = field;
}

void MessageFields::serialize(const Message &field, Json::Value &v)
{
    field.serializeToJson(v);
}

void MessageFields::map(const std::string &name, bool *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(*field, (*inValue)[name]);
}

void MessageFields::map(const std::string &name, const bool *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(const_cast<bool&>(*field), (*inValue)[name]);
}

void MessageFields::map(const std::string &name, int *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(*field, (*inValue)[name]);
}

void MessageFields::map(const std::string &name, const int *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(const_cast<int&>(*field), (*inValue)[name]);
}

void MessageFields::map(const std::string &name, int64_t *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(*field, (*inValue)[name]);
}

void MessageFields::map(const std::string &name, const int64_t *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(const_cast<int64_t&>(*field), (*inValue)[name]);
}

void MessageFields::map(const std::string &name, std::string *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(*field, (*inValue)[name]);
}

void MessageFields::map(const std::string &name, const std::string *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(const_cast<std::string&>(*field), (*inValue)[name]);
}

void MessageFields::map(const std::string &name, Message *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(*field, (*inValue)[name]);
}

void MessageFields::map(const std::string &name, const Message *field)
{
    if(outValue) serialize(*field, (*outValue)[name]);
    if(inValue) parse(const_cast<Message&>(*field), (*inValue)[name]);
}

} // namespace serialization

} // namespace message

} // namespace b0

