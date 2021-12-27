idf_component_register(SRCS "aws-iot-comp.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp-aws-iot tasks-comp
                    EMBED_FILES "ca_cert/aws-root-ca.pem"
                    )
