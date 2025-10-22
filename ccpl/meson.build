project('ccpl', 'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++17']
)

bison = find_program('bison')
flex = find_program('flex')

flex_gen = custom_target('lexer',
  input: 'src/lexer.l',
  output: 'lex.yy.cc',
  command: [flex, '-o', '@OUTPUT@', '@INPUT@']
)

parser_gen = custom_target('parser',
  input: 'src/parser.y',
  output: ['parser.tab.cc', 'parser.tab.hh'],
  command: [bison, '-d', '-o', '@OUTPUT0@', '@INPUT@']
)

sources = [
  'src/main.cc',
  'src/modules/tac.cc',
  'src/modules/opt.cc',
  'src/modules/block.cc',
  flex_gen,
  parser_gen
]

inc_dirs = include_directories('src', '.')
executable('ccpl',
  sources,
  include_directories: inc_dirs,
  cpp_args: ['-g3', '-O0'],
  install: false
)