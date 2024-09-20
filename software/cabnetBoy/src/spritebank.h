//      >>>>>  T-I-N-Y  A-R-K-A-N-O-I-D for ATTINY85  GPL v3 <<<<
//                Programmer: Daniel Champagne 2018-2021
//                 Contact EMAIL: phoenixbozo@gmail.com
//           https://sites.google.com/view/arduino-collection

//  Tiny-tris is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

//  2023 modified to work with CH32V003 by Stefan Wagner: github.com/wagiminator.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


typedef struct GAMEDATA
{
   
   
   
   }GAMEDATA;


typedef struct GROUPE{
uint8_t ANIMREFLECT;
uint8_t launch;
uint8_t Px;
uint8_t Py;
uint8_t BlocsGrid[6][5];
float Ballxpos;   //볼 위치 
float SIMBallxpos;
float Ballypos;
float SIMBallypos;  //볼 위치 
float BallSpeedx;
float SIMBallSpeedx;
float BallSpeedy;
float SIMBallSpeedy;
int8_t TrackAngleOut;
float Ballxposflip;
float Ballyposflip;
uint8_t BALLyDecal;
uint8_t Ypos;
uint8_t TrackBary;
uint8_t TrackBaryDecal;
uint8_t LEVEL;
uint8_t LEVELSPEED;
uint8_t live;
uint8_t Frame;
}GROUPE;

const uint8_t  LEVEL [] = {
0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,5,0,1,2,3,4,
5,1,2,3,5,5,1,2,3,4,0,1,2,3,5,0,1,2,3,4,5,5,5,5,5,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,
0,1,2,3,4,5,5,5,5,5,5,1,2,3,4,5,1,2,3,5,5,1,2,3,4,5,1,2,3,4,5,1,2,3,5,5,1,2,3,4,
5,5,5,5,5,0,0,0,0,0,5,5,0,5,5,5,5,0,5,5,0,0,0,0,0,5,5,5,5,5};

const uint8_t  LIVE [] = {0x3E, 0x41, 0x3E};

const uint8_t  BALL [] = {0x02, 0x05, 0x02};

const uint8_t  TRACKBAR [] = {0xFE, 0x0D, 0x0B, 0xFE, 0x7F, 0xB0, 0xD0, 0x7F};

