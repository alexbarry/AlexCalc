#!/usr/bin/env python3
from __future__ import print_function

import subprocess
import re
import sys
import math

NEG_SYMB = r'\text{-}'

calc_bin_file = sys.argv[1]
test_idx_to_run = set()
if len(sys.argv) >= 3:
	test_num      = int(sys.argv[2])
	print( 'Only running test num %d' % test_num)
	test_idx_to_run.add(test_num)

class bcolors:
	HEADER  = '\033[95m'
	OKBLUE  = '\033[94m'
	OKGREEN = '\033[92m'
	WARNING = '\033[93m'
	FAIL    = '\033[91m'
	ENDC    = '\033[0m'

tests = [
		( '0.6',                           0.6),
		( '.6',                            0.6),

		( '.6 + .4',                       1.0),


		( '3.14159',                     3.14159    ),
		( '1+2*3',                       7          ),
		( '3*(1+2)-4',                   5          ),
		( '3  *  (1  +  2 ) - 4',        5          ),
		( '2^3^4',                       2.41785e24 ),
		( '(2^3)^4',                     4096       ),
		( '2^(3^4)',                     2.41785e24 ),
		( '8^4',                         4096       ),
		( '3 + 4 * 5',                   23         ),
		( '3 * 4 + 5',                   17         ),
		( '3 * 4 * 5',                   60         ),
		( '3 + 4 + 5',                   12         ),
		( '0.3 + 3.9 * 5',               19.8       ),
		('-0.3 + 3.9 * 5',               19.2       ),
		( ' 1 +  2 * 3 + 4 *  5 ^ 6',    62507      ),
		( '1+2*3+4*5^6',                 62507      ),
		( '(1+2)*((3+4)*5)^2',           3675       ),
		( '(1+2)*(((3+4)*5)^2',          3675       ),
		( '((1+2)*((3+4)*5)^2)',         3675       ),
		( '(((1+2)*((3+4)*5)^2)',        3675       ),
		( '((1+2)*((3+4)*5)^2)',         3675       ),
		( '-1 +  2 * 3 + 4 *  5 ^ 6',    62505      ),
		( '1 +  -2 * 3 + 4 *  5 ^ 6',    62495      ),
		( '1 -  2 * 3 + 4 *  5 ^ 6',     62495      ),
		( '1 +  2 * 3 + 4 * -5 ^ 6',    -62493      ),
		( '1.0',                         1.0        ),
		( '1.1',                         1.1        ),
		( '1.9',                         1.9        ),
		( '-1.9',                       -1.9        ),
		( '-4.3',                       -4.3        ),
		( '-4.3e4',                     -4.3e4      ),
		( '4.3e4',                       4.3e4      ),
		( '6.02e23',                     6.02e23    ),
		( '-6.02e23',                   -6.02e23    ),
		( '6.02e-23',                    6.02e-23   ),
		( '-6.02e-23',                  -6.02e-23   ),
		( '1.1 + 2.2',                   1.1 + 2.2  ),
		( '1.1 - 2.2',                   1.1 - 2.2  ),
		( '1e6 * 1e3',                   1e6 * 1e3  ),
		# ( '2 + 2',                       4.00000001 ),
		#' 1 + -2 * 3 + 4 * -5 ^ 6',
		( 'e^(0*pi)+1/cos(0)*1',        2 ),
		( '5+4+3/(2+1)*1',              10),
		( '4+6/3*2',                     8),

		( 'sqrt(16)',                    4),
		( 'sqrt(3^2 + 4^2)',             5),
		( 'sin(pi/2)',                   1),
		( 'sin(pi/4)^(-2)',              2),
		( 'cos(pi/4)^(-2)',              2),
		( 'sin(pi/4)^2',                 0.5),
		( 'cos(pi/4)^2',                 0.5),
		( 'sin(pi/3)^2',                 0.5),
		( 'cos(pi/3)',                   0.5),
		( 'cos(pi/6 + pi/6)',            0.5),
		( 'cos(pi/6+pi/6)',              0.5),
		( 'cos(pi/6 +pi/6)',             0.5),
		( 'cos(pi/6+ pi/6)',             0.5),
		( 'sin(pi/3)^2',                 0.75),
		( 'sin( (pi*0.5 + pi/2)/(1 + 2))^2',                 0.75),
		( 'sqrt((e/3+2*e/3)^(i*pi)+2)+3',    4.0),

		( '-10*e^(j*pi) + 9angle180',           1, 'degree'),
		( 'i*i',                               -1, 'degree'),
		( 'i*i',                               -1),
		( 'i*i',                               -1, 'degree'),
		( 'i*i',                               -1),
		( '-i*i',                               1, 'degree'),
		( '-i*i',                               1),
		( '-i*-i',                             -1, 'degree'),
		( '-i*-i',                             -1),
		( 'i/i',                                1, 'degree'),
		( 'i/i',                                1),
		( '-i/i',                              -1, 'degree'),
		( '-i/i',                              -1),

		( '8i*4i',                               -32, 'degree'),
		( '8i*4i',                               -32),
		( '8i*4i',                               -32, 'degree'),
		( '8i*4i',                               -32),
		( '-8i*4i',                               32, 'degree'),
		( '-8i*4i',                               32),
		( '-8i*-4i',                             -32, 'degree'),
		( '-8i*-4i',                             -32),
		( '8i/4i',                                2, 'degree'),
		( '8i/4i',                                2),
		( '-8i/4i',                              -2, 'degree'),
		( '-8i/4i',                              -2),

		( 'i*1angle90',                        -1, 'degree'),
		( 'i*1angle-90',                        1, 'degree'),
		( 'i*1 angle 90',                      -1, 'degree'),
		( 'i*1 angle -90',                      1, 'degree'),
		( 'i*1 angle(90)',                     -1, 'degree'),
		( 'i*1 angle(-90)',                     1, 'degree'),
		#( '(i*1)angle(90)',                    -1, 'degree'),
		#( '(i*1)angle(-90)',                    1, 'degree'),
		( 'sqrt(1angle90)*sqrt(i)*i',          -1, 'degree'),
		( 'sqrt(1angle-90)*sqrt(-i)*i',         1, 'degree'),
		( 'getangle(sqrt(1angle178))',       89, 'degree'),
		( 'getangle(sqrt(1angle-178))',     -89, 'degree'),
		( 'getangle(sqrt(1angle176))',       88, 'degree'),
		( 'getangle(sqrt(1angle-176))',     -88, 'degree'),
		( 'getangle(sqrt(1angle90))',        45, 'degree'),
		( 'getangle(sqrt(1angle-90))',      -45, 'degree'),
		( 'getangle(sqrt((3+4j)*(3+4j)))',        math.atan2(4,3)),
		( 'getangle(-sqrt((3+4j)*-(3+4j)))',      math.atan2(4,3)),
		( 'getangle(sqrt((3+4j)*(3+4j)))',        math.atan2(4,3)),

		( 'sin(0)', 0),
		( 'sin(pi/6)', 1/2),
		( 'sin(pi/4)', 1/math.sqrt(2)),
		( 'sin(pi/3)', math.sqrt(3)/2),
		( 'sin(pi/2)', 1),
		( 'sin(pi)', 0),

		( 'cos(pi)', -1),
		( 'tan(pi)', 0),

		( 'sin(0)', 0,                              'degree'),
		( 'sin(30)', 1/2,                           'degree'),
		( 'sin(45)', 1/math.sqrt(2),                'degree'),
		( 'sin(60)', math.sqrt(3)/2,                'degree'),
		( 'sin(90)', 1,                             'degree'),
		( 'sin(180)', 0,                            'degree'),

		( 'cos(180)', -1,                           'degree'),
		( 'tan(180)', 0,                            'degree'),

		( '1e-15 + 1e-18', 1.001e-15),
		( '2e-15 + 8e-18', 2.008e-15),
		( '2e-105 + 8e-108', 2.008e-105),
		( '-2e-105 + -8e-108', -2.008e-105),
		( '-2e-105 - 8e-108', -2.008e-105),

		( '4e-100 * 3e-50', 12e-150),
		( '-4e-100 * 3e-50', -12e-150),
		( '4e-100 * -3e-50', -12e-150),
		( '-4e-100 * -3e-50', 12e-150),

		( '2e-20 ^ 2', 2e-20**2),
		( '2e-20   ^   2', 2e-20**2),
		( '2e-20^ 2', 2e-20**2),
		( '2e-20 ^2', 2e-20**2),
		( '2e-20^2', 2e-20**2),

		( '2e-20 ^ -2', 2e-20**-2),
		( '2e-20^-2', 2e-20**-2),
		( '2e-20 ^-2', 2e-20**-2),
		( '2e-20^ -2', 2e-20**-2),

		( 'sqrt(1e-100)', 1e-200),

		( '3e-100 * (1.5e-50 + 1.5e-50)', 9e-200),
		( 'sqrt(3e-100 * (1.5e-50 + 1.5e-50))', 3e-100),
		( 'sqrt(2e-100)', math.sqrt(2e-100)),


]

