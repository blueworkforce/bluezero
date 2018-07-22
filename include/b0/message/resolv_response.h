#ifndef B0__MESSAGE__RESOLV_RESPONSE_H__INCLUDED
#define B0__MESSAGE__RESOLV_RESPONSE_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>
#include <b0/message/announce_node_response.h>
#include <b0/message/shutdown_node_response.h>
#include <b0/message/announce_service_response.h>
#include <b0/message/resolve_service_response.h>
#include <b0/message/heartbeat_response.h>
#include <b0/message/node_topic_response.h>
#include <b0/message/node_service_response.h>
#include <b0/message/get_graph_response.h>

namespace b0
{

namespace message
{

/*!
 * \brief Response to ResolvRequest message
 *
 * \sa ResolvRequest, \ref protocol
 */
class ResolvResponse : public Message
{
public:
    boost::optional<AnnounceNodeResponse> announce_node;
    boost::optional<ShutdownNodeResponse> shutdown_node;
    boost::optional<AnnounceServiceResponse> announce_service;
    boost::optional<ResolveServiceResponse> resolve_service;
    boost::optional<HeartbeatResponse> heartbeat;
    boost::optional<NodeTopicResponse> node_topic;
    boost::optional<NodeServiceResponse> node_service;
    boost::optional<GetGraphResponse> get_graph;

public:
    std::string type() const override {return "ResolvResponse";}
};

} // namespace message

} // namespace b0

//! \cond HIDDEN_SYMBOLS

namespace spotify
{

namespace json
{

using b0::message::ResolvResponse;

template <>
struct default_codec_t<ResolvResponse>
{
    static codec::object_t<ResolvResponse> codec()
    {
        auto codec = codec::object<ResolvResponse>();
        codec.optional("announce_node", &ResolvResponse::announce_node);
        codec.optional("shutdown_node", &ResolvResponse::shutdown_node);
        codec.optional("announce_service", &ResolvResponse::announce_service);
        codec.optional("resolve_service", &ResolvResponse::resolve_service);
        codec.optional("heartbeat", &ResolvResponse::heartbeat);
        codec.optional("node_topic", &ResolvResponse::node_topic);
        codec.optional("node_service", &ResolvResponse::node_service);
        codec.optional("get_graph", &ResolvResponse::get_graph);
        return codec;
    }
};

} // namespace json

} // namespace spotify

//! \endcond

#endif // B0__MESSAGE__RESOLV_RESPONSE_H__INCLUDED
