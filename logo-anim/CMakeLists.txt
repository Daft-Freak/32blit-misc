set(PROJECT_SOURCE logo-anim.cpp)

blit_executable (logo-anim ${PROJECT_SOURCE})
blit_assets_yaml (logo-anim assets.yml)
blit_metadata (logo-anim metadata.yml)
