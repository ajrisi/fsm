env = Environment()
Export('env')

env.Append(CCFLAGS="-ggdb -static -Wall")

env.SConscript(['src/SConscript',
                'examples/SConscript'], 'env')
