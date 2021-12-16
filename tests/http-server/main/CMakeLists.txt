idf_component_register(SRCS "main.cpp" "ssl_server.cpp" "tcp_server.cpp"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "certs/cacert.pem"
                                   "certs/prvtkey.pem")