cursor = '\\text{[]}'

latex_tests = [
    ( '12345',              0,    ''         + cursor + '\\text{12345}'),
    ( '12345',              1,    '\\text{1}' + cursor + '\\text{2345}'),
    ( '12345',              2,    '\\text{12}' + cursor + '\\text{345}'),
    ( '12345',              3,    '\\text{123}' + cursor + '\\text{45}'),
    ( '12345',              4,    '\\text{1234}' + cursor + '\\text{5}'),
    ( '12345',              5,    '\\text{12345}' + cursor + ''),

    ( '12 + 34',            0,    r'' + cursor + r'\text{12} + \text{34}'),
    ( '12 + 34',            1,    r'\text{1}' + cursor + r'\text{2} + \text{34}'),
    ( '12 + 34',            2,    r'\text{12}' + cursor + r' + \text{34}'),
    ( '12 + 34',            3,    r'\text{12}' + cursor + r' + \text{34}'),
    ( '12 + 34',            4,    r'\text{12} + ' + cursor + r'\text{34}'),
    ( '12 + 34',            5,    r'\text{12} + ' + cursor + r'\text{34}'),
    ( '12 + 34',            6,    r'\text{12} + \text{3}' + cursor + r'\text{4}'),
    ( '12 + 34',            7,    r'\text{12} + \text{34}' + cursor),

    ( 'x + (12 + 34)',            0,    r'' + cursor + r'x + \left(\text{12} + \text{34}\right)'),
    ( 'x + (12 + 34)',            1,    r'x' + cursor + r' + \left(\text{12} + \text{34}\right)'),
    ( 'x + (12 + 34)',            2,    r'x' + cursor + r' + \left(\text{12} + \text{34}\right)'),
    ( 'x + (12 + 34)',            3,    r'x + ' + cursor + r'\left(\text{12} + \text{34}\right)'),
    ( 'x + (12 + 34)',            4,    r'x + ' + cursor + r'\left(\text{12} + \text{34}\right)'),
    ( 'x + (12 + 34)',            5,    r'x + \left(' + cursor + r'\text{12} + \text{34}\right)'),
    ( 'x + (12 + 34)',            6,    r'x + \left(\text{1}' + cursor + r'\text{2} + \text{34}\right)'),
    ( 'x + (12 + 34)',            7,    r'x + \left(\text{12}' + cursor + r' + \text{34}\right)'),
    ( 'x + (12 + 34)',            8,    r'x + \left(\text{12}' + cursor + r' + \text{34}\right)'),
    ( 'x + (12 + 34)',            9,    r'x + \left(\text{12} + ' + cursor + r'\text{34}\right)'),
    ( 'x + (12 + 34)',           10,    r'x + \left(\text{12} + ' + cursor + r'\text{34}\right)'),
    ( 'x + (12 + 34)',           11,    r'x + \left(\text{12} + \text{3}' + cursor + r'\text{4}\right)'),
    ( 'x + (12 + 34)',           12,    r'x + \left(\text{12} + \text{34}' + cursor + r'\right)'),
    ( 'x + (12 + 34)',           13,    r'x + \left(\text{12} + \text{34}\right)' + cursor),

    (  'pi',                      0,    r'' + cursor + r'\pi'),
    (  'pi',                      2,    r'\pi' + cursor + r''),

    (  'x',                       0,    r'' + cursor + r'x'),
    (  'x',                       1,    r'x' + cursor + r''),

    (  'alex',                    0,    r'' + cursor + r'\text{alex}'),
    (  'alex',                    4,    r'\text{alex}' + cursor + r''),


    (  'sqrt(alex^(i*pi)+1)+3',   0,   cursor + r'\sqrt{\left(\text{alex}^{\left(i \cdot \pi\right)} + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',   5,   r'\sqrt{\left(' + cursor + r'\text{alex}^{\left(i \cdot \pi\right)} + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',   9,   r'\sqrt{\left(\text{alex}' + cursor + r'^{\left(i \cdot \pi\right)} + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  10,   r'\sqrt{\left(\text{alex}^{' + cursor + r'\left(i \cdot \pi\right)} + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  11,   r'\sqrt{\left(\text{alex}^{\left(' + cursor + r'i \cdot \pi\right)} + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  12,   r'\sqrt{\left(\text{alex}^{\left(i' + cursor + r' \cdot \pi\right)} + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  13,   r'\sqrt{\left(\text{alex}^{\left(i \cdot ' + cursor + r'\pi\right)} + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  15,   r'\sqrt{\left(\text{alex}^{\left(i \cdot \pi' + cursor + r'\right)} + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  16,   r'\sqrt{\left(\text{alex}^{\left(i \cdot \pi\right)}' + cursor + r' + \text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  17,   r'\sqrt{\left(\text{alex}^{\left(i \cdot \pi\right)} + ' + cursor + r'\text{1}\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  18,   r'\sqrt{\left(\text{alex}^{\left(i \cdot \pi\right)} + \text{1}' + cursor + r'\right)} + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  19,   r'\sqrt{\left(\text{alex}^{\left(i \cdot \pi\right)} + \text{1}\right)}' + cursor + r' + \text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  20,   r'\sqrt{\left(\text{alex}^{\left(i \cdot \pi\right)} + \text{1}\right)} + ' + cursor + r'\text{3}'),
    (  'sqrt(alex^(i*pi)+1)+3',  21,   r'\sqrt{\left(\text{alex}^{\left(i \cdot \pi\right)} + \text{1}\right)} + \text{3}' + cursor + r''),

    #( '12345 +(x/789)^2',   0,    ''         + cursor + r'\text{12345} + \left(\frac{x}{\text{789}}\right)^{\text{2}}'),
    #( '12345 +(x/789)^2',   1,    r'\text{1}' + cursor + r'\text{2345} + \left(\frac{x}{\text{789}}\right)^{\text{2}}'),
    #( '12345 +(x/789)^2',   2,    r'\text{12}' + cursor + r'\text{345} + \left(\frac{x}{\text{789}}\right)^{\text{2}}'),
    #( '12345 +(x/789)^2',   3,    r'\text{123}' + cursor + r'\text{45} + \left(\frac{x}{\text{789}}\right)^{\text{2}}'),
    #( '12345 +(x/789)^2',   4,    r'\text{1234}' + cursor + r'\text{5} + \left(\frac{x}{\text{789}}\right)^{\text{2}}'),
    #( '12345 +(x/789)^2',   5,    r'\text{12345}' + cursor + r' + \left(\frac{x}{\text{789}}\right)^{\text{2}}'),
    #( '12345 +(x/789)^2',   6,    r'\text{12345}' + cursor + r' + \left(\frac{x}{\text{789}}\right)^{\text{2}}'),

    (  '1-(2+3)',  None,   r'1 - \left(2 + 3\right)'),
    (  '1+(2+3)',  None,   r'1 + 2 + 3'),
    (  '1-(2-3)',  None,   r'1 - \left(2 - 3\right)'),
    (  '1- 2-3',   None,   r'1 - 2 - 3'),
    (  '1-2*3',    None,   r'1 - 2 \cdot 3'),
    (  '1+2*3',    None,   r'1 + 2 \cdot 3'),
    (  '1*2-3',    None,   r'1 \cdot 2 - 3'),
    (  '1*-2-3',   None,   r'1 \cdot \left(\text{-}2\right) - 3'),
    (  '-x^y',     None,   r'\text{-}x^y'),
    (  '(-x)^y',   None,   r'\left(\text{-}x\right)^y'),
    (  'a+b^2*c/d*e-f',             None, r'a + \frac{b^2 \cdot c}{d} \cdot e - f'),
    (  '(a+b)^2*c/d*e-f',           None, r'\frac{\left(a + b\right)^2 \cdot c}{d} \cdot e - f'),
    (  '(a+b^2)*c/d*e-f',           None, r'\frac{\left(a + b^2\right) \cdot c}{d} \cdot e - f'),
    (  '(a+b^2*c)/d*e-f',           None, r'\frac{a + b^2 \cdot c}{d} \cdot e - f'),
    (  '(a+b^2*c/d)*e-f',           None, r'\left(a + \frac{b^2 \cdot c}{d}\right) \cdot e - f'),
    (  'a+b^2*c/d*(e-f)',           None, r'a + \frac{b^2 \cdot c}{d} \cdot \left(e - f\right)'),
	(  '(-b+sqrt(b^2-4*a*c))/(2*a)',None, r'\frac{\text{-}b + \sqrt{b^2 - 4 \cdot a \cdot c}}{2 \cdot a}'),
	(  '-(a+b)',                    None, r'\text{-}\left(a + b\right)'),
	(  '-(a-b)',                    None, r'\text{-}\left(a - b\right)'),
	(  '-(a*b)',                    None, r'\text{-}a \cdot b'),
	(  '-a*b',                      None, r'\text{-}a \cdot b'),
	(  '-(a/b)',                    None, r'\text{-}\frac{a}{b}'),
	(  '-a/b',                      None, r'\text{-}\frac{a}{b}'),
	(  '1-(a-b)',                   None, r'1 - \left(a - b\right)'),
	(  '1-(a-(b+(d-e)))',           None, r'1 - \left(a - \left(b + d - e\right)\right)'),
	(  '-(a-b)^2',                  None, r'\text{-}\left(a - b\right)^2'),
	(  'a-(b-(c-d))',               None, r'a - \left(b - \left(c - d\right)\right)'),

	( '1angle180',                 None, r'1 \measuredangle 180'),
	( '1angle 180',                None, r'1 \measuredangle 180'),
	( '1 angle180',                None, r'1 \measuredangle 180'),
	( '1 angle 180',               None, r'1 \measuredangle 180'),
	( '1   angle 180',             None, r'1 \measuredangle 180'),

	( '1angle180',                 0,    r'' + cursor + r'\text{1} \measuredangle \text{180}'),
	( '1angle180',                 1,    r'\text{1}' + cursor + r' \measuredangle \text{180}'),
	( '1angle180',                 6,    r'\text{1} \measuredangle ' + cursor + r'\text{180}'),
	( '1angle180',                 7,    r'\text{1} \measuredangle \text{1}' + cursor + r'\text{80}'),
	( '1angle180',                 8,    r'\text{1} \measuredangle \text{18}' + cursor + r'\text{0}'),
	( '1angle180',                 9,    r'\text{1} \measuredangle \text{180}' + cursor ),

	(  '1 km',                         0, r'' + cursor + r'\text{1}\,\text{km}'),
	(  '1 km',                         1, r'\text{1}' + cursor + r'\,\text{km}'),
	(  '1 km',                         2, r'\text{1}\,' + cursor + r'\text{km}'),
	(  '1 km',                         3, r'\text{1}\,\text{k}' + cursor + r'\text{m}'),
	(  '1 kPa',                        3, r'\text{1}\,\text{k}' + cursor + r'\text{Pa}'),
	(  '1 zxy',                        3, r'\text{1}\,\text{z}' + cursor + r'\text{xy}'),
	(  '1 km',                         4, r'\text{1}\,\text{km}' + cursor + r''),

	(  '1 km * 3 mi',                  0, r'' + cursor + r'\text{1}\,\text{km} \cdot \text{3}\,\text{mi}'),
	(  '1 km^2 * 3 mi',                0, r'' + cursor + r'\text{1}\,\text{km}^\text{2} \cdot \text{3}\,\text{mi}'),
	(  '1 m s^- kPa^-1',               0, r'' + cursor + r'\text{1}\,\text{m}\,\text{s}^\text{-}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              0, r'' + cursor + r'\text{1}\,\text{m}\,\text{s}^\text{-1}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              1, r'\text{1}' + cursor + r'\,\text{m}\,\text{s}^\text{-1}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              2, r'\text{1}\,' + cursor + r'\text{m}\,\text{s}^\text{-1}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              3, r'\text{1}\,\text{m}' + cursor + r'\,\text{s}^\text{-1}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              4, r'\text{1}\,\text{m}\,' + cursor + r'\text{s}^\text{-1}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              5, r'\text{1}\,\text{m}\,\text{s}' + cursor + r'^\text{-1}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              6, r'\text{1}\,\text{m}\,\text{s}^{' + cursor + r'\text{-1}}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              7, r'\text{1}\,\text{m}\,\text{s}^{\text{-}' + cursor + r'\text{1}}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              8, r'\text{1}\,\text{m}\,\text{s}^{\text{-1}' + cursor + r'}\,\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',              9, r'\text{1}\,\text{m}\,\text{s}^\text{-1}\,' + cursor + r'\text{kPa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',             10, r'\text{1}\,\text{m}\,\text{s}^\text{-1}\,\text{k}' + cursor + r'\text{Pa}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',             11, r'\text{1}\,\text{m}\,\text{s}^\text{-1}\,\text{kP}' + cursor + r'\text{a}^\text{-1}'),
	(  '1 m s^-1 kPa^-1',             12, r'\text{1}\,\text{m}\,\text{s}^\text{-1}\,\text{kPa}' + cursor + r'^\text{-1}'),
	(  '1 m s^-1 kPa^-1',             13, r'\text{1}\,\text{m}\,\text{s}^\text{-1}\,\text{kPa}^{' + cursor + r'\text{-1}}'),
	(  '1 m s^-1 kPa^-1',             14, r'\text{1}\,\text{m}\,\text{s}^\text{-1}\,\text{kPa}^{\text{-}' + cursor + r'\text{1}}'),
	(  '1 m s^-1 kPa^-1',             15, r'\text{1}\,\text{m}\,\text{s}^\text{-1}\,\text{kPa}^{\text{-1}' + cursor + r'}'),
	# TODO keep going, progressing cursor through wip token with units

	(  '1 s^-1 m',                  None, r'1\,\text{s}^{' + NEG_SYMB + r'1}\,\text{m}'),
	(  '1 s^2 m',                   None, r'1\,\text{s}^2\,\text{m}'),
	(  '1 s^-2 m',                  None, r'1\,\text{s}^{' + NEG_SYMB + r'2}\,\text{m}'),
	(  '1 m s^-1',                  None, r'1\,\text{m}\,\text{s}^{' + NEG_SYMB + r'1}'),
	(  '1 m s^-1 m',                None, r'1\,\text{m}\,\text{s}^{' + NEG_SYMB + r'1}\,\text{m}'),

	(  '1km',                       0,    r'' + cursor + r'\text{1}\,\text{km}'),

	# This one is a little ambiguous. Should the cursor go before or after the space?
	# If you enter a digit, it will render before the space, and your cursor will then
	#     definitely be before the space
	# If you enter a character, it will render after the space, and your cursor
	#     will then definitely be after the space
	# Maybe there should be no space in this case.
	(  '1km',                       1,    r'\text{1}' + cursor + r'\text{km}'),
	(  '1km',                       2,    r'\text{1}\,\text{k}' + cursor + r'\text{m}'),
	(  '1km',                       3,    r'\text{1}\,\text{km}' + cursor + r''),

        (  '(1 km)',                None,    r'1\,\text{km}'),
        (  '(1 km)^2',              None,    r'\left(1\,\text{km}\right)^2'),
        (  '(1 km)^-2',             None,    r'\left(1\,\text{km}\right)^{' + NEG_SYMB + r'2}'),
        (  '(1 km)^12',             None,    r'\left(1\,\text{km}\right)^{12}'),
        (  '(1 km^2)',              None,    r'1\,\text{km}^2'),
        (  '(1 km^2)^2',            None,    r'\left(1\,\text{km}^2\right)^2'),
        (  '(1 km^2)^-2',           None,    r'\left(1\,\text{km}^2\right)^{' + NEG_SYMB + r'2}'),
        (  '(1 km^-2)',             None,    r'1\,\text{km}^{' + NEG_SYMB + r'2}'),

        (  '(1 angle 90)',          None,    r'1 \measuredangle 90'),
        (  '(1 angle 90)^2',        None,    r'\left(1 \measuredangle 90\right)^2'),
        (  '(1 angle -90)',         None,    r'1 \measuredangle \left(' + NEG_SYMB + r'90\right)'),
        (  '(1 angle -90)^2',       None,    r'\left(1 \measuredangle \left(' + NEG_SYMB + r'90\right)\right)^2'),

        (  '1e3',                   None,    r'\left(1 \cdot 10^3\right)'),
        (  '1e-3',                  None,    r'\left(1 \cdot 10^{' + NEG_SYMB + r'3}\right)'),
        (  '4.56e-3',               None,    r'\left(4.56 \cdot 10^{' + NEG_SYMB + r'3}\right)'),
        (  'j1e3',                  None,    r'\left(j1 \cdot 10^3\right)'),
        (  'j1e-3',                 None,    r'\left(j1 \cdot 10^{' + NEG_SYMB + r'3}\right)'),
        (  'j4.56e-3',              None,    r'\left(j4.56 \cdot 10^{' + NEG_SYMB + r'3}\right)'),
        (  '1e3j',                  None,    r'\left(j1 \cdot 10^3\right)'),
        (  '1e-3j',                 None,    r'\left(j1 \cdot 10^{' + NEG_SYMB + r'3}\right)'),
        (  '4.56e-3j',              None,    r'\left(j4.56 \cdot 10^{' + NEG_SYMB + r'3}\right)'),

	( '0',    None, '0' ),
	( '0.6',  None, '0.6' ),
	( '0.65',  None, '0.65' ),

	( '3',  None, '3' ),
	( '3.1',  None, '3.1' ),

	( '0',    1, r'\text{0}' + cursor ),
	( '0.',   2, r'\text{0.}'  + cursor),
	( '0.6',  3, r'\text{0.6}'  + cursor),

	( '.6',   0, cursor + r'\text{.6}' ),
	( '.6',   1, r'\text{.}' + cursor + r'\text{6}' ),
	( '.61',   2, r'\text{.6}' + cursor + r'\text{1}' ),

	( '0.6',   0, cursor + r'\text{0.6}' ),
	( '0.6',   1, r'\text{0}' + cursor + r'\text{.6}' ),
	( '0.6',   2, r'\text{0.}' + cursor + r'\text{6}' ),
	( '0.6',   3, r'\text{0.6}' + cursor),

]

err_tests = [
	'.6.4',
	'.6 .4',
	'.6e.4',
	'e6',
	'6e',

	'',
]

# Calling these "unit tests" would be confusing, given that everything here is a "unit test"
tests_for_units = [
	( '1 cal',          4.184, 'J' ),

	( '1 kcal',         4184,  'J' ),
	( '1 Cal',          4184,  'J' ),
	( '1 Calorie',      4184,  'J' ),
	( '1 kilocalorie',  4184,  'J' ),

	( '101.325 kPa to atm', 1, 'atm'),
	( '101.325 kPa * 2 to atm', 2, 'atm'),

	# TODO consider defaulting output to kPa when dimension is N/m
	( '1 atm', 101325, 'Pa'),
]

def python_solve_test(test):
	test = re.sub( r'\^', r'**', test )
	return eval(test)
	

def run_test(test, cmds=None, parse_float=True):
	if not cmds: cmds = []
	#py_answer = python_solve_test( test )

	p = subprocess.Popen( calc_bin_file, stdin=subprocess.PIPE,
	                                     stdout=subprocess.PIPE,
	                                     stderr=subprocess.PIPE )
	cmd_str = '\n'.join(':%s' % x for x in cmds)
	if cmd_str: cmd_str += '\n'
	test_input = bytes( ":echo on\n" + cmd_str + test + '\n:alloc\n:exit\n', encoding='utf-8' )
	try:
		output, output_err   = p.communicate( input=test_input, timeout=1 )
	except subprocess.TimeoutExpired:
		print('Test %r caused timeout, killing process' % test)
		# If a timeout happens, the process may be stuck in an infinite loop.
		p.kill()
		raise

	output_err = output_err.decode('utf-8')
	output = output.decode( 'utf-8' )
	output = output.split( '\n' )
	cmd_degree_expected_output = 'Trig input and calc output will now be in degrees'
	for cmd in cmds:
		output_line = output.pop(0)
		if cmd == 'degree':
			if output_line != cmd_degree_expected_output:
				raise Exception('output_str from :degree did not match expected: %r' % output_line )
		else:
			print('Unsure how to validate cmd %r' % cmd)
	if len(output) < 2:
		raise Exception('expected calc_output and mem_test_output, recvd: %r' % output )
	calc_output = output[0]
	memory_test_output = output[1]

	if parse_float:
		try:
			calc_output = float(calc_output)
		except ValueError as e:
			#calc_output = repr(e)
			calc_output = e
	
	if memory_test_output == 'There are 0 nodes allocated from prior calls':
		memory_leak = None
	elif not re.match( r'There are (\d+) nodes allocated from prior calls', memory_test_output):
		raise Exception('output_str from :alloc did not match expected: %r' % memory_test_output)
	else:
		memory_leak = memory_test_output
		
	
	return calc_output, memory_leak, output_err

def run_latex_test(test, cursor_pos):
	#py_answer = python_solve_test( test )

	p = subprocess.Popen( calc_bin_file, stdin=subprocess.PIPE,
	                                     stdout=subprocess.PIPE,
	                                     stderr=subprocess.PIPE )
	cursor_cmd = ''
	if cursor_pos is not None:
		cursor_cmd = ':cursor {cursor_pos}\n'.format(cursor_pos=cursor_pos)
	input_str = (
        ':to_latex\n'
        '{cursor_cmd}'
        '{test_input}\n'
        ':alloc\n'
        ':exit').format(cursor_cmd = cursor_cmd,
                        test_input = test)

	test_input = bytes(input_str, encoding='utf-8' )
	output, output_err   = p.communicate( input=test_input, timeout=1 )
	output_err = output_err.decode('utf-8')
	output = output.decode( 'utf-8' )
	output = output.split( '\n' )
	#print(output)
	latex_output = output.pop(0)
	if latex_output != 'Output to latex is now: 1':
	    raise Exception('Expected "output to latex", recvd %r' % latex_output)
	if cursor_pos is not None:
		cursor_pos_output = output.pop(0)
		if cursor_pos_output != 'Cursor is now shown at pos %d' % cursor_pos:
		    raise Exception('Expected "cursor is now shown at ...", recvd %r' % cursor_pos_output)
	if len(output) < 2:
		raise Exception('expected line for output and mem test, recvd %r' % output )
	calc_output = output.pop(0)
	memory_test_output = output.pop(0)

	if memory_test_output == 'There are 0 nodes allocated from prior calls':
		memory_leak = None
	elif not re.match( r'There are (\d+) nodes allocated from prior calls', memory_test_output):
		raise Exception('output_str from :alloc did not match expected: %r' % memory_test_output)
	else:
		memory_leak = memory_test_output
		
	
	return calc_output, memory_leak, output_err

def run_err_test(test):
	calc_output, memory_leak, output_err = run_test(test, parse_float=False)
	#print('%r, %r, %r, %r' % (test, calc_output, memory_leak, output_err))
	# TODO this is ugly, but I guess I polluted stderr with debugging information?
	if 'exception' not in calc_output.lower():
		return False, calc_output

	return True, calc_output

def run_test_for_unit(test, expected_output_val, expected_output_units):
	calc_output, memory_leak, output_err = run_test(test, parse_float=False)

	mag_val, unit_val = calc_output.split(' ', maxsplit=1)
	mag_val = float(mag_val)

	test_passed = (expected_output_val == mag_val and expected_output_units == unit_val)
	if not test_passed:
		print('mag: %r, unit: %r' % (mag_val, unit_val))
		print('expected_output: val %r, unit %r' % (expected_output_val, expected_output_units))

	return test_passed, calc_output


tests_run = []
failed_tests = []
mem_leak_tests = []

print_all = False
#print_all = True

def within_frac(a,b, frac):
	if isinstance(b,Exception): return False
	if a == 0: return a == b
	return (a - b)/a < frac

print("Starting to run up to %d normal tests..." % len(tests))
for test_idx, test_params in enumerate(tests):
	if test_idx_to_run and test_idx not in test_idx_to_run:
		continue
	test, expected_answer = test_params[:2]
	cmds = []
	if len(tests) > 2:
		cmds = list(test_params[2:])
	if print_all: print( 'Giving calc input %r (w cmds %r)...' % (test, cmds), end=' ' )
	try:
		received_answer, mem_leak, output_err = run_test( test, cmds )
	except Exception as e:
		print('Cmd %r, %r caused exception' % (test, cmds))
		raise
	if mem_leak:
		mem_leak_tests.append( (test, mem_leak) )
	# if expected_answer == received_answer:
	if within_frac(expected_answer, received_answer, 1e-9):
		if print_all: print( 'received answer %r' % received_answer, end =' ' ) 
		if print_all: print( bcolors.OKGREEN + 'test passed' + bcolors.ENDC )
	else:
		if print_all: print( 'expected %r, received %r' % ( expected_answer, received_answer ), end=' '  ) 
		if print_all: print( bcolors.FAIL + 'test FAILED' + bcolors.ENDC )
		print('stderr: ')
		print(output_err)
		failed_tests.append( (test, test_idx, expected_answer, received_answer) )
	tests_run.append( (test, test_idx))

print("Starting to run up to %d LaTeX tests..." % len(latex_tests))
for test_idx, (str_input, cursor_pos, expected_output) in enumerate(latex_tests):
	if test_idx_to_run and test_idx not in test_idx_to_run:
		continue
	try:
		actual_output, mem_leak, output_err = run_latex_test(str_input, cursor_pos)
	except Exception as e:
		print('Cmd %r,%r caused exception' % (str_input, cursor_pos))
		raise

# ?? Why is this not part of the previous one?
print("Checking for memory leaks in up to %d LaTeX tests..." % len(latex_tests))
for str_input, cursor_pos, expected_output in latex_tests:
	try:
		actual_output, mem_leak, output_err = run_latex_test(str_input, cursor_pos)
	except Exception as e:
		print('Cmd %r,%r caused exception' % (str_input, cursor_pos))
		raise

	if mem_leak:
		mem_leak_tests.append( (test, mem_leak) )

	if expected_output == actual_output:
		if print_all: print( 'received answer %r' % actual_output, end =' ' ) 
		if print_all: print( bcolors.OKGREEN + 'test passed' + bcolors.ENDC )
	else:
		if print_all: print( 'expected %r, received %r' % ( expected_output, actual_output ), end=' '  ) 
		if print_all: print( bcolors.FAIL + 'test FAILED' + bcolors.ENDC )
		print('stderr: ')
		print(output_err)
		failed_tests.append(( '(cursor_pos:%s) %r' % (cursor_pos, str_input), test_idx, expected_output, actual_output))
	tests_run.append( (test, test_idx))

#err_tests = []
print('Checking that %d "err_tests" fail...' % len(err_tests))
for test_idx, str_input in enumerate(err_tests):
	test_passed, actual_output = run_err_test(str_input)
	if not test_passed:
		failed_tests.append( ('(err_test) %r' % str_input, test_idx, '<expected an error>', actual_output))
	tests_run.append( (test, test_idx))

print('Checking that %d "tests_for_units" pass...' % len(tests_for_units))
for test_idx, (str_input, expected_output_val, expected_output_unit) in enumerate(tests_for_units):
	test_passed, actual_output = run_test_for_unit(str_input, expected_output_val, expected_output_unit)
	if not test_passed:
		expected_output = 'val: %r, unit: %r' % (expected_output_val, expected_output_unit)
		failed_tests.append( ('(test for units) %r' % str_input, test_idx, expected_output, actual_output))
	tests_run.append( (test, test_idx))

rc = 0

total_test_count = len(tests_run)

def get_first_diff_pos(a, b):
	if type(a) != str or type(b) != str:
		return 0

	length = min(len(a), len(b))

	for i in range(length):
		if a[i] != b[i]: return i
	return length

if failed_tests:
	rc |= 1
	#print( bcolors.FAIL + '%d tests failed:' % len(failed_tests) + bcolors.ENDC )
	for test, test_idx, expected, received in failed_tests:
		expected = str(expected)
		received = str(received)
		print('Test idx %3d failed' % test_idx)
		if len(expected) < 20 and len(received) < 20:
			print( '%-60s expected "%s", actual "%s"' % ( test, expected, received ) )
		else:
			print( '%-60s' % test )
			print( 'expected "%s"' % expected)
			print( 'actual   "%s"' % received)
			diff_pos = get_first_diff_pos(expected, received)
			print( '          %s' % ( (diff_pos*' ') + '^') + '(pos=%d)' % diff_pos)

if failed_tests: print( bcolors.FAIL + '%d tests failed.' % len(failed_tests) + bcolors.ENDC )
else:
	print( bcolors.OKGREEN + 'All %d tests passed' % total_test_count + bcolors.ENDC )

if mem_leak_tests:
	rc |= 2
	for test, mem_leak_output in mem_leak_tests:
		print( '%-60r :alloc output %r' % ( test, mem_leak_output ) )
	if mem_leak_tests: print( bcolors.FAIL + '%d tests had memory leaks.' % len(mem_leak_tests) + bcolors.ENDC )
else:
	print( bcolors.OKGREEN + 'All %d tests had no memory leaks' % total_test_count + bcolors.ENDC )

sys.exit(rc)
