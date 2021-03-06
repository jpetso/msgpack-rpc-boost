CONFIGURE_FILE(version.h.in version.h)

INCLUDE_DIRECTORIES(../..)

SET(MSGPACK_RPC_SRC
	caller.h
	address.cc
	buffer.cc
	client.cc
	exception.cc
	future.cc
	loop.cc
	reqtable.cc
	request.cc
	server.cc
	session.cc
	session_pool.cc
)

SET(MSGPACK_RPC_TRANSPORT_SRC
    transport/tcp.cc
    transport/stream_handler.cc
    transport/udp.cc
    transport/dgram_handler.cc
)

IF(shared)
    ADD_LIBRARY(mprpc SHARED
        ${MSGPACK_RPC_SRC}
        ${MSGPACK_RPC_TRANSPORT_SRC}
    )
ELSE()
    ADD_LIBRARY(mprpc STATIC
        ${MSGPACK_RPC_SRC}
        ${MSGPACK_RPC_TRANSPORT_SRC}
    )
ENDIF()

##########################################

SET(MPRPC_HEADERS
	atomic_ops.h
	address.h
	buffer.h
	caller.h
	client.h
	exception.h
	future.h
	impl_fwd.h
	loop.h
	protocol.h
	request.h
	server.h
	session.h
	session_pool.h
	types.h
	transport.h
)

SET(MPRPC_TRANSPORT_HEADERS
	transport/tcp.h
	transport/udp.h
)

INSTALL(FILES ${MPRPC_HEADERS} DESTINATION include/msgpack/rpc)
INSTALL(FILES ${MPRPC_TRANSPORT_HEADERS} DESTINATION include/msgpack/rpc/transport)
INSTALL(TARGETS mprpc
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
