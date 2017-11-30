/**The MIT License (MIT)
Copyright (c) 2015 by Daniel Eichhorn
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
See more at http://blog.squix.ch
*/

// Created by http://oleddisplay.squix.ch/ Consider a donation
// In case of problems make sure that you are using the font file with the correct version!

// Bodmer fix: End character is 0x7D not 0x7E, so bug in last line of the file corrected
// this avoids screen corruption if ~ is printer
#pragma once
const uint8_t ArialRoundedMTBold_14Bitmaps[] PROGMEM = {

  // Bitmap Data:
  0x00, // ' '
  0xFF,0xF8,0xF0, // '!'
  0xDE,0xF6, // '"'
  0x12,0x32,0x36,0xFF,0xFF,0x24,0xFF,0xFF,0x4C,0x48, // '#'
  0x10,0x61,0xF6,0xAD,0x7A,0x1E,0x0E,0xD7,0xAF,0x5B,0xE1,0x02,0x04,0x00, // '$'
  0x60,0x92,0x22,0x44,0x49,0x07,0x60,0x0B,0x82,0x48,0xC9,0x11,0x24,0x18, // '%'
  0x3C,0x19,0x82,0x60,0xF0,0x39,0x33,0x6C,0x73,0x1C,0xFF,0x8E,0x30, // '&'
  0xFC, // '''
  0x32,0x64,0xCC,0xCC,0xC4,0x62,0x30, // '('
  0xC4,0x62,0x33,0x33,0x32,0x64,0xC0, // ')'
  0x21,0x2A,0xE5,0x28, // '*'
  0x18,0x18,0x18,0xFF,0xFF,0x18,0x18, // '+'
  0xF6, // ','
  0xFF, // '-'
  0xF0, // '.'
  0x33,0x32,0x66,0x4C,0xCC, // '/'
  0x38,0xFB,0x9E,0x3C,0x78,0xF1,0xF6,0x7C,0x70, // '0'
  0x19,0xDF,0xB1,0x8C,0x63,0x18,0xC0, // '1'
  0x38,0xFF,0x1E,0x30,0xC3,0x0C,0x30,0xFF,0xFC, // '2'
  0x79,0x9B,0x10,0x63,0xC7,0x81,0xC3,0xC6,0xF0, // '3'
  0x06,0x0E,0x1E,0x16,0x26,0x46,0xFF,0xFF,0x06,0x06, // '4'
  0x7E,0xFD,0x06,0x0F,0xD8,0xC1,0xC3,0xCC,0xF0, // '5'
  0x38,0xDB,0x1E,0x0F,0xD8,0xF1,0xE3,0x66,0x78, // '6'
  0xFF,0xFC,0x30,0x41,0x82,0x0C,0x18,0x30,0xC0, // '7'
  0x38,0xDB,0x1F,0x63,0x98,0xF1,0xE3,0xC6,0xF8, // '8'
  0x3C,0x66,0xC3,0xC3,0xE7,0x3F,0x03,0xC3,0x66,0x3C, // '9'
  0xF0,0x3C, // ':'
  0xF0,0x3D,0x80, // ';'
  0x02,0x1D,0xF7,0x0E,0x0F,0x83,0x81, // '<'
  0xFF,0xFC,0x07,0xFF,0xE0, // '='
  0x81,0xC1,0xF0,0x70,0xEF,0xB8,0x40, // '>'
  0x3C,0xFF,0x1E,0x30,0xC7,0x0C,0x00,0x30,0x60, // '?'
  0x0F,0x83,0x06,0x60,0x24,0xED,0x99,0x9B,0x19,0xB1,0xBB,0x12,0xBF,0xE4,0xDC,0x40,0x13,0x06,0x0F,0xC0, // '@'
  0x1C,0x0E,0x05,0x06,0xC3,0x63,0x19,0xFC,0xFE,0xC1,0xE0,0xC0, // 'A'
  0xFC,0xFE,0xC7,0xC6,0xFE,0xFE,0xC3,0xC3,0xFF,0xFE, // 'B'
  0x3E,0x3F,0xB8,0xF8,0x3C,0x06,0x03,0x06,0xC7,0x7F,0x0F,0x00, // 'C'
  0xFE,0x7F,0xB0,0xF8,0x3C,0x1E,0x0F,0x07,0x87,0xFF,0x7F,0x00, // 'D'
  0xFE,0xFF,0xC0,0xC0,0xFE,0xFE,0xC0,0xC0,0xFF,0xFF, // 'E'
  0xFF,0xFF,0x06,0x0F,0xDF,0xB0,0x60,0xC1,0x80, // 'F'
  0x1E,0x3F,0x98,0xF8,0x2C,0x06,0x3F,0x1E,0xC3,0x7F,0x9F,0x00, // 'G'
  0xC1,0xE0,0xF0,0x78,0x3F,0xFF,0xFF,0x07,0x83,0xC1,0xE0,0xC0, // 'H'
  0xFF,0xFF,0xF0, // 'I'
  0x06,0x0C,0x18,0x30,0x60,0xF1,0xF3,0x7E,0x78, // 'J'
  0xC3,0x63,0xB3,0x9B,0x8F,0x87,0x63,0x19,0x8E,0xC3,0x60,0xC0, // 'K'
  0xC1,0x83,0x06,0x0C,0x18,0x30,0x60,0xFF,0xFC, // 'L'
  0xE1,0xFC,0xFF,0x3F,0xCF,0xD2,0xF7,0xBD,0xEF,0x7B,0xCC,0xF3,0x30, // 'M'
  0xC1,0xF0,0xFC,0x7E,0x3D,0x9E,0x6F,0x3F,0x8F,0xC3,0xE0,0xC0, // 'N'
  0x1E,0x1F,0xE6,0x1B,0x03,0xC0,0xF0,0x3C,0x0D,0x86,0x7F,0x87,0x80, // 'O'
  0xFE,0xFF,0xC3,0xC3,0xFF,0xFE,0xC0,0xC0,0xC0,0xC0, // 'P'
  0x1E,0x0F,0xF1,0x87,0x60,0x6C,0x0D,0x81,0xB1,0x33,0x7C,0x7F,0x83,0xD8,0x01,0x80, // 'Q'
  0xFE,0xFF,0xC3,0xC3,0xFE,0xFC,0xCE,0xC6,0xC3,0xC3, // 'R'
  0x7C,0xFE,0xC7,0xC2,0x7C,0x0F,0xC3,0xC3,0x7E,0x3C, // 'S'
  0xFF,0xFF,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, // 'T'
  0xC1,0xE0,0xF0,0x78,0x3C,0x1E,0x0F,0x07,0xC7,0x7F,0x1F,0x00, // 'U'
  0xC1,0xE0,0xD8,0xCC,0x66,0x31,0xB0,0xD8,0x6C,0x1C,0x0E,0x00, // 'V'
  0xC7,0x1E,0x38,0xF1,0x46,0xDB,0x66,0xDB,0x36,0xD9,0xA2,0xC7,0x1C,0x38,0xE1,0x83,0x00, // 'W'
  0xC3,0x66,0x7E,0x3C,0x18,0x3C,0x7E,0x66,0xC3,0xC3, // 'X'
  0xC3,0xC3,0x66,0x3E,0x3C,0x18,0x18,0x18,0x18,0x18, // 'Y'
  0x7F,0x3F,0x80,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFF,0xFF,0xC0, // 'Z'
  0xFF,0xCC,0xCC,0xCC,0xCC,0xCF,0xF0, // '['
  0xCC,0x44,0x66,0x22,0x33, // '\'
  0xFF,0x33,0x33,0x33,0x33,0x3F,0xF0, // ']'
  0x30,0xE7,0x9A,0xCF,0x30, // '^'
  0xFE, // '_'
  0xD0, // '`'
  0x7D,0x8C,0x7F,0x3C,0x79,0xDD,0x80, // 'a'
  0xC1,0x83,0x06,0xEF,0xF8,0xF1,0xE3,0xFF,0xB8, // 'b'
  0x3C,0xFF,0x1E,0x0C,0x6F,0xCF,0x00, // 'c'
  0x06,0x0C,0x1B,0xBF,0xF8,0xF1,0xE3,0xFE,0xEC, // 'd'
  0x3C,0xCF,0x1F,0xFC,0x0C,0xCF,0x00, // 'e'
  0x3B,0x19,0xF6,0x31,0x8C,0x63,0x00, // 'f'
  0x77,0xFF,0x1E,0x3C,0x7F,0xDD,0xE3,0xC6,0xF8, // 'g'
  0xC1,0x83,0x06,0xEF,0xF8,0xF1,0xE3,0xC7,0x8C, // 'h'
  0xF3,0xFF,0xF0, // 'i'
  0x33,0x03,0x33,0x33,0x33,0x3F,0xE0, // 'j'
  0xC1,0x83,0x06,0x6D,0x9E,0x3E,0x66,0xCD,0x8C, // 'k'
  0xFF,0xFF,0xF0, // 'l'
  0xD9,0xDF,0xFF,0x31,0xE6,0x3C,0xC7,0x98,0xF3,0x18, // 'm'
  0xDD,0xFF,0x1E,0x3C,0x78,0xF1,0x80, // 'n'
  0x38,0xFB,0x1E,0x3C,0x6F,0x8E,0x00, // 'o'
  0xDD,0xFF,0x1E,0x3C,0x7F,0xF7,0x60,0xC1,0x80, // 'p'
  0x77,0xFF,0x1E,0x3C,0x7F,0xDD,0x83,0x06,0x0C, // 'q'
  0xDF,0xF1,0x8C,0x63,0x00, // 'r'
  0x7B,0x3E,0x1E,0x0F,0x37,0x80, // 's'
  0x63,0x19,0xF6,0x31,0x8C,0x79,0xC0, // 't'
  0xC7,0x8F,0x1E,0x3C,0x7F,0xDD,0x80, // 'u'
  0xC7,0x8D,0x93,0x62,0x87,0x04,0x00, // 'v'
  0xC4,0x79,0xCD,0x29,0x35,0x67,0xBC,0x63,0x0C,0x60, // 'w'
  0xC6,0xD9,0xF1,0xC7,0xCD,0xB1,0x80, // 'x'
  0xC7,0x8D,0x93,0x62,0xC7,0x06,0x18,0xF1,0xC0, // 'y'
  0xFE,0x18,0x61,0x86,0x1F,0xFF,0x80, // 'z'
  0x19,0xCC,0x63,0x3B,0x8E,0x31,0x8C,0x71,0x80, // '{'
  0xFF,0xFF,0xFF,0xC0, // '|'
  0xC7,0x18,0xC6,0x38,0xEE,0x63,0x19,0xCC,0x00 // '}'
};
const GFXglyph ArialRoundedMTBold_14Glyphs[] PROGMEM = {
// bitmapOffset, width, height, xAdvance, xOffset, yOffset
    {     0,   1,   1,   5,    0,    0 }, // ' '
    {     1,   2,  10,   6,    1,  -10 }, // '!'
    {     4,   5,   3,   8,    1,  -10 }, // '"'
    {     6,   8,  10,   9,    0,  -10 }, // '#'
    {    16,   7,  15,   9,    1,  -12 }, // '$'
    {    30,  11,  10,  13,    0,  -10 }, // '%'
    {    44,  10,  10,  12,    1,  -10 }, // '&'
    {    57,   2,   3,   4,    1,  -10 }, // '''
    {    58,   4,  13,   6,    1,  -10 }, // '('
    {    65,   4,  13,   6,    0,  -10 }, // ')'
    {    72,   5,   6,   7,    1,  -11 }, // '*'
    {    76,   8,   7,   9,    0,   -9 }, // '+'
    {    83,   2,   4,   5,    1,   -2 }, // ','
    {    84,   4,   2,   6,    0,   -5 }, // '-'
    {    85,   2,   2,   5,    1,   -2 }, // '.'
    {    86,   4,  10,   5,    0,  -10 }, // '/'
    {    91,   7,  10,   9,    1,  -10 }, // '0'
    {   100,   5,  10,   9,    1,  -10 }, // '1'
    {   107,   7,  10,   9,    1,  -10 }, // '2'
    {   116,   7,  10,   9,    1,  -10 }, // '3'
    {   125,   8,  10,   9,    0,  -10 }, // '4'
    {   135,   7,  10,   9,    1,  -10 }, // '5'
    {   144,   7,  10,   9,    1,  -10 }, // '6'
    {   153,   7,  10,   9,    1,  -10 }, // '7'
    {   162,   7,  10,   9,    1,  -10 }, // '8'
    {   171,   8,  10,   9,    0,  -10 }, // '9'
    {   181,   2,   7,   5,    1,   -7 }, // ':'
    {   183,   2,   9,   5,    1,   -7 }, // ';'
    {   186,   7,   8,   9,    1,   -9 }, // '<'
    {   193,   7,   5,   9,    1,   -7 }, // '='
    {   198,   7,   8,   9,    1,   -9 }, // '>'
    {   205,   7,  10,   9,    1,  -10 }, // '?'
    {   214,  12,  13,  15,    1,  -10 }, // '@'
    {   234,   9,  10,  11,    1,  -10 }, // 'A'
    {   246,   8,  10,  11,    1,  -10 }, // 'B'
    {   256,   9,  10,  11,    1,  -10 }, // 'C'
    {   268,   9,  10,  11,    1,  -10 }, // 'D'
    {   280,   8,  10,  10,    1,  -10 }, // 'E'
    {   290,   7,  10,   9,    1,  -10 }, // 'F'
    {   299,   9,  10,  12,    1,  -10 }, // 'G'
    {   311,   9,  10,  12,    1,  -10 }, // 'H'
    {   323,   2,  10,   5,    1,  -10 }, // 'I'
    {   326,   7,  10,   9,    0,  -10 }, // 'J'
    {   335,   9,  10,  11,    1,  -10 }, // 'K'
    {   347,   7,  10,   9,    1,  -10 }, // 'L'
    {   356,  10,  10,  13,    1,  -10 }, // 'M'
    {   369,   9,  10,  12,    1,  -10 }, // 'N'
    {   381,  10,  10,  12,    1,  -10 }, // 'O'
    {   394,   8,  10,  10,    1,  -10 }, // 'P'
    {   404,  11,  11,  12,    1,  -10 }, // 'Q'
    {   420,   8,  10,  11,    1,  -10 }, // 'R'
    {   430,   8,  10,  10,    1,  -10 }, // 'S'
    {   440,   8,  10,  10,    0,  -10 }, // 'T'
    {   450,   9,  10,  12,    1,  -10 }, // 'U'
    {   462,   9,  10,  11,    0,  -10 }, // 'V'
    {   474,  13,  10,  14,    0,  -10 }, // 'W'
    {   491,   8,  10,   9,    0,  -10 }, // 'X'
    {   501,   8,  10,  10,    0,  -10 }, // 'Y'
    {   511,   9,  10,  10,    0,  -10 }, // 'Z'
    {   523,   4,  13,   6,    1,  -10 }, // '['
    {   530,   4,  10,   5,    0,  -10 }, // '\'
    {   535,   4,  13,   6,    0,  -10 }, // ']'
    {   542,   6,   6,   9,    1,  -10 }, // '^'
    {   547,   7,   1,   8,    0,    1 }, // '_'
    {   548,   2,   2,   6,    1,  -10 }, // '`'
    {   549,   7,   7,   9,    1,   -7 }, // 'a'
    {   556,   7,  10,  10,    1,  -10 }, // 'b'
    {   565,   7,   7,   9,    1,   -7 }, // 'c'
    {   572,   7,  10,  10,    1,  -10 }, // 'd'
    {   581,   7,   7,   9,    1,   -7 }, // 'e'
    {   588,   5,  10,   6,    0,  -10 }, // 'f'
    {   595,   7,  10,  10,    1,   -7 }, // 'g'
    {   604,   7,  10,   9,    1,  -10 }, // 'h'
    {   613,   2,  10,   5,    1,  -10 }, // 'i'
    {   616,   4,  13,   5,   -1,  -10 }, // 'j'
    {   623,   7,  10,   9,    1,  -10 }, // 'k'
    {   632,   2,  10,   5,    1,  -10 }, // 'l'
    {   635,  11,   7,  13,    1,   -7 }, // 'm'
    {   645,   7,   7,   9,    1,   -7 }, // 'n'
    {   652,   7,   7,   9,    1,   -7 }, // 'o'
    {   659,   7,  10,  10,    1,   -7 }, // 'p'
    {   668,   7,  10,  10,    1,   -7 }, // 'q'
    {   677,   5,   7,   7,    1,   -7 }, // 'r'
    {   682,   6,   7,   9,    1,   -7 }, // 's'
    {   688,   5,  10,   6,    0,  -10 }, // 't'
    {   695,   7,   7,   9,    1,   -7 }, // 'u'
    {   702,   7,   7,   9,    0,   -7 }, // 'v'
    {   709,  11,   7,  12,    0,   -7 }, // 'w'
    {   719,   7,   7,   8,    0,   -7 }, // 'x'
    {   726,   7,  10,   9,    0,   -7 }, // 'y'
    {   735,   7,   7,   8,    0,   -7 }, // 'z'
    {   742,   5,  13,   6,    0,  -10 }, // '{'
    {   751,   2,  13,   5,    1,  -10 }, // '|'
    {   755,   5,  13,   6,    1,  -10 } // '}' character 0x7D
};
const GFXfont ArialRoundedMTBold_14 PROGMEM = {    // Last character bug fixed 0x7E to 0x7D
(uint8_t  *)ArialRoundedMTBold_14Bitmaps,(GFXglyph *)ArialRoundedMTBold_14Glyphs,0x20, 0x7D, 17};

