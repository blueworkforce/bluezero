import platform
import struct
import sys
import os
import ctypes as ct

libb0 = None
ext = '.so'
if platform.system() in ('cli', 'Windows'):
    ext = '.dll'
if platform.system() in ('Darwin', ):
    ext = '.dylib'
for path in ('.', 'build', '../../build'):
    fullpath = os.path.join(os.path.dirname(__file__), path)
    if not os.path.isdir(fullpath): continue
    libb0_fullpath = os.path.join(fullpath, 'libb0' + ext)
    if os.path.exists(libb0_fullpath):
        libb0 = ct.CDLL(libb0_fullpath)
        break
if libb0 is None:
    raise RuntimeError('libb0%s not found' % ext)

b0_buffer_new = ct.CFUNCTYPE(ct.c_void_p, ct.c_size_t)(("b0_buffer_new", libb0))
b0_buffer_delete = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_buffer_delete", libb0))
b0_node_new = ct.CFUNCTYPE(ct.c_void_p, ct.c_char_p)(("b0_node_new", libb0))
b0_node_delete = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_node_delete", libb0))
b0_node_init = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_node_init", libb0))
b0_node_shutdown = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_node_shutdown", libb0))
b0_node_shutdown_requested = ct.CFUNCTYPE(ct.c_int, ct.c_void_p)(("b0_node_shutdown_requested", libb0))
b0_node_spin_once = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_node_spin_once", libb0))
b0_node_spin = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_node_spin", libb0))
b0_node_cleanup = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_node_cleanup", libb0))
b0_node_get_name = ct.CFUNCTYPE(ct.c_char_p, ct.c_void_p)(("b0_node_get_name", libb0))
b0_node_get_state = ct.CFUNCTYPE(ct.c_int, ct.c_void_p)(("b0_node_get_state", libb0))
b0_node_get_context = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p)(("b0_node_get_context", libb0))
b0_node_hardware_time_usec = ct.CFUNCTYPE(ct.c_longlong, ct.c_void_p)(("b0_node_hardware_time_usec", libb0))
b0_node_time_usec = ct.CFUNCTYPE(ct.c_longlong, ct.c_void_p)(("b0_node_time_usec", libb0))
b0_node_log = ct.CFUNCTYPE(None, ct.c_void_p, ct.c_int, ct.c_char_p)(("b0_node_log", libb0))
b0_publisher_new_ex = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_char_p, ct.c_int, ct.c_int)(("b0_publisher_new_ex", libb0))
b0_publisher_new = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_char_p)(("b0_publisher_new", libb0))
b0_publisher_delete = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_publisher_delete", libb0))
b0_publisher_init = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_publisher_init", libb0))
b0_publisher_cleanup = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_publisher_cleanup", libb0))
b0_publisher_spin_once = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_publisher_spin_once", libb0))
b0_publisher_get_topic_name = ct.CFUNCTYPE(ct.c_char_p, ct.c_void_p)(("b0_publisher_get_topic_name", libb0))
b0_publisher_publish = ct.CFUNCTYPE(None, ct.c_void_p, ct.c_void_p, ct.c_size_t)(("b0_publisher_publish", libb0))
b0_publisher_log = ct.CFUNCTYPE(None, ct.c_void_p, ct.c_int, ct.c_char_p)(("b0_publisher_log", libb0))
b0_subscriber_new_ex = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_char_p, ct.c_void_p, ct.c_int, ct.c_int)(("b0_subscriber_new_ex", libb0))
b0_subscriber_new = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_char_p, ct.c_void_p)(("b0_subscriber_new", libb0))
b0_subscriber_delete = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_subscriber_delete", libb0))
b0_subscriber_init = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_subscriber_init", libb0))
b0_subscriber_cleanup = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_subscriber_cleanup", libb0))
b0_subscriber_spin_once = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_subscriber_spin_once", libb0))
b0_subscriber_get_topic_name = ct.CFUNCTYPE(ct.c_char_p, ct.c_void_p)(("b0_subscriber_get_topic_name", libb0))
b0_subscriber_log = ct.CFUNCTYPE(None, ct.c_void_p, ct.c_int, ct.c_char_p)(("b0_subscriber_log", libb0))
b0_service_client_new_ex = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_char_p, ct.c_int, ct.c_int)(("b0_service_client_new_ex", libb0))
b0_service_client_new = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_char_p)(("b0_service_client_new", libb0))
b0_service_client_delete = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_service_client_delete", libb0))
b0_service_client_init = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_service_client_init", libb0))
b0_service_client_cleanup = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_service_client_cleanup", libb0))
b0_service_client_spin_once = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_service_client_spin_once", libb0))
b0_service_client_get_service_name = ct.CFUNCTYPE(ct.c_char_p, ct.c_void_p)(("b0_service_client_get_service_name", libb0))
b0_service_client_call = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_void_p, ct.c_size_t, ct.POINTER(ct.c_size_t))(("b0_service_client_call", libb0))
b0_service_client_log = ct.CFUNCTYPE(None, ct.c_void_p, ct.c_int, ct.c_char_p)(("b0_service_client_log", libb0))
b0_service_server_new_ex = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_char_p, ct.c_void_p, ct.c_int, ct.c_int)(("b0_service_server_new_ex", libb0))
b0_service_server_new = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_char_p, ct.c_void_p)(("b0_service_server_new", libb0))
b0_service_server_delete = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_service_server_delete", libb0))
b0_service_server_init = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_service_server_init", libb0))
b0_service_server_cleanup = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_service_server_cleanup", libb0))
b0_service_server_spin_once = ct.CFUNCTYPE(None, ct.c_void_p)(("b0_service_server_spin_once", libb0))
b0_service_server_get_service_name = ct.CFUNCTYPE(ct.c_char_p, ct.c_void_p)(("b0_service_server_get_service_name", libb0))
b0_service_server_log = ct.CFUNCTYPE(None, ct.c_void_p, ct.c_int, ct.c_char_p)(("b0_service_server_log", libb0))

