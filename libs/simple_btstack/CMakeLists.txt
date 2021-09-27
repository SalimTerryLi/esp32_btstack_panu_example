set(srcs "src/simple_btstack.c"
         "src/sbt_common.c"
         "src/sbt_debug.c"
         "src/sbt_pairing.c"
         "src/sbt_sdp.c"
         "src/sbt_bnep.c"
         "src/utils/session.c"
)
set(include "include")
set(private_include "private_include")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${include}
                    PRIV_INCLUDE_DIRS ${private_include}
                    REQUIRES btstack)
