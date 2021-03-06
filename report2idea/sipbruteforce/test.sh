#!/bin/bash

# input data:
in='
H4sIAH+AYlkAA2NiAAFGucyCxJSUIoVgJzeX+GD/0CBnVx1koRDHIHfXEJ2SzNxUiIBTkKujs0d8
iKevK5Kos6tjsCtEsDQzr8TMBCLsGubqFxLv6YKkEiKEodLH08873skzJN7N09XHBSxlbASRcgwJ
cfUNCAlGFQxzR5UwNFNwCQ6JD/APCoHxg4OcEXwLFOsjA1yRRQOC/EP8nf19dIpLijLz0iGCocGu
QcAQsmTIZoCChql/Of4DAYwfIma6CpkPAsIFeREBTx+u5px2hgnE983Pi3CAyvEIQvARYWMRBiAt
yGBmbmnmYGGiZ2SkZ2qsZ2huQD3LWGwZGKTYQJYBPQGzzADVsiiYQdqn0ttRLFuun4JumTzQsgbn
i3DLOICWcUDllEBpiQFk2Q1hJkFI0mIAABb0jBljAgAA
'

# output data:
out='
H4sIAKQLM14AA+2UzYrbMBSF930Ko3Ul9GdJ9qrTZAKBEkITKLQMgyzJwZBYQVZapmHevZbSRQKh
05RSZlHwwvY99/hc6bOOYKKj2/jwBOriC7iL0e32EX3wm64HD28LsPDWpdIRrD5lycLtnAZjpQmH
6B5bH4x7tC46E33ILeunfW4Bs63/lpSrqGM3xM7o7clT75IAmO/IuKF3EfXJFA3dPptmT/CcpPdf
XR/X3UlPMZEQE4jLNRE1ljUnn5P/WoeNi6eUSx/yXYkFTgbzJc9ZFEeUopIhInEOsQw++lw62H1y
Gb8OHvJHJ77vJ/7QJyPOBB9fzXzY6fQM5tP7O5z0E6cHdz0awXVZ5WiT4Mb1PVNVENOs4jWXNT6p
Fj7mep626IYiuO3YZovoi8PgQqGNSXnqQshKvLuYJa2vP6QFu5yfSHI+P6EKEV4hOraplxZgmrfz
pdTzaaoq3GDVKAuNVS3kVmHYGC6ha1zbUG0o1Q3InoMJ3T52vk9tq/myeJ82e5Y2u9gm4Ar9Ez/w
/Ob4H8xfg0lKQfHrIhP/Fpn4n2DJmgrz1rZQGMEhJ2ULG6okNLLhRlquWEWvYDkdcQhdc0gj3ojo
R2d8j1ZG933Xb147o6Jm9BZGiZCIS0TwTZCyPzg7yXjdQOjVww+ri/ycIYrpGH4Ek5V/EzPKsCiZ
0pBVykDOKwO1lQJKy0TpGlu2pb52+o2UFL7NiJ3/RkPi6gdB3LBtlAcAAA==
'

test -z "$srcdir" && export srcdir=.

. $srcdir/../test.sh

test_conversion "sipbf" "sipbf" "$in" "$out"

