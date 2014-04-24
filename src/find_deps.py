import re
import glob


# Scan the file provided as argument looking for '#include "..."'
# and return all the matches
def get_includes(f):
    ret = []
    fin = open(f, 'r')

    while 1:
        line = fin.readline()
        if line == '':
            break
        m = re.match(r'[ \t]*#[ \t]*include [ \t]*"([a-z._]+)"', line)
        if m:
            ret.append(m.group(1))
    fin.close()

    return ret


# Some filename transformations for the sake of readability
def transform_name(s):
    ret = s.replace('.cpp', 'O').replace('.hpp', 'H').replace('.hh', 'H')

    # Transform 'xxxx_yyyy' into 'yyyy'
    m = re.match(r'[^_]+_([^_]+)', ret)
    if m:
        ret = m.group(1)

    return ret


def get_files(extensions, blacklist):
    ret = []

    # Find all the files having one of the specified extension
    for ext in extensions:
        ret = ret + glob.glob('*.' + ext)

    # Remove from the result the blacklisted files
    for b in blacklist:
        if b in ret:
            ret.remove(b)

    return ret


# Recursive function used to find cycles
def find_cycles_from(g, cur, trace, visited):
    #print ('cur = ' + cur)
    visited.append(cur)
    if cur not in g.keys():
        print('Warning: "' + cur + '" not found')
        return
    for f in g[cur]:
        if f in visited:
            if f in trace:
                print('Cycle: ' + str(trace) + '\n')
        else:
            trace.append(f)
            #print(trace)
            find_cycles_from(g, f, trace, visited)
            trace.pop(-1)
            #print(trace)


# Finds all the cycles in the graph "g" and print them on the standard
# output
def find_cycles(g):
    start_pool = list(g.keys())

    while len(start_pool) > 0:
        start = start_pool[0]
        trace = []
        visited = []
        find_cycles_from(g, start, trace, visited)
        for f in visited:
            if f in start_pool:
                start_pool.remove(f)


# Recursive function used to find include dependencies
def find_include_deps_from(g, cur, vis, inc):
    vis.append(cur)
    if cur not in g.keys():
        print('Warning: "' + cur + '" not found')
        return
    if cur.endswith('.hpp') or cur.endswith('.hh'):
        inc.append(cur)
    for nxt in g[cur]:
        if nxt not in vis:
            find_include_deps_from(g, nxt, vis, inc)


# Find the include dependencies for each module (*.cpp)
def find_include_deps(g):
    res = []
    tot_deps = 0

    for e in g.keys():
        if e.endswith('.cpp'):
            visited = []
            includes = []
            find_include_deps_from(g, e, visited, includes)
            if len(includes) > 0:
                line = e.replace('.cpp', '.o') + ': '
                for h in includes:
                    line += h + ' '
                res.append(line)
            tot_deps += len(includes)

    print('Total number of dependencies: ' + str(tot_deps))

    return res


############################## MAIN ###############################

# Build the graph from the sources and save it into "deps.gv"
extensions = ['cpp', 'hpp', 'hh']
blacklist = ['test-serializer.cpp']

files = get_files(extensions, blacklist)

graph = dict()

fout = open('deps.gv', 'w')
fout.write('digraph G {\n')

for f in files:
    includes = get_includes(f)
    left = transform_name(f)
    graph[f] = []
    fout.write(left + ' [style="filled", fillcolor="yellow"]')
    for i in includes:
        right = transform_name(i)
        graph[f].append(i)
        fout.write(left + ' -> ' + right + '\n')

fout.write('}\n')
fout.close()

# Find cyclical dependencies, if any
find_cycles(graph)

# Generate 'Makefile.gen'
incdeps = find_include_deps(graph)

fin = open('Makefile.ske', 'r')
fout = open('Makefile.gen', 'w')

while 1:
    line = fin.readline()
    if line == '':
        break
    if re.match(r'### INITGEN ###', line):
        fout.write('### GENERATED ###\n')
        fout.write('OBJS=')
        for e in graph.keys():
            if e.endswith('.cpp'):
                fout.write(e.replace('.cpp', '.o') + ' ')
        fout.write('\n\n')
        for h in incdeps:
            fout.write(h + '\n\n')
        fout.write('### DETARENEG ###\n')
    else:
        fout.write(line)

fin.close()
fout.close()

