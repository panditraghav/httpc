project('httpc', 'c')

incdir = ['.']
src = ['./main.c', './array.c', './http.c']

cc = meson.get_compiler('c')
math = cc.find_library('m')

executable('httpc', src, include_directories: incdir, dependencies: [math])
