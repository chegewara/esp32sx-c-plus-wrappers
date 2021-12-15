idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "certs/cacert.pem"
                                   "certs/prvtkey.pem")