class Node:
    def __init__(self, name='node'):
        self._node = b0_node_new(name)

    def __del__(self):
        b0_node_delete(self._node)

    def init(self):
        b0_node_init(self._node)

    def shutdown(self):
        b0_node_shutdown(self._node)

    def shutdown_requested(self):
        return b0_node_shutdown_requested(self._node)

    def spin_once(self):
        b0_node_spin_once(self._node)

    def spin(self):
        b0_node_spin(self._node)

    def cleanup(self):
        b0_node_cleanup(self._node)

    def get_name(self):
        return b0_node_get_name(self._node)

    def get_state(self):
        return b0_node_get_state(self._node)

    def get_context(self):
        return b0_node_get_context(self._node)

    def hardware_time_usec(self):
        return b0_node_hardware_time_usec(self._node)

    def time_usec(self):
        return b0_node_time_usec(self._node)

    def log(self, level, message):
        b0_node_log(self._node, level, message)

class Publisher:
    def __init__(self, node, topic_name, managed=1, notify_graph=1):
        self._pub = b0_publisher_new_ex(node._node, topic_name, managed, notify_graph)

    def __del__(self):
        b0_publisher_delete(self._pub)

    def init(self):
        b0_publisher_init(self._pub)

    def cleanup(self):
        b0_publisher_cleanup(self._pub)

    def spin_once(self):
        b0_publisher_spin_once(self._pub)

    def get_topic_name(self):
        return b0_publisher_get_topic_name(self._pub)

    def publish(self, data):
        buf = ct.c_char_p(data)
        b0_publisher_publish(self._pub, buf, len(data))

    def log(self, level, message):
        b0_publisher_log(self._pub, level, message)

class Subscriber:
    def __init__(self, node, topic_name, callback, managed=1, notify_graph=1):
        def w(data, size):
            data_str = ''.join(map(chr, ct.cast(data, ct.POINTER(ct.c_ubyte * size)).contents))
            return callback(data_str)
        self._cb = ct.CFUNCTYPE(None, ct.c_void_p, ct.c_size_t)(w)
        self._sub = b0_subscriber_new_ex(node._node, topic_name, self._cb, managed, notify_graph)

    def __del__(self):
        b0_subscriber_delete(self._sub)

    def init(self):
        b0_subscriber_init(self._sub)

    def cleanup(self):
        b0_subscriber_cleanup(self._sub)

    def spin_once(self):
        b0_subscriber_spin_once(self._sub)

    def get_topic_name(self):
        return b0_subscriber_get_topic_name(self._sub)

    def log(self, level, message):
        b0_subscriber_log(self._sub, level, message)

class ServiceClient:
    def __init__(self, node, topic_name, managed=1, notify_graph=1):
        self._cli = b0_service_client_new_ex(node._node, topic_name, managed, notify_graph)

    def __del__(self):
        b0_service_client_delete(self._cli)

    def init(self):
        b0_service_client_init(self._cli)

    def cleanup(self):
        b0_service_client_cleanup(self._cli)

    def spin_once(self):
        b0_service_client_spin_once(self._cli)

    def get_service_name(self):
        return b0_service_client_get_service_name(self._cli)

    def call(self, data):
        buf = ct.c_char_p(data)
        sz = len(data)
        outsz = ct.c_size_t()
        outbuf = b0_service_client_call(self._cli, buf, sz, ct.byref(outsz))
        outarr = ct.cast(outbuf, ct.POINTER(ct.c_ubyte * outsz.value))
        rep_str = ''.join(map(chr, outarr.contents))
        return rep_str

    def log(self, level, message):
        b0_service_client_log(self._cli, level, message)

class ServiceServer:
    def __init__(self, node, topic_name, callback, managed=1, notify_graph=1):
        def w(data, size, outsize):
            req_str = ''.join(map(chr, ct.cast(data, ct.POINTER(ct.c_ubyte * size)).contents))
            resp_str = callback(req_str)
            outsize[0] = len(resp_str)
            outdata = b0_buffer_new(outsize[0])
            outarr = ct.cast(outdata, ct.POINTER(ct.c_ubyte * outsize[0]))
            #for i, c in enumerate(resp_str): outarr.contents[i] = ord(c)
            ct.memmove(outarr, ct.c_char_p(resp_str), len(resp_str))
            return outdata
        self._cb = ct.CFUNCTYPE(ct.c_void_p, ct.c_void_p, ct.c_size_t, ct.POINTER(ct.c_size_t))(w)
        self._srv = b0_service_server_new_ex(node._node, topic_name, self._cb, managed, notify_graph)

    def __del__(self):
        b0_service_server_delete(self._srv)

    def init(self):
        b0_service_server_init(self._srv)

    def cleanup(self):
        b0_service_server_cleanup(self._srv)

    def spin_once(self):
        b0_service_server_spin_once(self._srv)

    def get_service_name(self):
        return b0_service_server_get_service_name(self._srv)

    def log(self, level, message):
        b0_service_server_log(self._srv, level, message)

