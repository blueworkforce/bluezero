#ifndef B0__MESSAGE__MESSAGE_H__INCLUDED
#define B0__MESSAGE__MESSAGE_H__INCLUDED

#include <string>
#include <json/json.h>

namespace b0
{

namespace message
{

namespace serialization
{

class MessageFields;

} // namespace serialization

/*!
 * \brief The base class for all BlueZero's protocol messages
 *
 * It contains some utility methods for string serialization/parsing.
 */
class Message
{
public:
    //! \brief destructor
    virtual ~Message();

    //! \brief Returns a string with the type (typically the name of the class)
    virtual std::string type() const = 0;

    //! \brief Parse from a JSON object
    virtual void parseFromJson(const Json::Value &v);

    //! \brief Parse from a string
    virtual void parseFromString(const std::string &s);

    //! \brief Serialize to a JSON object
    virtual void serializeToJson(Json::Value &v) const;

    //! \brief Serialize to a string
    virtual void serializeToString(std::string &s) const;

private:
    //! \brief Declarative parse/serialize
    virtual void serialize(serialization::MessageFields &fields) const = 0;

    //! \brief Declarative parse/serialize (non-const wrapper)
    void serialize(serialization::MessageFields &fields);
};

namespace serialization
{

/*!
 * \brief A helper structure used for message parsing/serialization
 *
 * The Message::setup(serialization::MessageFields&) method must call
 * MessageFields::map() for each field that it needs to parse/serialize.
 */
struct MessageFields final
{
    //! \brief Output JSON object (when serializing)
    Json::Value *outValue = nullptr;

    //! \brief Input JSON object (when parsing)
    const Json::Value *inValue = nullptr;

    //! \brief Parse a bool field from JSON
    virtual void parse(bool &field, const Json::Value &v);

    //! \brief Parse a int field from JSON
    virtual void parse(int &field, const Json::Value &v);

    //! \brief Parse a int64 field from JSON
    virtual void parse(int64_t &field, const Json::Value &v);

    //! \brief Parse a string field from JSON
    virtual void parse(std::string &field, const Json::Value &v);

    //! \brief Parse a Message field from JSON
    virtual void parse(Message &field, const Json::Value &v);

    //! \brief Serialize a bool field to JSON
    virtual void serialize(const bool &field, Json::Value &v);

    //! \brief Serialize a int field to JSON
    virtual void serialize(const int &field, Json::Value &v);

    //! \brief Serialize a int64 field to JSON
    virtual void serialize(const int64_t &field, Json::Value &v);

    //! \brief Serialize a string field to JSON
    virtual void serialize(const std::string &field, Json::Value &v);

    //! \brief Serialize a Message field to JSON
    virtual void serialize(const Message &field, Json::Value &v);

    //! \brief Map a message bool field to a field with the specified name in the JSON output object
    virtual void map(const std::string &name, bool *field);

    //! \brief Map a message bool field to a field with the specified name in the JSON output object (const version)
    virtual void map(const std::string &name, const bool *field);

    //! \brief Map a message int field to a field with the specified name in the JSON output object
    virtual void map(const std::string &name, int *field);

    //! \brief Map a message int field to a field with the specified name in the JSON output object (const version)
    virtual void map(const std::string &name, const int *field);

    //! \brief Map a message int64 field to a field with the specified name in the JSON output object
    virtual void map(const std::string &name, int64_t *field);

    //! \brief Map a message int64 field to a field with the specified name in the JSON output object (const version)
    virtual void map(const std::string &name, const int64_t *field);

    //! \brief Map a message string field to a field with the specified name in the JSON output object
    virtual void map(const std::string &name, std::string *field);

    //! \brief Map a message string field to a field with the specified name in the JSON output object (const version)
    virtual void map(const std::string &name, const std::string *field);

    //! \brief Map a message Message field to a field with the specified name in the JSON output object
    virtual void map(const std::string &name, Message *field);

    //! \brief Map a message Message field to a field with the specified name in the JSON output object (const version)
    virtual void map(const std::string &name, const Message *field);

    //! \brief Map a message vector field to a field with the specified name in the JSON output object
    template<typename T>
    void map(const std::string &name, std::vector<T> *field)
    {
        if(outValue)
        {
            for(size_t i = 0; i < field->size(); i++)
                serialize(field->at(i), (*outValue)[name][int(i)]);
        }
        if(inValue)
        {
            field->resize((*inValue)[name].size());
            for(size_t i = 0; i < (*inValue)[name].size(); i++)
                parse(field->at(i), (*inValue)[name][int(i)]);
        }
    }

    //! \brief Map a message vector field to a field with the specified name in the JSON output object
    template<typename T>
    void map(const std::string &name, const std::vector<T> *field)
    {
        if(outValue)
        {
            for(size_t i = 0; i < field->size(); i++)
                serialize(field->at(i), (*outValue)[name][int(i)]);
        }
        if(inValue)
        {
            std::vector<T> *v = const_cast<std::vector<T>*>(field);
            v->resize((*inValue)[name].size());
            for(size_t i = 0; i < (*inValue)[name].size(); i++)
                parse(v->at(i), (*inValue)[name][int(i)]);
        }
    }
};

} // namespace serialization

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__MESSAGE_H__INCLUDED
