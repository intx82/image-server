project('image_server', 'cpp', default_options: ['cpp_std=c++17'])


spdlog_dep = dependency('spdlog', required: true)
gtest_dep = dependency('gtest', required: true)
magic_dep = dependency('libmagic', required: true)
jsoncpp_dep = dependency('jsoncpp', required: true)

executable('image_server', ['main.cc', 'config.cc', 'sha256.cc', 'base64.cc', 'endpoints.cc'],
  dependencies: [spdlog_dep, jsoncpp_dep, magic_dep]
)
