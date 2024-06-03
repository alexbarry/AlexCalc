#!/usr/bin/env python3

licenses = [
    ('Polyfill',            'src/android/app/src/main/assets/html/node_modules/@babel/polyfill/LICENSE'),
    ('core-js',             'src/android/app/src/main/assets/html/node_modules/core-js/LICENSE'),
    ('MathJax',             'src/android/app/src/main/assets/html/node_modules/mathjax/LICENSE'),
    ('regenerator-runtime', 'src/android/app/src/main/assets/html/node_modules/regenerator-runtime/LICENSE'),
]

f = open('combined_other_licenses.html', 'w')

f.write("""\
<!doctype html>
<html>
<head>
    <title>Other Licenses</title>
</head>
<body>
""")

f.write('<ul>\n')
for name, path in licenses:
    f.write('<li><a href="#%s">%s</a></li>\n' % (name, name))
f.write('</ul>\n')

for name, path in licenses:
    f.write('<h2 id="%s">%s</h2>\n' % (name, name))
    f.write('<pre>\n')
    f2 = open(path, 'r')
    for line in f2:
        f.write(line)
    f2.close()
    f.write('</pre>\n')
    f.write('\n\n')

f.write("""
</body>
</html>
""")

f.close()
