try_compile(
    ASIO_GRPC_BOOST_ASIO_HAS_CO_AWAIT "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/check_boost_asio_has_co_await.cpp"
    LINK_LIBRARIES Boost::headers CXX_STANDARD 20 CXX_STANDARD_REQUIRED on)
