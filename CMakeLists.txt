# mod-auctionhouse CMakeLists.txt
# This file is part of the AzerothCore Project. See AUTHORS file for Copyright information

# Collect source files
CollectSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} PRIVATE_SOURCES_MODULES)
CollectIncludeDirectories(${CMAKE_CURRENT_SOURCE_DIR} PUBLIC_INCLUDES)

# Add module to the build
# The parent modules/CMakeLists.txt handles the actual library creation