const uint8_t  BLOCKREFLECT [] = {
0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t  BLOCK [] = {
0x01, 0x3F, 0x3F, 0x3F, 0x7F, 0x00, 0x01, 0x2D, 0x1B, 0x37, 0x7F, 0x00, 0x01, 0x15, 0x2B, 0x15,
0x7F, 0x00, 0x01, 0x13, 0x25, 0x09, 0x7F, 0x00, 0x01, 0x01, 0x01, 0x01, 0x7F, 0x00, 0x7F, 0x41,
0x41, 0x41, 0x7F, 0x00
};

const uint8_t  DIGITAL [] = {
0xF0, 0x90, 0x90, 0x90, 0x90, 0x90, 0xF0, 0xE0, 0x40, 0x40, 0x40, 0x40, 0x60, 0x40, 0xF0, 0x10,
0x10, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x80, 0x80, 0xE0, 0x80, 0x80, 0xF0, 0x80, 0x80, 0x80, 0xF0,
0x90, 0x90, 0x90, 0xF0, 0x80, 0x80, 0xF0, 0x10, 0x10, 0xF0, 0xF0, 0x90, 0x90, 0xF0, 0x10, 0x10,
0xF0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF0, 0xF0, 0x90, 0x90, 0xF0, 0x90, 0x90, 0xF0, 0xF0,
0x80, 0x80, 0xF0, 0x90, 0x90, 0xF0
};

const uint8_t  Music1 [] = {
125,255,125,255,0,200,125,255,125,255,145,255,145,255,145,255,145,255,145,255,
145,255,145,255,145,255,145,255,145,255,145,255,145,255,140,255,140,255,140,255,
125,255,125,255,125,255,105,255,105,255,105,255,135,255,135,255,135,255,125,255,
125,255,125,255,125,255,125,255,125,255,125,255,125,255,0,125,125,255,125,255,
125,255,125,255,125,255,125,255,125,255,125,255};


const uint8_t  back_UP [] = {
0x05,0x05,0x05,0x05,0x05,0x47,0x0F,0x08,0x0F,0x08,0x2F,0x08,0x0F,0x07,0x0D,0x05,0x05,0x05,0x05,0x05,
0x45,0x05,0x05,0x0D,0x05,0x25,0x05,0x07,0x0F,0x08,0x0F,0x08,0x0F,0x08,0x0F,0x47,0x05,0x05,0x0D,0x05,
0x25,0x05,0x05,0x05,0x0D,0x05,0x05,0x07,0x0F,0x08,0x4F,0x08,0x0F,0x08,0x0F,0x27,0x05,0x05,0x05,0x0D,
0x05,0x05,0x05,0x05,0x05,0x45,0x05,0x05,0x07,0x0F,0x28,0x0F,0x08,0x0F,0x08,0x0F,0x07,0x05,0x05,0x05,
0x45,0x05,0x05,0x0D,0x05,0x25,0x05,0x05,0x07,0x0F,0x08,0x0F,0x08,0x0F,0x08,0x4F,0x07,0x05,0x0D,0x05,
0x25,0x05,0x05,0x05,0x05,0x05,0x05
};


const uint8_t  back_DOWN [] = {
0xA0,0xA0,0xA1,0xA0,0xA4,0xE0,0xF0,0x10,0xF8,0x10,0xF0,0x10,0xF0,0xE0,0xA8,0xA0,0xA0,0xA1,0xA0,0xA4,
0xA0,0xA0,0xA0,0xA8,0xA0,0xA0,0xA0,0xE0,0xF0,0x18,0xF0,0x10,0xF1,0x10,0xF4,0xE0,0xA0,0xA0,0xA8,0xA0,
0xA0,0xA0,0xA0,0xA0,0xA8,0xA0,0xA0,0xE1,0xF0,0x14,0xF0,0x10,0xF0,0x18,0xF0,0xE0,0xA0,0xA0,0xA0,0xA8,
0xA0,0xA0,0xA1,0xA0,0xA4,0xA0,0xA0,0xA0,0xE8,0xF0,0x10,0xF0,0x10,0xF0,0x18,0xF0,0xE0,0xA1,0xA0,0xA4,
0xA0,0xA0,0xA0,0xA8,0xA0,0xA0,0xA0,0xA0,0xE0,0xF8,0x10,0xF0,0x11,0xF0,0x14,0xF0,0xE0,0xA0,0xA8,0xA0,
0xA0,0xA0,0xA0,0xA0,0xA8,0xA0,0xA0
};


const uint8_t  back_RIGHT  [] = {
0x05,0xF9,0x03,0xFE,0x00,0x00,0x00,0x00,0xFE,0xFF,0x07,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x07,0xFF,
0xFE,0x00,0xFE,0xAB,0xAB,0xAB,0xFE,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xFF,0xFF,0x00,0x00,0xFF,0x01,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFF,0xFF,0x00,0xE0,0xBF,0xB0,0xBF,0xE0,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x0F,0xFA,0x1A,0xFA,0x0F,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0xFF,0x80,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,
0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0xFF,0xFF,0x00,0x7F,0xD5,0xD5,0xD5,0x7F,0x00,0x00,0x00,
0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0xA0,0x9F,0xC0,0x7F,0x00,0x00,
0x00,0x00,0x7F,0xFF,0xE0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xE0,0xFF,0x7F,0x00
};

const uint8_t  texture [] = {
0x08,0x00,0x00,0x01,0x00,0x04,0x40,0x00,0x00,0x08,0x00,0x20,0x00,0x00,0x00
};

const uint8_t  MAIN [] = {
0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x15, 0x0A, 0x05, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x32, 0x7F, 0x77, 0x63, 0x63, 0x77, 0x7F, 0x71, 0x41, 0x42, 0x5A, 0x54, 0x44, 0x48,
0x48, 0x50, 0x50, 0x60, 0x60, 0x40, 0x00, 0x38, 0x70, 0xF4, 0xF0, 0xD0, 0xD0, 0xD0, 0xD6, 0xDA,
0x86, 0x7C, 0xF8, 0x80, 0x00, 0xFE, 0x03, 0xC1, 0x21, 0x21, 0xA1, 0x21, 0x21, 0xC1, 0x03, 0xFE,
0xAA, 0x05, 0x02, 0xFF, 0xFF, 0x03, 0x03, 0xC3, 0x03, 0x03, 0x03, 0x03, 0x83, 0x43, 0x43, 0x43,
0x43, 0x83, 0x03, 0xFF, 0xFE, 0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0B, 0x1B, 0x39, 0x7D, 0xF3,
0xE1, 0xC2, 0xCC, 0x98, 0x68, 0x08, 0xC8, 0x30, 0x60, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x00,
0x00, 0x00, 0xA1, 0xB1, 0xB1, 0xBB, 0xBB, 0xBF, 0x9F, 0x8F, 0x91, 0x91, 0xA5, 0xA5, 0xCD, 0xCD,
0xA5, 0xA5, 0xA1, 0x91, 0x99, 0x8F, 0x00, 0x10, 0x70, 0xF0, 0xF0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
0xD1, 0xD6, 0xD6, 0xE3, 0x00, 0xFF, 0x00, 0x07, 0x88, 0x8B, 0x88, 0x8B, 0x88, 0x07, 0x00, 0xFF,
0xAA, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x03, 0x04, 0x04, 0x04, 0x04, 0x03, 0x00, 0x00, 0x00,
0x00, 0xE7, 0x00, 0xFF, 0xFF, 0x55, 0xAA, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF0, 0x9C, 0x84, 0xC4, 0x34, 0x0C, 0x0C, 0x93, 0x60,
0x01, 0x01, 0xC1, 0x30, 0x0C, 0x03, 0x02, 0x04, 0x18, 0x20, 0xC3, 0x0C, 0x10, 0xE0, 0x80, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x10, 0xB9, 0xB9, 0x9F, 0x9F, 0x8F, 0x8F, 0x88, 0x08, 0x12, 0x26, 0x66, 0x2A,
0x3A, 0x12, 0x12, 0x02, 0x02, 0x03, 0x00, 0x04, 0xCC, 0xEC, 0xEC, 0xFC, 0x9C, 0xE4, 0xF4, 0xD4,
0xCC, 0xCC, 0x8C, 0x84, 0x00, 0xFF, 0x00, 0x00, 0xA9, 0xAA, 0xBA, 0xAA, 0x91, 0x00, 0x00, 0xFF,
0xAA, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0xE1, 0x21, 0x41,
0x41, 0x8F, 0x00, 0xFF, 0xFF, 0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
0x6C, 0xD8, 0xD8, 0xD8, 0xC8, 0xE8, 0x98, 0x10, 0x3B, 0x47, 0x87, 0x07, 0x06, 0x06, 0x81, 0x60,
0x18, 0x07, 0x00, 0x00, 0x00, 0xE0, 0xD8, 0xBC, 0x72, 0xE2, 0xC4, 0x2D, 0x37, 0xC1, 0x00, 0x03,
0x0C, 0x10, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
0x00, 0x00, 0xDB, 0xDB, 0xDB, 0xD9, 0xD9, 0xD9, 0xDB, 0xDF, 0xD8, 0x51, 0x51, 0x52, 0x52, 0x54,
0x54, 0x58, 0xD8, 0xD0, 0xD0, 0x41, 0x00, 0x71, 0x71, 0xF1, 0xE1, 0xA1, 0xA1, 0xA1, 0x21, 0x11,
0xD1, 0xE9, 0x39, 0x1D, 0x00, 0xFF, 0x00, 0x00, 0xD4, 0x54, 0xD6, 0x55, 0xD4, 0x00, 0x00, 0xFF,
0xAA, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x24, 0x24, 0x24, 0x24, 0xA4, 0x64, 0xE7, 0x24, 0x22,
0x22, 0xE1, 0x00, 0xFF, 0xFF, 0x55, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
0x40, 0x00, 0x01, 0x21, 0x7B, 0xA7, 0x6F, 0xAF, 0x6E, 0xAC, 0x68, 0xE7, 0x98, 0x0E, 0x11, 0x20,
0x40, 0x80, 0x00, 0x00, 0x00, 0x3D, 0xFD, 0xFB, 0xFB, 0x7A, 0xBF, 0x40, 0xC0, 0x3F, 0x80, 0x60,
0x1C, 0x03, 0x00, 0xFF, 0x1B, 0x26, 0x0C, 0x10, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x40, 0xC0, 0xE0, 0xF0, 0xF0, 0xFC, 0xFC, 0xFE, 0xA3, 0xA0, 0xB8, 0xBC, 0xB4, 0xB2,
0xB3, 0xB1, 0xB0, 0xB0, 0xA0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x03,
0x06, 0x04, 0x0D, 0x3E, 0x00, 0xFF, 0x00, 0x00, 0x5D, 0x04, 0x04, 0x04, 0x05, 0x00, 0x00, 0xFF,
0xAA, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x82, 0x82, 0x82, 0x81, 0x80, 0x80, 0x81, 0x82, 0x82,
0x82, 0xF1, 0x00, 0xFF, 0xFF, 0x55, 0xEA, 0x00, 0x04, 0x00, 0x04, 0x2E, 0x04, 0x00, 0x04, 0x00,
0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x06, 0x0D, 0x1A, 0x35, 0x2B, 0x56, 0xAC,
0x58, 0xB0, 0x61, 0xB2, 0x4C, 0x83, 0x00, 0x03, 0x02, 0x05, 0x02, 0x81, 0x70, 0x0C, 0x03, 0x00,
0xC0, 0x70, 0x8E, 0x81, 0x66, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xF0, 0xF8, 0xFC, 0xFC, 0xFE, 0xFE, 0x1E, 0xAF, 0xF1, 0xA9, 0xA9, 0xF9, 0xA9, 0xA9,
0xF1, 0xA2, 0x02, 0x04, 0x08, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x86, 0x81, 0x01, 0x01, 0x86, 0x00, 0x00, 0xFF,
0xAA, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
0xC0, 0xC7, 0xC0, 0xFF, 0xFF, 0x55, 0xAA, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x02, 0x05, 0x06, 0x0D, 0x1A, 0x35, 0x5A, 0x74, 0xD8, 0xB6, 0x59, 0xA8, 0xC4, 0x82, 0x03,
0x07, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x21, 0x23, 0x27, 0x67, 0xE7, 0xEF, 0xFF, 0xFE, 0xB1, 0xB2, 0xB2, 0xB3, 0xB2, 0xB2,
0xB1, 0xA8, 0xA8, 0xA4, 0xA3, 0xE0, 0x00, 0x00, 0x04, 0x10, 0x00, 0x10, 0xBA, 0x10, 0x00, 0x10,
0x00, 0x00, 0x08, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x93, 0xA8, 0xA9, 0xAA, 0x91, 0x00, 0x00, 0xFF,
0xAA, 0x54, 0xA8, 0x54, 0xA8, 0x54, 0xA8, 0x54, 0xA8, 0x54, 0xA8, 0x54, 0xA8, 0x54, 0xA8, 0x54,
0xA8, 0x54, 0xA8, 0x54, 0xAA, 0x55, 0xAA, 0x50, 0xA0, 0x40, 0x80, 0x08, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x02, 0x04, 0x01, 0x03,
0x02, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F, 0x11, 0x11, 0x21, 0x25, 0x4D, 0x45,
0xD5, 0xD5, 0x4D, 0x4D, 0x21, 0x23, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
0x00, 0x02, 0x00, 0x00, 0x00, 0x7F, 0xC0, 0x80, 0x8C, 0x92, 0x8C, 0x92, 0x8C, 0x80, 0xC0, 0x7F
};

#ifdef __cplusplus
};
#endif
