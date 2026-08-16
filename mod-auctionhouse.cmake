# mod-auctionhouse custom CMake configuration
# This file is included by the parent modules/CMakeLists.txt if it exists

# Define module-specific compile options
target_compile_options(modules INTERFACE -DMOD_AUCTIONHOUSE)

# Module version
set(MOD_AUCTIONHOUSE_VERSION "1.0.0")
target_compile_definitions(modules INTERFACE MOD_AUCTIONHOUSE_VERSION="${MOD_AUCTIONHOUSE_VERSION}")