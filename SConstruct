env = Environment()
Export('env')

env.Append(CCFLAGS="-ggdb -static -Wall")
env.Append(CPPPATH=['#src'])

env.SConscript(['src/SConscript',
                'examples/SConscript'], 'env')