// Created by http://oleddisplay.squix.ch/ Consider a donation
// In case of problems make sure that you are using the font file with the correct version!
const uint8_t Lato_Black_21Bitmaps[] PROGMEM = {

	// Bitmap Data:
	0x00, // ' '
	0x77,0x77,0x77,0x77,0x66,0x06,0xFF,0x60, // '!'
	0xEF,0xDF,0xBF,0x7C,0xF8,0x80, // '"'
	0x0C,0xE1,0xCE,0x1C,0xC1,0xDC,0x7F,0xF7,0xFF,0x19,0xC3,0x9C,0x39,0x8F,0xFE,0xFF,0xE3,0xB8,0x33,0x87,0x38,0x73,0x00, // '#'
	0x01,0x00,0x30,0x0F,0x83,0xFE,0x3F,0xE7,0x34,0x73,0x07,0xB0,0x7E,0x03,0xFC,0x0F,0xE0,0x7E,0x06,0xF0,0x67,0x76,0xEF,0xFE,0x7F,0xC1,0xF0,0x0C,0x00,0xC0, // '$'
	0x78,0x0E,0xFC,0x1C,0xEE,0x38,0xC6,0x70,0xCE,0x70,0xFE,0xE0,0x7D,0xC0,0x03,0x80,0x07,0xBE,0x07,0x7E,0x0E,0x67,0x1C,0x67,0x38,0x67,0x38,0x7E,0xF0,0x3C, // '%'
	0x0F,0x00,0xFF,0x03,0xDC,0x0E,0x70,0x38,0x00,0xE0,0x03,0xC3,0x9F,0x8E,0xFF,0x3B,0x9F,0xCE,0x3F,0x38,0x78,0xF3,0xF1,0xFF,0xE3,0xE3,0xC0, // '&'
	0xFF,0xFD,0x80, // '''
	0x23,0x77,0x6E,0xEE,0xEC,0xCC,0xEE,0xEE,0x67,0x73, // '('
	0x4C,0xEE,0xE7,0x77,0x77,0x77,0x77,0x77,0xEE,0xEC, // ')'
	0x11,0x27,0xF9,0xCF,0xF2,0x44,0x00, // '*'
	0x0C,0x03,0x00,0xC0,0x30,0x0C,0x3F,0xFF,0xFC,0x30,0x0C,0x03,0x00,0xC0, // '+'
	0xFF,0xB7,0x80, // ','
	0xFF,0xF0, // '-'
	0xFF,0xF0, // '.'
	0x03,0x83,0x81,0xC0,0xC0,0xE0,0x60,0x70,0x38,0x18,0x1C,0x0C,0x0E,0x07,0x03,0x03,0x81,0xC0, // '/'
	0x0F,0x03,0xFC,0x3D,0xE7,0x0E,0x70,0xE7,0x0F,0xF0,0xFF,0x0F,0xF0,0xF7,0x0F,0x70,0xE7,0x0E,0x3D,0xE3,0xFC,0x0F,0x00, // '0'
	0x0F,0x07,0xC3,0xF1,0xFC,0xFF,0x1B,0xC0,0xF0,0x3C,0x0F,0x03,0xC0,0xF0,0x3C,0x0F,0x1F,0xF7,0xFC, // '1'
	0x1F,0x1F,0xEF,0x3F,0x87,0x61,0xC0,0x70,0x3C,0x1E,0x0F,0x07,0x83,0xE1,0xF0,0xF8,0x3F,0xFF,0xFC, // '2'
	0x1F,0x1F,0xEF,0xBF,0x87,0x21,0xC0,0xF0,0xF8,0x3E,0x07,0xC0,0x74,0x1F,0x87,0xF3,0xDF,0xE3,0xF0, // '3'
	0x01,0xC0,0x3C,0x07,0xC0,0xFC,0x0F,0xC1,0xDC,0x39,0xC7,0x1C,0xF1,0xCF,0xFF,0xFF,0xF0,0x1C,0x01,0xC0,0x1C,0x01,0xC0, // '4'
	0x3F,0xCF,0xF7,0x01,0xC0,0x70,0x1F,0xC7,0xF9,0xDF,0x01,0xC0,0x70,0x1D,0x07,0xF7,0xBF,0xE3,0xE0, // '5'
	0x07,0x81,0xE0,0x78,0x0E,0x03,0xC0,0xFC,0x1F,0xE7,0xFE,0xE1,0xFC,0x3F,0x87,0xF0,0xEF,0xFC,0xFF,0x07,0xC0, // '6'
	0xFF,0xFF,0xFC,0x07,0x01,0xE0,0x38,0x0F,0x01,0xC0,0x78,0x0E,0x01,0xC0,0x78,0x0E,0x03,0xC0,0x70,0x1C,0x00, // '7'
	0x1F,0x0F,0xF3,0xCF,0x70,0xEE,0x1C,0xE7,0x8F,0xC3,0xFC,0xF1,0xDC,0x3B,0x87,0xF0,0xEF,0x3C,0xFF,0x0F,0xC0, // '8'
	0x1F,0x1F,0xEF,0x3F,0x87,0xE1,0xF8,0x7F,0x3D,0xFF,0x3F,0x81,0xE0,0xF0,0x38,0x1E,0x0F,0x07,0x80, // '9'
	0x6F,0xF6,0x00,0x06,0xFF,0x60, // ':'
	0x6F,0xF6,0x00,0x04,0xFF,0x76,0x6C, // ';'
	0x00,0x81,0xC3,0xE7,0xCF,0xC7,0xE0,0xFC,0x1F,0x03,0x80,0x40, // '<'
	0xFF,0xFF,0xF0,0x00,0x00,0xFF,0xFF,0xF0, // '='
	0x80,0x70,0x3E,0x07,0xC0,0xF8,0xFC,0xF9,0xF0,0xE0,0x40,0x00, // '>'
	0x7C,0xFF,0xEF,0x07,0x07,0x0E,0x1C,0x38,0x38,0x00,0x00,0x38,0x38,0x38,0x38, // '?'
	0x07,0xF0,0x1F,0xFC,0x3C,0x1E,0x70,0x06,0x61,0xF7,0xE3,0xF3,0xC7,0x33,0xCE,0x73,0xCE,0x63,0xCC,0x67,0xEE,0xFE,0xEF,0xFE,0x77,0x38,0x78,0x04,0x3F,0x3E,0x0F,0xFC,0x03,0xF0, // '@'
	0x07,0x80,0x0F,0x80,0x1F,0x00,0x77,0x00,0xEE,0x03,0xDC,0x07,0x1C,0x0E,0x38,0x3C,0x78,0x7F,0xF0,0xFF,0xE3,0xC1,0xE7,0x01,0xDE,0x03,0xF8,0x07,0x80, // 'A'
	0xFF,0xCF,0xFE,0xF1,0xFF,0x0F,0xF0,0xFF,0x0F,0xFF,0xCF,0xFE,0xF1,0xFF,0x07,0xF0,0x7F,0x07,0xF1,0xFF,0xFE,0xFF,0x80, // 'B'
	0x0F,0xC1,0xFF,0xDF,0x3C,0xF0,0x4F,0x00,0x78,0x03,0x80,0x1C,0x00,0xE0,0x07,0x80,0x3C,0x00,0xF0,0x47,0xEF,0x9F,0xF8,0x3F,0x00, // 'C'
	0xFF,0xC3,0xFF,0x8F,0x1F,0xBC,0x1E,0xF0,0x3F,0xC0,0xFF,0x01,0xFC,0x07,0xF0,0x1F,0xC0,0xFF,0x03,0xFC,0x1E,0xF1,0xFB,0xFF,0x8F,0xFC,0x00, // 'D'
	0xFF,0xFF,0xFF,0x03,0xC0,0xF0,0x3C,0x0F,0xFB,0xFE,0xF0,0x3C,0x0F,0x03,0xC0,0xF0,0x3F,0xFF,0xFC, // 'E'
	0xFF,0xFF,0xFF,0x03,0xC0,0xF0,0x3C,0x0F,0x03,0xFE,0xFF,0xBC,0x0F,0x03,0xC0,0xF0,0x3C,0x0F,0x00, // 'F'
	0x0F,0xE0,0xFF,0xDF,0x9E,0xF0,0x2F,0x00,0x70,0x03,0x80,0x1C,0x1F,0xE0,0xFF,0x81,0xFC,0x0E,0xF0,0x73,0xE7,0x8F,0xFC,0x1F,0x80, // 'G'
	0xF0,0x3F,0xC0,0xFF,0x03,0xFC,0x0F,0xF0,0x3F,0xC0,0xFF,0xFF,0xFF,0xFF,0xF0,0x3F,0xC0,0xFF,0x03,0xFC,0x0F,0xF0,0x3F,0xC0,0xFF,0x03,0xC0, // 'H'
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0, // 'I'
	0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x1E,0xFC,0xF8, // 'J'
	0xF0,0x7F,0xC1,0xEF,0x0F,0x3C,0x78,0xF3,0xC3,0xCF,0x0F,0xF8,0x3F,0xE0,0xF3,0xC3,0xCF,0x8F,0x1E,0x3C,0x3C,0xF0,0xFB,0xC1,0xEF,0x03,0xC0, // 'K'
	0xF0,0x3C,0x0F,0x03,0xC0,0xF0,0x3C,0x0F,0x03,0xC0,0xF0,0x3C,0x0F,0x03,0xC0,0xFF,0xFF,0xFF,0xFC, // 'L'
	0xF0,0x03,0xFE,0x01,0xFF,0x80,0x7F,0xF0,0x3F,0xFC,0x0F,0xFF,0x87,0x7F,0xE1,0xDF,0xDC,0xE7,0xF7,0x39,0xFC,0xFC,0x7F,0x3F,0x1F,0xC7,0x87,0xF1,0xE1,0xFC,0x00,0x7F,0x00,0x1C, // 'M'
	0xE0,0x1F,0xC0,0x7F,0x81,0xFF,0x07,0xFE,0x1F,0xF8,0x7F,0xF1,0xFD,0xE7,0xF3,0xDF,0xC7,0xFF,0x0F,0xFC,0x3F,0xF0,0x7F,0xC0,0xFF,0x01,0xC0, // 'N'
	0x0F,0xE0,0x1F,0xF8,0x7E,0xFC,0x78,0x3C,0xF0,0x1E,0xE0,0x0E,0xE0,0x0E,0xE0,0x0F,0xE0,0x0E,0xE0,0x0E,0xF0,0x1E,0x78,0x3C,0x7E,0xFC,0x1F,0xF8,0x0F,0xE0, // 'O'
	0xFF,0x8F,0xFE,0xF1,0xEF,0x0F,0xF0,0xFF,0x0F,0xF0,0xFF,0x1E,0xFF,0xEF,0xF8,0xF0,0x0F,0x00,0xF0,0x0F,0x00,0xF0,0x00, // 'P'
	0x07,0xE0,0x1F,0xF0,0x7E,0xFC,0x78,0x3C,0xF0,0x1E,0xE0,0x0E,0xE0,0x0E,0xE0,0x0F,0xE0,0x0E,0xE0,0x0E,0xF0,0x1E,0x78,0x3E,0x7E,0xFC,0x1F,0xF8,0x07,0xF8,0x00,0x3C,0x00,0x1F, // 'Q'
	0xFF,0x87,0xFE,0x3C,0xF9,0xE1,0xEF,0x0F,0x78,0x73,0xC7,0x9F,0xF8,0xFF,0x87,0xBE,0x3C,0xF1,0xE3,0xCF,0x1E,0x78,0x7B,0xC1,0xE0, // 'R'
	0x0F,0x87,0xFD,0xE7,0x38,0x07,0x00,0xF8,0x1F,0xE1,0xFE,0x0F,0xE0,0x3C,0x03,0x90,0x77,0xBD,0xFF,0x87,0xC0, // 'S'
	0xFF,0xFF,0xFF,0x07,0x00,0x70,0x07,0x00,0x70,0x07,0x00,0x70,0x07,0x00,0x70,0x07,0x00,0x70,0x07,0x00,0x70,0x07,0x00, // 'T'
	0xF0,0x3F,0x81,0xFC,0x0F,0xE0,0x7F,0x03,0xF8,0x1F,0xC0,0xFE,0x07,0xF0,0x3F,0x81,0xFC,0x0E,0xF0,0xF7,0xCF,0x1F,0xF0,0x3F,0x00, // 'U'
	0xF0,0x1F,0xE0,0x3D,0xC0,0x73,0xC1,0xE3,0x83,0x87,0x8F,0x0F,0x1E,0x0E,0x38,0x1E,0xF0,0x1D,0xC0,0x3B,0x80,0x7F,0x00,0x7C,0x00,0xF8,0x00,0xE0,0x00, // 'V'
	0xF0,0x38,0x3D,0xC1,0xE0,0xF7,0x87,0xC3,0xDE,0x1F,0x0E,0x78,0xFC,0x78,0xE3,0xF1,0xE3,0xCE,0xE7,0x0F,0x73,0x9C,0x1D,0xCE,0xF0,0x7F,0x1F,0x81,0xF8,0x7E,0x03,0xE1,0xF8,0x0F,0x83,0xC0,0x3C,0x0F,0x00,0x70,0x3C,0x00, // 'W'
	0xF0,0x3E,0xF0,0x70,0xF1,0xE1,0xE3,0x81,0xEE,0x01,0xFC,0x03,0xF0,0x03,0xE0,0x0F,0xE0,0x3F,0xC0,0x73,0xC1,0xE7,0x87,0x87,0x8F,0x07,0xBC,0x0F,0x80, // 'X'
	0xF0,0x3D,0xC0,0xE7,0x87,0x8E,0x1C,0x3C,0xF0,0x73,0x80,0xFC,0x03,0xF0,0x07,0x80,0x1E,0x00,0x78,0x01,0xE0,0x07,0x80,0x1E,0x00,0x78,0x00, // 'Y'
	0x7F,0xF7,0xFF,0x00,0xF0,0x1E,0x03,0xC0,0x3C,0x07,0x80,0xF0,0x0F,0x01,0xE0,0x3C,0x07,0xC0,0x78,0x0F,0xFF,0xFF,0xF0, // 'Z'
	0xFF,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFF, // '['
	0xC0,0x70,0x38,0x0E,0x07,0x01,0x80,0xE0,0x70,0x18,0x0E,0x03,0x01,0xC0,0xE0,0x30,0x1C,0x07, // '\'
	0xFF,0xCE,0x73,0x9C,0xE7,0x39,0xCE,0x73,0x9C,0xE7,0x39,0xFF,0xF0, // ']'
	0x0C,0x07,0x81,0xE0,0xFC,0x33,0x9C,0xE6,0x1F,0x87, // '^'
	0xFF,0xFF, // '_'
	0xF0,0xE1,0xC0, // '`'
	0x1F,0x1F,0xE7,0x3C,0x07,0x01,0xCF,0xF7,0xFF,0xC7,0xF3,0xDF,0xF3,0xDC, // 'a'
	0xE0,0x1C,0x03,0x80,0x70,0x0E,0x01,0xDE,0x3F,0xE7,0xDE,0xE1,0xDC,0x3B,0x87,0xF0,0xEE,0x1D,0xE7,0xBF,0xE7,0x78, // 'b'
	0x3E,0x3F,0xFE,0xDC,0x0E,0x07,0x03,0x81,0xC0,0xFB,0xBF,0xCF,0x80, // 'c'
	0x01,0xC0,0x70,0x1C,0x07,0x01,0xCF,0x77,0xFF,0xCF,0xE1,0xF8,0x7E,0x1F,0x87,0xE1,0xFC,0xF7,0xFC,0xF7, // 'd'
	0x3E,0x1F,0xEF,0x3B,0x87,0xFF,0xFF,0xFE,0x03,0x80,0xFB,0x9F,0xF1,0xF0, // 'e'
	0x0F,0x1F,0x3C,0x38,0xFF,0xFF,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38, // 'f'
	0x1F,0xEF,0xFD,0xCF,0x38,0xE7,0xF8,0x7F,0x0C,0x03,0x80,0x78,0x0F,0xF8,0xFF,0xF8,0xF7,0xFC,0x7E,0x00, // 'g'
	0xE0,0x38,0x0E,0x03,0x80,0xE0,0x3B,0xCF,0xFB,0xEF,0xE1,0xF8,0x7E,0x1F,0x87,0xE1,0xF8,0x7E,0x1F,0x87, // 'h'
	0x6F,0xF6,0x0E,0xEE,0xEE,0xEE,0xEE,0xEE, // 'i'
	0x18,0xF3,0xC6,0x00,0xE3,0x8E,0x38,0xE3,0x8E,0x38,0xE3,0x8E,0x3B,0xEF,0x00, // 'j'
	0xE0,0x1C,0x03,0x80,0x70,0x0E,0x01,0xC3,0xF8,0xF7,0x3C,0xE7,0x1F,0xC3,0xFC,0x77,0x8E,0x79,0xC7,0x38,0xF7,0x0F, // 'k'
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // 'l'
	0xEF,0x3C,0xFF,0x7E,0xF3,0xEF,0xE3,0xC7,0xE3,0xC7,0xE3,0xC7,0xE3,0xC7,0xE3,0xC7,0xE3,0xC7,0xE3,0xC7,0xE3,0xC7, // 'm'
	0xEF,0x3F,0xEF,0xBF,0x87,0xE1,0xF8,0x7E,0x1F,0x87,0xE1,0xF8,0x7E,0x1C, // 'n'
	0x1F,0x0F,0xF3,0xCF,0x70,0xEE,0x1F,0xC3,0xF8,0x7F,0x0E,0xF3,0xCF,0xF0,0x7C,0x00, // 'o'
	0xEF,0x1F,0xF3,0xCF,0x70,0xEE,0x1D,0xC3,0xF8,0x77,0x0E,0xF3,0xDF,0xF3,0xBC,0x70,0x0E,0x01,0xC0,0x00, // 'p'
	0x3D,0xDF,0xFF,0x3F,0x87,0xE1,0xF8,0x7E,0x1F,0x87,0xF7,0xDF,0xF3,0xDC,0x07,0x01,0xC0,0x70, // 'q'
	0xEF,0xFF,0xE7,0x8E,0x1C,0x38,0x70,0xE1,0xC3,0x80, // 'r'
	0x1E,0x3F,0xDC,0xCE,0x07,0xC1,0xF8,0x3E,0x07,0x77,0xFF,0x8F,0x80, // 's'
	0x18,0x38,0x38,0xFF,0xFF,0x38,0x38,0x38,0x38,0x38,0x38,0x3E,0x3F,0x1E, // 't'
	0xE1,0xF8,0x7E,0x1F,0x87,0xE1,0xF8,0x7E,0x1F,0x87,0xF3,0xDF,0xF3,0xDC, // 'u'
	0xF0,0xEE,0x1D,0xC7,0x38,0xE3,0x9C,0x77,0x0E,0xE0,0xF8,0x1F,0x01,0xE0,0x38,0x00, // 'v'
	0xF1,0xC3,0xB8,0xF1,0xDC,0x79,0xCE,0x7C,0xE7,0x36,0x71,0xDB,0xB0,0xFD,0xF8,0x7E,0x7C,0x1E,0x3E,0x0F,0x1E,0x07,0x87,0x00, // 'w'
	0xF0,0xE7,0x9C,0x39,0xC3,0xF8,0x1F,0x00,0xF0,0x1F,0x83,0xB8,0x3B,0xC7,0x1E,0xF0,0xF0, // 'x'
	0xF0,0xEE,0x1D,0xC7,0x3C,0xE3,0x98,0x7F,0x07,0xE0,0xF8,0x0F,0x01,0xC0,0x38,0x0E,0x01,0xC0,0x78,0x00, // 'y'
	0x7F,0xBF,0xC1,0xE0,0xE0,0xE0,0xF0,0xF0,0x70,0x70,0x7F,0xFF,0xE0, // 'z'
	0x1C,0xF7,0x9C,0x71,0xC3,0x0C,0x73,0xCE,0x1C,0x30,0xC7,0x1C,0x71,0xE3,0xC7, // '{'
	0xFF,0xFF,0xFF,0xFF,0xFF, // '|'
	0xE7,0x9C,0xE7,0x39,0xCE,0x71,0xCE,0xE7,0x39,0xCE,0x73,0xBD,0xC0 // '}'
};
const GFXglyph Lato_Black_21Glyphs[] PROGMEM = {
// bitmapOffset, width, height, xAdvance, xOffset, yOffset
	  {     0,   1,   1,   6,    0,    0 }, // ' '
	  {     1,   4,  15,   7,    1,  -15 }, // '!'
	  {     9,   7,   6,  10,    1,  -15 }, // '"'
	  {    15,  12,  15,  13,    0,  -15 }, // '#'
	  {    38,  12,  20,  13,    0,  -17 }, // '$'
	  {    68,  16,  15,  19,    1,  -15 }, // '%'
	  {    98,  14,  15,  16,    1,  -15 }, // '&'
	  {   125,   3,   6,   6,    1,  -15 }, // '''
	  {   128,   4,  20,   7,    1,  -17 }, // '('
	  {   138,   4,  20,   7,    1,  -17 }, // ')'
	  {   148,   7,   7,  10,    1,  -16 }, // '*'
	  {   155,  10,  11,  13,    1,  -13 }, // '+'
	  {   169,   3,   6,   6,    1,   -3 }, // ','
	  {   172,   6,   2,   9,    1,   -7 }, // '-'
	  {   174,   3,   4,   6,    1,   -4 }, // '.'
	  {   176,   9,  16,  11,    1,  -16 }, // '/'
	  {   194,  12,  15,  13,    0,  -15 }, // '0'
	  {   217,  10,  15,  13,    1,  -15 }, // '1'
	  {   236,  10,  15,  13,    1,  -15 }, // '2'
	  {   255,  10,  15,  13,    1,  -15 }, // '3'
	  {   274,  12,  15,  13,    0,  -15 }, // '4'
	  {   297,  10,  15,  13,    1,  -15 }, // '5'
	  {   316,  11,  15,  13,    1,  -15 }, // '6'
	  {   337,  11,  15,  13,    1,  -15 }, // '7'
	  {   358,  11,  15,  13,    1,  -15 }, // '8'
	  {   379,  10,  15,  13,    1,  -15 }, // '9'
	  {   398,   4,  11,   7,    1,  -11 }, // ':'
	  {   404,   4,  14,   7,    1,  -11 }, // ';'
	  {   411,   9,  10,  13,    1,  -12 }, // '<'
	  {   423,  10,   6,  13,    1,  -10 }, // '='
	  {   431,   9,  10,  13,    2,  -12 }, // '>'
	  {   443,   8,  15,  11,    1,  -15 }, // '?'
	  {   458,  16,  17,  19,    1,  -14 }, // '@'
	  {   492,  15,  15,  16,    0,  -15 }, // 'A'
	  {   521,  12,  15,  15,    1,  -15 }, // 'B'
	  {   544,  13,  15,  15,    1,  -15 }, // 'C'
	  {   569,  14,  15,  17,    1,  -15 }, // 'D'
	  {   596,  10,  15,  13,    1,  -15 }, // 'E'
	  {   615,  10,  15,  13,    1,  -15 }, // 'F'
	  {   634,  13,  15,  16,    1,  -15 }, // 'G'
	  {   659,  14,  15,  17,    1,  -15 }, // 'H'
	  {   686,   4,  15,   8,    1,  -15 }, // 'I'
	  {   694,   8,  15,  10,    0,  -15 }, // 'J'
	  {   709,  14,  15,  16,    1,  -15 }, // 'K'
	  {   736,  10,  15,  12,    1,  -15 }, // 'L'
	  {   755,  18,  15,  21,    1,  -15 }, // 'M'
	  {   789,  14,  15,  17,    1,  -15 }, // 'N'
	  {   816,  16,  15,  18,    1,  -15 }, // 'O'
	  {   846,  12,  15,  14,    1,  -15 }, // 'P'
	  {   869,  16,  17,  18,    1,  -15 }, // 'Q'
	  {   903,  13,  15,  15,    1,  -15 }, // 'R'
	  {   928,  11,  15,  13,    0,  -15 }, // 'S'
	  {   949,  12,  15,  14,    0,  -15 }, // 'T'
	  {   972,  13,  15,  17,    1,  -15 }, // 'U'
	  {   997,  15,  15,  16,    0,  -15 }, // 'V'
	  {  1026,  22,  15,  23,    0,  -15 }, // 'W'
	  {  1068,  15,  15,  15,    0,  -15 }, // 'X'
	  {  1097,  14,  15,  15,    0,  -15 }, // 'Y'
	  {  1124,  12,  15,  14,    0,  -15 }, // 'Z'
	  {  1147,   4,  20,   8,    2,  -17 }, // '['
	  {  1157,   9,  16,  11,    1,  -16 }, // '\'
	  {  1175,   5,  20,   8,    0,  -17 }, // ']'
	  {  1188,  10,   8,  13,    1,  -15 }, // '^'
	  {  1198,   8,   2,  11,    1,    1 }, // '_'
	  {  1200,   6,   3,   9,    0,  -15 }, // '`'
	  {  1203,  10,  11,  12,    0,  -11 }, // 'a'
	  {  1217,  11,  16,  13,    1,  -16 }, // 'b'
	  {  1239,   9,  11,  11,    1,  -11 }, // 'c'
	  {  1252,  10,  16,  13,    1,  -16 }, // 'd'
	  {  1272,  10,  11,  12,    1,  -11 }, // 'e'
	  {  1286,   8,  15,   9,    0,  -15 }, // 'f'
	  {  1301,  11,  14,  12,    0,  -11 }, // 'g'
	  {  1321,  10,  16,  13,    1,  -16 }, // 'h'
	  {  1341,   4,  16,   7,    1,  -16 }, // 'i'
	  {  1349,   6,  19,   7,   -1,  -16 }, // 'j'
	  {  1364,  11,  16,  13,    1,  -16 }, // 'k'
	  {  1386,   3,  16,   6,    1,  -16 }, // 'l'
	  {  1392,  16,  11,  19,    1,  -11 }, // 'm'
	  {  1414,  10,  11,  13,    1,  -11 }, // 'n'
	  {  1428,  11,  11,  13,    1,  -11 }, // 'o'
	  {  1444,  11,  14,  13,    1,  -11 }, // 'p'
	  {  1464,  10,  14,  13,    1,  -11 }, // 'q'
	  {  1482,   7,  11,   9,    1,  -11 }, // 'r'
	  {  1492,   9,  11,  10,    0,  -11 }, // 's'
	  {  1505,   8,  14,   9,    0,  -14 }, // 't'
	  {  1519,  10,  11,  13,    1,  -11 }, // 'u'
	  {  1533,  11,  11,  12,    0,  -11 }, // 'v'
	  {  1549,  17,  11,  18,    0,  -11 }, // 'w'
	  {  1573,  12,  11,  12,    0,  -11 }, // 'x'
	  {  1590,  11,  14,  12,    0,  -11 }, // 'y'
	  {  1610,   9,  11,  11,    0,  -11 }, // 'z'
	  {  1623,   6,  20,   8,    0,  -17 }, // '{'
	  {  1638,   2,  20,   7,    2,  -17 }, // '|'
	  {  1643,   5,  20,   8,    1,  -17 } // '}'
};
const GFXfont Lato_Black_21 PROGMEM = {
(uint8_t  *)Lato_Black_21Bitmaps,(GFXglyph *)Lato_Black_21Glyphs,0x20, 0x7E, 26};

