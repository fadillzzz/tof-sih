add_library(TOFInternal SHARED
    main.cpp
)
target_precompile_headers(TOFInternal PUBLIC pch.hpp)