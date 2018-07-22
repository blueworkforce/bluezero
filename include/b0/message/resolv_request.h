#ifndef B0__MESSAGE__RESOLV_REQUEST_H__INCLUDED
#define B0__MESSAGE__RESOLV_REQUEST_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>
#include <b0/message/announce_node_request.h>
#include <b0/message/shutdown_node_request.h>
#include <b0/message/announce_service_request.h>
#include <b0/message/resolve_service_request.h>
#include <b0/message/heartbeat_request.h>
#include <b0/message/node_topic_request.h>
#include <b0/message/node_service_request.h>
#include <b0/message/get_graph_request.h>

namespace b0
{

namespace message
{

/*!
 * \brief Message sent to resolver
 *
 * \sa ResolvResponse, \ref protocol
 */
class ResolvRequest : public Message
{
public:
    boost::optional<AnnounceNodeRequest> announce_node;
    boost::optional<ShutdownNodeRequest> shutdown_node;
    boost::optional<AnnounceServiceRequest> announce_service;
    boost::optional<ResolveServiceRequest> resolve_service;
    boost::optional<HeartbeatRequest> heartbeat;
    boost::optional<NodeTopicRequest> node_topic;
    boost::optional<NodeServiceRequest> node_service;
    boost::optional<GetGraphRequest> get_graph;

public:
    std::string type() const override {return "ResolvRequest";}
};

} // namespace message

} // namespace b0

//! \cond HIDDEN_SYMBOLS

namespace spotify
{

namespace json
{

using b0::message::ResolvRequest;

template <>
struct default_codec_t<ResolvRequest>
{
    static codec::object_t<ResolvRequest> codec()
    {
        auto codec = codec::object<ResolvRequest>();
        codec.optional("announce_node", &ResolvRequest::announce_node);
        codec.optional("shutdown_node", &ResolvRequest::shutdown_node);
        codec.optional("announce_service", &ResolvRequest::announce_service);
        codec.optional("resolve_service", &ResolvRequest::resolve_service);
        codec.optional("heartbeat", &ResolvRequest::heartbeat);
        codec.optional("node_topic", &ResolvRequest::node_topic);
        codec.optional("node_service", &ResolvRequest::node_service);
        codec.optional("get_graph", &ResolvRequest::get_graph);
        return codec;
    }
};

} // namespace json

} // namespace spotify

//! \endcond

#endif // B0__MESSAGE__RESOLV_REQUEST_H__INCLUDED
