#!/bin/bash

in='H4sIACYjc18AA22RT2jUQBjFv8RqoVZY18siFHMSRTuUaqzgoRSzlYV2cbvrRYWQzEyabJpkN39W
XQoKnvSioBcL1j21oKgHa+1BRHTrQSh48aAH/6EHERHPHnQm7sgu+AXy+B3em/fNyJDOlFMzCAkV
rVzRC8f3d6g8c5RT7HhUqRSm8/pkYaZc6eKpCYZRHDr+rKIVy3pJL05M5wHk22BCZ059XrJ/sxH8
vH79O+fsxacv16y9ulAABO5oWCPnjMA2/WTOH0tMJ2j4ga+qiVmnRt002A8ZiCKqosgFTWTuOdm3
0XvGnU+cb/5YXmqx7EF533IrPSMDxI+QF1mxjyMH4cCD8d0KwGBbeIEqUn8OJPaxkTR5MznyPrjP
3EIBtsLsGQMlVVpDuAlF4XxV6ZO7ayy28QLn89XH8V1mFAqwE6zQGKZWk1UZNjDBKLAsB9O0kMYL
5KRMJg2RIKNICoxcyD3sxMqnrzw7cYnlCOVrxUYSJhFqWqiKE94qL1q03kgD3a129f/cwXl+XF18
wNzfSmupAmwHnXhGiJEbU9T8G1MSto1f60O9d/xolfMNNivMfu/Yx4WVNGYIyCgZUev00MHDtjtm
IzwXJMQKAz9GPo1hUkRsc98pvZFX25wbT269vcyihAJk4T/3lFekLfNZ5UPqvfZl00D6kOv/HtJ+
ffbri1XmF8o3tAg+gCzTQ40kNkO24R+PQxxHAgMAAA=='

out='H4sIAFcdh18AA+1WTW/jIBC9769AnNuRcfwVbt1mP3KJVmqlPVRRhQEnNDE4gFMlVf77Yqe3vZTd
va2FjPAAb4Y3z4OX+sj2SiCvWuk8azuHlEZup7pOChRsjm0kRW/4q7Et85givFx8uUvwDbo2fM+8
3Bh7ClNPeKm97Z0yGj4br6UPy/BjAMbrMFhIx63qfJgecK4rkLcnpTfIG8RN22vFAx56VX4b3gOa
2SMn7VFaFHDDusW3uwH0wfSWy8HnG348dePoHXH0tfyRjSYyz4GQGZCsgnRW4vXlBv2+YzjG/bjv
u3Fes/Y6u0ttJ07MbGvd73XZ18octdF53tcHyQ41Cx0wkCBzcLuAPSCsjHiP6+HniLKSrWSDixD6
s5Becm/s6Gx1dYTFhl3tAzOX9eXT8n/JSkkgTYu4rAjtoHWN19wpCNFNvH+A92I0pUlCaEkqSkKj
JMtomuRx7G9eGfQvsgN+noj/uODzCsi8gPkcSJ7FMd5Ydiubc9D8LeOCg2kaxeWk/Hjlhy6hs6RK
KKnqnNI0LhGe9eFQcG7ghfeT/mP0PyNQQpqR8BHEcf4sWmY57LyE80T6H96y1fBEil2kIskPssiq
7a7cAt+bXjQ2RAWjmykJ0T+g2XADxCVhKvl/U/KLNKEJ5TxU+qKKvHMFn0FTt3DsfW3/bdn5BTye
FH5zDQAA'

. ../test.sh
    test_conversion "dgadetection" "dgadetection" "$in" "$out"